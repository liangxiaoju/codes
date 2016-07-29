#include "TutorialScene.h"
#include "TutorialData.h"
#include "XQFile/XQJsonFile.h"
#include "SchoolScene.h"

bool TutorialMenuScene::init()
{
    if (!Scene::init())
        return false;

    auto vsize = Director::getInstance()->getVisibleSize();
    LinearLayoutParameter* lp = LinearLayoutParameter::create();
    lp->setGravity(LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);

    _category_button = Button::create("ChallengeScene/tga_title_fat.png");
    _category_button->setZoomScale(0.1);
    _category_button->setTitleFontSize(40);
    _category_button->setTitleFontName("");
    _category_button->retain();
    _node_button = Button::create("ChallengeScene/buttonPost.png");
    _node_button->setZoomScale(0.1);
    _node_button->setTitleFontSize(40);
    _node_button->setTitleFontName("");
    _node_button->retain();

    auto refresh = [this](int id) {
        std::vector<TutorialData::TutorialCategory> categorys;
        TutorialData::getInstance()->queryTutorialCategory(id, categorys);
        std::vector<TutorialData::TutorialNode> nodes;
        TutorialData::getInstance()->queryTutorialNode(id, nodes);

        log("%s %d", __func__, __LINE__);
        //_listview->removeAllItems();
        for (auto &widget : _listview->getItems()) {
            widget->removeFromParent();
        }
        _listview->requestDoLayout();
        log("%s %d", __func__, __LINE__);

        for (auto &category : categorys) {
            Button* btn = (Button*)_category_button->clone();
            btn->setName("Category");
            btn->setTag(category.id);
            btn->setTitleText(category.name);
            _listview->pushBackCustomItem(btn);
            log("Category: %s", category.name.c_str());
        }

        for (auto &node : nodes) {
            Button* btn = (Button*)_node_button->clone();
            btn->setName("Node");
            btn->setTag(node.id);
            btn->setTitleText(node.name);
            _listview->pushBackCustomItem(btn);
            log("Node: %s", node.name.c_str());
        }
    };

    _listview = ListView::create();
    _listview->setDirection(ScrollView::Direction::VERTICAL);
    _listview->setBounceEnabled(true);
    _listview->setScrollBarEnabled(false);
    _listview->setGravity(ListView::Gravity::CENTER_HORIZONTAL);
    _listview->setItemsMargin(50);

    _listview->addEventListener([this,refresh](Ref *pSender, ListView::EventType type){
    switch (type)
    {
        case ListView::EventType::ON_SELECTED_ITEM_START:
            {
                ListView* listView = static_cast<ListView*>(pSender);
                auto item = listView->getItem(listView->getCurSelectedIndex());
                CCLOG("select child start index = %d", item->getTag());
                break;
            }
        case ListView::EventType::ON_SELECTED_ITEM_END:
            {
                ListView* listView = static_cast<ListView*>(pSender);
                auto item = listView->getItem(listView->getCurSelectedIndex());
                CCLOG("select child end index = %d", item->getTag());

                int id = item->getTag();
                Button *btn = dynamic_cast<Button*>(item);
                log("+++");

                if (btn->getName() == "Node") {
                    auto node = TutorialData::getInstance()->getTutorialNode(id);
                    auto xqFile = new XQJsonFile();
                    xqFile->load(node.json);
                    auto scene = SchoolScene::create(xqFile);
                    Director::getInstance()->pushScene(scene);
                } else if (btn->getName() == "Category") {
                    refresh(id);
                }
                log("---");

                break;
            }
        default:
            break;
    }
    });

    auto vbox = VBox::create();
    _headview = Layout::create();

    vbox->setContentSize(vsize);
    _headview->setContentSize(Size(vsize.width, 200));
    _listview->setContentSize(Size(vsize.width, vsize.height-200-100));

    _headview->setLayoutParameter(lp->clone());
    _listview->setLayoutParameter(lp->clone());

    vbox->addChild(_headview);
    vbox->addChild(_listview);

    vbox->setBackGroundImage("MainMenuScene/common_bg_new2.png");
    addChild(vbox);
    _layout = vbox;

    auto back = Button::create("DifficultyScene/look_back.png");
    back->addClickEventListener([](Ref *ref){
            Director::getInstance()->popScene();
            });
    back->setZoomScale(0.1);
    back->setPosition(Vec2(100, vsize.height-100));
    addChild(back);

    refresh(1);

    return true;
}

TutorialMenuScene::~TutorialMenuScene()
{
    CC_SAFE_RELEASE(_category_button);
    CC_SAFE_RELEASE(_node_button);
}
