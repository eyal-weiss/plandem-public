#ifndef SEARCH_ENGINES_ANYTIME_BEAUTY_H
#define SEARCH_ENGINES_ANYTIME_BEAUTY_H

#include "../option_parser_util.h"
#include "../search_engine.h"

#include "../options/registries.h"
#include "../options/predefinitions.h"

namespace options {
class Options;
}

namespace anytime_beauty {
class AnytimeBeauty : public SearchEngine {
    const std::vector<options::ParseTree> engine_configs;
    /*
      We need to copy the registry and predefinitions here since they live
      longer than the objects referenced in the constructor.
    */
    options::Registry registry;
    options::Predefinitions predefinitions;

    int iter;
    bool solution_obtained;
    const int max_iter;

    std::shared_ptr<SearchEngine> get_search_engine();
    SearchStatus step_return_value();

    virtual SearchStatus step() override;

public:
    AnytimeBeauty(const options::Options &opts, options::Registry &registry,
                   const options::Predefinitions &predefinitions);

    virtual void save_plan_if_necessary() override;
    virtual void print_statistics() const override;
};
}

#endif