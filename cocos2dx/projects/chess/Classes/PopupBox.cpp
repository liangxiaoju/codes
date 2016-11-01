#include "PopupBox.h"
#include "Localization.h"

PopupBox::PopupBox()
{
}

PopupBox::~PopupBox()
{
}

PopupBox* PopupBox::create()
{
    PopupBox* widget = new (std::nothrow) PopupBox();
    if (widget && widget->init()) {
        widget->autorelease();
        return widget;
    }
    CC_SAFE_DELETE(widget);
    return nullptr;
}

bool PopupBox::init()
{
    if (!LayerColor::initWithColor(Color4B(0, 0, 0, 0)))
        return false;

    Size vsize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(PopupBox::onTouchBegan, this);
    listener->setSwallowTouches(true);
    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

    auto maskLayer = LayerColor::create(Color4B(0, 0, 0, 0));
    addChild(maskLayer);
    maskLayer->runAction(FadeTo::create(0.25, 255*0.6));

    _layout = VBox::create();
    _layout->setBackGroundImage("common/B2popupbox9_66x66_294x194_2@3x.png");
    _layout->setBackGroundImageScale9Enabled(true);
    _layout->setBackGroundImageCapInsets(Rect(66, 66, 294-66, 194-66));
    _layout->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    _layout->setPosition(Vec2(origin.x+vsize.width/2, origin.y+vsize.height/2));
    _layout->setContentSize(Size(0, 0));
    addChild(_layout);

    return true;
}

void PopupBox::addElement(Widget *widget)
{
    LayoutParameter *param = widget->getLayoutParameter();
    Margin margin = Margin(0, 0, 0, 0);
    if (param)
        margin = param->getMargin();

    auto wsize = widget->getContentSize();
    auto size = Size(wsize.width+margin.left+margin.right,
            wsize.height+margin.top+margin.bottom);
    auto lsize = _layout->getContentSize();
    auto width = std::max(size.width, lsize.width);
    auto height = size.height + lsize.height;

    _layout->setContentSize(Size(width, height));
    _layout->addChild(widget);
}

bool PopupBox::onTouchBegan(Touch *touch, Event *event)
{
    return true;
}

Rect PopupBox::getInnerBoundingBox()
{
    return _layout->getBoundingBox();
}

MenuBox *MenuBox::create(std::vector<std::string> menus,
        std::function<void(int index)> cb)
{
    auto pRet = new (std::nothrow) MenuBox();
    if (pRet && pRet->init(menus, cb)) {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return nullptr;
}

bool MenuBox::init(std::vector<std::string> menus,
        std::function<void(int index)> cb)
{
    if (!PopupBox::init())
        return false;

    LinearLayoutParameter *param;

    auto callback = [this, cb](Ref *ref) {
        auto button = dynamic_cast<Button*>(ref);
        int index = button->getTag();

        auto hide = Hide::create();
        auto delay = DelayTime::create(0.1);
        auto callfunc = CallFunc::create([cb, index]() {cb(index);});
        auto remove = RemoveSelf::create();
        auto seq = Sequence::create(hide, delay, callfunc, remove, nullptr);
        runAction(seq);
    };

    for (int i = 0; i < menus.size(); i++) {
        auto b = Button::create("common/button3.png");
        b->setTitleText(menus[i]);
        b->setTitleFontSize(60);
        b->setTitleColor(Color3B(240, 213, 172));
        b->setTag(i);
        b->addClickEventListener(callback);

        param = LinearLayoutParameter::create();
        if (i == 0)
            param->setMargin(Margin(40, 50, 40, 0));
        else
            param->setMargin(Margin(40, 0, 40, 0));
        b->setLayoutParameter(param);
        addElement(b);
    }

    auto b1 = Button::create("common/button1.png");
    b1->setTitleText(TR("continue game"));
    b1->setTitleFontSize(60);
    b1->setTitleColor(Color3B(240, 213, 172));
    b1->addClickEventListener([this](Ref *ref) { removeFromParent(); });

    auto b2 = Button::create("common/button2.png");
    b2->setTitleText(TR("back to home"));
    b2->setTitleFontSize(60);
    b2->setTitleColor(Color3B(240, 213, 172));
    b2->addClickEventListener([](Ref *ref) {
            Director::getInstance()->popScene();
            });

    auto bsize = b1->getContentSize();
    auto hbox = HBox::create(Size(bsize.width*2+84, bsize.height));
    param = LinearLayoutParameter::create();
    param->setMargin(Margin(60, 0, 60, 0));
    hbox->setLayoutParameter(param);

    param = LinearLayoutParameter::create();
    param->setGravity(LinearLayoutParameter::LinearGravity::LEFT);
    param->setMargin(Margin(0.0f, 0.0f, 0.0f, 0.0f));
    b2->setLayoutParameter(param->clone());

    param->setGravity(LinearLayoutParameter::LinearGravity::RIGHT);
    param->setMargin(Margin(84, 0.0f, 0.0f, 0.0f));
    b1->setLayoutParameter(param->clone());

    hbox->addChild(b2);
    hbox->addChild(b1);
    addElement(hbox);

    Director::getInstance()->getRunningScene()->addChild(this);

    return true;
}

bool MenuBox::onTouchBegan(Touch *touch, Event *event)
{
    Vec2 touchPos = touch->getLocation();
    Rect boundRect = getInnerBoundingBox();

    if (!boundRect.containsPoint(touchPos)) {
        removeFromParent();
    }

    return true;
}

DialogBox* DialogBox::create(Text *text, Button *positive, Button *negative)
{
    DialogBox* widget = new (std::nothrow) DialogBox();
    if (widget && widget->init(text, positive, negative)) {
        widget->autorelease();
        return widget;
    }
    CC_SAFE_DELETE(widget);
    return nullptr;
}

DialogBox* DialogBox::create(std::string text,
                             std::string positive,
                             std::string negative,
                             std::function<void(bool positive)> cb)
{
    DialogBox* widget = new (std::nothrow) DialogBox();
    if (widget && widget->init(text, positive, negative, cb)) {
        widget->autorelease();
        return widget;
    }
    CC_SAFE_DELETE(widget);
    return nullptr;
}

bool DialogBox::init(Text *text, Button *positive, Button *negative)
{
    if (!PopupBox::init())
        return false;

    LinearLayoutParameter *param;

    param = LinearLayoutParameter::create();
    param->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
    param->setMargin(Margin(0, 200, 0, 240-82));
    text->setLayoutParameter(param);
    addElement(text);

    auto bsize = positive->getContentSize();
    auto hbox = HBox::create(Size(bsize.width*2+84, bsize.height));
    param = LinearLayoutParameter::create();
    //param->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
    param->setMargin(Margin(60, 0, 60, 0));
    hbox->setLayoutParameter(param);

    param = LinearLayoutParameter::create();
    param->setGravity(LinearLayoutParameter::LinearGravity::LEFT);
    param->setMargin(Margin(0.0f, 0.0f, 0.0f, 0.0f));
    positive->setLayoutParameter(param->clone());

    param->setGravity(LinearLayoutParameter::LinearGravity::RIGHT);
    param->setMargin(Margin(84, 0.0f, 0.0f, 0.0f));
    negative->setLayoutParameter(param->clone());

    hbox->addChild(positive);
    hbox->addChild(negative);
    addElement(hbox);

    return true;
}

bool DialogBox::init(std::string text,
                     std::string positive,
                     std::string negative,
                     std::function<void(bool positive)> cb)
{
    auto t = Text::create(text, "", 60);
    t->setTextColor(Color4B(240, 213, 172, 255));

    auto p = Button::create("common/button2.png");
    p->setTitleText(positive);
    p->setTitleFontSize(60);
    p->setTitleColor(Color3B(240, 213, 172));
    p->addClickEventListener([this, cb](Ref *ref){
            cb(true);
            removeFromParent();
        });

    auto n = Button::create("common/button2.png");
    n->setTitleText(negative);
    n->setTitleFontSize(60);
    n->setTitleColor(Color3B(240, 213, 172));
    n->addClickEventListener([this, cb](Ref *ref){
            cb(false);
            removeFromParent();
        });

    if (!init(t, p, n))
        return false;

    Director::getInstance()->getRunningScene()->addChild(this);

    return true;
}

PopupMessage* PopupMessage::create(std::string text, std::function<void(void)>cb)
{
    PopupMessage* widget = new (std::nothrow) PopupMessage();
    if (widget && widget->init(text, cb)) {
        widget->autorelease();
        return widget;
    }
    CC_SAFE_DELETE(widget);
    return nullptr;
}

bool PopupMessage::init(std::string text, std::function<void(void)>cb)
{
    if (!PopupBox::init())
        return false;

    LinearLayoutParameter *param;

    auto t = Text::create(text, "", 60);
    t->setTextColor(Color4B(240, 213, 172, 255));

    param = LinearLayoutParameter::create();
    param->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
    param->setMargin(Margin(0, 200, 0, 240-82));
    t->setLayoutParameter(param);

    auto button = Button::create("common/button2.png");
    button->setTitleText(TR("OK"));
    button->setTitleFontSize(60);
    button->setTitleColor(Color3B(240, 213, 172));
    button->addClickEventListener([this, cb](Ref *ref) {
            if (cb != nullptr)
                cb();
            removeFromParent();
        });

    param = LinearLayoutParameter::create();
    param->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
    param->setMargin(Margin(60, 0, 60, 0));
    button->setLayoutParameter(param);

    addElement(t);
    addElement(button);

    Director::getInstance()->getRunningScene()->addChild(this);

    return true;
}

PopupMenu *PopupMenu::create(std::vector<std::string> vec,
                             std::function<void(int index)>cb)
{
    PopupMenu* widget = new (std::nothrow) PopupMenu();
    if (widget && widget->init(vec, cb)) {
        widget->autorelease();
        return widget;
    }
    CC_SAFE_DELETE(widget);
    return nullptr;
}

bool PopupMenu::init(std::vector<std::string> vec,
                     std::function<void(int index)>cb)
{
    if (!PopupBox::init())
        return false;

    Size winSize = Director::getInstance()->getWinSize();

    auto listview = ListView::create();
    listview->setDirection(ListView::Direction::VERTICAL);
    listview->setBounceEnabled(true);
    listview->setScrollBarEnabled(false);
    listview->setGravity(ListView::Gravity::CENTER_HORIZONTAL);
    listview->setItemsMargin(50);
    listview->addEventListener([this, cb](Ref *pSender, ListView::EventType type) {
            ListView *listview = static_cast<ListView*>(pSender);
            switch (type) {
            case ListView::EventType::ON_SELECTED_ITEM_START: {
                log("select start index = %d", listview->getCurSelectedIndex());
                break;
            }
            case ListView::EventType::ON_SELECTED_ITEM_END: {
                log("select end index = %d", listview->getCurSelectedIndex());
                int index = listview->getCurSelectedIndex();
                //somehow we cannot remove listview in its event callback
                getScheduler()->performFunctionInCocosThread([this, cb, index]() {
                        this->removeFromParent();
                        cb(index);
                    });
                break;
            }
            default:
                break;
            }
        });

    float width=0, height=0;
    for (auto &str : vec) {
        auto button = Button::create("button.png");
        button->setZoomScale(0.1);
        button->setTitleFontSize(35);
        button->setTitleFontName("");
        button->setTitleText(str);
        listview->pushBackCustomItem(button);

        width = std::max(width, button->getContentSize().width);
        height += button->getContentSize().height;
    }
    height += (listview->getItems().size() - 1) * listview->getItemsMargin();
    height = std::min(height, winSize.height/2);
    listview->setContentSize(Size(width, height));

    auto param = LinearLayoutParameter::create();
    param->setMargin(Margin(50, 50, 50, 50));
    listview->setLayoutParameter(param);

    addElement(listview);

    Director::getInstance()->getRunningScene()->addChild(this);

    return true;
}
