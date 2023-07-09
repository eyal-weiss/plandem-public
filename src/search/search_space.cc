#include "search_space.h"

#include "search_node_info.h"
#include "task_proxy.h"

#include "task_utils/task_properties.h"
#include "utils/logging.h"

#include <cassert>

using namespace std;

SearchNode::SearchNode(const State &state, SearchNodeInfo &info)
    : state(state), info(info) {
    assert(state.get_id() != StateID::no_state);
}

const State &SearchNode::get_state() const {
    return state;
}

bool SearchNode::is_open() const {
    return info.status == SearchNodeInfo::OPEN;
}

bool SearchNode::is_closed() const {
    return info.status == SearchNodeInfo::CLOSED;
}

bool SearchNode::is_dead_end() const {
    return info.status == SearchNodeInfo::DEAD_END;
}

bool SearchNode::is_new() const {
    return info.status == SearchNodeInfo::NEW;
}

int SearchNode::get_g() const {
    assert(info.g >= 0);
    return info.g;
}

int SearchNode::get_real_g() const {
    return info.real_g;
}

int SearchNode::get_min_g() const {
    return info.curr_estimation.min_g;
}

int SearchNode::get_max_g() const {
    return info.curr_estimation.max_g;
}

int SearchNode::get_min_cost() const {
    return info.curr_estimation.min_cost;
}

int SearchNode::get_max_cost() const {
    return info.curr_estimation.max_cost;
}

int SearchNode::get_rank() const {
    return info.curr_estimation.rank;
}

bool SearchNode::get_try_next() const{
    return info.curr_estimation.try_next;
}

StateID SearchNode::get_parent_state_id() const{
    return info.parent_state_id;
}

OperatorID SearchNode::get_creating_operator() const {
    return info.creating_operator;
}

void SearchNode::open_initial() {
    assert(info.status == SearchNodeInfo::NEW);
    info.status = SearchNodeInfo::OPEN;
    info.g = 0;
    info.real_g = 0;
    info.parent_state_id = StateID::no_state;
    info.creating_operator = OperatorID::no_operator;
    info.curr_estimation.max_g = 0;
    info.curr_estimation.max_cost = 0;
    info.curr_estimation.try_next = false; 
}

void SearchNode::open(const SearchNode &parent_node,
                      const OperatorProxy &parent_op,
                      int adjusted_cost,
                      EstimationInfo *estimated_g) {
    assert(info.status == SearchNodeInfo::NEW);
    info.status = SearchNodeInfo::OPEN;
    info.g = parent_node.info.g + adjusted_cost;
    info.real_g = parent_node.info.real_g + parent_op.get_cost();
    if (estimated_g != nullptr) {
        info.curr_estimation.min_g = estimated_g->min_g;
        info.curr_estimation.max_g = estimated_g->max_g;
        info.curr_estimation.min_cost = estimated_g->min_cost;
        info.curr_estimation.max_cost = estimated_g->max_cost;
        info.curr_estimation.rank = estimated_g->rank;
        info.curr_estimation.try_next = estimated_g->try_next;
    } 
    info.parent_state_id = parent_node.get_state().get_id();
    info.creating_operator = OperatorID(parent_op.get_id());
}

void SearchNode::reopen(const SearchNode &parent_node,
                        const OperatorProxy &parent_op,
                        int adjusted_cost,
                        EstimationInfo *estimated_g) {
    assert(info.status == SearchNodeInfo::OPEN ||
           info.status == SearchNodeInfo::CLOSED);

    // The latter possibility is for inconsistent heuristics, which
    // may require reopening closed nodes.
    info.status = SearchNodeInfo::OPEN;
    info.g = parent_node.info.g + adjusted_cost;
    info.real_g = parent_node.info.real_g + parent_op.get_cost();
    if (estimated_g != nullptr) {
        info.curr_estimation.min_g = estimated_g->min_g;
        info.curr_estimation.max_g = estimated_g->max_g;
        info.curr_estimation.min_cost = estimated_g->min_cost;
        info.curr_estimation.max_cost = estimated_g->max_cost;
        info.curr_estimation.rank = estimated_g->rank;
        info.curr_estimation.try_next = estimated_g->try_next;
    }
    info.parent_state_id = parent_node.get_state().get_id();
    info.creating_operator = OperatorID(parent_op.get_id());
}

// like reopen, except doesn't change status
void SearchNode::update_parent(const SearchNode &parent_node,
                               const OperatorProxy &parent_op,
                               int adjusted_cost,
                               EstimationInfo *estimated_g) {
    assert(info.status == SearchNodeInfo::OPEN ||
           info.status == SearchNodeInfo::CLOSED);
    // The latter possibility is for inconsistent heuristics, which
    // may require reopening closed nodes.
    info.g = parent_node.info.g + adjusted_cost;
    info.real_g = parent_node.info.real_g + parent_op.get_cost();
    if (estimated_g != nullptr) {
        info.curr_estimation.min_g = estimated_g->min_g;
        info.curr_estimation.max_g = estimated_g->max_g;
        info.curr_estimation.min_cost = estimated_g->min_cost;
        info.curr_estimation.max_cost = estimated_g->max_cost;
        info.curr_estimation.rank = estimated_g->rank;
        info.curr_estimation.try_next = estimated_g->try_next;
    }
    info.parent_state_id = parent_node.get_state().get_id();
    info.creating_operator = OperatorID(parent_op.get_id());
}

bool SearchNode::is_same_edge(const SearchNode &parent_node,
                              const OperatorProxy &parent_op) const{
    return (info.parent_state_id == parent_node.get_state().get_id()
            and info.creating_operator == OperatorID(parent_op.get_id()));
}

void SearchNode::close() {
    assert(info.status == SearchNodeInfo::OPEN);
    info.status = SearchNodeInfo::CLOSED;
}

void SearchNode::mark_as_dead_end() {
    info.status = SearchNodeInfo::DEAD_END;
}

void SearchNode::dump(const TaskProxy &task_proxy) const {
    utils::g_log << state.get_id() << ": ";
    task_properties::dump_fdr(state);
    if (info.creating_operator != OperatorID::no_operator) {
        OperatorsProxy operators = task_proxy.get_operators();
        OperatorProxy op = operators[info.creating_operator.get_index()];
        utils::g_log << " created by " << op.get_name()
                     << " from " << info.parent_state_id << endl;
    } else {
        utils::g_log << " no parent" << endl;
    }
}

SearchSpace::SearchSpace(StateRegistry &state_registry)
    : state_registry(state_registry) {
}

SearchNode SearchSpace::get_node(const State &state) {
    return SearchNode(state, search_node_infos[state]);
}

void SearchSpace::set_estimation_info_based_on_edge(EstimationInfo &estimation_info,
                                                    const SearchNode &parent_node,
                                                    const SearchNode &curr_node) {
    estimation_info.try_next = curr_node.get_try_next();
    estimation_info.rank = curr_node.get_rank();
    estimation_info.min_cost = curr_node.get_min_cost();
    estimation_info.max_cost = curr_node.get_max_cost();
    estimation_info.min_g = parent_node.get_min_g() + estimation_info.min_cost;
    estimation_info.max_g = parent_node.get_max_g() + estimation_info.max_cost;
}

void SearchSpace::trace_path(const State &goal_state,
                             vector<OperatorID> &path) const {
    State current_state = goal_state;
    assert(current_state.get_registry() == &state_registry);
    assert(path.empty());
    for (;;) {
        const SearchNodeInfo &info = search_node_infos[current_state];
        if (info.creating_operator == OperatorID::no_operator) {
            assert(info.parent_state_id == StateID::no_state);
            break;
        }
        path.push_back(info.creating_operator);
        current_state = state_registry.lookup_state(info.parent_state_id);
    }
    reverse(path.begin(), path.end());
}

void SearchSpace::dump(const TaskProxy &task_proxy) const {
    OperatorsProxy operators = task_proxy.get_operators();
    for (StateID id : state_registry) {
        /* The body duplicates SearchNode::dump() but we cannot create
           a search node without discarding the const qualifier. */
        State state = state_registry.lookup_state(id);
        const SearchNodeInfo &node_info = search_node_infos[state];
        utils::g_log << id << ": ";
        task_properties::dump_fdr(state);
        if (node_info.creating_operator != OperatorID::no_operator &&
            node_info.parent_state_id != StateID::no_state) {
            OperatorProxy op = operators[node_info.creating_operator.get_index()];
            utils::g_log << " created by " << op.get_name()
                         << " from " << node_info.parent_state_id << endl;
        } else {
            utils::g_log << "has no parent" << endl;
        }
    }
}

void SearchSpace::print_statistics() const {
    state_registry.print_statistics();
}
