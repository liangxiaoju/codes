#ifndef __ROOMPACKET_H__
#define __ROOMPACKET_H__

#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>

/* notice: find_first_of will match any char in delim */
void inline split(std::string& s, std::vector<std::string>& ret,
		const std::string delim)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim,last);

	while (index != std::string::npos)
	{
		ret.push_back(s.substr(last,index-last));
		last=index+1;
		index=s.find_first_of(delim,last);
	}
	if (index-last>0)
	{
		ret.push_back(s.substr(last,index-last));
	}
}


// 'TYPE:type;FROM:who;TO:who;CONTENT:message;'
#define PREFIX "^^"
#define POSTFIX "$$"
class RoomPacket
{
public:
	static std::vector<RoomPacket> parser(std::string &message)
	{
		size_t index = 0, prefix, postfix;
		std::vector<RoomPacket> packets;

		do {
			prefix = message.find(PREFIX, index);
			if (prefix == std::string::npos)
				break;
			postfix = message.find(POSTFIX, index);
			if (postfix == std::string::npos)
				break;
			index = postfix + strlen(POSTFIX);

			packets.push_back(RoomPacket(message.substr(prefix, index)));

		} while (true);

		return packets;
	}

	RoomPacket()
	{
	}

	std::string& operator[] (const std::string& k)
	{
		return _msgMap[k];
	}

	const std::string operator[] (const std::string& k) const
	{
		auto iter = _msgMap.find(k);
		if (iter == _msgMap.end())
			return "";
		return _msgMap.at(k);
	}

	RoomPacket(std::string message)
	{
		size_t prefix = message.find(PREFIX);
		size_t postfix = message.find(POSTFIX);

		if ((prefix != std::string::npos) &&
				(postfix != std::string::npos)) {
			std::string msg = message.substr(prefix+strlen(PREFIX), postfix);

			std::vector<std::string> sections;
			split(msg, sections, ";");
			for (auto &section : sections) {
				size_t pos = section.find_first_of(":");
				if (pos != std::string::npos) {
					std::string k = section.substr(0, pos);
					std::string v = section.substr(pos+1);
					_msgMap[k] = v;
				}
			}
		}
	}

	std::string toString() const
	{
		std::string s(PREFIX);
		for (auto &kv : _msgMap) {
			s += kv.first + ":" + kv.second + ";";
		}
		s += POSTFIX;
		return s;
	}

	bool operator== (const RoomPacket& p) const
	{
		return toString() == p.toString();
	}

private:
	std::unordered_map<std::string, std::string> _msgMap;
};

#endif
