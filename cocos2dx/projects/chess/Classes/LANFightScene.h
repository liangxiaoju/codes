#ifndef __LANFIGHTSCENE_H__
#define __LANFIGHTSCENE_H__

#include "cocos2d.h"
#include "Player.h"
#include "UIPlayer.h"
#include "NetPlayer.h"
#include "Board.h"
#include "GameLayer.h"
#include "ControlLayer.h"
#include "room/RoomManager.h"
#include <memory>  // for std::shared_ptr
#include <atomic>
#include "UserData.h"

USING_NS_CC;

class LANScene : public Scene
{
public:
    virtual bool init();
    CREATE_FUNC(LANScene);

    virtual void setTitle(const std::string &title);
    virtual void pushBackView(Widget *child);

    virtual ~LANScene();

private:
    Layout *_layout;
    Layout *_viewLayout;
    Text *_text;

protected:
    std::shared_ptr<std::atomic<bool>> _isDestroyed;
};

class LANTopScene : public LANScene
{
public:
    bool init();
    CREATE_FUNC(LANTopScene);
};

class LANCreateScene : public LANScene
{
public:
    bool init();
    CREATE_FUNC(LANCreateScene);

private:
    std::string _fen;
    std::string _side;
    bool _regret;
};

class LANLoadScene : public LANScene
{
public:
    bool init();
    CREATE_FUNC(LANLoadScene);

private:
    std::vector<UserData::SaveElement> _saveElements;
};

class LANWaitScene : public LANScene
{
public:
    bool init(std::string fen, std::string side);
    void onWait();
    static LANWaitScene *create(std::string fen, std::string side) {
        auto *pRet = new (std::nothrow) LANWaitScene();
        if (pRet && pRet->init(fen, side)) {
            pRet->autorelease();
            return pRet;
        } else {
            CC_SAFE_DELETE(pRet);
            return nullptr;
        }
    }
    virtual ~LANWaitScene();

private:
    std::string _fen;
    std::string _side;
    RoomServer *_server;
    RoomClient *_client;
};

class LANJoinScene : public LANScene
{
public:
    bool init();
    CREATE_FUNC(LANJoinScene);
    void onJoin();

private:
    std::string _fen;
    std::string _side;
    RoomClient *_client;
    std::vector<RoomManager::RoomInfo> _roomInfos;
};

class LANFightScene : public Scene
{
public:
    bool init(std::string fen, std::string side,
              RoomServer *server, RoomClient *client);
    static LANFightScene *create(std::string fen, std::string side,
                                 RoomServer *server, RoomClient *client) {
        auto *pRet = new (std::nothrow) LANFightScene();
        if (pRet && pRet->init(fen, side, server, client)) {
            pRet->autorelease();
            return pRet;
        } else {
            CC_SAFE_DELETE(pRet);
            return nullptr;
        }
    }
    virtual ~LANFightScene();

private:
    Player *_playerWhite;
    Player *_playerBlack;
    UIPlayer *_playerUI;
    NetPlayer *_playerNet;
    Board *_board;
    std::string _fen;
    std::string _side;
    RoomClient *_client;
    RoomServer *_server;
    std::shared_ptr<std::atomic<bool>> _isDestroyed;
};

#endif
