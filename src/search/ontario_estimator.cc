#include "ontario_estimator.h"
using namespace std;

OntarioEstimator * get_estimator(EstimationInfo &estimation_info,
                          const int adjusted_cost) {
    if (!estimation_info.try_next) {
        return NULL;
    }
    
    double bounds_ratio;
    int rank = estimation_info.rank;
    int lower_bound;
    int upper_bound;
    std::tuple<int,int> bounds;
    
    switch (rank)
    {
    case 0:
        if (adjusted_cost != 10) { // Return an estimator.
            ++estimation_info.rank;
            
            bounds = OntarioEstimator::estimationMap[std::make_tuple(adjusted_cost,rank)];
            lower_bound = get<0>(bounds);
            upper_bound = get<1>(bounds);

            bounds_ratio = (double)upper_bound/lower_bound;
        } else { // No estimation, we use the default cost and get perfect knowledge.
            estimation_info.try_next = false;
            bounds_ratio = 1;
            lower_bound = adjusted_cost;
            upper_bound = adjusted_cost;
        }
        break;
    case 1:
        ++estimation_info.rank;
        
        bounds = OntarioEstimator::estimationMap[std::make_tuple(adjusted_cost,rank)];
        lower_bound = get<0>(bounds);
        upper_bound = get<1>(bounds);

        bounds_ratio = (double)upper_bound/lower_bound;
        break;
    default:
        estimation_info.try_next = false;
        return NULL;
        break;
    }
    return new OntarioEstimator(bounds_ratio, lower_bound, upper_bound);
}

OntarioEstimator::OntarioEstimator(
    double bounds_ratio, int lower_bound, int upper_bound)
    : bounds_ratio(bounds_ratio),
    lower_bound(lower_bound),
    upper_bound(upper_bound) {
}

OntarioEstimator::EstimationMap OntarioEstimator::estimationMap = { 
    { std::make_tuple(275,0), std::make_tuple(24,57) },
    { std::make_tuple(275,1), std::make_tuple(27,40) },
    { std::make_tuple(281,0), std::make_tuple(24,58) },
    { std::make_tuple(281,1), std::make_tuple(30,46) },
    { std::make_tuple(1101,0), std::make_tuple(95,227) },
    { std::make_tuple(1101,1), std::make_tuple(94,112) },
    { std::make_tuple(1106,0), std::make_tuple(95,228) },
    { std::make_tuple(1106,1), std::make_tuple(94,112) },
    { std::make_tuple(1035,0), std::make_tuple(89,213) },
    { std::make_tuple(1035,1), std::make_tuple(88,95) },
    { std::make_tuple(1020,0), std::make_tuple(88,210) },
    { std::make_tuple(1020,1), std::make_tuple(87,97) },
    { std::make_tuple(1678,0), std::make_tuple(144,346) },
    { std::make_tuple(1678,1), std::make_tuple(139,154) },
    { std::make_tuple(1683,0), std::make_tuple(145,347) },
    { std::make_tuple(1683,1), std::make_tuple(139,159) },
    { std::make_tuple(852,0), std::make_tuple(73,176) },
    { std::make_tuple(852,1), std::make_tuple(75,83) },
    { std::make_tuple(837,0), std::make_tuple(72,172) },
    { std::make_tuple(837,1), std::make_tuple(72,82) },
    { std::make_tuple(479,0), std::make_tuple(41,99) },
    { std::make_tuple(479,1), std::make_tuple(42,49) },
    { std::make_tuple(478,0), std::make_tuple(41,98) },
    { std::make_tuple(478,1), std::make_tuple(42,49) },
    { std::make_tuple(657,0), std::make_tuple(57,135) },
    { std::make_tuple(657,1), std::make_tuple(62,77) },
    { std::make_tuple(646,0), std::make_tuple(56,133) },
    { std::make_tuple(646,1), std::make_tuple(59,71) },
    { std::make_tuple(408,0), std::make_tuple(35,84) },
    { std::make_tuple(408,1), std::make_tuple(37,48) },
    { std::make_tuple(407,0), std::make_tuple(35,84) },
    { std::make_tuple(407,1), std::make_tuple(37,45) },
    { std::make_tuple(522,0), std::make_tuple(45,108) },
    { std::make_tuple(522,1), std::make_tuple(52,69) },
    { std::make_tuple(476,0), std::make_tuple(41,98) },
    { std::make_tuple(476,1), std::make_tuple(49,67) },
    { std::make_tuple(442,0), std::make_tuple(38,91) },
    { std::make_tuple(442,1), std::make_tuple(39,47) },
    { std::make_tuple(443,0), std::make_tuple(38,91) },
    { std::make_tuple(443,1), std::make_tuple(40,47) },
    { std::make_tuple(562,0), std::make_tuple(48,116) },
    { std::make_tuple(562,1), std::make_tuple(51,61) },
    { std::make_tuple(578,0), std::make_tuple(50,119) },
    { std::make_tuple(578,1), std::make_tuple(52,65) },
    { std::make_tuple(506,0), std::make_tuple(44,104) },
    { std::make_tuple(506,1), std::make_tuple(48,58) },
    { std::make_tuple(496,0), std::make_tuple(43,102) },
    { std::make_tuple(496,1), std::make_tuple(48,58) },
    { std::make_tuple(376,0), std::make_tuple(32,77) },
    { std::make_tuple(376,1), std::make_tuple(39,44) },
    { std::make_tuple(377,0), std::make_tuple(32,78) },
    { std::make_tuple(377,1), std::make_tuple(36,44) },
    { std::make_tuple(607,0), std::make_tuple(52,125) },
    { std::make_tuple(607,1), std::make_tuple(59,71) },
    { std::make_tuple(606,0), std::make_tuple(52,125) },
    { std::make_tuple(606,1), std::make_tuple(59,71) },
    { std::make_tuple(552,0), std::make_tuple(47,114) },
    { std::make_tuple(552,1), std::make_tuple(48,61) },
    { std::make_tuple(544,0), std::make_tuple(47,112) },
    { std::make_tuple(544,1), std::make_tuple(48,60) }
};  

void OntarioEstimator::estimate(int &min_cost, int &max_cost) {
    min_cost = lower_bound;
    max_cost = upper_bound;
}

double OntarioEstimator::get_bounds_ratio() {
    return bounds_ratio;
}