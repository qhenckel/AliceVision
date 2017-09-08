#pragma once

#include "mv_mesh_analyze.h"
#include "output3D/mv_output3D.h"

class mv_mesh_energy_opt : public mv_mesh_analyze
{
public:
    std::string tmpDir;

    mv_mesh_energy_opt(multiviewParams* _mp);
    ~mv_mesh_energy_opt();

    staticVector<point3d>* computeLaplacianPts();
    staticVector<point3d>* computeLaplacianPtsParallel();
    bool optimizeSmooth(float lambda, float epsilon, int type, int niter, staticVectorBool* ptsCanMove);

private:
    void updateGradientParallel(float lambda, float epsilon, int type, const point3d& LU, const point3d& RD,
                                staticVectorBool* ptsCanMove);
};
