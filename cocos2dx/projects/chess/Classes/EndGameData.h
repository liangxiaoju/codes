#ifndef __ENDGAMEDATA_H__
#define __ENDGAMEDATA_H__

#include "cocos2d.h"
#include "json/rapidjson.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "sqlite/sqlite3.h"

USING_NS_CC;

class EndGameData
{
public:
	struct EndGameClass {
		int id;
		int tid;
		int isNeedPay;
		int progress;
		int subCount;
		std::string json;

		struct {
			std::string title;
			int hp;
			int sort;
			std::string desc;
			int status;
			int tid;
			int prompt;
			int revival;
			int repentance;
			int version;
		} data;
	};

	struct EndGameItem {
		int id;
		int tid;
		int sort;
		std::string json;

		struct MoveItem {
			int src;
			int dst;
			std::string comment;
			std::vector<int> sub;
		};

		typedef std::vector<MoveItem> MoveList;
		typedef std::vector<MoveList> SubMoveList;

		struct {
			int id;
			std::string subtitle;
			std::string fen;
			int pay;
			int prompt;
			int sort;
			MoveList movelist;
			SubMoveList submovelist;
		} data;
	};

	static EndGameData *getInstance()
	{
		if (s_endGameData == nullptr) {
			std::string src = FileUtils::getInstance()
				->fullPathForFilename("endgate.db");

			std::string dst = FileUtils::getInstance()
				->getWritablePath().append("endgate.db");

			if (!FileUtils::getInstance()->isFileExist(dst)) {
				Data data = FileUtils::getInstance()->getDataFromFile(src);
				if (!FileUtils::getInstance()->writeDataToFile(data, dst))
					log("Failed to copy %s to %s", src.c_str(), dst.c_str());
			}

			log("db path: %s", dst.c_str());
			s_endGameData= new EndGameData(dst);
		}
		return s_endGameData;
	}

	void queryEndGameClass(std::vector<EndGameClass> &vector);
	void queryEndGameItem(int tid, std::vector<EndGameItem> &vector);
	void updateEndGameClass(EndGameClass cls);

	EndGameData(std::string fullpath);
	~EndGameData();

private:
	sqlite3 *_db;

	std::vector<EndGameClass> _endGameClass;
	std::vector<EndGameItem> _endGameItem;

	static EndGameData *s_endGameData;
};

#endif
