#ifndef __AIPLAYER2_H__
#define __AIPLAYER2_H__

#include "cocos2d.h"
#include "Player.h"
#include "HeaderSprite.h"

USING_NS_CC;

class AIPlayer2 : public Player
{
public:
	typedef std::function<void(std::string)> RequestCallback;
	struct Request
	{
		std::string cmd;
		std::string reply;
		int timeout;
		RequestCallback cb;
	};

	bool init();
	CREATE_FUNC(AIPlayer2);
	void ponder();
	void go(float timeout);
	void stop();
	bool askForDraw();
	void setDifficulty(int level);
	void setName(std::string first, std::string second);
	virtual ~AIPlayer2();

private:
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
	std::mutex _mutex;
	std::condition_variable _condition;
	std::queue<Request> _queue;

	std::thread _cmdThread;
	std::thread _aiThread;

	int _difficulty;
	HeaderSprite *_head;
	int _pipe[2];
};

#endif
