#include "UIPlayer.h"
#include "HeaderSprite.h"
#include "Sound.h"
#include "PopupBox.h"

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
    _forceStop = false;

	/* response for regret */
	auto regret_cb = [this](EventCustom* ev) {
		_delegate->onRegretRequest();
	};
	/* response for resign */
	auto resign_cb = [this](EventCustom* ev) {
		_delegate->onResignRequest();
	};
	/* response for draw */
	auto draw_cb = [this](EventCustom* ev) {
		_delegate->onDrawRequest();
	};
    auto request_deny_cb = [this](EventCustom* ev) {
        std::string event = (const char *)ev->getUserData();
        if (event.find("DRAW:") != std::string::npos) {
            auto cb = [this](bool positive) {};
            auto box = DialogBox::create("Disagree with draw !", "OK", "OK", cb);
            getScene()->addChild(box);
            log("request deny: draw");
        } else if (event.find("REGRET:") != std::string::npos) {

        }
    };

	setOnEnterCallback([this, regret_cb, resign_cb, draw_cb, request_deny_cb](){
		getEventDispatcher()->addCustomEventListener(EVENT_REGRET, regret_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_RESIGN, resign_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_DRAW, draw_cb);
		getEventDispatcher()->addCustomEventListener(EVENT_REQUEST_DENY, request_deny_cb);
	});

	setOnExitCallback([this](){
		getEventDispatcher()->removeCustomEventListeners(EVENT_REGRET);
		getEventDispatcher()->removeCustomEventListeners(EVENT_RESIGN);
		getEventDispatcher()->removeCustomEventListeners(EVENT_DRAW);
		getEventDispatcher()->removeCustomEventListeners(EVENT_REQUEST_DENY);
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
    if (!_forceStop)
        _touchListener->setEnabled(true);
}

void UIPlayer::stop()
{
	_touchListener->setEnabled(false);
}

void UIPlayer::forceStop(bool stop)
{
    _forceStop = stop;
    if (stop)
        this->stop();
}

void UIPlayer::onRequest(std::string req, std::string args,
                         std::function<void(bool)>callback)
{
	if (req == "draw") {
        //popup
        auto cb = [callback](bool positive) {
            callback(positive);
        };
        auto box = DialogBox::create("Accept draw ?", "Yes", "No", cb);
        addChild(box);
	} else if (req == "regret") {
        callback(true);
    } else if (req == "resign") {
        //popup
        callback(true);
    } else if (req == "move") {
        _delegate->onMoveRequest(args);
    }

    if (callback)
        callback(false);
}

