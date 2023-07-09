#ifndef SEARCH_ENGINES_BEAUTY_H
#define SEARCH_ENGINES_BEAUTY_H

#include "../open_list.h"
#include "../search_engine.h"

#include <memory>
#include <vector>

class Evaluator;
class PruningMethod;

namespace options {
class OptionParser;
class Options;
}

namespace beauty {
class Beauty : public SearchEngine {
    const bool reopen_closed_nodes;

    std::unique_ptr<StateOpenList> open_list;
    std::shared_ptr<Evaluator> f_evaluator;

    std::vector<Evaluator *> path_dependent_evaluators;
    std::vector<std::shared_ptr<Evaluator>> preferred_operator_evaluators;
    std::shared_ptr<Evaluator> lazy_evaluator;

    std::shared_ptr<PruningMethod> pruning_method;

    // cost relaxation bound
    const int factor_first;
    const int factor_second;
    const int factor_third;
    const int seed;

    void start_f_value_statistics(EvaluationContext &eval_context);
    void update_f_value_statistics(EvaluationContext &eval_context);
    void reward_progress();
    void perform_end_of_search_estimations(const State &state);

protected:
    virtual void initialize() override;
    virtual SearchStatus step() override;

public:
    explicit Beauty(const options::Options &opts);
    virtual ~Beauty() = default;

    virtual void print_statistics() const override;

    void dump_search_space() const;
};

extern void add_options_to_parser(options::OptionParser &parser);
}

#endif
