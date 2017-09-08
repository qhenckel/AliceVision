#include "delaunaycut/mv_delaunay_GC.h"
#include "delaunaycut/mv_delaunay_meshSmooth.h"
#include "largeScale/reconstructionPlan.h"

#include "planeSweeping/ps_refine_rc.h"
#include "CUDAInterfaces/refine.h"
#include "structures/mv_filesio.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>


namespace bfs = boost::filesystem;
namespace po = boost::program_options;

bool checkHardwareCompatibility()
{
    if(listCUDADevices(false) < 1)
    {
        std::cerr << "ERROR: no CUDA capable devices were detected." << std::endl;
        return false;
    }
    return true;
}

bfs::path absolutePathNoExt(const bfs::path& p)
{
    return p.parent_path() / p.stem();
}

#define ALICEVISION_COUT(x) std::cout << x << std::endl
#define ALICEVISION_CERR(x) std::cerr << x << std::endl

int main(int argc, char* argv[])
{
    long startTime = clock();

    std::string iniFilepath;
    std::string inputMeshFilepath;
    std::string outputFolder;
    bool flipNormals = false;
    TexturingParams texParams;

    po::options_description inputParams;

    inputParams.add_options()
        ("ini", po::value<std::string>(&iniFilepath)->required(),
            "Configuration file (mvs.ini).")
        ("output", po::value<std::string>(&outputFolder)->required(),
            "Folder for output mesh: OBJ, material and texture files.")
        ("textureSide", po::value<unsigned int>(&texParams.textureSide),
            "Output texture size")
        ("padding", po::value<unsigned int>(&texParams.padding),
            "Texture edge padding size in pixel")
        ("downscale", po::value<unsigned int>(&texParams.downscale),
            "Texture downscale factor")
        ("inputMesh", po::value<std::string>(&inputMeshFilepath),
            "Optional input mesh to texture. By default, it will texture the result of the reconstruction.")
        ("flipNormals", po::bool_switch(&flipNormals),
            "Option to flip face normals. It can be needed as it depends on the vertices order in triangles and the convention change from one software to another.");
    po::variables_map vm;

    try
    {
      po::store(po::parse_command_line(argc, argv, inputParams), vm);

      if(vm.count("help") || (argc == 1))
      {
        ALICEVISION_COUT(inputParams);
        return EXIT_SUCCESS;
      }

      po::notify(vm);
    }
    catch(boost::program_options::required_option& e)
    {
      ALICEVISION_CERR("ERROR: " << e.what() << std::endl);
      ALICEVISION_COUT("Usage:\n\n" << inputParams);
      return EXIT_FAILURE;
    }
    catch(boost::program_options::error& e)
    {
      ALICEVISION_CERR("ERROR: " << e.what() << std::endl);
      ALICEVISION_COUT("Usage:\n\n" << inputParams);
      return EXIT_FAILURE;
    }

    // check hardware compatibility
    if(!checkHardwareCompatibility())
        return EXIT_FAILURE;

    ALICEVISION_COUT("ini file: " << iniFilepath);
    ALICEVISION_COUT("inputMesh: " << inputMeshFilepath);

    // .ini parsing
    multiviewInputParams mip(iniFilepath);
    const double simThr = mip._ini.get<double>("global.simThr", 0.0);
    multiviewParams mp(mip.getNbCameras(), &mip, (float) simThr);
    mv_prematch_cams pc(&mp);

    std::string refMeshFilepath = mip.mvDir + "mesh.bin";

    meshRetex mesh;
    mesh.texParams = texParams;
    mesh.me = new mv_mesh();

    if(!mesh.me->loadFromBin(refMeshFilepath))
    {
        std::cerr << "Unable to load: " << refMeshFilepath << std::endl;
        return EXIT_FAILURE;
    }

    staticVector<staticVector<int>*>* ptsCams = loadArrayOfArraysFromFile<int>(mip.mvDir + "meshPtsCamsFromDGC.bin");
    if(ptsCams->size() != mesh.me->pts->size())
        throw std::runtime_error("Error: Reference mesh and associated visibilities don't have the same size.");
    // filterPtsCamsByMinimalPixelSize(refMesh, refPtsCams, &mp);
    bfs::create_directory(outputFolder);

    //std::cout << "Points with no visibilities " << std::count(ptsCams->begin(), ptsCams->end(), nullptr) << std::endl;

    // texturing from input mesh
    if(!inputMeshFilepath.empty())
    {
        // remap visibilities from reconstruction onto input mesh
        mv_delaunay_GC delaunayGC(&mp, &pc);
        delaunayGC.initTetrahedralizationFromMeshVertices(mesh.me, false);
        delete mesh.me;
        mesh.me = new mv_mesh();
        mesh.loadFromOBJ(inputMeshFilepath, flipNormals);
        staticVector<staticVector<int>*>* otherPtsCams = delaunayGC.createPtsCamsForAnotherMesh(ptsCams, *mesh.me);
        std::swap(ptsCams, otherPtsCams);    
        deleteArrayOfArrays<int>(&otherPtsCams);
    }
    if(!mesh.hasUVs())
    {
        // generate UV coordinates based on automatic uv atlas
        auto* updatedPtsCams = mesh.generateUVs(mp, ptsCams);
        std::swap(ptsCams, updatedPtsCams);
        deleteArrayOfArrays<int>(&updatedPtsCams);
        mesh.saveAsOBJ(outputFolder, "texturedMesh");
    }

    // generate textures
    mesh.generateTextures(mp, ptsCams, outputFolder);

    printfElapsedTime(startTime, "#");
    return EXIT_SUCCESS;
}
