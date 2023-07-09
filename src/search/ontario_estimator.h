#ifndef ONTARIO_ESTIMATOR_H
#define ONTARIO_ESTIMATOR_H

#include "estimation_info.h"
#include "task_proxy.h"
#include <map>
#include <tuple>

class OntarioEstimator {
    double bounds_ratio;
    int lower_bound;
    int upper_bound;

public:
    OntarioEstimator(
        double bounds_ratio, int lower_bound, int upper_bound);
    void estimate(int &min_cost, int &max_cost);
    double get_bounds_ratio();
    typedef std::map<std::tuple<int,int>, std::tuple<int,int>> EstimationMap;
    static EstimationMap estimationMap;
};

extern OntarioEstimator * get_estimator(EstimationInfo &estimation_info,
                                 const int adjusted_cost);

#endif