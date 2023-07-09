#include "synchronic_estimation_search.h"
#include "search_common.h"

#include "../option_parser.h"
#include "../plugin.h"

using namespace std;

namespace plugin_synchronic_estimation {
static shared_ptr<SearchEngine> _parse(OptionParser &parser) {
    parser.document_synopsis("Synchronic edge-cost estimation best-first search", "");

    parser.add_option<shared_ptr<OpenListFactory>>("open", "open list");
    parser.add_option<bool>("reopen_closed",
                            "reopen closed nodes", "false");
    parser.add_option<shared_ptr<Evaluator>>(
        "f_eval",
        "set evaluator for jump statistics. "
        "(Optional; if no evaluator is used, jump statistics will not be displayed.)",
        OptionParser::NONE);
    parser.add_option<double>(
        "epsilon",
        "sub-optimality bound, default value set to 1",
        "1");
    parser.add_option<int>(
        "edge_estimation_avg_time",
        "estimation time in microseconds, default value set to 0",
        "0");
    parser.add_option<int>(
        "edge_estimation_time_interval",
        "interval of randomness time in microseconds, default value set to 0",
        "0");
    parser.add_option<double>(
        "first_estimator_probability",
        "probability for first estimator creation, default value set to 0.1",
        "0.1");
    parser.add_option<double>(
        "second_estimator_probability",
        "probability for second estimator creation, default value set to 1",
        "1");
    parser.add_option<double>(
        "third_estimator_probability",
        "probability for third estimator creation, default value set to 1",
        "1");
    parser.add_option<bool>(
        "end_of_search_estimations",
        "perform end-of-search asynchronous estimations, default value set to false",
        "false");
    parser.add_list_option<shared_ptr<Evaluator>>(
        "preferred",
        "use preferred operators of these evaluators", "[]");

    synchronic_estimation_search::add_options_to_parser(parser);
    Options opts = parser.parse();

    shared_ptr<synchronic_estimation_search::SynchronicEstimationSearch> engine;
    if (!parser.dry_run()) {
        engine = make_shared<synchronic_estimation_search::SynchronicEstimationSearch>(opts);
    }

    return engine;
}

static Plugin<SearchEngine> _plugin("synchronic", _parse);
}
