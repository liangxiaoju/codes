#include "AIPlayer.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

int socket_client_init()
{
	int sockfd, portno, i=0;
	struct sockaddr_in serv_addr;

	portno = 6669;

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portno);

	while (i++ < 10) {
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) {
			log("ERROR opening socket");
			continue;
		}

		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
			log("ERROR connecting, retry %d", i);
			close(sockfd);
			sockfd = -1;
			usleep(200*1000);
		} else {
			break;
		}
	}

	return sockfd;
}

int socket_server_init()
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	const int on = 1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		log("ERROR opening socket");

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 6669;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0) 
		log("ERROR on binding");

	listen(sockfd,1);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, 
			(struct sockaddr *) &cli_addr, 
			&clilen);
	if (newsockfd < 0) 
		log("ERROR on accept");

	close(sockfd);

	return newsockfd;
}

extern void emain();
extern int server_socketfd;
void AIPlayer::eleeye_thread()
{
	server_socketfd = socket_server_init();

	emain();
}

bool AIPlayer::init()
{
	if (!Player::init())
		return false;

	_head = HeaderSprite::createWithType(HeaderSprite::Type::RIGHT);
	_head->setNameLine("Computer");
	auto s = _head->getContentSize();
	_head->setPosition(s.width/2, s.height/2);
	addChild(_head);
	setContentSize(s);

	/* have to be inited before onEnter */
	_thread = std::thread(std::bind(
				&AIPlayer::eleeye_thread, this));

	client_socketfd = socket_client_init();

	std::string reply;
	reply = sendWithReply("ucci", "ucciok");
	reply = sendWithReply("isready", "readyok");

	return true;
}

void AIPlayer::setName(std::string first, std::string second)
{
	if (!first.empty()) {
		_head->setNameLine(first);
	}

	if (!second.empty()) {
		_head->setInfoLine(second);
	}
}

void AIPlayer::setDifficulty(int level)
{
	_difficulty = level;
	switch (level) {
	case 0:
		_difficulty = 0; break;
	case 1:
		_difficulty = 2; break;
	case 2:
		_difficulty = 4; break;
	case 3:
		_difficulty = 8; break;
	default:
		_difficulty = level; break;
	}
	_head->setInfoLine("Level: " + Utils::toString(level));
}

AIPlayer::~AIPlayer()
{
	sendWithoutReply("stop");
	sendWithReply("stop", "nobestmove");
	sendWithReply("quit", "bye");
	_thread.join();
	log("~AIPlayer");
}

ssize_t readline(int fd, char* ptr, size_t maxlen)
{
    size_t n, rc;
    char c;

    for( n = 0; n < maxlen - 1; n++ ) {
        if( (rc = recv(fd, &c, 1, 0)) ==1 ) {
            *ptr++ = c;
            if(c == '\n') {
                break;
            }
        } else if( rc == 0 ) {
            return 0;
        } else if( errno == EINTR ) {
            continue;
        } else {
            return -1;
        }
    }

    *ptr = 0;
    return n;
}

int AIPlayer::sendWithoutReply(std::string msg)
{
	msg += "\n";
	log("sendWithoutReply: %s", msg.c_str());
	return send(client_socketfd, msg.c_str(), msg.size(), 0);
}

std::string AIPlayer::sendWithReply(std::string msg, std::string match)
{
	char line[1024];
	int n;

	msg += "\n";

	log("sendWithReply: %s expect: %s", msg.c_str(), match.c_str());

	mutex.lock();

	struct timeval timeout = {0, 0};
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(client_socketfd, &readfds);
	int nready = select(client_socketfd+1, &readfds, nullptr, nullptr, &timeout);
	if (nready > 0) {
		if (FD_ISSET(client_socketfd, &readfds))
			recv(client_socketfd, line, sizeof(line), 0);
	}

	if (send(client_socketfd, msg.c_str(), msg.size(), 0) < 0)
		return nullptr;

	while ((n = readline(client_socketfd, line, sizeof(line))) >= 0) {
		//log("socket: %s", line);
		if (strstr(line, match.c_str()) != nullptr) {
			break;
		}
	}

	mutex.unlock();

	log("Got Reply: %s", line);

	return std::string(line);
}

int AIPlayer::sendWithCallBack(std::string msg, std::string match, const std::function<void (std::string)> &cb)
{
	auto callback = [cb](EventCustom* ev){
		std::string *reply = (std::string *)ev->getUserData();
		log("callback %s", (*reply).c_str());
		cb(*reply);
	};

	auto task = [this, msg, match, callback](){
		retain();
		std::string *reply = new std::string();
		*reply = this->sendWithReply(msg, match);

		Director::getInstance()->getEventDispatcher()->removeCustomEventListeners("sendWithCallBack");
		Director::getInstance()->getEventDispatcher()->addCustomEventListener("sendWithCallBack", callback);

		Director::getInstance()->getScheduler()->performFunctionInCocosThread([this, reply](){
			Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("sendWithCallBack", (void *)reply);
			delete reply;
		});
		release();
	};

	std::thread sendThread(task);
	sendThread.detach();

	return 0;
}

void AIPlayer::ponder()
{
}

void AIPlayer::go(float timeout)
{
	getEventDispatcher()->dispatchCustomEvent(EVENT_AIPLAYER_GO);
	_head->setActive(true);
	std::string cmd = std::string("position fen ") + getBoard()->getFenWithMove();
	sendWithoutReply(cmd);
	cmd = "go time " + Utils::toString(int(_difficulty*1000+timeout));
	sendWithCallBack(cmd, "bestmove", [&](std::string reply)
		{
			if (reply.find("nobestmove") != std::string::npos) {
				auto l = getListener();
				if (l != nullptr)
					l->onResignRequest();
			} else if (reply.find("draw") != std::string::npos) {
				auto l = getListener();
				if (l != nullptr)
					l->onDrawRequest();
			} else {
				auto substr = Utils::splitString(reply, ' ');
				auto mv = substr[1];
				auto vecs = Utils::toVecMove(mv);

				auto b = getBoard();
				int ret = 0;
				if (b != nullptr)
					ret = b->move(vecs[0], vecs[1], false);

				auto l = getListener();

				if (l != nullptr) {
					if (ret < 0) {
						if (ret == -3)
							l->onResignRequest();
					} else {
						l->onMoved(mv);
					}
				}
			}
		});
}

void AIPlayer::stop()
{
	sendWithoutReply("stop");
	sendWithReply("stop", "nobestmove");
	_head->setActive(false);
	getEventDispatcher()->removeCustomEventListeners("sendWithCallBack");
}

bool AIPlayer::askForDraw()
{
	return true;
}
