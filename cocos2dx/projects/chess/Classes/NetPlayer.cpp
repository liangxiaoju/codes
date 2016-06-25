#include "NetPlayer.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

bool NetPlayer::init()
{
	if (!Player::init())
		return false;

	_head = HeaderSprite::createWithType(HeaderSprite::Type::RIGHT);
	_head->setNameLine("Net");
	auto s = _head->getContentSize();
	_head->setPosition(s.width/2, s.height/2);
	addChild(_head);
	setContentSize(s);

	_active = false;

	RoomManager *manager = RoomManager::getInstance();
	std::vector<RoomManager::RoomInfo> v;
	v = manager->scanRoom(1, 5);

	if (v.size() == 0) {
		log("Failed to scan Room.");
		return false;
	}

	char name[16];
	std::srand(std::time(0));
	snprintf(name, sizeof(name), "%d", int(std::rand()*1000.0f/RAND_MAX));
	_name = name;
	log("%s", _name.c_str());

	_client = manager->joinRoom(_name, v[0]);

	auto onAction = [this](RoomClient *client, const RoomPacket &packet) {

		if (packet["TYPE"] == "action") {
			if (!_active) {
				log("@@ inactive: ignore.");
				return;
			}

			auto content = packet["CONTENT"];

			if (content.find("position") != std::string::npos) {

				auto cb = [this, content]() {
					log("@@ Got: %s", content.c_str());

					auto substr = Utils::splitString(content, ' ');

					auto mv = substr.back();
					auto vecs = Utils::toVecMove(mv);

					int ret = getBoard()->move(vecs[0], vecs[1], true);

					if (ret == -3)
						getListener()->onResignRequest();
					else if (ret == 0)
						getListener()->onMoved(mv);
				};

				_client->emit("action", "ok");

				if (content.find_first_of(getBoard()->getFenWithMove()) == std::string::npos) {
					log("@@ position not match: %s", content.c_str());
					return;
				}

				if (content.find_first_of("moves") == std::string::npos) {
					log("@@ ignore: %s", content.c_str());
					return;
				}

				CallFunc *callback = CallFunc::create(cb);
				runAction(callback);

			} else if (content.find("ok") != std::string::npos) {
				log("@@ Got Reply");
				stopAllActionsByTag(1000);
			}
		}
	};

	_client->on("action", onAction);

	return true;
}

NetPlayer::~NetPlayer()
{
	RoomManager *manager = RoomManager::getInstance();
	manager->leaveRoom(_client);
	log("~NetPlayer");
}

void NetPlayer::go(float timeout)
{
	getEventDispatcher()->dispatchCustomEvent(EVENT_AIPLAYER_GO);
	_head->setActive(true);

	_sendcount = 0;
	CallFunc *callback = CallFunc::create([this]() {
			std::string cmd = std::string("position fen ") +
			getBoard()->getFenWithMove();
			_client->emit("action", cmd);

			log("@@ Send: %d", ++_sendcount);
			});
	auto seq = Sequence::create(callback, DelayTime::create(1), nullptr);
	auto action =  Repeat::create(seq, 5);
	action->setTag(1000);
	runAction(action);

	_active = true;
}

void NetPlayer::stop()
{
	_active = false;
	_head->setActive(false);
    stopAllActions();
}

bool NetPlayer::askForDraw()
{
	return true;
}
