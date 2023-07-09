#ifndef SEARCH_ENGINES_ITERATED_SYNC_H
#define SEARCH_ENGINES_ITERATED_SYNC_H

#include "../option_parser_util.h"
#include "../search_engine.h"

#include "../options/registries.h"
#include "../options/predefinitions.h"

namespace options {
class Options;
}

namespace iterated_sync {
class IteratedSync : public SearchEngine {
    const std::vector<options::ParseTree> engine_configs;
    /*
      We need to copy the registry and predefinitions here since they live
      longer than the objects referenced in the constructor.
    */
    options::Registry registry;
    options::Predefinitions predefinitions;

    int iter;
    bool iterated_found_solution;
    const double epsilon;
    const double initial_epsilon;
    double target_epsilon;
    double best_uncertainty_bound;
    double overshoot;
    double eta_effective;
    double shrinkage_factor;
    double threshold;

    std::shared_ptr<SearchEngine> get_search_engine();
    void update_target_epsilon();
    void update_overshoot();
    SearchStatus step_return_value();

    virtual SearchStatus step() override;

public:
    IteratedSync(const options::Options &opts, options::Registry &registry,
                   const options::Predefinitions &predefinitions);

    virtual void save_plan_if_necessary() override;
    virtual void print_statistics() const override;
};
}

#endif