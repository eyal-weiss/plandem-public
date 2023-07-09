#include "beauty_hash_estimator.h"
using namespace std;

BeautyHashEstimator * get_estimator(EstimationInfo &estimation_info,
                          const int adjusted_cost,
                          const int seed) {
    if (!estimation_info.try_next) {
        return NULL;
    }
    
    int rank = estimation_info.rank;
    int lower_bound;
    
    // Determine estimator settings using simple hash function.
    int hash_value = (adjusted_cost + seed) % 9;
    int factor_first;
    int factor_second;
    int factor_third;
    switch (hash_value)
    {
    case 0:
        factor_first = 1;
        factor_second = factor_first + 1;
        break;
    case 1:
        factor_first = 2;
        factor_second = factor_first + 1;
        break;
    case 2:
        factor_first = 3;
        factor_second = factor_first + 1;
        break;
    case 3:
        factor_first = 1;
        factor_second = factor_first + 2;
        break;
    case 4:
        factor_first = 2;
        factor_second = factor_first + 2;
        break;
    case 5:
        factor_first = 3;
        factor_second = factor_first + 2;
        break;
    case 6:
        factor_first = 1;
        factor_second = factor_first + 3;
        break;
    case 7:
        factor_first = 2;
        factor_second = factor_first + 3;
        break;
    
    default: // case 8
        factor_first = 3;
        factor_second = factor_first + 3;
        break;
    }
    factor_third = factor_second + 1;

    // Set estimator values according to rank.
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
    return new BeautyHashEstimator(lower_bound);
}

BeautyHashEstimator::BeautyHashEstimator(int lower_bound)
    : lower_bound(lower_bound) {
}

void BeautyHashEstimator::estimate(int &min_cost) {
    min_cost = lower_bound;
}