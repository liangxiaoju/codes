#include "EndGameData.h"

EndGameData *EndGameData::s_endGameData = nullptr;

void EndGameData::updateEndGameClass(EndGameClass cls)
{
	char *sql = sqlite3_mprintf(
			"UPDATE Gate SET progress=%d WHERE id=%d",
			cls.progress, cls.id);

	int ok = sqlite3_exec(_db, sql, 0, 0, 0);
	
	if( ok != SQLITE_OK)
		log("Error in updateEndGameClass()");
}

void EndGameData::queryEndGameClass(std::vector<EndGameClass> &vector)
{
	auto callback = [](void *v, int argc, char **argv, char **colName)->int{

		std::vector<EndGameClass> *vector = (std::vector<EndGameClass> *)v;

		if (argc != 6) {
			log("argc!=6");
			return SQLITE_ERROR;
		}

		EndGameClass e;
		e.id = atoi(argv[0]);
		e.tid = atoi(argv[1]);
		e.isNeedPay = atoi(argv[2]);
		e.progress = atoi(argv[3]);
		e.subCount = atoi(argv[4]);
		e.json = argv[5];

		rapidjson::Document doc;
		doc.Parse<0>(e.json.c_str());

		if (doc.HasMember("title"))
			e.data.title = doc["title"].GetString();
		if (doc.HasMember("hp"))
			e.data.hp = atoi(doc["hp"].GetString());
		if (doc.HasMember("sort"))
			e.data.sort = atoi(doc["sort"].GetString());
		if (doc.HasMember("desc"))
			e.data.desc = doc["desc"].GetString();
		if (doc.HasMember("status"))
			e.data.status = atoi(doc["status"].GetString());
		if (doc.HasMember("tid"))
			e.data.tid = atoi(doc["tid"].GetString());
		if (doc.HasMember("prompt"))
			e.data.prompt = atoi(doc["prompt"].GetString());
		if (doc.HasMember("revival"))
			e.data.revival = atoi(doc["revival"].GetString());
		if (doc.HasMember("repentance"))
			e.data.repentance = atoi(doc["repentance"].GetString());
		if (doc.HasMember("version"))
			e.data.version = atoi(doc["version"].GetString());

		vector->push_back(e);

		return 0;
	};

	int ok = sqlite3_exec(_db, "SELECT * from Gate", callback, &vector, 0);

	if( ok != SQLITE_OK)
		log("Error in queryEndGameClass()");
}

void EndGameData::queryEndGameItem(int tid, std::vector<EndGameItem> &vector)
{
	auto callback = [](void *v, int argc, char **argv, char **colName)->int{

		std::vector<EndGameItem> *vector = (std::vector<EndGameItem> *)v;

		if (argc != 4) {
			log("argc!=4");
			return SQLITE_ERROR;
		}

		EndGameItem e;
		e.id = atoi(argv[0]);
		e.tid = atoi(argv[1]);
		e.sort = atoi(argv[2]);
		e.json = argv[3];

		rapidjson::Document doc;
		doc.Parse<0>(e.json.c_str());

		if (doc.HasMember("id"))
			e.data.id = atoi(doc["id"].GetString());
		if (doc.HasMember("SubTitle"))
			e.data.subtitle = doc["SubTitle"].GetString();
		if (doc.HasMember("fen"))
			e.data.fen = doc["fen"].GetString();
		if (doc.HasMember("pay"))
			e.data.pay = atoi(doc["pay"].GetString());
		if (doc.HasMember("prompt"))
			e.data.prompt = atoi(doc["prompt"].GetString());
		if (doc.HasMember("sort"))
			e.data.sort = atoi(doc["sort"].GetString());

		auto handleMoveList = [](EndGameItem::MoveList &movelist,
				rapidjson::Value &array) {
			for (int i=0; i < array.Size(); i++) {
				EndGameItem::MoveItem item;
				if (array[i].HasMember("src") && array[i].HasMember("dst")) {
					item.src = atoi(array[i]["src"].GetString());
					item.dst = atoi(array[i]["dst"].GetString());
				}
				if (array[i].HasMember("comment"))
					item.comment = array[i]["comment"].GetString();
				if (array[i].HasMember("sub")) {
					for (int j=0; j < array[i]["sub"].Size(); j++) {
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
				handleMoveList(e.data.movelist, move["movelist"]);
			}
			if (move.HasMember("submovelist")) {
				int size = move["submovelist"].MemberCount();
				e.data.submovelist.resize(size+1);
				for (int i=1; i <= size; i++) {
					char key[8];
					snprintf(key, sizeof(key), "%d", i);
					if (move["submovelist"].HasMember(key)) {
						handleMoveList(e.data.submovelist[i], move["submovelist"][key]);
					}
				}
			}
		}

		vector->push_back(e);

		return 0;
	};

	const char *sql = sqlite3_mprintf("SELECT * from SubGate WHERE tid='%d'", tid);
	int ok = sqlite3_exec(_db, sql, callback, &vector, 0);

	if( ok != SQLITE_OK)
		log("Error in queryEndGameItem()");
}

EndGameData::EndGameData(std::string fullpath)
{
	int ret = 0;

	if (fullpath.empty())
		ret = sqlite3_open(":memory:",&_db);
	else
		ret = sqlite3_open(fullpath.c_str(), &_db);

	if( ret != SQLITE_OK ) {
		log("Error initializing DB\n");
	}
}

EndGameData::~EndGameData()
{
	sqlite3_close(_db);
}
