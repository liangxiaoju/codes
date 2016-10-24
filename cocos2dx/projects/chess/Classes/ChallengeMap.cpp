#include "ChallengeMap.h"
#include "Utils.h"

bool ChallengeMap::init(int total, int progress)
{
    if (!Layout::init())
        return false;

    _clickEventCallback = nullptr;
    _currentFlag = nullptr;

    auto vsize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto image = ImageView::create("challenge_map.png");
    auto size = image->getContentSize();
    image->setAnchorPoint(Vec2(0, 0));

    int i;
    float y = 0;
    for (i = 0; i < (total+(6-1))/6; i++) {
        auto clone = image->clone();
        clone->setPosition(Vec2(0, y));
        y += size.height;
        addChild(clone);
    }

    int num = i;
    setContentSize(Size(size.width, size.height*num));

    float w = 694;
    float h = 587;

    for (i = 0; i < num; i++) {
        float x = 198;
        float y = 387 + i*size.height;
        _positions.push_back(Vec2(x, y));
        _positions.push_back(Vec2(x+w, y+h));
        _positions.push_back(Vec2(x, y+h*2));
        _positions.push_back(Vec2(x+w, y+h*3));
        _positions.push_back(Vec2(x, y+h*4));
        _positions.push_back(Vec2(x+w, y+h*5));
    }

    _total = total;
    setProgress(progress);

    return true;
}

void ChallengeMap::setProgress(int progress)
{
    if (_currentFlag)
        removeChild(_currentFlag);

    for (auto flag : _passFlags)
        removeChild(flag);

    auto addText = [this](int i) {
        std::string str = std::string("No.") + Utils::toString(i+1);
        auto text = Text::create(str, "", 35);
        text->setTextColor(Color4B::BLACK);
        text->setPosition(Vec2(_positions[i].x, _positions[i].y-90));
        addChild(text);
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
        addChild(flag);
        flag->setTag(i);
        flag->addClickEventListener(callback);
        addText(i);

        _passFlags.push_back(flag);
    }

    if (progress < _total) {
        auto flag = Button::create("break_flag_red.png");
        flag->setPosition(_positions[progress]);
        addChild(flag);
        flag->setTag(progress);
        flag->addClickEventListener(callback);
        flag->setName("focus");
        addText(progress);

        _currentFlag = flag;
    }

    _progress = progress;
}

void ChallengeMap::addClickEventListener(
        const std::function<void(int)> &callback)
{
    _clickEventCallback = callback;
}

