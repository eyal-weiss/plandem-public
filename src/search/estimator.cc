#include "estimator.h"
#include <chrono>
#include <math.h> 
#include <random>
#include <thread>
using namespace std;

Estimator * get_estimator(EstimationInfo &estimation_info,
                          const int adjusted_cost,
                          const double epsilon,
                          const int edge_estimation_avg_time,
                          const int edge_estimation_time_interval,
                          const double first_estimator_probability,
                          const double second_estimator_probability,
                          const double third_estimator_probability) {
    if (!estimation_info.try_next) {
        return NULL;
    }
    
    int estimated_time = edge_estimation_avg_time;
    double bounds_ratio;
    int rank = estimation_info.rank; 
    // int uncertainty_factor = (int)ceil(epsilon + 1); // Arbitrary int value which is >= 2.
    int uncertainty_factor = 2;
    int delay_randomness = edge_estimation_time_interval;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib(0.0, 1.0);
    double sample_result = distrib(gen);
    switch (rank)
    {
    case 0:
        if ((sample_result < first_estimator_probability) and 
            (adjusted_cost > 0)) { // Return an estimator.
            ++estimation_info.rank;
            bounds_ratio = 2 * uncertainty_factor;
            // estimated_time *= const_1;
        } else { // No estimation, we use the default cost and get perfect knowledge.
            estimation_info.try_next = false;
            bounds_ratio = 1;
            estimated_time = 0;
        }
        break;
    case 1:
        if (sample_result < second_estimator_probability) { // Return an estimator.
            ++estimation_info.rank;
            bounds_ratio = uncertainty_factor;
            // estimated_time *= const_2;
        } else { // No more estimations.
            estimation_info.try_next = false;
            return NULL;
        }
        break;
    case 2:
        if (sample_result < third_estimator_probability) { // Return an estimator.
            ++estimation_info.rank;
            bounds_ratio = 1; // Perfect estimation.
            // estimated_time *= const_3;
        } else { // No more estimations.
            estimation_info.try_next = false;
            return NULL;
        }
        break;
    default:
        estimation_info.try_next = false;
        return NULL;
        break;
    }
    return new Estimator(estimated_time, bounds_ratio, estimation_info.rank,
                         adjusted_cost, uncertainty_factor, delay_randomness);
}

Estimator::Estimator(
    int estimated_time, double bounds_ratio, int rank,
    int cost, int uncertainty_factor, int delay_randomness)
    : estimated_time(estimated_time), 
    bounds_ratio(bounds_ratio),
    rank(rank),
    cost(cost),
    uncertainty_factor(uncertainty_factor),
    delay_randomness(delay_randomness) {
}

void Estimator::estimate(int &lower_bound, int &upper_bound) {
    using namespace std::this_thread; 
    using namespace std::chrono;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, delay_randomness);
    int delay = estimated_time;
    if (estimated_time > delay_randomness/2) {
        delay += (distrib(gen) - delay_randomness/2);
    }
    switch (rank)
    {
    case 0:
        lower_bound = cost;
        upper_bound = cost;
        break;
    case 1:
        sleep_for(microseconds(delay));
        lower_bound = cost;
        upper_bound = cost*2*uncertainty_factor;
        break;
    case 2:
        sleep_for(microseconds(delay));
        lower_bound = cost*2;
        upper_bound = cost*2*uncertainty_factor;
        break;
    case 3:
        sleep_for(microseconds(delay));
        lower_bound = cost*uncertainty_factor;
        upper_bound = cost*uncertainty_factor;
        break;
    default:
        // TODO: throw exception!
        break;
    }
}

int Estimator::get_estimated_time() {
    return estimated_time;
}

double Estimator::get_bounds_ratio() {
    return bounds_ratio;
}