#ifndef BEAUTY_ESTIMATOR_H
#define BEAUTY_ESTIMATOR_H

#include "estimation_info.h"
#include "task_proxy.h"

class BeautyEstimator {
    int lower_bound;

public:
    BeautyEstimator(int lower_bound);
    void estimate(int &min_cost);
};

extern BeautyEstimator * get_estimator(EstimationInfo &estimation_info,
                                 const int adjusted_cost,
                                 const int factor_first,
                                 const int factor_second,
                                 const int factor_third);

#endif