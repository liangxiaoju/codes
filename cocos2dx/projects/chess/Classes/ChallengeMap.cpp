#include "ChallengeMap.h"
#include "Utils.h"

bool ChallengeMap::init(int total)
{
    if (!ScrollView::init())
        return false;

    _clickEventCallback = nullptr;

    auto vsize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto image = ImageView::create("challenge_map.png");
    auto size = image->getContentSize();
    image->setScale(vsize.width/size.width);
    size = size * image->getScale();
    image->setAnchorPoint(Vec2(0, 0));

    _innerLayout = Layout::create();

    int i;
    float y = 0;
    for (i = 0; i < (total+(6-1))/6; i++) {
        auto clone = image->clone();
        clone->setPosition(Vec2(0, y));
        y += size.height;
        _innerLayout->addChild(clone);
    }
    int num = i;
    _innerLayout->setContentSize(Size(size.width, size.height*num));

    float w = 694 * image->getScale();
    float h = 587 * image->getScale();

    for (i = 0; i < num; i++) {
        float x = 198 * image->getScale();
        float y = 387 * image->getScale() + i*size.height;
        _positions.push_back(Vec2(x, y));
        _positions.push_back(Vec2(x+w, y+h));
        _positions.push_back(Vec2(x, y+h*2));
        _positions.push_back(Vec2(x+w, y+h*3));
        _positions.push_back(Vec2(x, y+h*4));
        _positions.push_back(Vec2(x+w, y+h*5));
    }

    _total = total;

    this->setScrollBarEnabled(false);
    this->setBounceEnabled(false);
    this->setDirection(ScrollView::Direction::VERTICAL);
    this->setContentSize(vsize);
    this->setPosition(Vec2(0, 0));

    this->setInnerContainerSize(_innerLayout->getContentSize());
    this->addChild(_innerLayout);
    this->jumpToBottom();

    return true;
}

void ChallengeMap::setProgress(int progress)
{
    if (progress < 0 || progress > _total)
        return;

    for (auto flag : _flags)
        _innerLayout->removeChild(flag);
    _flags.clear();

    auto addText = [this](int i) {
        std::string str = std::string("No.") + Utils::toString(i+1);
        auto text = Text::create(str, "", 30);
        text->setTextColor(Color4B::BLACK);
        text->setPosition(Vec2(_positions[i].x, _positions[i].y-90));
        _innerLayout->addChild(text);
    };

    auto callback = [this](Ref *ref) {
        auto flag = dynamic_cast<Button*>(ref);
        int tag = flag->getTag();

        if (_clickEventCallback)
            _clickEventCallback(tag);
    };

    for (int i = 0; i < progress; i++) {
        auto flag = Button::create("break_flag_lv.png");
        flag->setPosition(_positions[i]);
        _innerLayout->addChild(flag);
        flag->setTag(i);
        flag->addClickEventListener(callback);
        addText(i);

        _flags.push_back(flag);
    }

    if (progress < _total) {
        auto flag = Button::create("break_flag_red.png");
        flag->setPosition(_positions[progress]);
        _innerLayout->addChild(flag);
        flag->setTag(progress);
        flag->addClickEventListener(callback);
        addText(progress);

        _flags.push_back(flag);
    }

    _progress = progress;
}

void ChallengeMap::addClickEventListener(
        const std::function<void(int)> &callback)
{
    _clickEventCallback = callback;
}

void ChallengeMap::jumpToProgress(int progress)
{
    auto vsize = Director::getInstance()->getVisibleSize();

    if (progress >= 0 && progress <= _total) {
        auto pos = _positions[progress];
        auto size = _innerLayout->getContentSize();
        float percent = 100 -
            100*(pos.y-vsize.height/2)/(size.height-vsize.height);
        percent = std::max(percent, 0.0f);
        percent = std::min(percent, 100.0f);
        this->jumpToPercentVertical(percent);
    } else {
        this->jumpToBottom();
    }
}
