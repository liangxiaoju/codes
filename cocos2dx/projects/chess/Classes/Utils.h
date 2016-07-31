#ifndef __UTILS_H__
#define __UTILS_H__

#include "cocos2d.h"

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

#define EVENT_NEXT "EV_NEXT"

#define EVENT_SUSPEND "EV_SUSPEND"
#define EVENT_RESUME "EV_RESUME"

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
};

#endif
