#ifndef __AIPLAYER_H__
#define __AIPLAYER_H__

#include "cocos2d.h"
#include "Player.h"
#include "HeaderSprite.h"

USING_NS_CC;

class AIPlayer : public Player
{
public:
	virtual bool init();
	CREATE_FUNC(AIPlayer);

	virtual void ponder() override;
	virtual void go(float timeout) override;
	virtual void stop();

	virtual bool askForDraw();

	void setDifficulty(int level);
	void setName(std::string first, std::string second);

	virtual ~AIPlayer();

private:
	void eleeye_thread();
	int client_socketfd;
	int sendWithoutReply(std::string msg);
	std::string sendWithReply(std::string msg, std::string match);
	int sendWithCallBack(std::string msg, std::string match, const std::function<void (std::string)> &cb);

	int _difficulty;

	HeaderSprite *_head;
	std::mutex mutex;
	std::thread _thread;
};

#endif
