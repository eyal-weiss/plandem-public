#include "beauty.h"

// #include "../beauty_estimator.h"
#include "../beauty_hash_estimator.h"
#include "../evaluation_context.h"
#include "../evaluator.h"
#include "../open_list_factory.h"
#include "../option_parser.h"
#include "../pruning_method.h"

#include "../algorithms/ordered_set.h"
#include "../task_utils/successor_generator.h"
#include "../utils/logging.h"

#include <cassert>
#include <cstdlib>
#include <limits>
#include <memory>
#include <set>

#include "../ext/optional.hh"

using namespace std;

namespace beauty {
Beauty::Beauty(const Options &opts)
    : SearchEngine(opts),
      reopen_closed_nodes(opts.get<bool>("reopen_closed")),
      open_list(opts.get<shared_ptr<OpenListFactory>>("open")->
                create_state_open_list()),
      f_evaluator(opts.get<shared_ptr<Evaluator>>("f_eval", nullptr)),
      preferred_operator_evaluators(opts.get_list<shared_ptr<Evaluator>>("preferred")),
      lazy_evaluator(opts.get<shared_ptr<Evaluator>>("lazy_evaluator", nullptr)),
      pruning_method(opts.get<shared_ptr<PruningMethod>>("pruning")),
      seed(opts.get<int>("seed")),
      factor_first(opts.get<int>("factor_first")),
      factor_second(opts.get<int>("factor_second")),
      factor_third(opts.get<int>("factor_third")) {
    if (lazy_evaluator && !lazy_evaluator->does_cache_estimates()) {
        cerr << "lazy_evaluator must cache its estimates" << endl;
        utils::exit_with(utils::ExitCode::SEARCH_INPUT_ERROR);
    }
}

void Beauty::initialize() {
    utils::g_log << "Conducting best first search"
                 << (reopen_closed_nodes ? " with" : " without")
                 << " reopening closed nodes, (real) bound = " << bound
                 << ", l_est = " << l_est
                 << ", l_prune = " << l_prune
                 << endl;
    assert(open_list);

    set<Evaluator *> evals;
    open_list->get_path_dependent_evaluators(evals);

    /*
      Collect path-dependent evaluators that are used for preferred operators
      (in case they are not also used in the open list).
    */
    for (const shared_ptr<Evaluator> &evaluator : preferred_operator_evaluators) {
        evaluator->get_path_dependent_evaluators(evals);
    }

    /*
      Collect path-dependent evaluators that are used in the f_evaluator.
      They are usually also used in the open list and will hence already be
      included, but we want to be sure.
    */
    if (f_evaluator) {
        f_evaluator->get_path_dependent_evaluators(evals);
    }

    /*
      Collect path-dependent evaluators that are used in the lazy_evaluator
      (in case they are not already included).
    */
    if (lazy_evaluator) {
        lazy_evaluator->get_path_dependent_evaluators(evals);
    }

    path_dependent_evaluators.assign(evals.begin(), evals.end());

    State initial_state = state_registry.get_initial_state();
    for (Evaluator *evaluator : path_dependent_evaluators) {
        evaluator->notify_initial_state(initial_state);
    }

    /*
      Note: we consider the initial state as reached by a preferred
      operator.
    */
    EvaluationContext eval_context(initial_state, 0, true, &statistics);

    statistics.inc_evaluated_states();

    if (open_list->is_dead_end(eval_context)) {
        utils::g_log << "Initial state is a dead end." << endl;
    } else {
        if (search_progress.check_progress(eval_context))
            statistics.print_checkpoint_line(0);
        start_f_value_statistics(eval_context);
        SearchNode node = search_space.get_node(initial_state);
        node.open_initial();

        open_list->insert(eval_context, initial_state.get_id());
    }

    print_initial_evaluator_values(eval_context);

    pruning_method->initialize(task);
}

void Beauty::print_statistics() const {
    statistics.print_detailed_statistics();
    search_space.print_statistics();
    pruning_method->print_statistics();
}

SearchStatus Beauty::step() {
    tl::optional<SearchNode> node;
    while (true) {
        if (open_list->empty()) {
            utils::g_log << "Completely explored state space -- no solution!" << endl;
            return FAILED;
        }
        StateID id = open_list->remove_min();
        State s = state_registry.lookup_state(id);
        node.emplace(search_space.get_node(s));

        if (node->is_closed())
            continue;

        /*
          We can pass calculate_preferred=false here since preferred
          operators are computed when the state is expanded.
        */
        EvaluationContext eval_context(s, node->get_g(), false, &statistics);

        if (lazy_evaluator) {
            /*
              With lazy evaluators (and only with these) we can have dead nodes
              in the open list.

              For example, consider a state s that is reached twice before it is expanded.
              The first time we insert it into the open list, we compute a finite
              heuristic value. The second time we insert it, the cached value is reused.

              During first expansion, the heuristic value is recomputed and might become
              infinite, for example because the reevaluation uses a stronger heuristic or
              because the heuristic is path-dependent and we have accumulated more
              information in the meantime. Then upon second expansion we have a dead-end
              node which we must ignore.
            */
            if (node->is_dead_end())
                continue;

            if (lazy_evaluator->is_estimate_cached(s)) {
                int old_h = lazy_evaluator->get_cached_estimate(s);
                int new_h = eval_context.get_evaluator_value_or_infinity(lazy_evaluator.get());
                if (open_list->is_dead_end(eval_context)) {
                    node->mark_as_dead_end();
                    statistics.inc_dead_ends();
                    continue;
                }
                if (new_h != old_h) {
                    open_list->insert(eval_context, id);
                    continue;
                }
            }
        }

        node->close();
        assert(!node->is_dead_end());
        update_f_value_statistics(eval_context);
        statistics.inc_expanded();
        break;
    }

    const State &s = node->get_state();
    if (check_goal_and_set_plan(s)) {
        utils::g_log << "Estimations before ESE: " << statistics.get_estimations() << endl;
        perform_end_of_search_estimations(s);
        utils::g_log << "Estimations after ESE: " << statistics.get_estimations() << endl;
        if (opt) {
            utils::g_log << "The plan found is optimal" << endl;
            utils::g_log << "Final l* is: " << l_high << endl;
        } else {
            utils::g_log << "The plan found is not necessarily optimal" << endl;
            utils::g_log << "Final lower bound for l* is: " << l_low << endl;
            utils::g_log << "Final upper bound for l* is: " << l_high << endl;
        }
        open_list->clear();
        return SOLVED;
    }
    // statistics.print_checkpoint_line(node->get_g()); // for debugging

    vector<OperatorID> applicable_ops;
    successor_generator.generate_applicable_ops(s, applicable_ops);

    /*
      TODO: When preferred operators are in use, a preferred operator will be
      considered by the preferred operator queues even when it is pruned.
    */
    pruning_method->prune_operators(s, applicable_ops);

    // This evaluates the expanded state (again) to get preferred ops
    EvaluationContext eval_context(s, node->get_g(), false, &statistics, true);
    ordered_set::OrderedSet<OperatorID> preferred_operators;
    for (const shared_ptr<Evaluator> &preferred_operator_evaluator : preferred_operator_evaluators) {
        collect_preferred_operators(eval_context,
                                    preferred_operator_evaluator.get(),
                                    preferred_operators);
    }

    // utils::g_log << "applicable_ops.size is: " << applicable_ops.size() << endl; // for debugging

    for (OperatorID op_id : applicable_ops) {
        OperatorProxy op = task_proxy.get_operators()[op_id];
        if ((node->get_real_g() + op.get_cost()) >= bound)
            continue;

        State succ_state = state_registry.get_successor_state(s, op);
        statistics.inc_generated();
        bool is_preferred = preferred_operators.contains(op_id);

        SearchNode succ_node = search_space.get_node(succ_state);

        for (Evaluator *evaluator : path_dependent_evaluators) {
            evaluator->notify_state_transition(s, op_id, succ_state);
        }

        // Previously encountered dead end. Don't re-evaluate.
        if (succ_node.is_dead_end())
            continue;

        EstimationInfo estimation_info;
        bool first_estimate = true;

        if (succ_node.is_new()) {
            statistics.inc_edges();
            // For non-synthetic data the signature should be get_estimator(s, op)
            // BeautyEstimator *estimator_ptr = get_estimator(estimation_info, 
            //                                                 get_adjusted_cost(op),
            //                                                 factor_first,
            //                                                 factor_second,
            //                                                 factor_third);
            BeautyHashEstimator *estimator_ptr = get_estimator(estimation_info, 
                                                                get_adjusted_cost(op),
                                                                seed);
            if (estimation_info.try_next) {
                statistics.inc_estimated_edges();
            }
            do
            {
                if (not first_estimate) { 
                    // estimator_ptr = get_estimator(estimation_info, 
                    //                                 get_adjusted_cost(op),
                    //                                 factor_first,
                    //                                 factor_second,
                    //                                 factor_third);
                    estimator_ptr = get_estimator(estimation_info, 
                                                    get_adjusted_cost(op),
                                                    seed);
                    if (estimator_ptr == NULL) {
                        break;
                    }
                }

                if (estimation_info.try_next) {
                    statistics.inc_estimations();
                }

                switch (estimation_info.rank)
                {
                case 1:
                    statistics.inc_l1_estimations();
                    break;
                case 2:
                    statistics.inc_l2_estimations();
                    break;
                case 3:
                    statistics.inc_l3_estimations();
                    break;
                default:
                    break;
                }

                estimator_ptr->estimate(estimation_info.min_cost);
                estimation_info.min_g = node->get_min_g() + estimation_info.min_cost;
                first_estimate = false;
                // utils::g_log << "new node, estimation_info.min_g is: " << estimation_info.min_g  << endl; // for debugging
            } while (estimation_info.min_g <= l_est);

            // We have not seen this state before.
            // Evaluate and create a new node.

            // Careful: succ_node.get_g() is not available here yet,
            // hence the stupid computation of succ_g.
            // TODO: Make this less fragile.
            int succ_g = node->get_g() + get_adjusted_cost(op);

            EvaluationContext succ_eval_context(
                succ_state, succ_g, is_preferred, &statistics, &estimation_info);
            statistics.inc_evaluated_states();

            if (open_list->is_dead_end(succ_eval_context)) {
                succ_node.mark_as_dead_end();
                statistics.inc_dead_ends();
                continue;
            }

            if (estimation_info.min_g > l_prune) {
                statistics.inc_pruned_states();
                continue;
            }

            succ_node.open(*node, op, get_adjusted_cost(op), &estimation_info);

            open_list->insert(succ_eval_context, succ_state.get_id());
            if (search_progress.check_progress(succ_eval_context)) {
                statistics.print_checkpoint_line(succ_node.get_g());
                // utils::g_log << "Current lower bound for l* is: " << estimation_info.min_g  << endl; // for debugging
                reward_progress();
            }
        } else {
            if (succ_node.is_same_edge(*node, op)) { // Already done edge estimations.
                search_space.set_estimation_info_based_on_edge(estimation_info,
                                                               *node, succ_node);
            } else { // New edge, need to estimate.
                statistics.inc_edges();
                // BeautyEstimator *estimator_ptr = get_estimator(estimation_info, 
                //                                                 get_adjusted_cost(op),
                //                                                 factor_first,
                //                                                 factor_second,
                //                                                 factor_third);
                BeautyHashEstimator *estimator_ptr = get_estimator(estimation_info, 
                                                                    get_adjusted_cost(op),
                                                                    seed);
                if (estimation_info.try_next) {
                    statistics.inc_estimated_edges();
                }
                do
                {
                    if (not first_estimate) { 
                        // estimator_ptr = get_estimator(estimation_info, 
                        //                                 get_adjusted_cost(op),
                        //                                 factor_first,
                        //                                 factor_second,
                        //                                 factor_third);
                        estimator_ptr = get_estimator(estimation_info, 
                                                        get_adjusted_cost(op),
                                                        seed);
                        if (estimator_ptr == NULL) {
                            break;
                        }
                    }

                    if (estimation_info.try_next) {
                        statistics.inc_estimations();
                    }

                    switch (estimation_info.rank)
                    {
                    case 1:
                        statistics.inc_l1_estimations();
                        break;
                    case 2:
                        statistics.inc_l2_estimations();
                        break;
                    case 3:
                        statistics.inc_l3_estimations();
                        break;
                    default:
                        break;
                    }

                    estimator_ptr->estimate(estimation_info.min_cost);
                    estimation_info.min_g = node->get_min_g() + estimation_info.min_cost; 
                    first_estimate = false;            
                    // utils::g_log << "old node, estimation_info.min_g is: " << estimation_info.min_g  << endl; // for debugging
                } while ((estimation_info.min_g <= l_est) and
                         (estimation_info.min_g < succ_node.get_min_g()));
            }
            
            if ((estimation_info.min_g < succ_node.get_min_g()) and
                (estimation_info.min_g <= l_prune)) { 
                // We found a new cheapest path to an open or closed state.
                if (reopen_closed_nodes) {
                    if (succ_node.is_closed()) {
                        /*
                        TODO: It would be nice if we had a way to test
                        that reopening is expected behaviour, i.e., exit
                        with an error when this is something where
                        reopening should not occur (e.g. A* with a
                        consistent heuristic).
                        */
                        statistics.inc_reopened();
                    }

                    /*
                        succ_node.info.curr_estimation.min_g = succ_curr_cost_min;
                        succ_node.info.curr_estimation.max_g = succ_curr_cost_max;
                    */

                    succ_node.reopen(*node, op, get_adjusted_cost(op), &estimation_info);

                    EvaluationContext succ_eval_context(
                        succ_state, succ_node.get_g(), is_preferred, 
                        &statistics, &estimation_info);

                    /*
                    Note: our old code used to retrieve the h value from
                    the search node here. Our new code recomputes it as
                    necessary, thus avoiding the incredible ugliness of
                    the old "set_evaluator_value" approach, which also
                    did not generalize properly to settings with more
                    than one evaluator.

                    Reopening should not happen all that frequently, so
                    the performance impact of this is hopefully not that
                    large. In the medium term, we want the evaluators to
                    remember evaluator values for states themselves if
                    desired by the user, so that such recomputations
                    will just involve a look-up by the Evaluator object
                    rather than a recomputation of the evaluator value
                    from scratch.
                    */
                    open_list->insert(succ_eval_context, succ_state.get_id());
                } else {
                    // If we do not reopen closed nodes, we just update the parent pointers.
                    // Note that this could cause an incompatibility between
                    // the g-value and the actual path that is traced back.
                    succ_node.update_parent(*node, op, get_adjusted_cost(op), &estimation_info);
                }
            } 
        }
    }
    return IN_PROGRESS;
}

void Beauty::perform_end_of_search_estimations(const State &state) {   
    // initialization
    State curr_state = state;
    SearchNode curr_node = search_space.get_node(curr_state);
    int lower_bound = curr_node.get_min_g();
    opt = true;
    l_low = lower_bound;
    // get the best g_value from OPEN
    int l_alt = 0;
    if (open_list->empty()) {
        l_alt = numeric_limits<int>::max();
    } else {
        StateID alt_id = open_list->remove_min();
        State alt_s = state_registry.lookup_state(alt_id);
        SearchNode alt_node = search_space.get_node(alt_s);
        l_alt = alt_node.get_min_g();
    }
    
    // outer loop over edges
    for (;;) {
        SearchNode curr_node = search_space.get_node(curr_state);
        StateID parent_state_id = curr_node.get_parent_state_id();
        OperatorID creating_operator_id = curr_node.get_creating_operator();
        OperatorProxy op = task_proxy.get_operators()[creating_operator_id];
        if (creating_operator_id == OperatorID::no_operator) {
            assert(parent_state_id == StateID::no_state);
            break;
        }
        // inner loop over estimators for edge
        State parent_state = state_registry.lookup_state(parent_state_id);
        SearchNode parent_node = search_space.get_node(parent_state);
        EstimationInfo estimation_info;
        search_space.set_estimation_info_based_on_edge(estimation_info, parent_node, curr_node);
        // BeautyEstimator *estimator_ptr = get_estimator(estimation_info, 
        //                                                 get_adjusted_cost(op),
        //                                                 factor_first,
        //                                                 factor_second,
        //                                                 factor_third);
        BeautyHashEstimator *estimator_ptr = get_estimator(estimation_info, 
                                                            get_adjusted_cost(op),
                                                            seed);
        while (estimator_ptr != NULL) {
            statistics.inc_estimations();

            switch (estimation_info.rank)
                {
                case 2:
                    statistics.inc_l2_estimations();
                    break;
                case 3:
                    statistics.inc_l3_estimations();
                    break;
                default:
                    break;
                }


            int prev_min_cost = estimation_info.min_cost;
            estimator_ptr->estimate(estimation_info.min_cost);
            lower_bound += estimation_info.min_cost - prev_min_cost;
            // estimator_ptr = get_estimator(estimation_info, 
            //                                 get_adjusted_cost(op),
            //                                 factor_first,
            //                                 factor_second,
            //                                 factor_third);
            estimator_ptr = get_estimator(estimation_info, 
                                            get_adjusted_cost(op),
                                            seed);
        }
        curr_state = state_registry.lookup_state(parent_state_id);
    }

    // update optimality and upper bound
    l_high = lower_bound;
    if ((lower_bound > l_alt) and (l_high > l_low)) {
        opt = false;
    }
}

void Beauty::reward_progress() {
    // Boost the "preferred operator" open lists somewhat whenever
    // one of the heuristics finds a state with a new best h value.
    open_list->boost_preferred();
}

void Beauty::dump_search_space() const {
    search_space.dump(task_proxy);
}

void Beauty::start_f_value_statistics(EvaluationContext &eval_context) {
    if (f_evaluator) {
        int f_value = eval_context.get_evaluator_value(f_evaluator.get());
        statistics.report_f_value_progress(f_value);
    }
}

/* TODO: HACK! This is very inefficient for simply looking up an h value.
   Also, if h values are not saved it would recompute h for each and every state. */
void Beauty::update_f_value_statistics(EvaluationContext &eval_context) {
    if (f_evaluator) {
        int f_value = eval_context.get_evaluator_value(f_evaluator.get());
        statistics.report_f_value_progress(f_value);
    }
}

void add_options_to_parser(OptionParser &parser) {
    SearchEngine::add_pruning_option(parser);
    SearchEngine::add_options_to_parser(parser);
}
}
