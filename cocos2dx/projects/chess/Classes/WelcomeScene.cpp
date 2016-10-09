#include "WelcomeScene.h"
#include "MainMenuScene.h"
#include "BGLayer.h"
#include "Sound.h"
#include "SettingMenu.h"

using namespace CocosDenshion;

bool WelcomeScene::init()
{
    if (!Scene::init())
        return false;

    auto layer = WelcomeLayer::create();
    addChild(layer);

    return true;
}

bool WelcomeLayer::init()
{
    if (!Layer::init())
        return false;

    Size vsize = Director::getInstance()->getVisibleSize();

    auto bg = BGLayer::create();
    addChild(bg);

    auto rotateBy = RotateBy::create(0.1, -15);
    Repeat *repeat = Repeat::create(rotateBy, 5);
    CallFunc *callback = CallFunc::create([]() {
            Director::getInstance()->replaceScene(MainMenuScene::create());
        });
    Sequence *seq = Sequence::create(repeat, callback, nullptr);

    auto sp1 = Sprite::create("head/load_outter.png");
    sp1->runAction(seq);
    sp1->setPosition(vsize.width/2, vsize.height/2);
    addChild(sp1, 0, "outter");

    auto sp2 = Sprite::create("load_inner.png");
    sp2->setPosition(vsize.width/2, vsize.height/2);
    addChild(sp2, 0, "inner");

    //load setting
    SettingMenu::getInstance();

    return true;
}

//setSearchResolutionsOrder
/*
 * if 1024x768 is set, "ui/menu/xx.png" will search "ui/menu/1024x768/xx.png"
 */

//loadFilenameLookupDictionaryFromFile
/*
 * @code
 * <?xml version="1.0" encoding="UTF-8"?>
 * <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
 * <plist version="1.0">
 * <dict>
 *     <key>filenames</key>
 *     <dict>
 *         <key>sounds/click.wav</key>
 *         <string>sounds/click.caf</string>
 *         <key>sounds/endgame.wav</key>
 *         <string>sounds/endgame.caf</string>
 *         <key>sounds/gem-0.wav</key>
 *         <string>sounds/gem-0.caf</string>
 *     </dict>
 *     <key>metadata</key>
 *     <dict>
 *         <key>version</key>
 *         <integer>1</integer>
 *     </dict>
 * </dict>
 * </plist>
 * @endcode
 */

//setSearchPaths
/*
 *        For instance:
 *            On Android, the default resource root path is "assets/".
 *            If "/mnt/sdcard/" and "resources-large" were set to the search paths vector,
 *            "resources-large" will be converted to "assets/resources-large" since it was a relative path.
 */
