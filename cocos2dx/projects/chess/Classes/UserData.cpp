#include "UserData.h"

UserData *UserData::s_userData = nullptr;

void UserData::createTable()
{
	const char *sql_createtable;
	sqlite3_stmt *stmt;
	int ok;

	sql_createtable = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS "
					   "saveTbl("
							   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
							   "date TEXT, "
							   "mode INTEGER, "
							   "side INTEGER, "
							   "level INTEGER, "
							   "white TEXT, "
							   "black TEXT, "
							   "fen TEXT)"
							   );
	ok = sqlite3_prepare_v2(_db, sql_createtable, -1, &stmt, nullptr);
	ok |= sqlite3_step(stmt);
	ok |= sqlite3_finalize(stmt);
	
	if( ok != SQLITE_OK && ok != SQLITE_DONE)
		log("Error in CREATE TABLE saveTbl");

	sql_createtable = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS "
					   "recordTbl("
							   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
							   "date TEXT, "
							   "mode INTEGER, "
							   "side INTEGER, "
							   "level INTEGER, "
							   "white TEXT, "
							   "black TEXT, "
							   "win TEXT, "
							   "fen TEXT)"
							   );
	ok = sqlite3_prepare_v2(_db, sql_createtable, -1, &stmt, nullptr);
	ok |= sqlite3_step(stmt);
	ok |= sqlite3_finalize(stmt);
	
	if( ok != SQLITE_OK && ok != SQLITE_DONE)
		log("Error in CREATE TABLE recordTbl");
}

void UserData::querySaveTbl(std::vector<SaveElement> &vector)
{
	auto callback = [](void *v, int argc, char **argv, char **colName)->int{

		std::vector<SaveElement> *vector = (std::vector<SaveElement> *)v;

		if (argc != 8) {
			log("argc!=8");
			return SQLITE_ERROR;
		}

		SaveElement e;
		e.id = atoi(argv[0]);
		e.date = argv[1];
		e.mode = atoi(argv[2]);
		e.side = atoi(argv[3]);
		e.level = atoi(argv[4]);
		e.white = argv[5];
		e.black = argv[6];
		e.fen = argv[7];

		vector->push_back(e);

		return 0;
	};

	int ok = sqlite3_exec(_db, "SELECT * from saveTbl", callback, &vector, 0);

	if( ok != SQLITE_OK)
		log("Error in querySaveTbl()");
}

void UserData::queryRecordTbl(std::vector<RecordElement> &vector)
{
	auto callback = [](void *v, int argc, char **argv, char **colName)->int{

		std::vector<RecordElement> *vector = (std::vector<RecordElement> *)v;

		if (argc != 9) {
			log("argc!=9");
			return SQLITE_ERROR;
		}

		RecordElement e;
		e.id = atoi(argv[0]);
		e.date = argv[1];
		e.mode = atoi(argv[2]);
		e.side = atoi(argv[3]);
		e.level = atoi(argv[4]);
		e.white = argv[5];
		e.black = argv[6];
		e.win = argv[7];
		e.fen = argv[8];

		vector->push_back(e);

		return 0;
	};

	int ok = sqlite3_exec(_db, "SELECT * from recordTbl", callback, &vector, 0);

	if( ok != SQLITE_OK)
		log("Error in queryRecordTbl()");
}

void UserData::insertRecordElement(RecordElement element)
{
	char *sql = sqlite3_mprintf(
			"INSERT INTO "
			"recordTbl (date,mode,side,level,white,black,win,fen) "
			"VALUES (datetime('now', 'localtime'),%d,%d,%d,'%s','%s','%s','%s')",
			element.mode,
			element.side,
			element.level,
			element.white.c_str(),
			element.black.c_str(),
			element.win.c_str(),
			element.fen.c_str());

	int ok = sqlite3_exec(_db, sql, 0, 0, 0);
	
	if( ok != SQLITE_OK)
		log("Error in insertRecordElement()");
}

void UserData::insertSaveElement(SaveElement element)
{
	char *sql = sqlite3_mprintf(
			"INSERT INTO "
			"saveTbl (date,mode,side,level,white,black,fen) "
			"VALUES (datetime('now', 'localtime'),%d,%d,%d,'%s','%s','%s')",
			element.mode,
			element.side,
			element.level,
			element.white.c_str(),
			element.black.c_str(),
			element.fen.c_str());

	int ok = sqlite3_exec(_db, sql, 0, 0, 0);
	
	if(ok != SQLITE_OK)
		log("Error in insertSaveElement()");
}

void UserData::deleteRecordElement(int id)
{
	char *sql = sqlite3_mprintf("DELETE FROM recordTbl WHERE id='%d'", id);
	int ok = sqlite3_exec(_db, sql, nullptr, nullptr, nullptr);

	if( ok != SQLITE_OK && ok != SQLITE_DONE)
		log("Error in deleteRecordElement()");
}

void UserData::deleteSaveElement(int id)
{
	char *sql = sqlite3_mprintf("DELETE FROM saveTbl WHERE id='%d'", id);
	int ok = sqlite3_exec(_db, sql, nullptr, nullptr, nullptr);

	if( ok != SQLITE_OK)
		log("Error in deleteSaveElement()");
}

void UserData::clearRecordTbl()
{
	int ok = sqlite3_exec(_db, "DELETE FROM recordTbl", nullptr, nullptr, nullptr);
    
    if( ok != SQLITE_OK)
        log("Error in clearRecordTbl()");
}

void UserData::clearSaveTbl()
{
	int ok = sqlite3_exec(_db, "DELETE FROM saveTbl", 0, 0, 0);
    if( ok != SQLITE_OK)
        log("Error in clearSaveTbl()");
}

UserData::UserData(std::string fullpath)
{
	int ret = 0;

	if (fullpath.empty())
		ret = sqlite3_open(":memory:",&_db);
	else
		ret = sqlite3_open(fullpath.c_str(), &_db);

	if( ret != SQLITE_OK ) {
		log("Error initializing DB\n");
	}

	createTable();
}

UserData::~UserData()
{
	sqlite3_close(_db);
}
