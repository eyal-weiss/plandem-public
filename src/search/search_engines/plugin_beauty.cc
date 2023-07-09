#include "beauty.h"
#include "search_common.h"

#include "../option_parser.h"
#include "../plugin.h"

using namespace std;

namespace plugin_beauty {
static shared_ptr<SearchEngine> _parse(OptionParser &parser) {
    parser.document_synopsis("Beauty edge-cost estimation best-first search", "");

    parser.add_option<shared_ptr<OpenListFactory>>("open", "open list");
    parser.add_option<bool>("reopen_closed",
                            "reopen closed nodes", "false");
    parser.add_option<shared_ptr<Evaluator>>(
        "f_eval",
        "set evaluator for jump statistics. "
        "(Optional; if no evaluator is used, jump statistics will not be displayed.)",
        OptionParser::NONE);
    parser.add_option<int>(
        "seed",
        "used for changing the hash function of the estimators, default value set to 0",
        "0");
    parser.add_option<int>(
        "factor_first",
        "multiplicative factor of second estimator, default value set to 1",
        "1");
    parser.add_option<int>(
        "factor_second",
        "multiplicative factor of second estimator, default value set to 3",
        "3");
    parser.add_option<int>(
        "factor_third",
        "multiplicative factor of third estimator, default value set to 4",
        "4");
    parser.add_list_option<shared_ptr<Evaluator>>(
        "preferred",
        "use preferred operators of these evaluators", "[]");

    beauty::add_options_to_parser(parser);
    Options opts = parser.parse();

    shared_ptr<beauty::Beauty> engine;
    if (!parser.dry_run()) {
        engine = make_shared<beauty::Beauty>(opts);
    }

    return engine;
}

static Plugin<SearchEngine> _plugin("beauty", _parse);
}
