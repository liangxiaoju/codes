#include "NetPlayer.h"
#include <string.h>
#include <unistd.h>

bool NetPlayer::init(RoomClient *client)
{
	if (!Player::init("Net"))
		return false;

	_isDestroyed = std::make_shared<std::atomic<bool>>(false);

	_active = false;
	_client = client;

	std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;

	auto onAction = [this, isDestroyed](RoomClient *client, const RoomPacket &packet) {

		if (*isDestroyed) {
			log("onAction: NetPlayer instance was destroyed!");
			return;
		}

		if (!_active) {
			log("@@ inactive: ignore.");
			return;
		}

		auto content = packet["CONTENT"];

		if (content.find("position") != std::string::npos) {

			_client->emit("action", "ok");
/*
  // if regret ?
			if (content.find(_fen) == std::string::npos) {
				log("@@ position not match: %s", content.c_str());
				return;
			}
*/
			if (content.find("moves") == std::string::npos) {
				log("@@ ignore: %s", content.c_str());
				return;
			}

			log("@@ Got: %s", content.c_str());

			auto substr = Utils::splitString(content, ' ');
			auto mv = substr.back();

			_delegate->onMoveRequest(mv);

		} else if (content.find("ok") != std::string::npos) {
			log("@@ Got Reply");
			stopAllActionsByTag(1000);
		}
	};

	auto onControl = [this, isDestroyed](RoomClient *client, const RoomPacket &packet) {

		if (*isDestroyed) {
			log("onControl: NetPlayer instance was destroyed!");
			return;
		}

		auto content = packet["CONTENT"];

		if (content.find("request:regret") != std::string::npos) {
			log("@@ request regret");
			_delegate->onRegretRequest();
		} else if (content.find("request:draw") != std::string::npos) {
			log("@@ request draw");
			_delegate->onDrawRequest();
		} else if (content.find("request:reset") != std::string::npos) {
			log("@@ request reset");
			getEventDispatcher()->dispatchCustomEvent(
                EVENT_RESET, (void*)"Net");
        } else if (content.find("request:resign") != std::string::npos) {
            log("@@ request resign");
            _delegate->onResignRequest();

		} else if (content.find("reply:regret:deny") != std::string::npos) {
			log("@@ reply request deny");
			getEventDispatcher()->dispatchCustomEvent(
                EVENT_REQUEST_REPLY, (void*)"REPLY:REGRET:DENY");
            getEventDispatcher()->removeCustomEventListeners(EVENT_REQUEST_REPLY);
        } else if (content.find("reply:regret:accept") != std::string::npos) {
			log("@@ reply request accept");
			getEventDispatcher()->dispatchCustomEvent(
                EVENT_REQUEST_REPLY, (void*)"REPLY:REGRET:ACCEPT");
            getEventDispatcher()->removeCustomEventListeners(EVENT_REQUEST_REPLY);

        } else if (content.find("reply:draw:accept") != std::string::npos) {
			log("@@ reply draw accept");
			getEventDispatcher()->dispatchCustomEvent(
                EVENT_REQUEST_REPLY, (void*)"REPLY:DRAW:ACCEPT");
            getEventDispatcher()->removeCustomEventListeners(EVENT_REQUEST_REPLY);
        } else if (content.find("reply:draw:deny") != std::string::npos) {
			log("@@ reply draw deny");
			getEventDispatcher()->dispatchCustomEvent(
                EVENT_REQUEST_REPLY, (void*)"REPLY:DRAW:DENY");
            getEventDispatcher()->removeCustomEventListeners(EVENT_REQUEST_REPLY);

        } else if (content.find("reply:reset:accept") != std::string::npos) {
            log("@@ reply reset accept");
			getEventDispatcher()->dispatchCustomEvent(
                EVENT_REQUEST_REPLY, (void*)"REPLY:RESET:ACCEPT");
            getEventDispatcher()->removeCustomEventListeners(EVENT_REQUEST_REPLY);
        } else if (content.find("reply:reset:deny") != std::string::npos) {
            log("@@ reply reset deny");
			getEventDispatcher()->dispatchCustomEvent(
                EVENT_REQUEST_REPLY, (void*)"REPLY:RESET:DENY");
            getEventDispatcher()->removeCustomEventListeners(EVENT_REQUEST_REPLY);
        }
	};

	_client->on("action", [onAction](RoomClient *client, const RoomPacket &packet) {
		Director::getInstance()->getScheduler()->performFunctionInCocosThread(
			[onAction, client, packet]() {
			onAction(client, packet);
		});
	});
	_client->on("control", [onControl](RoomClient *client, const RoomPacket &packet) {
		Director::getInstance()->getScheduler()->performFunctionInCocosThread(
		[onControl, client, packet]() {
			onControl(client, packet);
		});
	});

	return true;
}

NetPlayer::~NetPlayer()
{
	log("~NetPlayer");
}

void NetPlayer::start(std::string fen)
{
	_sendcount = 0;
	CallFunc *callback = CallFunc::create([this, fen]() {
			std::string cmd = std::string("position fen ") + fen;
			_client->emit("action", cmd);

			log("@@ Send: %d", ++_sendcount);
			});
	auto seq = Sequence::create(callback, DelayTime::create(1), nullptr);
	auto action =  Repeat::create(seq, 5);
	action->setTag(1000);
	runAction(action);

	_fen = fen;
	_active = true;
}

void NetPlayer::stop()
{
	_active = false;
    stopAllActions();
}

void NetPlayer::onRequest(std::string req, std::string args,
                          std::function<void(bool)>callback)
{
	std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;

	if (req == "draw") {

        Director::getInstance()->getEventDispatcher()->addCustomEventListener(
            EVENT_REQUEST_REPLY, [this, isDestroyed, callback](EventCustom *ev) {
                if (*isDestroyed) {
                    log("onRequest: NetPlayer instance was destroyed!");
                    return;
                }
                std::string event = (const char *)ev->getUserData();

                log("@@ REPLY: %s", event.c_str());

                if (event.find("REPLY:DRAW:DENY") != std::string::npos) {
                    if (callback)
                        callback(false);
                } else if (event.find("REPLY:DRAW:ACCEPT") != std::string::npos) {
                    if (callback)
                        callback(true);
                }
            });

		_client->emit("control", "request:draw");

	} else if (req == "regret") {

        Director::getInstance()->getEventDispatcher()->addCustomEventListener(
            EVENT_REQUEST_REPLY, [this, isDestroyed, callback](EventCustom *ev) {
                if (*isDestroyed) {
                    log("onRequest: NetPlayer instance was destroyed!");
                    return;
                }
                std::string event = (const char *)ev->getUserData();

                log("@@ REPLY: %s", event.c_str());

                if (event.find("REPLY:REGRET:DENY") != std::string::npos) {
                    if (callback)
                        callback(false);
                } else if (event.find("REPLY:REGRET:ACCEPT") != std::string::npos) {
                    if (callback)
                        callback(true);
                }
            });

        _client->emit("control", "request:regret");

	} else if (req == "reset") {

        Director::getInstance()->getEventDispatcher()->addCustomEventListener(
            EVENT_REQUEST_REPLY, [this, isDestroyed, callback](EventCustom *ev) {
                if (*isDestroyed) {
                    log("onRequest: NetPlayer instance was destroyed!");
                    return;
                }
                std::string event = (const char *)ev->getUserData();

                log("@@ REPLY: %s", event.c_str());

                if (event.find("REPLY:RESET:DENY") != std::string::npos) {
                    if (callback)
                        callback(false);
                } else if (event.find("REPLY:RESET:ACCEPT") != std::string::npos) {
                    if (callback)
                        callback(true);
                }
            });

        _client->emit("control", "request:reset");
    } else if (req == "resign") {

        _client->emit("control", "request:resign");

        if (callback)
            callback(true);
    }
}

void NetPlayer::onReply(std::string reply, std::string args)
{
    log("@@ onReply: %s %s", reply.c_str(), args.c_str());

    if (reply == "regret") {
        if (args == "accept")
            _client->emit("control", "reply:regret:accept");
        else
            _client->emit("control", "reply:regret:deny");
    } else if (reply == "draw") {
        if (args == "accept")
            _client->emit("control", "reply:draw:accept");
        else
            _client->emit("control", "reply:draw:deny");
    } else if (reply == "reset") {
        if (args == "accept")
            _client->emit("control", "reply:reset:accept");
        else
            _client->emit("control", "reply:reset:deny");
    }
}
