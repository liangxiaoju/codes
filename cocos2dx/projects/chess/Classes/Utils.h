#ifndef __UTILS_H__
#define __UTILS_H__

#include "cocos2d.h"
#include "Localization.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxHelper.h"
#else
#include <iconv.h>
#endif

USING_NS_CC;

#define EVENT_GAMEOVER "EV_GAMEOVER"
#define EVENT_WHITE_START "EV_WHITE_START"
#define EVENT_BLACK_START "EV_BLACK_START"

#define EVENT_REGRET "EV_REGRET"
#define EVENT_RESIGN "EV_RESIGN"
#define EVENT_DRAW "EV_DRAW"

#define EVENT_RESET "EV_RESET"
#define EVENT_SWITCH "EV_SWITCH"

#define EVENT_SAVE "EV_SAVE"

#define EVENT_TIP "EV_TIP"

#define EVENT_BACK "EV_BACK"
#define EVENT_LEVEL "EV_LEVEL"
#define EVENT_LEVEL_CHANGE "EV_LEVEL_CHANGE"
#define EVENT_SETTING "EV_SETTING"

#define EVENT_NEXT "EV_NEXT"

#define EVENT_SUSPEND "EV_SUSPEND"
#define EVENT_RESUME "EV_RESUME"

//#define EVENT_REQUEST_DENY "EV_REQUEST_DENY"
//#define EVENT_REQUEST_ACCEPT "EV_REQUEST_ACCEPT"
#define EVENT_REQUEST_REPLY "EV_REQUEST_REPLY"

namespace std {
	template <>
	struct hash<cocos2d::Vec2>
	{
		std::size_t operator()(const cocos2d::Vec2& k) const
		{
			using std::size_t;
			using std::hash;
			return hash<float>()(k.x*10+k.y);
		}
	};
}

class Utils
{
public:
	template<typename T>
	static std::string toString(T arg)
	{
		std::stringstream ss;
		ss << arg;
		return ss.str();
	}
	static std::string toUcciMove(Vec2 src, Vec2 dst)
	{
		return toString(char('a'+src.x)) + toString(src.y) +
			toString(char('a'+dst.x)) + toString(dst.y);
	}

	static std::vector<Vec2> toVecMove(std::string mv)
	{
		std::vector<Vec2> vecMv;
		vecMv.push_back(Vec2(mv[0]-'a', mv[1]-'0'));
		vecMv.push_back(Vec2(mv[2]-'a', mv[3]-'0'));
		return vecMv;
	}
	static std::vector<std::string> splitString(const std::string &s, char delim)
	{
		std::vector<std::string> elems;
		std::stringstream ss(s);
		std::string item;

		while (std::getline(ss, item, delim)) {
			elems.push_back(item);
		}

		return elems;
	}

    static std::string convertGBKToUTF8(std::string gbkstr)
    {
        auto inSize = gbkstr.size();
        auto outSize = inSize * 4;
        auto outBuf = new char[outSize];

        memset(outBuf, 0, outSize);

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
        conversionEncodingJNI(gbkstr.c_str(), inSize, "GB2312", outBuf, "UTF-8");
#else
        iconv_t conv = iconv_open("utf-8", "gb2312");
        if (conv == (iconv_t)-1) {
            log("conversion from gb2312 to utf8 not available");
        } else {
            const char *pin = gbkstr.c_str();
            char *pout = outBuf;
            iconv(conv, (char **)(&pin), &inSize, &pout, &outSize);
            iconv_close(conv);
        }
#endif

        std::string utf8str(outBuf);

        delete [] outBuf;

        return utf8str;
    }

    static std::string convertMoveToString(std::string mv, char symbol)
    {
        auto v = toVecMove(mv);
        auto src = v[0];
        auto dst = v[1];

        if ((src.x < 0 || src.x > 8 || src.y < 0 || src.y > 9) ||
            (dst.x < 0 || dst.x > 8 || dst.y < 0 || dst.y > 9))
                return "";

        std::map<char, std::string> symbolMap;
        symbolMap['r'] = TR("rook");
        symbolMap['n'] = TR("knight");
        symbolMap['b'] = TR("bishop");
        symbolMap['a'] = TR("advisor");
        symbolMap['k'] = TR("king");
        symbolMap['c'] = TR("cannon");
        symbolMap['p'] = TR("pawn");
        symbolMap['R'] = TR("rook");
        symbolMap['N'] = TR("KNIGHT");
        symbolMap['B'] = TR("BISHOP");
        symbolMap['A'] = TR("ADVISOR");
        symbolMap['K'] = TR("KING");
        symbolMap['C'] = TR("CANNON");
        symbolMap['P'] = TR("PAWN");

        std::string name = symbolMap.at(symbol);

        std::vector<std::string> numRedMap = {
            TR("I"),TR("II"),TR("III"),TR("IV"),TR("V"),TR("VI"),TR("VII"),TR("VIII"),TR("IX")};
        std::vector<std::string> numBlackMap = {"1","2","3","4","5","6","7","8","9"};

        std::vector<std::string> numMap;
        if (std::string("RNBAKCP").find(symbol) != std::string::npos) {
            numMap = numRedMap;
            src.x = 8 - src.x;
            dst.x = 8 - dst.x;
        } else {
            numMap = numBlackMap;
            src.y = 9 - src.y;
            dst.y = 9 - dst.y;
        }

        std::string action = "";
        if (src.y == dst.y) {
            action = TR("horizontal");
        } else if (src.y < dst.y) {
            action = TR("forward");
        } else {
            action = TR("backward");
        }

        std::string sname = numMap[src.x];

        std::string dname;
        if (std::string("rRcCpPkK").find(symbol) != std::string::npos) {
            if (src.y == dst.y)
                dname = numMap[dst.x];
            else
                dname = numMap[std::abs(dst.y-src.y) - 1];
        } else {
            dname = numMap[dst.x];
        }

        return name+sname+action+dname;
    }
};

#endif
