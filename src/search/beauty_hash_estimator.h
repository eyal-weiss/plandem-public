#ifndef BEAUTY_HASH_ESTIMATOR_H
#define BEAUTY_HASH_ESTIMATOR_H

#include "estimation_info.h"
#include "task_proxy.h"

class BeautyHashEstimator {
    int lower_bound;

public:
    BeautyHashEstimator(int lower_bound);
    void estimate(int &min_cost);
};

extern BeautyHashEstimator * get_estimator(EstimationInfo &estimation_info,
                                 const int adjusted_cost,
                                 const int seed);

#endif