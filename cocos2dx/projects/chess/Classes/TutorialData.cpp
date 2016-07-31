#include "TutorialData.h"

TutorialData *TutorialData::s_tutorialData = nullptr;

void parseJson(TutorialData::TutorialNode::Data &data, std::string json)
{
    rapidjson::Document doc;
    doc.Parse<0>(json.c_str());

    if (doc.HasMember("id"))
        data.id = atoi(doc["id"].GetString());
    if (doc.HasMember("SubTitle"))
        data.subtitle = doc["SubTitle"].GetString();
    if (doc.HasMember("fen"))
        data.fen = doc["fen"].GetString();
    if (doc.HasMember("pay"))
        data.pay = atoi(doc["pay"].GetString());
    if (doc.HasMember("prompt"))
        data.prompt = atoi(doc["prompt"].GetString());
    if (doc.HasMember("sort"))
        data.sort = atoi(doc["sort"].GetString());

    auto handleMoveList = [](TutorialData::TutorialNode::MoveList &movelist,
                             rapidjson::Value &array) {
        for (unsigned i=0; i < array.Size(); i++) {
            TutorialData::TutorialNode::MoveItem item;
            if (array[i].HasMember("src") && array[i].HasMember("dst")) {
                item.src = array[i]["src"].GetString();
                item.dst = array[i]["dst"].GetString();
            }
            if (array[i].HasMember("comment"))
                item.comment = array[i]["comment"].GetString();
            if (array[i].HasMember("sub")) {
                for (unsigned j=0; j < array[i]["sub"].Size(); j++) {
                    int n = atoi(array[i]["sub"][j].GetString());
                    item.sub.push_back(n);
                }
            }

            movelist.push_back(item);
        }
    };

    if (doc.HasMember("move")) {
        rapidjson::Value &move = doc["move"];
        if (move.HasMember("movelist")) {
            handleMoveList(data.movelist, move["movelist"]);
        }
        if (move.HasMember("submovelist")) {
            int size = move["submovelist"].MemberCount();
            data.submovelist.resize(size+1);
            for (int i=1; i <= size; i++) {
                char key[8];
                snprintf(key, sizeof(key), "%d", i);
                if (move["submovelist"].HasMember(key)) {
                    handleMoveList(data.submovelist[i], move["submovelist"][key]);
                }
            }
        }
    }

}

void TutorialData::queryTutorialCategory(int pid, std::vector<TutorialCategory> &vector)
{
    auto callback = [](void *v, int argc, char **argv, char **colName)->int{
        std::vector<TutorialCategory> *vector = (std::vector<TutorialCategory>*)v;

        if (argc != 8) {
            log("argc!=8");
            return SQLITE_ERROR;
        }

        TutorialCategory e;
        e.id = atoi(argv[0]);
        e.pid = atoi(argv[1]);
        e.status = atoi(argv[2]);
        e.type = atoi(argv[3]);
        e.progress = atoi(argv[4]);
        e.name = argv[5];
        e.desc = argv[6];
        e.content= "";

        vector->push_back(e);
        return 0;
    };

    const char *sql = sqlite3_mprintf("SELECT * from Category WHERE pid='%d'", pid);
    int ok = sqlite3_exec(_db, sql, callback, &vector, 0);

    if (ok != SQLITE_OK)
        log("Error in queryTutorialcategory()");
}

void TutorialData::queryTutorialNode(int pid, std::vector<TutorialNode> &vector)
{
    auto callback = [](void *v, int argc, char **argv, char **colName)->int{
        std::vector<TutorialNode> *vector = (std::vector<TutorialNode>*)v;

        if (argc != 8) {
            log("argc!=8");
            return SQLITE_ERROR;
        }

        TutorialNode e;
        e.id = atoi(argv[0]);
        e.pid = atoi(argv[1]);
        e.status = atoi(argv[2]);
        e.type = atoi(argv[3]);
        e.progress = atoi(argv[4]);
        e.name = argv[5];
        e.desc = argv[6];
        e.content= argv[7];

        //parseJson(e.data, e.content);

        vector->push_back(e);
        return 0;
    };

    const char *sql = sqlite3_mprintf("SELECT * from Node WHERE pid='%d'", pid);
    int ok = sqlite3_exec(_db, sql, callback, &vector, 0);

    if (ok != SQLITE_OK)
        log("Error in queryTutorialNode()");
}

TutorialData::TutorialNode TutorialData::getTutorialNode(int id)
{
    auto callback = [](void *v, int argc, char **argv, char **colName)->int{
        TutorialNode *node = (TutorialNode*)v;

        if (argc != 8) {
            log("argc!=8");
            return SQLITE_ERROR;
        }

        node->id = atoi(argv[0]);
        node->pid = atoi(argv[1]);
        node->status = atoi(argv[2]);
        node->type = atoi(argv[3]);
        node->progress = atoi(argv[4]);
        node->name = argv[5];
        node->desc = argv[6];
        node->content= argv[7];

        //parseJson(node->data, node->content);

        return 0;
    };

    TutorialNode node;
    const char *sql = sqlite3_mprintf("SELECT * from Node WHERE id='%d'", id);
    int ok = sqlite3_exec(_db, sql, callback, &node, 0);

    if (ok != SQLITE_OK)
        log("Error in getTutorialNode()");

    return node;
}

TutorialData::TutorialData(std::string fullpath)
{
    int ret = 0;

    if (fullpath.empty())
        ret = sqlite3_open(":memory:",&_db);
    else
        ret = sqlite3_open(fullpath.c_str(), &_db);

    if( ret != SQLITE_OK ) {
        log("Error initializing DB");
    }
}

TutorialData::~TutorialData()
{
    sqlite3_close(_db);
}
