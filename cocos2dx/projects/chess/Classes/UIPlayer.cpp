#include "UIPlayer.h"
#include "HeaderSprite.h"
#include "Sound.h"

bool UIPlayer::init()
{
	if (!Player::init("UI"))
			return false;

	_selectedPiece = nullptr;

	_touchListener = EventListenerTouchOneByOne::create();
	_touchListener->onTouchBegan = CC_CALLBACK_2(UIPlayer::onTouchBegan, this);
	_touchListener->onTouchMoved = CC_CALLBACK_2(UIPlayer::onTouchMoved, this);
	_touchListener->onTouchEnded = CC_CALLBACK_2(UIPlayer::onTouchEnded, this);
	_touchListener->onTouchCancelled = CC_CALLBACK_2(UIPlayer::onTouchCancelled, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_touchListener, this);

	_touchListener->setEnabled(false);

	/* response for regret */
	auto regret_cb = [this](EventCustom* ev) {
		_delegate->onRegretRequest();
	};
	/* response for resign */
	auto resign_cb = [this](EventCustom* ev) {
		_delegate->onResignRequest();
	};
	/* response for draw*/
	auto draw_cb = [this](EventCustom* ev) {
		_delegate->onDrawRequest();
	};

	setOnEnterCallback([this, regret_cb, resign_cb, draw_cb](){
		getEventDispatcher()->addCustomEventListener(EVENT_REGRET, regret_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_RESIGN, resign_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_DRAW, draw_cb);
	});

	setOnExitCallback([this](){
		getEventDispatcher()->removeCustomEventListeners(EVENT_REGRET);
		getEventDispatcher()->removeCustomEventListeners(EVENT_RESIGN);
		getEventDispatcher()->removeCustomEventListeners(EVENT_DRAW);
	});

	return true;
}

bool UIPlayer::onTouchBegan(Touch *touch, Event *event) {
	Vec2 touchPos = touch->getLocation();

	Rect boardRect = _board->getBoundingBox();

	if (!boardRect.containsPoint(touchPos)) {
		return false;
	}

	Vec2 index = _board->convertWorldCoordToIndex(touchPos);

	Piece *p = _board->pick(index);
	if (_selectedPiece != nullptr) {
		/* move piece */
		Vec2 src = _board->convertLocalCoordToIndex(
				_selectedPiece->getPosition());
		Vec2 dst = index;
		auto srcSide = _selectedPiece->getSide();

		if (p != nullptr && srcSide == p->getSide()) {
			/* select other piece */
			_board->unselect(src);
			_board->select(dst);
			_selectedPiece = p;
		} else {
			/* move piece */
			_selectedPiece = nullptr;
			std::string mv = Utils::toUcciMove(src, dst);
			_delegate->onMoveRequest(mv);
		}
	} else {
		/* select piece */
		_board->select(index);
		_selectedPiece = p;
		if (p != nullptr && _board->getCurrentSide() != p->getSide()) {
			/* cannot select other side's piece */
			_selectedPiece = nullptr;
		}
	}

    if (_selectedPiece != nullptr)
        Sound::getInstance()->playEffect("click");

	return true;
}

void UIPlayer::onTouchMoved(Touch *touch, Event *event) {
}

void UIPlayer::onTouchEnded(Touch *touch, Event *event) {
	if (_selectedPiece == nullptr) {
		Vec2 touchPos = touch->getStartLocation();
		Vec2 index = _board->convertWorldCoordToIndex(touchPos);
		_board->unselect(index);
	}
}

void UIPlayer::onTouchCancelled(Touch *touch, Event *event) {
}

void UIPlayer::start(std::string fen)
{
	_touchListener->setEnabled(true);
}

void UIPlayer::stop()
{
	_touchListener->setEnabled(false);
}

bool UIPlayer::onRequest(std::string req)
{
	if (req == "draw") {
		return true;
	} else if (req == "regret") {
		return true;
	}

	return false;
}

