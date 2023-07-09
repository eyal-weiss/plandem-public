#include "estimated_g_evaluator.h"

#include "../evaluation_context.h"
#include "../evaluation_result.h"
#include "../option_parser.h"
#include "../plugin.h"

using namespace std;

namespace estimated_g_evaluator {
EvaluationResult EstimatedGEvaluator::compute_result(EvaluationContext &eval_context) {
    EvaluationResult result;
    result.set_evaluator_value(eval_context.get_estimation_info().min_g);
    return result;
}

static shared_ptr<Evaluator> _parse(OptionParser &parser) {
    parser.document_synopsis(
        "Estimated g-value evaluator",
        "Returns the estimated g-value (path cost) of the search node.");
    parser.parse();
    if (parser.dry_run())
        return nullptr;
    else
        return make_shared<EstimatedGEvaluator>();
}

static Plugin<Evaluator> _plugin("estimated_g", _parse, "evaluators_basic");
}
