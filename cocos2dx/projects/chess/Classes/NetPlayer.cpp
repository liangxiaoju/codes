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
		log("onAction: %s", packet.toString().c_str());

		if (packet["TYPE"] == "action") {
			if (!_active) {
				_client->emit("action", "ok");
				return;
			}

			auto contents = Utils::splitString(packet["CONTENT"], '\n');

			for (auto &content : contents) {

				if (content.find("position") != std::string::npos) {
					std::unique_lock<std::mutex> lock(_mutex);
					_queue.emplace(content);
				} else if (content.find("go") != std::string::npos) {

					if (_queue.size() > 0) {
						_client->emit("action", "ok");
					} else {
						return;
					}

					auto cb = [this]() {
						if (_queue.size() == 0) {
							log("No position before go.");
							return;
						}

						while (_queue.size()) {
							std::string reply = _queue.front();
							_queue.pop();

							if (reply.find_first_of(getBoard()->getFenWithMove()) == std::string::npos) {
								log("position not match: %s", reply.c_str());
								continue;
							}

							auto substr = Utils::splitString(reply, ' ');

							auto mv = substr.back();
							auto vecs = Utils::toVecMove(mv);

							int ret = getBoard()->move(vecs[0], vecs[1], true);

							if (ret == -3)
								getListener()->onResignRequest();
							else if (ret == 0)
								getListener()->onMoved(mv);
						}
					};

					_active = false;
					CallFunc *callback = CallFunc::create(cb);
					runAction(callback);

				} else if (content.find("ok") != std::string::npos) {
					log("*** Got Reply ***");
					stopAllActions();
				}
			}
		}
	};

	auto onMessage = [this](RoomClient *client, const RoomPacket &packet) {
		log("%s", packet.toString().c_str());
	};

	_client->on("action", onAction);
	_client->on("message", onMessage);

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

	{
		std::unique_lock<std::mutex> lock(_mutex);
		while(_queue.size())
			_queue.pop();
	}

	_sendcount = 0;
	CallFunc *callback = CallFunc::create([this]() {
			std::string cmd = std::string("position fen ") +
			getBoard()->getFenWithMove();
			_client->emit("action", cmd);
			_client->emit("action", "go");

			log("*** Send: %d ***", ++_sendcount);
			});
	Sequence *seq = Sequence::create(callback, DelayTime::create(1), nullptr);
	runAction(Repeat::create(seq, 5));

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
