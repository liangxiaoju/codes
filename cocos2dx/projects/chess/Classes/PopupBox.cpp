#include "PopupBox.h"
#include "Localization.h"

PopupBox::PopupBox()
{
   _default_parameter = LinearLayoutParameter::create();
    _default_parameter->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
    _default_parameter->setMargin(Margin(0.0f, 50.0f, 0.0f, 0.0f));
    CC_SAFE_RETAIN(_default_parameter);
}

PopupBox::~PopupBox()
{
    CC_SAFE_RELEASE(_default_parameter);
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

    Size winSize = Director::getInstance()->getWinSize();

    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(PopupBox::onTouchBegan, this);
    listener->setSwallowTouches(true);
    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

    auto maskLayer = LayerColor::create(Color4B(0, 0, 0, 0));
    addChild(maskLayer);
    maskLayer->runAction(FadeTo::create(0.15, 90));

    _layout = VBox::create();
    _layout->setBackGroundImage("pop_bg.png");
    _layout->setBackGroundImageScale9Enabled(true);
    _layout->setBackGroundImageCapInsets(Rect(15, 15, 565-30, 199-30));
    _layout->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    _layout->setPosition(Vec2(winSize.width/2, winSize.height/2));
    _layout->setContentSize(Size(100, 50));
    addChild(_layout);

    setOnEnterCallback([this]() {
            _layout->setScale(0.8f);
            auto s1 = ScaleTo::create(0.1f, 1.1f);
            auto s2 = ScaleTo::create(0.07f, 0.9f);
            auto s3 = ScaleTo::create(0.05f, 1.0f);
            Sequence *seq = Sequence::create(s1, s2, s3, nullptr);
            _layout->runAction(seq);
        });

    return true;
}

bool PopupBox::onTouchBegan(Touch *touch, Event *event)
{
    return true;
}

Rect PopupBox::getInnerBoundingBox()
{
    return _layout->getBoundingBox();
}

void PopupBox::pushBackView(Widget* child)
{
    Widget* widget = dynamic_cast<Widget*>(child);
    widget->setLayoutParameter(_default_parameter->clone());
    _layout->addChild(child);

    Size vsize = Director::getInstance()->getVisibleSize();
    Size wsize = widget->getContentSize();
    Size lsize = _layout->getContentSize();
    float width = std::max(lsize.width, wsize.width+100);
    float height = std::min(lsize.height + 50 + wsize.height, vsize.height);
    _layout->setContentSize(Size(width, height));
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

    auto bsize = positive->getContentSize();
    auto hbox = HBox::create(Size(bsize.width*2.5f, bsize.height));
    auto param = LinearLayoutParameter::create();
    param->setGravity(LinearLayoutParameter::LinearGravity::LEFT);
    param->setMargin(Margin(0.0f, 0.0f, 0.0f, 0.0f));
    positive->setLayoutParameter(param->clone());
    param->setGravity(LinearLayoutParameter::LinearGravity::RIGHT);
    param->setMargin(Margin(bsize.width*0.5f, 0.0f, 0.0f, 0.0f));
    negative->setLayoutParameter(param->clone());
    hbox->addChild(positive);
    hbox->addChild(negative);

    pushBackView(text);
    pushBackView(hbox);

    return true;
}

bool DialogBox::init(std::string text,
                     std::string positive,
                     std::string negative,
                     std::function<void(bool positive)> cb)
{
    auto t = Text::create(text, "", 40);
    t->setTextColor(Color4B::BLUE);
    auto p = Button::create("button.png");
    p->setTitleText(positive);
    p->setTitleFontSize(35);
    p->addClickEventListener([this, cb](Ref *ref){
            cb(true);
            removeFromParent();
        });
    auto n = Button::create("button.png");
    n->setTitleText(negative);
    n->setTitleFontSize(35);
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

    auto t = Text::create(text, "", 40);
    t->setTextColor(Color4B::BLUE);

    auto button = Button::create("button.png");
    button->setTitleText(TR("OK"));
    button->setTitleFontSize(35);
    button->addClickEventListener([this, cb](Ref *ref) {
            if (cb != nullptr)
                cb();
            removeFromParent();
        });

    pushBackView(t);
    pushBackView(button);

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

    pushBackView(listview);

    Director::getInstance()->getRunningScene()->addChild(this);

    return true;
}
