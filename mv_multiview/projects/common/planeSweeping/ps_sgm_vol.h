#pragma once

#include "ps_sgm_params.h"

class ps_sgm_vol
{
public:
    ps_sgm_vol(float _volGpuMB, int _volDimX, int _volDimY, int _volDimZ, ps_sgm_params* _sp);
    ~ps_sgm_vol(void);

    void copyVolume(const staticVector<int>* volume);
    void copyVolume(const staticVector<unsigned char>* volume, int zFrom, int nZSteps);
    void addVolumeMin(const staticVector<unsigned char>* volume, int zFrom, int nZSteps);
    void addVolumeSecondMin(const staticVector<unsigned char>* volume, int zFrom, int nZSteps);
    void addVolumeAvg(int n, const staticVector<unsigned char>* volume, int zFrom, int nZSteps);

    void cloneVolumeStepZ();
    void cloneVolumeSecondStepZ();

    void SGMoptimizeVolumeStepZ(int rc, int volStepXY, int volLUX, int volLUY, int scale);
    staticVector<idValue>* getOrigVolumeBestIdValFromVolumeStepZ(int zborder);

    staticVector<unsigned char>* getZSlice(int z) const;
    void showVolume() const;


private:
    ps_sgm_params* sp;

    float volGpuMB;
    int volDimX;
    int volDimY;
    int volDimZ;
    int volStepZ;

    /// Volume containing the second best value accross multiple input volumes
    staticVector<unsigned char>* _volumeSecondBest;

    /// Volume containing the best value accross multiple input volumes
    staticVector<unsigned char>* _volume;

    /// The similarity volume after Z reduction. Volume dimension is (X, Y, Z/step).
    staticVector<unsigned char>* _volumeStepZ;
    /// Volume with the index of the original plane. Volume dimension (X, Y, Z/step).
    staticVector<int>* _volumeBestZ;
};