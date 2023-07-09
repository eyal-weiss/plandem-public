#include "iterated_sync.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../utils/logging.h"

#include <iostream>
#include <limits>

using namespace std;

namespace iterated_sync {
IteratedSync::IteratedSync(const Options &opts, options::Registry &registry,
                               const options::Predefinitions &predefinitions)
    : SearchEngine(opts),
      engine_configs(opts.get_list<ParseTree>("engine_configs")),
      registry(registry),
      predefinitions(predefinitions),
      iter(1),
      best_uncertainty_bound(numeric_limits<double>::infinity()),
      eta_effective(numeric_limits<double>::infinity()),
      overshoot(numeric_limits<double>::infinity()),
      iterated_found_solution(false),
      epsilon(opts.get<double>("epsilon")),
      initial_epsilon(opts.get<double>("initial_epsilon")){
    target_epsilon = initial_epsilon;
    if ((opts.get<double>("shrinkage_factor") >= 0) and (opts.get<double>("shrinkage_factor") <= 1)) {
        shrinkage_factor = opts.get<double>("shrinkage_factor");
    } else {
        shrinkage_factor = 1;
    }

    if (opts.get<double>("threshold") >= 0) {
        threshold = opts.get<double>("threshold")/100;
    } else {
        threshold = 0.1;
    }
}

shared_ptr<SearchEngine> IteratedSync::get_search_engine() {
    OptionParser parser(engine_configs[0], registry, predefinitions, false);
    shared_ptr<SearchEngine> engine(parser.start_parsing<shared_ptr<SearchEngine>>());

    ostringstream stream;
    kptree::print_tree_bracketed(engine_configs[0], stream);
    utils::g_log << "Starting search: " << stream.str() << endl;

    return engine;
}

void IteratedSync::update_target_epsilon() {
    if (iter > 1) {
        double curr_target = 1 + shrinkage_factor*((initial_epsilon - 1)/overshoot);
        if ((1 - curr_target/target_epsilon) > threshold) {
            target_epsilon = curr_target;
        } else {
            target_epsilon = 1;
        }
    }
}

void IteratedSync::update_overshoot() {
    if (target_epsilon == 1) {
        if (eta_effective == 1) {
            overshoot = 1;
        } else {
            overshoot = numeric_limits<double>::infinity();
        }
    } else {
        overshoot = (eta_effective - 1)/(target_epsilon - 1);
    }
}

SearchStatus IteratedSync::step() {
    if ((iter > 1) and (target_epsilon == 1)) {
        utils::g_log << endl << "Search exhausted and no epsilon-optimal solution found" << endl;
        utils::g_log << "Total number of iterations in IteratedSync: " << iter - 1 << endl;
        return found_solution() ? SOLVED : FAILED;
    }
    
    shared_ptr<SearchEngine> current_search = get_search_engine();
    update_target_epsilon();
    current_search->set_target_epsilon(target_epsilon);
    utils::g_log << "IteratedSync, iteration number: " << iter << endl;
    ++iter;
    current_search->search();

    Plan found_plan;
    if (current_search->found_solution()) {
        iterated_found_solution = true;
        found_plan = current_search->get_plan();
        eta_effective = current_search->get_uncertainty_ratio();
        if (eta_effective < best_uncertainty_bound) {
            plan_manager.save_plan(found_plan, task_proxy, true);
            best_uncertainty_bound = eta_effective;
            set_plan(found_plan);
        }
    } else {
        eta_effective = numeric_limits<double>::infinity();
    }
    update_overshoot();
    current_search->print_statistics();

    const SearchStatistics &current_stats = current_search->get_statistics();
    statistics.inc_edges(current_stats.get_edges());
    statistics.inc_expanded(current_stats.get_expanded());
    statistics.inc_evaluated_states(current_stats.get_evaluated_states());
    statistics.inc_estimated_edges(current_stats.get_estimated_edges());
    statistics.inc_evaluations(current_stats.get_evaluations());
    statistics.inc_estimations(current_stats.get_estimations());
    statistics.inc_generated(current_stats.get_generated());
    statistics.inc_generated_ops(current_stats.get_generated_ops());
    statistics.inc_reopened(current_stats.get_reopened());

    return step_return_value();
}

SearchStatus IteratedSync::step_return_value() {
    if (iterated_found_solution) {
        utils::g_log << endl << "Best uncertainty bound so far: " << best_uncertainty_bound << endl;
    } else {
        utils::g_log << endl << "Search exhausted and no solution found" << endl;
        return FAILED;
    }

    if (best_uncertainty_bound <= epsilon) {
        utils::g_log << endl << "Epsilon-optimal solution found - stop searching" << endl;
        utils::g_log << "Total number of iterations in IteratedSync: " << iter - 1 << endl;
        return SOLVED;
    } else {
        utils::g_log << "No epsilon-optimal solution found - keep searching" << endl;
        return IN_PROGRESS;
    }
}

void IteratedSync::print_statistics() const {
    utils::g_log << "Cumulative statistics:" << endl;
    statistics.print_detailed_statistics();
}

void IteratedSync::save_plan_if_necessary() {
    // We don't need to save here, as we automatically save after
    // each successful search iteration.
}

static shared_ptr<SearchEngine> _parse(OptionParser &parser) {
    parser.document_synopsis("Iterated synchronic edge-cost estimation search", "");
    parser.document_note(
        "Note 1",
        "We don't cache edge-cost estimation values between search iterations at"
        " the moment.");
    parser.add_list_option<ParseTree>("engine_configs",
                                      "list of search engines for each phase");
    parser.add_option<double>(
        "epsilon",
        "Sub-optimality bound, default value set to 1",
        "1");
    parser.add_option<double>(
        "initial_epsilon",
        "Initial target bound, default value set to 1",
        "1");
    parser.add_option<double>(
        "shrinkage_factor",
        "shrinkage factor for rolling target sub-optimality bound, default value set to 1",
        "1");
    parser.add_option<double>(
        "threshold",
        "Threshold for rolling target sub-optimality bound decrease between iterations in percentages, " 
        "default value set to 10",
        "10");
    SearchEngine::add_options_to_parser(parser);
    Options opts = parser.parse();

    opts.verify_list_non_empty<ParseTree>("engine_configs");

    if (parser.help_mode()) {
        return nullptr;
    } else if (parser.dry_run()) {
        //check if the supplied search engines can be parsed
        for (const ParseTree &config : opts.get_list<ParseTree>("engine_configs")) {
            OptionParser test_parser(config, parser.get_registry(),
                                     parser.get_predefinitions(), true);
            test_parser.start_parsing<shared_ptr<SearchEngine>>();
        }
        return nullptr;
    } else {
        return make_shared<IteratedSync>(opts, parser.get_registry(),
                                           parser.get_predefinitions());
    }
}

static Plugin<SearchEngine> _plugin("iterated_sync", _parse);
}
