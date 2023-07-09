#include "search_statistics.h"

#include "utils/logging.h"
#include "utils/timer.h"
#include "utils/system.h"

#include <iostream>

using namespace std;


SearchStatistics::SearchStatistics(utils::Verbosity verbosity)
    : verbosity(verbosity) {
    edges = 0;
    expanded_states = 0;
    reopened_states = 0;
    evaluated_states = 0;
    pruned_states = 0;
    estimated_edges = 0;
    evaluations = 0;
    estimations = 0;
    l1_estimations = 0;
    l2_estimations = 0;
    l3_estimations = 0;
    generated_states = 0;
    dead_end_states = 0;
    generated_ops = 0;

    lastjump_expanded_states = 0;
    lastjump_reopened_states = 0;
    lastjump_evaluated_states = 0;
    lastjump_estimated_edges = 0;
    lastjump_generated_states = 0;

    lastjump_f_value = -1;
}

void SearchStatistics::report_f_value_progress(int f) {
    if (f > lastjump_f_value) {
        lastjump_f_value = f;
        print_f_line();
        lastjump_expanded_states = expanded_states;
        lastjump_reopened_states = reopened_states;
        lastjump_evaluated_states = evaluated_states;
        lastjump_estimated_edges = estimated_edges;
        lastjump_generated_states = generated_states;
    }
}

void SearchStatistics::print_f_line() const {
    if (verbosity >= utils::Verbosity::NORMAL) {
        utils::g_log << "f = " << lastjump_f_value
                     << ", ";
        print_basic_statistics();
        utils::g_log << endl;
    }
}

void SearchStatistics::print_checkpoint_line(int g) const {
    if (verbosity >= utils::Verbosity::NORMAL) {
        utils::g_log << "g=" << g << ", ";
        print_basic_statistics();
        utils::g_log << endl;
    }
}

void SearchStatistics::print_basic_statistics() const {
    utils::g_log << evaluated_states << " evaluated, "
                 << expanded_states << " expanded, "
                 << estimated_edges << " estimated";
    if (reopened_states > 0) {
        utils::g_log << ", " << reopened_states << " reopened";
    }
}

void SearchStatistics::print_detailed_statistics() const {
    utils::g_log << "Encountered " << edges << " edge(s)." << endl;
    utils::g_log << "Expanded " << expanded_states << " state(s)." << endl;
    utils::g_log << "Reopened " << reopened_states << " state(s)." << endl;
    utils::g_log << "Evaluated " << evaluated_states << " state(s)." << endl;
    utils::g_log << "Pruned " << pruned_states << " state(s)." << endl;
    utils::g_log << "Estimated " << estimated_edges << " edge(s)." << endl;
    utils::g_log << "Evaluations: " << evaluations << endl;
    utils::g_log << "Estimations: " << estimations << endl;
    utils::g_log << "L1 estimations: " << l1_estimations << endl;
    utils::g_log << "L2 estimations: " << l2_estimations << endl;
    utils::g_log << "L3 estimations: " << l3_estimations << endl;
    utils::g_log << "Generated " << generated_states << " state(s)." << endl;
    utils::g_log << "Dead ends: " << dead_end_states << " state(s)." << endl;

    if (lastjump_f_value >= 0) {
        utils::g_log << "Expanded until last jump: "
                     << lastjump_expanded_states << " state(s)." << endl;
        utils::g_log << "Reopened until last jump: "
                     << lastjump_reopened_states << " state(s)." << endl;
        utils::g_log << "Evaluated until last jump: "
                     << lastjump_evaluated_states << " state(s)." << endl;
        utils::g_log << "Estimated until last jump: "
                     << lastjump_estimated_edges << " edge(s)." << endl;
        utils::g_log << "Generated until last jump: "
                     << lastjump_generated_states << " state(s)." << endl;
    }
}
