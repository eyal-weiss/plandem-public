#ifndef ESTIMATION_INFO_H
#define ESTIMATION_INFO_H

#include <limits>

// Holds action cost estimation bounds.

struct EstimationInfo{
    int min_g;
    int max_g;
    int min_cost;
    int max_cost;
    // Rank values are ordered from 0 to k, where 0 indicates that no estimation was performed.
    unsigned int rank : 4; // Limit number of bits, assumed no need for many estimators.
    bool try_next;

    EstimationInfo()
        : min_g(0), max_g(std::numeric_limits<int>::max()), 
          min_cost(0), max_cost(std::numeric_limits<int>::max()),
          rank(0), try_next(true) {
    }
};

#endif
