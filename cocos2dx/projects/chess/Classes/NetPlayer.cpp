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

			if (content.find_first_of(_fen) == std::string::npos) {
				log("@@ position not match: %s", content.c_str());
				return;
			}

			if (content.find_first_of("moves") == std::string::npos) {
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

		if (content.find("regret") != std::string::npos) {
			log("@@ request regret");
			_delegate->onRegretRequest();
		} else if (content.find("draw") != std::string::npos) {
			log("@@ request draw");
			_delegate->onDrawRequest();
		} else if (content.find("reset") != std::string::npos) {
			log("@@ request reset");
			Director::getInstance()->getEventDispatcher()
				->dispatchCustomEvent(EVENT_RESET, (void*)"Net");
		} else if (content.find("switch") != std::string::npos) {
			log("@@ request switch");
			Director::getInstance()->getEventDispatcher()
				->dispatchCustomEvent(EVENT_SWITCH, (void*)"Net");
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

bool NetPlayer::onRequest(std::string req)
{
	if (req == "draw") {
		_client->emit("control", "draw");
		return true;
	} else if (req == "regret") {
		_client->emit("control", "regret");
		/* TODO: return true/false according to the ack */
		return true;
	} else if (req == "reset") {
		_client->emit("control", "reset");
		return true;
	} else if (req == "switch") {
		_client->emit("control", "switch");
		return true;
	}

	return false;
}
