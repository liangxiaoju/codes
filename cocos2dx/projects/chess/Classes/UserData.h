#ifndef __USERDATA_H__
#define __USERDATA_H__

#include "cocos2d.h"
#include "sqlite/sqlite3.h"

USING_NS_CC;

class UserData {
public:

	struct SaveElement {
		int id;
		std::string date;
		int roleWhite;
		int roleBlack;
		int level;
		std::string white;
		std::string black;
		std::string fen;
	};

	struct RecordElement {
		int id;
		std::string date;
		int roleWhite;
		int roleBlack;
		int level;
		std::string white;
		std::string black;
		std::string win;
		std::string fen;
	};

	static UserData *getInstance()
	{
		if (s_userData == nullptr) {
			std::string fullpath = FileUtils::getInstance()
				->getWritablePath().append("userdata.db");
			log("db path: %s", fullpath.c_str());
			s_userData = new UserData(fullpath);
		}
		return s_userData;
	}

	void insertSaveElement(SaveElement element);
	void insertRecordElement(RecordElement element);
	void deleteSaveElement(int id);
	void deleteRecordElement(int id);

	void querySaveTbl(std::vector<SaveElement> &vector);
	void queryRecordTbl(std::vector<RecordElement> &vector);

	void clearRecordTbl();
	void clearSaveTbl();
    void clearUserInfoTbl();

    int getIntegerForKey(std::string key, int defaultValue);
    void setIntegerForKey(std::string key, int value);
    std::string getStringForKey(std::string key, std::string defaultValue);
    void setStringForKey(std::string key, std::string value);

	UserData(std::string fullpath);
	virtual ~UserData();

private:
	sqlite3 *_db;

	void createTable();

	static UserData *s_userData;
};

#endif
