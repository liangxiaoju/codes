#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "cocos2d.h"

USING_NS_CC;

class Player : public Node
{
public:
	virtual bool init(const std::string name)
	{
		if (!Node::init())
			return false;

		setAnchorPoint(Vec2::ANCHOR_MIDDLE);

		_name = name;

		return true;
	}

	std::string getName() { return _name; }

	virtual void start(std::string fen) = 0;

	virtual void stop() = 0;

	virtual bool onRequest(std::string req) = 0;

	class Delegate
	{
	public:
		typedef std::function<void(std::string)> ccMoveRequestCallback;
		typedef std::function<void()> ccResignRequestCallback;
		typedef std::function<void()> ccDrawRequestCallback;
		typedef std::function<void()> ccRegretRequestCallback;

		ccMoveRequestCallback onMoveRequest;
		ccResignRequestCallback onResignRequest;
		ccDrawRequestCallback onDrawRequest;
		ccRegretRequestCallback onRegretRequest;
	};

	void setDelegate(Delegate *delegate) { _delegate = delegate; }

protected:
	std::string _name;
	Delegate *_delegate;
};

#endif
