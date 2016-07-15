#include "AIPlayer.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

//#define USE_SOCKET

extern void AIEngine();
int serverReadFD;
int serverWriteFD;

bool AIPlayer::init()
{
    int pipefd[2];

	if (!Player::init("AI"))
		return false;

	_stop = false;
	_isDestroyed = std::make_shared<std::atomic<bool>>(false);

    pipe(pipefd);
    _wakeReadFD = pipefd[0];
    _wakeWriteFD = pipefd[1];

#ifndef USE_SOCKET
    pipe(pipefd);
    _AIReadFD = pipefd[0];
    serverWriteFD = pipefd[1];

    pipe(pipefd);
    _AIWriteFD = pipefd[1];
    serverReadFD = pipefd[0];
#endif

	_aiThread = std::thread(std::bind(&AIPlayer::AI_Thread, this));
	_cmdThread = std::thread(std::bind(&AIPlayer::CMD_Thread, this));

#ifdef USE_SOCKET
	int i = 0;
    int sockfd;
	while (i++ < 10) {
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(6669);

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			log("Failed to create socket");
			continue;
		}
		if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			log("Failed to connect, retry %d", i);
			close(sockfd);
			sockfd = -1;
			usleep(200*1000);
		} else
			break;
	}

	if (sockfd < 0)
		return false;

    _AIReadFD = _AIWriteFD = sockfd;
#endif

    _AIReadFile = fdopen(_AIReadFD, "r");
    _AIWriteFile = fdopen(_AIWriteFD, "w");
	setbuf(_AIReadFile, NULL);
	setbuf(_AIWriteFile, NULL);

	std::string reply;
	reply = sendWithReply("ucci", "ucciok");
	reply = sendWithReply("isready", "readyok");

	return true;
}

AIPlayer::~AIPlayer()
{
	sendWithoutReply("stop");
	sendWithReply("stop", "nobestmove");
	sendWithReply("quit", "bye");
	_aiThread.join();

	_stop = true;
	*_isDestroyed = true;

	fclose(_AIReadFile);
    fclose(_AIWriteFile);
	_condition.notify_all();
	_cmdThread.join();

	close(_wakeReadFD);
	close(_wakeWriteFD);

	log("~AIPlayer");
}

void AIPlayer::AI_Thread()
{
#ifdef USE_SOCKET
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

	serverReadFD = serverWriteFD = newsockfd;
#endif

	log("Eleeye_Thread start.");

	AIEngine();

	log("Eleeye_Thread exit.");
}

void AIPlayer::CMD_Thread()
{
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
		FD_SET(_AIReadFD, &fdset);
		struct timeval timeout = {0, 0};

		if (select(_AIReadFD+1, &fdset, NULL, NULL, &timeout) > 0) {
			char tmp[1024];
			read(_AIReadFD, tmp, sizeof(tmp));
		}

		FD_ZERO(&fdset);
		FD_SET(_wakeReadFD, &fdset);
		memset(&timeout, 0, sizeof(timeout));

		if (select(_wakeReadFD+1, &fdset, NULL, NULL, &timeout) > 0) {
			char c[32];
			read(_wakeReadFD, c, sizeof(c));
		}

		if (request.cmd.size() > 0) {
			fprintf(_AIWriteFile, "%s\n", request.cmd.c_str());
			fflush(_AIWriteFile);
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
				FD_SET(_AIReadFD, &fdset);
				FD_SET(_wakeReadFD, &fdset);
				int n = select(std::max(_AIReadFD, _wakeReadFD)+1,
						&fdset, NULL, NULL, &timeout);
				if (_stop)
					return;
				if (n == 0) {
					log("select timeout");
					break;
				}
				if (FD_ISSET(_wakeReadFD, &fdset)) {
					char c[32];
					read(_wakeReadFD, c, sizeof(c));
					break;
				}
				if (FD_ISSET(_AIReadFD, &fdset)) {
					if (fgets(line, len, _AIReadFile) == NULL)
						continue;

					if (strstr(line, request.reply.c_str()) != nullptr) {
						break;
					}
				}

				memset(line, 0, len);

			} while (1);

			std::string reply(line);
			free(line);
            if (reply.back() == '\n')
                reply.pop_back();
			request.cb(reply);
		}

	} while(1);
}

void AIPlayer::enqueueRequest(Request req)
{
	std::unique_lock<std::mutex> lock(_mutex);
	_queue.emplace(req);
	_condition.notify_one();
}

void AIPlayer::sendWithoutReply(std::string msg)
{
	Request req;
	req.cmd = msg;
	req.timeout = 0;
	enqueueRequest(req);
	log("sendWithoutReply: %s", msg.c_str());
}

std::string AIPlayer::sendWithReply(std::string msg,
		std::string match, int timeout)
{
	Request req;
	std::string r;
	std::mutex mutex;
	std::condition_variable cond;
	bool done = false;

	auto cb = [&r, &cond, &done, &mutex](std::string reply){
        std::unique_lock<std::mutex> lock(mutex);
		r = reply;
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

void AIPlayer::sendWithCallBack(std::string msg,
		std::string match, int timeout,
		RequestCallback &cb)
{
	Request req;

	RequestCallback _cb = [this, cb](std::string reply){
		log("sendWithCallBack Got(%s)", reply.c_str());

        std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;
		getScheduler()->performFunctionInCocosThread([cb, reply, isDestroyed]() {
			if (*isDestroyed) {
				log("AIPlayer instance was destroyed!");
			} else {
				cb(reply);
			}
		});

	};

	req.cmd = msg;
	req.reply = match;
	req.timeout = timeout;
	req.cb = _cb;

	enqueueRequest(req);
	log("sendWithCallBack(cmd:%s reply:%s timeout:%d)",
			msg.c_str(), match.c_str(), timeout);
}

void AIPlayer::start(std::string fen)
{
	std::string cmd = std::string("position fen ") + fen;
	sendWithoutReply(cmd);

	RequestCallback cb = [&](std::string reply){
		if (reply.size() == 0) {
			return;
		}

		if (reply.find("nobestmove") != std::string::npos) {
				_delegate->onResignRequest();
		/*
		} else if (reply.find("draw") != std::string::npos) {
				_delegate->onDrawRequest();
		*/
		} else {
			auto substr = Utils::splitString(reply, ' ');
			auto mv = substr[1];
			_delegate->onMoveRequest(mv);
		}
	};

	int t = int(_level*1000);
	cmd = "go time " + Utils::toString(t);
	sendWithCallBack(cmd, "bestmove", t/1000 + 1, cb);
}

void AIPlayer::stop()
{
	write(_wakeWriteFD, "W", 1);
	sendWithoutReply("stop");
	sendWithReply("stop", "nobestmove");
}

bool AIPlayer::onRequest(std::string req)
{
	if (req == "draw") {
		return true;
	} else if (req == "regret") {
		return true;
	}

	return false;
}
