#ifndef __XQJSONFILE_H__
#define __XQJSONFILE_H__

#include "XQFile.h"
#include "json/rapidjson.h"
#include "json/document.h"
#include "json/stringbuffer.h"

class XQJsonFile : public XQFile
{
public:
    void load(std::string name) override;

private:
    XQNode *handleMoveList(XQNode *parent, rapidjson::Value &array);
    rapidjson::Document _doc;
};

#endif
