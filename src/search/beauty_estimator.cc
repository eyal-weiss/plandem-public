#include "beauty_estimator.h"
using namespace std;

BeautyEstimator * get_estimator(EstimationInfo &estimation_info,
                          const int adjusted_cost,
                          const int factor_first,
                          const int factor_second,
                          const int factor_third) {
    if (!estimation_info.try_next) {
        return NULL;
    }
    
    int rank = estimation_info.rank;
    int lower_bound;
    
    switch (rank)
    {
    case 0:
        if (adjusted_cost > 0) { // Return an estimator.
            ++estimation_info.rank;
            lower_bound = adjusted_cost*factor_first;
        } else { // No estimation, we use the default cost and get perfect knowledge.
            estimation_info.try_next = false;
            lower_bound = adjusted_cost;
        }
        break;
    case 1:
        ++estimation_info.rank;
        lower_bound = adjusted_cost*factor_second;
        break;
    case 2:
        ++estimation_info.rank;
        lower_bound = adjusted_cost*factor_third;
        break;
    default:
        estimation_info.try_next = false;
        return NULL;
        break;
    }
    return new BeautyEstimator(lower_bound);
}

BeautyEstimator::BeautyEstimator(int lower_bound)
    : lower_bound(lower_bound) {
}

void BeautyEstimator::estimate(int &min_cost) {
    min_cost = lower_bound;
}