#include "AIPlayer2.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

bool AIPlayer2::init()
{
	if (!Player::init())
		return false;

	_stop = false;

	_head = HeaderSprite::createWithType(HeaderSprite::Type::RIGHT);
	_head->setNameLine("Computer2");
	auto s = _head->getContentSize();
	_head->setPosition(s.width/2, s.height/2);
	addChild(_head);
	setContentSize(s);

	pipe(_pipe);

	_aiThread = std::thread(std::bind(&AIPlayer2::AI_Thread, this));
	_cmdThread = std::thread(std::bind(&AIPlayer2::CMD_Thread, this));

	int i = 0;
	while (i++ < 10) {
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(6669);

		if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			log("Failed to create socket");
			continue;
		}
		if (connect(_sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			log("Failed to connect, retry %d", i);
			close(_sockfd);
			_sockfd = -1;
			usleep(200*1000);
		} else
			break;
	}

	if (_sockfd < 0)
		return false;

	_sockfile = fdopen(_sockfd, "r+");
	setbuf(_sockfile, NULL);

	std::string reply;
	reply = sendWithReply("ucci", "ucciok");
	reply = sendWithReply("isready", "readyok");

	return true;
}

void AIPlayer2::setName(std::string first, std::string second)
{
	if (!first.empty()) {
		_head->setNameLine(first);
	}

	if (!second.empty()) {
		_head->setInfoLine(second);
	}
}

void AIPlayer2::setDifficulty(int level)
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

AIPlayer2::~AIPlayer2()
{
	sendWithoutReply("stop");
	sendWithReply("stop", "nobestmove");
	sendWithReply("quit", "bye");
	_aiThread.join();

    stopAllActions();

	_stop = true;

	fclose(_sockfile);
	_condition.notify_all();
	_cmdThread.join();

	close(_pipe[0]);
	close(_pipe[1]);

	log("~AIPlayer");
}

extern void emain();
int server_socketfd;

void AIPlayer2::AI_Thread()
{
	int sockfd, newsockfd;
	socklen_t len;
	struct sockaddr_in srvaddr, cliaddr;
	const int on = 1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		log("ERROR opening socket");

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
			(const char*)&on, sizeof(on));

	memset(&srvaddr , 0, sizeof(struct sockaddr_in));

	srvaddr.sin_family = AF_INET;
	srvaddr.sin_addr.s_addr = INADDR_ANY;
	srvaddr.sin_port = htons(6669);

	if (bind(sockfd, (struct sockaddr *) &srvaddr,
				sizeof(struct sockaddr_in)) < 0) 
		log("ERROR on binding");

	listen(sockfd, 1);
	len = sizeof(cliaddr);

	newsockfd = accept(sockfd, 
			(struct sockaddr *) &cliaddr, 
			&len);
	if (newsockfd < 0) 
		log("ERROR on accept");

	close(sockfd);

	server_socketfd = newsockfd;

	log("Eleeye_Thread start.");

	emain();

	log("Eleeye_Thread exit.");
}

void AIPlayer2::CMD_Thread()
{
	/*
	 * android ndk<21 has no getline()
	 * our getline() do not return '\n'
	 */
	auto getline = [](char **lineptr, size_t *len, FILE *fp)->size_t{
		char *ptr = *lineptr;
		int fd = fileno(fp);
		size_t n, rc, maxlen = *len;
		char c;

		if (ptr == NULL) return -1;

		for( n = 0; n < maxlen - 1; n++ ) {
			if((rc = recv(fd, &c, 1, 0)) == 1) {
				*ptr = c;
				if(c == '\n') {
					break;
				}
				ptr++;
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
	};

	do {
		Request request;
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_condition.wait(lock, [this] {
					return _stop || !_queue.empty();
					});
			if (_stop)
				return;

			request = _queue.front();
			_queue.pop();
		}

		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(_sockfd, &fdset);
		struct timeval timeout = {0, 0};

		if (select(_sockfd+1, &fdset, NULL, NULL, &timeout) > 0) {
			char tmp[1024];
			recv(_sockfd, tmp, sizeof(tmp), 0);
		}

		if (request.cmd.size() > 0) {
			fprintf(_sockfile, "%s\n", request.cmd.c_str());
			fflush(_sockfile);
		}

		memset(&timeout, 0, sizeof(timeout));
		if (request.timeout > 0) {
			timeout.tv_sec = request.timeout;
		}

		if (request.reply.size() > 0) {
			size_t len = 1024;
			char *line = (char*)malloc(len);
			if (!line)
				continue;
			memset(line, 0, len);

			do {
				FD_ZERO(&fdset);
				FD_SET(_sockfd, &fdset);
				FD_SET(_pipe[0], &fdset);
				int n = select(std::max(_sockfd, _pipe[0])+1,
						&fdset, NULL, NULL, &timeout);
				if (_stop)
					return;
				if (n == 0) {
					log("select timeout");
					break;
				}
				if (FD_ISSET(_pipe[0], &fdset)) {
					log("pipe %s %d", __func__, __LINE__);
					char c[32];
					read(_pipe[0], c, sizeof(c));
					log("pipe %s %d", __func__, __LINE__);
					break;
				}
				if (FD_ISSET(_sockfd, &fdset)) {
#if 0
					if ((n = getline(&line, &len, _sockfile)) < 0)
						continue;
#else
					if (fgets(line, len, _sockfile) == NULL)
						continue;
#endif
					if (strstr(line, request.reply.c_str()) != nullptr) {
						break;
					}
				}

				memset(line, 0, len);

			} while (1);

			std::string reply(line);
			free(line);
			request.cb(reply);
		}

	} while(1);
}

void AIPlayer2::enqueueRequest(Request req)
{
	std::unique_lock<std::mutex> lock(_mutex);
	_queue.emplace(req);
	_condition.notify_one();
}

void AIPlayer2::sendWithoutReply(std::string msg)
{
	Request req;
	req.cmd = msg;
	req.timeout = 0;
	enqueueRequest(req);
	log("sendWithoutReply: %s", msg.c_str());
}

std::string AIPlayer2::sendWithReply(std::string msg,
		std::string match, int timeout)
{
	Request req;
	std::string r;
	std::mutex mutex;
	std::condition_variable cond;
	bool done = false;

	auto cb = [&r, &cond, &done](std::string reply){
		r = reply;
		log("%s %d (%s)", __func__, __LINE__, reply.c_str());
		done = true;
		cond.notify_one();
	};

	req.cmd = msg;
	req.reply = match;
	req.timeout = timeout;
	req.cb = cb;

	enqueueRequest(req);
	log("sendWithReply(cmd:%s reply:%s timeout:%d)",
			msg.c_str(), match.c_str(), timeout);

	std::unique_lock<std::mutex> lock(mutex);
	if (!done)
		cond.wait_for(lock, std::chrono::seconds(timeout));

	log("sendWithReply Got(%s)", r.c_str());

	return r;
}

void AIPlayer2::sendWithCallBack(std::string msg,
		std::string match, int timeout,
		RequestCallback &cb)
{
	Request req;

	RequestCallback _cb = [this, cb](std::string reply){
		log("sendWithCallBack Got(%s)", reply.c_str());

		CallFunc *callback = CallFunc::create([cb, reply]() {
				cb(reply);
				});
		runAction(callback);

	};

	req.cmd = msg;
	req.reply = match;
	req.timeout = timeout;
	req.cb = _cb;

	enqueueRequest(req);
	log("sendWithCallBack(cmd:%s reply:%s timeout:%d)",
			msg.c_str(), match.c_str(), timeout);
}

void AIPlayer2::ponder()
{
}

void AIPlayer2::go(float timeout)
{
	getEventDispatcher()->dispatchCustomEvent(EVENT_AIPLAYER_GO);
	_head->setActive(true);

	std::string cmd = std::string("position fen ") +
		getBoard()->getFenWithMove();
	sendWithoutReply(cmd);

	RequestCallback cb = [&](std::string reply){
		log("*** %s %d", __func__, __LINE__);
		if (reply.size() == 0) {
			log("no reply %s %d", __func__, __LINE__);
			return;
		}

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
	};

	int t = int(_difficulty*1000+timeout);
	cmd = "go time " + Utils::toString(t);
	sendWithCallBack(cmd, "bestmove", t/1000 + 1, cb);
}

void AIPlayer2::stop()
{
	write(_pipe[1], "W", 1);
	sendWithoutReply("stop");
	log("*** %s %d", __func__, __LINE__);
	sendWithReply("stop", "nobestmove");
	log("*** %s %d", __func__, __LINE__);
	_head->setActive(false);
    stopAllActions();
}

bool AIPlayer2::askForDraw()
{
	return true;
}
