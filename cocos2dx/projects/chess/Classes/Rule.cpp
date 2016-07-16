#include "Rule.h"
#include "Utils.h"

using namespace SimplePosition;

Rule *Rule::s_rule = nullptr;

Rule *Rule::getInstance() {
    if (!s_rule) {
        s_rule= new(std::nothrow) Rule();
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

	//log("%s", fen.c_str());

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

bool Rule::isChecked(std::string fen)
{
    _pos.FromFen(fen.c_str());
    return !!_pos.InCheck();
}

bool Rule::isCaptured(std::string fen)
{
    _pos.FromFen(fen.c_str());
    return !!_pos.Captured();
}

std::vector<std::string> Rule::generateMoves(std::string fen, Vec2 src)
{
	int mvs[MAX_GEN_MOVES];
	int sqSrc = COORD_XY(src.x+FILE_LEFT, 9-src.y+RANK_TOP);
	int num, i;
	std::vector<std::string> v;

	_pos.FromFen(fen.c_str());
	num = _pos.GenerateMove(sqSrc, mvs);

	for (i = 0; i < num; i++) {
		int m1 = SRC(mvs[i]);
		int m2 = DST(mvs[i]);
		std::string s, d;

		s = Utils::toString(char(FILE_X(m1) - FILE_LEFT + 'a')) +
			Utils::toString(char('9' - (RANK_Y(m1) - RANK_TOP)));
		d = Utils::toString(char(FILE_X(m2) - FILE_LEFT + 'a')) +
			Utils::toString(char('9' - (RANK_Y(m2) - RANK_TOP)));

		v.push_back(s+d);
	}

	return v;
}
