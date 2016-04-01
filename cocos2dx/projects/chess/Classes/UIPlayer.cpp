#include "UIPlayer.h"

bool UIPlayer::init()
{
	if (!Player::init())
			return false;

	_selectedPiece = nullptr;

	_touchListener = EventListenerTouchOneByOne::create();
	_touchListener->onTouchBegan = CC_CALLBACK_2(UIPlayer::onTouchBegan, this);
	_touchListener->onTouchMoved = CC_CALLBACK_2(UIPlayer::onTouchMoved, this);
	_touchListener->onTouchEnded = CC_CALLBACK_2(UIPlayer::onTouchEnded, this);
	_touchListener->onTouchCancelled = CC_CALLBACK_2(UIPlayer::onTouchCancelled, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_touchListener, this);

	_touchListener->setEnabled(false);

	return true;
}

bool UIPlayer::onTouchBegan(Touch *touch, Event *event) {
	Vec2 touchPos = touch->getLocation();

	Rect boardRect = getBoard()->getBoundingBox();

	if (!boardRect.containsPoint(touchPos)) {
		return false;
	}

	Vec2 index = getBoard()->convertWorldCoordToIndex(touchPos);

	Piece *p = getBoard()->pick(index);
	if (_selectedPiece != nullptr) {
		/* move piece */
		Vec2 src = getBoard()->convertLocalCoordToIndex(
				_selectedPiece->getPosition());
		Vec2 dst = index;
		auto srcSide = _selectedPiece->getSide();

		if (p != nullptr && srcSide == p->getSide()) {
			/* select other piece */
			getBoard()->unselect(src);
			getBoard()->select(dst);
			_selectedPiece = p;
		} else {
			/* move piece */
			int status = getBoard()->move(src, dst);
			_selectedPiece = nullptr;

			if (status < 0) {
				;
			} else {
				getListener()->onMoved("TODO");
				_touchListener->setEnabled(false);
			}
		}
	} else {
		/* select piece */
		getBoard()->select(index);
		_selectedPiece = p;
		if (p != nullptr && getSide() != p->getSide()) {
			/* cannot select other side's piece */
			_selectedPiece = nullptr;
		}
	}

	return true;
}

void UIPlayer::onTouchMoved(Touch *touch, Event *event) {
}

void UIPlayer::onTouchEnded(Touch *touch, Event *event) {
	if (_selectedPiece == nullptr) {
		Vec2 touchPos = touch->getStartLocation();
		Vec2 index = getBoard()->convertWorldCoordToIndex(touchPos);
		getBoard()->unselect(index);
	}
}

void UIPlayer::onTouchCancelled(Touch *touch, Event *event) {
}

void UIPlayer::ponder()
{
}

void UIPlayer::go(float timeout)
{
	_touchListener->setEnabled(true);
}
