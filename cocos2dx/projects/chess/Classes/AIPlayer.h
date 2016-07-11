#ifndef __AIPLAYER_H__
#define __AIPLAYER_H__

#include "cocos2d.h"
#include "Player.h"
#include "Utils.h"
#include <memory>  // for std::shared_ptr
#include <atomic>

USING_NS_CC;

class AIPlayer : public Player
{
public:
	bool init() override;
	CREATE_FUNC(AIPlayer);
	void start(std::string fen) override;
	void stop() override;
	bool onRequest(std::string req) override;

	void setLevel(int level) { _level = level; }
	virtual ~AIPlayer();

private:
	typedef std::function<void(std::string)> RequestCallback;
	struct Request
	{
		std::string cmd;
		std::string reply;
		int timeout;
		RequestCallback cb;
	};

	void enqueueRequest(Request req);
	void sendWithoutReply(std::string msg);
	std::string sendWithReply(std::string msg, std::string match,
			int timeout=2);
	void sendWithCallBack(std::string msg, std::string match, int timeout,
		RequestCallback &cb);

	void AI_Thread();
	void CMD_Thread();

	int _sockfd;
	FILE *_sockfile;

	bool _stop;
    std::shared_ptr<std::atomic<bool>> _isDestroyed;
	std::mutex _mutex;
	std::condition_variable _condition;
	std::queue<Request> _queue;

	std::thread _cmdThread;
	std::thread _aiThread;

	int _level;
	int _pipe[2];
};

#endif
