#ifndef __UTILS_H__
#define __UTILS_H__

#define EVENT_GAMEOVER "EV_GAMEOVER"
#define EVENT_WHITE_GO "EV_WHITE_GO"
#define EVENT_BLACK_GO "EV_BLACK_GO"
#define EVENT_UIPLAYER_GO "EV_UIPLAYER_GO"
#define EVENT_AIPLAYER_GO "EV_AIPLAYER_GO"
#define EVENT_WHITE_WIN "EV_WHITE_WIN"
#define EVENT_BLACK_WIN "EV_BLACK_WIN"
#define EVENT_DRAW "EV_DRAW"

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
};

#endif
