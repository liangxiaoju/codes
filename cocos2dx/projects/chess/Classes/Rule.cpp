#include "Rule.h"
#include "Utils.h"

Rule *Rule::s_rule = nullptr;

Rule *Rule::getInstance() {
    if (!s_rule) {
        s_rule= new Rule();
        if (s_rule) {
            s_rule->init();
        }
    }
    return s_rule;
}

bool Rule::init()
{
	return true;
}

int Rule::check(std::string fen)
{
	int ret = 0;
	size_t i = fen.find(" moves ");
	std::string f = fen.substr(0, i);
	std::vector<std::string> mvs = Utils::splitString(fen.substr(i+7), ' ');

	log("%s", fen.c_str());

    _pos.FromFen(f.c_str());

	for (auto mv : mvs) {
		int imv = _pos.toArrayMv(mv.c_str());
		if (imv == 0) {
			log("imv == 0");
			ret = -1;
			break;
		}
		if (!_pos.LegalMove(imv)) {
			log("!legalMove");
			ret = -1;
			break;
		}
		if (!_pos.MakeMove(imv)) {
			log("!Will be checked");
			ret = -2;
			break;
		}
		if ((ret = _pos.RepStatus(3)) > 0) {
			log("!repeatMove %d", ret);
			log("RepValue=%d", _pos.RepValue(ret));
			ret = -3;
			break;
		}
	}
	log("evaluate=%d", _pos.Evaluate());

	return ret;
}

bool Rule::isMate(std::string fen)
{
    _pos.FromFen(fen.c_str());
	return !!_pos.IsMate();
}
