#ifndef EVALUATORS_ESTIMATED_G_EVALUATOR_H
#define EVALUATORS_ESTIMATED_G_EVALUATOR_H

#include "../evaluator.h"

namespace estimated_g_evaluator {
class EstimatedGEvaluator : public Evaluator {
public:
    EstimatedGEvaluator() = default;
    virtual ~EstimatedGEvaluator() override = default;

    virtual EvaluationResult compute_result(
        EvaluationContext &eval_context) override;

    virtual void get_path_dependent_evaluators(std::set<Evaluator *> &) override {}
};
}

#endif
