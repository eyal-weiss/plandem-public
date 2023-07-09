#ifndef SEARCH_SPACE_H
#define SEARCH_SPACE_H

#include "operator_cost.h"
#include "per_state_information.h"
#include "search_node_info.h"

#include <vector>

class OperatorProxy;
class State;
class TaskProxy;


class SearchNode {
    State state;
    SearchNodeInfo &info;
public:
    SearchNode(const State &state, SearchNodeInfo &info);

    const State &get_state() const;

    bool is_new() const;
    bool is_open() const;
    bool is_closed() const;
    bool is_dead_end() const;
    bool get_try_next() const;
    bool is_same_edge(const SearchNode &parent_node,
                      const OperatorProxy &parent_op) const;

    int get_g() const;
    int get_real_g() const;
    int get_min_g() const;
    int get_max_g() const;
    int get_min_cost() const;
    int get_max_cost() const;
    int get_rank() const;
    
    StateID get_parent_state_id() const;
    OperatorID get_creating_operator() const;

    void open_initial();
    void open(const SearchNode &parent_node,
              const OperatorProxy &parent_op,
              int adjusted_cost,
              EstimationInfo *estimated_g = nullptr);
    void reopen(const SearchNode &parent_node,
                const OperatorProxy &parent_op,
                int adjusted_cost,
                EstimationInfo *estimated_g = nullptr);
    void update_parent(const SearchNode &parent_node,
                       const OperatorProxy &parent_op,
                       int adjusted_cost,
                       EstimationInfo *estimated_g = nullptr);
    void close();
    void mark_as_dead_end();

    void dump(const TaskProxy &task_proxy) const;
};


class SearchSpace {
    PerStateInformation<SearchNodeInfo> search_node_infos;

    StateRegistry &state_registry;
public:
    explicit SearchSpace(StateRegistry &state_registry);

    SearchNode get_node(const State &state);
    void trace_path(const State &goal_state,
                    std::vector<OperatorID> &path) const;

    void dump(const TaskProxy &task_proxy) const;
    void print_statistics() const;
    
    void set_estimation_info_based_on_edge(EstimationInfo &estimation_info,
                                           const SearchNode &parent_node,
                                           const SearchNode &curr_node);
};

#endif
