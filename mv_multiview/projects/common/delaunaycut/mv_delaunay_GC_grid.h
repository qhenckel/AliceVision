#pragma once

#include "mv_delaunay_GC.h"
#include "output3D/mv_output3D.h"

class mv_delanuay_GC_grid : public mv_delanuay_GC
{
public:
    point3d vx, vy, vz;
    float sx, sy, sz;

    mv_delanuay_GC_grid(multiviewParams* _mp, mv_prematch_cams* _pc)
        : mv_delanuay_GC(_mp, _pc){};

    void addCams(GC_Vertex_handle& vh, staticVector<int>* ptcams);

    // void fillBall(point3d pt, float lambda_vis_in, int cam);

    void createTriangulationFromDepthMapsCamsVoxelGrid(staticVector<int>* cams, std::string fileNameWrl, int scale,
                                                       staticVector<int>* voxelsIds, point3d voxel[8], int numSubVoxs,
                                                       bool doMoveIntoGridPosition);
    staticVector<int>* getCamsForWhoseThePointIsVisible(point3d p, point3d tris[12][3], staticVector<int>* cams);
    void createTriangulationFromDepthMapsCamsVoxelGrid1(staticVector<int>* cams, std::string fileNameWrl, int scale,
                                                        staticVector<int>* voxelsIds, point3d voxel[8], int numSubVoxs);

    bool createTriangulationForVoxelGrid(point3d voxel[8], staticVector<int>* voxelsIds, std::string folderName,
                                         float inflateHexahfactor, int numSubVoxs, bool doMoveIntoGridPosition);

    // virtual void fillGraph(bool negVisIn, bool negVisOut, bool facetConfPhoto, float sigmaPixelSize);
    bool reconstructVoxelGrid(point3d hexah[8], staticVector<int>* voxelsIds, std::string folderName,
                              std::string tmpCamsPtsFolderName, int numSubVoxs, bool doMoveIntoGridPosition);
};
