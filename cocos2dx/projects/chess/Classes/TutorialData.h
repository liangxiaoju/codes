#ifndef __TUTORIALDATA_H__
#define __TUTORIALDATA_H__

#include "cocos2d.h"
#include "json/rapidjson.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "sqlite/sqlite3.h"

USING_NS_CC;

class TutorialData
{
public:
    struct TutorialCategory {
        int id;
        int pid;
        int status;
        int type;
        int progress;
        std::string name;
        std::string desc;
        std::string content;
    };

    struct TutorialNode {
        int id;
        int pid;
        int status;
        int type;
        int progress;
        std::string name;
        std::string desc;
        std::string content;

        struct MoveItem {
            std::string src;
            std::string dst;
            std::string comment;
            std::vector<int> sub;
        };

        typedef std::vector<MoveItem> MoveList;
        typedef std::vector<MoveList> SubMoveList;

        struct Data {
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

    static TutorialData *getInstance()
    {
        if (s_tutorialData == nullptr) {
            std::string src = FileUtils::getInstance()
                ->fullPathForFilename("tutorial.db");

            std::string dst = FileUtils::getInstance()
                ->getWritablePath().append("tutorial.db");

            if (!FileUtils::getInstance()->isFileExist(dst)) {
                Data data = FileUtils::getInstance()->getDataFromFile(src);
                if (!FileUtils::getInstance()->writeDataToFile(data, dst))
                    log("Failed to copy %s to %s", src.c_str(), dst.c_str());
            }

            log("db path: %s", dst.c_str());
            s_tutorialData = new TutorialData(dst);
        }
        return s_tutorialData;
    }

    void queryTutorialCategory(int pid, std::vector<TutorialCategory> &vector);
    void queryTutorialNode(int pid, std::vector<TutorialNode> &vector);
    TutorialNode getTutorialNode(int id);

    TutorialData(std::string fullpath);
    ~TutorialData();

private:
    sqlite3 *_db;

    static TutorialData *s_tutorialData;
};

#endif
