#include "XQJsonFile.h"
#include "Utils.h"
#include "cocos2d.h"

USING_NS_CC;

void XQJsonFile::load(std::string name)
{
    log("JSON: %s", name.c_str());
    _doc.Parse<0>(name.c_str());
    _data.header.title = _doc["SubTitle"].GetString();
    _data.header.fen = _doc["fen"].GetString();

    if (_doc.HasMember("move")) {
        rapidjson::Value &move = _doc["move"];
        if (move.HasMember("movelist")) {
            _data.pRoot = handleMoveList(nullptr, move["movelist"]);
        }
    }
}

XQNode *XQJsonFile::handleMoveList(XQNode *parent, rapidjson::Value &array)
{
    XQNode *retNode;

    rapidjson::Value &move = _doc["move"];

    for (unsigned i = 0; i < array.Size(); i++) {
        XQNode *node = new XQNode();
        if (node == nullptr) {
            return nullptr;
        }

        if (i == 0)
            retNode = node;

        node->pParent = parent;
        if (parent) {
            parent->pCurChild = parent->pFirstChild = node;
        }

        if (array[i].HasMember("src") && array[i].HasMember("dst")) {
            std::string src = array[i]["src"].GetString();
            std::string dst = array[i]["dst"].GetString();
            if (src[0] >= 'a' && src[0] <= 'i') {
                node->mv = src + dst;
            } else {
                auto s = Vec2(src[0]-'0', 9-(src[1]-'0'));
                auto d = Vec2(dst[0]-'0', 9-(dst[1]-'0'));
                node->mv = Utils::toUcciMove(s, d);
            }
            log("mv: %s", node->mv.c_str());
        }

        if (array[i].HasMember("comment")) {
            node->comment = array[i]["comment"].GetString();
            log("comment: %s", node->comment.c_str());
        }

        if (array[i].HasMember("sub")) {
            XQNode *sibling = node;
            for (unsigned j = 0; j < array[i]["sub"].Size(); j++) {
                std::string sub = array[i]["sub"][j].GetString();
                log("sub: %s", sub.c_str());
                XQNode *right = handleMoveList(nullptr, move["submovelist"][sub.c_str()]);
                if (right) {
                    sibling->pRight = right;
                    right->pLeft = sibling;
                    right->pParent = parent;
                    sibling = right;
                }
            }
        }

        parent = node;
    }

    return retNode;
}
