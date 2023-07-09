#include "anytime_beauty.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../utils/logging.h"

#include <iostream>
#include <limits>

using namespace std;

namespace anytime_beauty {
AnytimeBeauty::AnytimeBeauty(const Options &opts, options::Registry &registry,
                               const options::Predefinitions &predefinitions)
    : SearchEngine(opts),
      engine_configs(opts.get_list<ParseTree>("engine_configs")),
      registry(registry),
      predefinitions(predefinitions),
      iter(1),
      solution_obtained(false),
      max_iter(opts.get<int>("max_iter"))
      {
    if (engine_configs.empty()) {
        cerr << "Error: No search engine specified" << endl;
        utils::exit_with(utils::ExitCode::SEARCH_INPUT_ERROR);
    }
}

shared_ptr<SearchEngine> AnytimeBeauty::get_search_engine() {
    OptionParser parser(engine_configs[0], registry, predefinitions, false);
    shared_ptr<SearchEngine> engine(parser.start_parsing<shared_ptr<SearchEngine>>());

    ostringstream stream;
    kptree::print_tree_bracketed(engine_configs[0], stream);
    utils::g_log << "Starting search: " << stream.str() << endl;

    return engine;
}

SearchStatus AnytimeBeauty::step() {
    shared_ptr<SearchEngine> current_search = get_search_engine();
    if (iter > max_iter) {
        cerr << "Error: too many iterations. Need to debug!" << endl;
        utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
    } 
    
    if (iter == max_iter) {
        current_search->set_l_est(l_high);
        current_search->set_l_prune(l_high);
    } else if (iter > 1) {
        current_search->set_l_est(l_low);
        current_search->set_l_prune(l_high);
    } // else: first iteration, no pruning, and no expensive estimation

    utils::g_log << "AnytimeBeauty, iteration number: " << iter << endl;
    ++iter;
    current_search->search();

    Plan the_plan;
    if (current_search->found_solution()) {
        solution_obtained = true;
        the_plan = current_search->get_plan();
        opt = current_search->get_opt();
        l_low = current_search->get_l_low();
        if (current_search->get_l_high() <= l_high) {
            l_high = current_search->get_l_high();
            plan_manager.save_plan(the_plan, task_proxy, true);
            set_plan(the_plan);
        }
    } else {
        solution_obtained = false;
    }

    current_search->print_statistics();
    const SearchStatistics &current_stats = current_search->get_statistics();

    int l1_diff = abs(statistics.get_l1_estimations() - current_stats.get_l1_estimations()); // TODO: check if this is correct
    int l2_diff = abs(statistics.get_l2_estimations() - current_stats.get_l2_estimations());
    int l3_diff = abs(statistics.get_l3_estimations() - current_stats.get_l3_estimations());
    utils::g_log << "New L1 estimations: " << l1_diff << endl;
    utils::g_log << "New L2 estimations: " << l2_diff << endl;
    utils::g_log << "New L3 estimations: " << l3_diff << endl;
    utils::g_log << "New overall estimations: " << l1_diff + l2_diff + l3_diff << endl;

    statistics.inc_edges(current_stats.get_edges());
    statistics.inc_expanded(current_stats.get_expanded());
    statistics.inc_pruned_states(current_stats.get_pruned_states());
    statistics.inc_evaluated_states(current_stats.get_evaluated_states());
    statistics.inc_estimated_edges(current_stats.get_estimated_edges());
    statistics.inc_l1_estimations(l1_diff);
    statistics.inc_l2_estimations(l2_diff);
    statistics.inc_l3_estimations(l3_diff);
    statistics.inc_evaluations(current_stats.get_evaluations());
    statistics.inc_estimations(l1_diff + l2_diff + l3_diff);
    statistics.inc_generated(current_stats.get_generated());
    statistics.inc_generated_ops(current_stats.get_generated_ops());
    statistics.inc_reopened(current_stats.get_reopened());

    current_search.reset();
    return step_return_value();
}

SearchStatus AnytimeBeauty::step_return_value() {
    if (not solution_obtained) {
        utils::g_log << endl << "Search exhausted and no solution found" << endl;
        return FAILED;
    }

    if (opt) {
        utils::g_log << endl << "Optimal solution found - stop searching" << endl;
        utils::g_log << "Total number of iterations in AnytimeBeauty: " << iter - 1 << endl;
        return SOLVED;
    } else {
        utils::g_log << "Optimal solution not yet found - keep searching" << endl;
        return IN_PROGRESS;
    }
}

void AnytimeBeauty::print_statistics() const {
    utils::g_log << "Cumulative statistics:" << endl;
    statistics.print_detailed_statistics();
}

void AnytimeBeauty::save_plan_if_necessary() {
    // We don't need to save here, as we automatically save after
    // each successful search iteration.
}

static shared_ptr<SearchEngine> _parse(OptionParser &parser) {
    parser.document_synopsis("Anytime beauty edge-cost estimation search", "");
    parser.add_list_option<ParseTree>("engine_configs",
                                      "list of search engines for each phase");
    parser.add_option<int>(
        "max_iter",
        "Maximum number of iterations, " 
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
        return make_shared<AnytimeBeauty>(opts, parser.get_registry(),
                                           parser.get_predefinitions());
    }
}

static Plugin<SearchEngine> _plugin("anytime_beauty", _parse);
}
