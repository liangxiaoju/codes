#include "HeaderView.h"

bool EnemyHeaderView::init(int level, Side side)
{
    if (!RelativeBox::init())
        return false;

    RelativeLayoutParameter *param;

    auto head = ImageView::create("common/head_level1.png");
    param = RelativeLayoutParameter::create();
    param->setAlign(
        RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_CENTER_VERTICAL);
    param->setRelativeName("head");
    head->setLayoutParameter(param);
    addChild(head);

    auto str = LevelMenu::getUserLevelString();

    auto text = Text::create(str, "", 60);
    text->setTextColor(Color4B(88, 49, 14, 255));
    param = RelativeLayoutParameter::create();
    param->setAlign(
        RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_TOPALIGN);
    param->setRelativeName("level");
    param->setRelativeToWidgetName("head");
    text->setLayoutParameter(param);
    addChild(text);

    auto piece = ImageView::create("");
    param = RelativeLayoutParameter::create();
    param->setAlign(
        RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_LEFTALIGN);
    param->setRelativeToWidgetName("level");
    piece->setLayoutParameter(param);
    addChild(piece);

    auto headSize = head->getContentSize();
    auto textSize = text->getContentSize();
    auto pieceSize = piece->getContentSize();
    float w = headSize.width+std::max(textSize.width, pieceSize.width);
    float h = headSize.height;
    setContentSize(Size(w, h));

    setAnchorPoint(Vec2(0.5, 0.5));

    _head = head;
    _text = text;
    _piece = piece;

    return true;
}

bool SelfHeaderView::init()
{
    if (!RelativeBox::init())
        return false;

    RelativeLayoutParameter *param;

    auto head = ImageView::create("common/head_level1.png");
    head->setFlippedX(true);
    param = RelativeLayoutParameter::create();
    param->setAlign(
        RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_CENTER_VERTICAL);
    param->setRelativeName("head");
    head->setLayoutParameter(param);
    addChild(head);

    auto str = LevelMenu::getUserLevelString();

    auto text = Text::create(str, "", 60);
    text->setTextColor(Color4B(88, 49, 14, 255));
    param = RelativeLayoutParameter::create();
    param->setAlign(
        RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_BOTTOMALIGN);
    param->setRelativeName("text");
    param->setRelativeToWidgetName("head");
    text->setLayoutParameter(param);
    addChild(text);

    auto piece = ImageView::create("");
    param = RelativeLayoutParameter::create();
    param->setAlign(
        RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_CENTER);
    param->setRelativeToWidgetName("text");
    piece->setLayoutParameter(param);
    addChild(piece);

    auto headSize = head->getContentSize();
    auto textSize = text->getContentSize();
    auto pieceSize = piece->getContentSize();
    float w = headSize.width+std::max(textSize.width, pieceSize.width);
    float h = headSize.height;
    setContentSize(Size(w, h));

    setAnchorPoint(Vec2(0.5, 0.5));

    return true;
}
