#ifndef ESTIMATOR_H
#define ESTIMATOR_H

#include "estimation_info.h"
#include "task_proxy.h"

class Estimator {
    int estimated_time; // In microseconds.
    double bounds_ratio;
    int rank;
    int cost;
    int uncertainty_factor;
    int delay_randomness;
public:
    Estimator(
        int estimated_time, double bounds_ratio, int rank,
        int cost, int uncertainty_factor, int delay_randomness);
    void estimate(int &lower_bound, int &upper_bound);
    int get_estimated_time();
    double get_bounds_ratio();
};

extern Estimator * get_estimator(EstimationInfo &estimation_info,
                                 const int adjusted_cost, 
                                 const double epsilon,
                                 const int edge_estimation_avg_time,
                                 const int edge_estimation_time_interval,
                                 const double first_estimator_probability,
                                 const double second_estimator_probability,
                                 const double third_estimator_probability);

#endif