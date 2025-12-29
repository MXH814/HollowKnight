#include "MainMenuScene.h"
#include "LoadingScene.h" 
#include <GameScene.h>
#include <BossScene.h>
#include "AudioManager.h"
#include "SettingsPanel.h"

USING_NS_CC;

Scene* MainMenuScene::createScene()
{
    return MainMenuScene::create();
}

bool MainMenuScene::init()
{
    if (!Scene::init())
        return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 背景图片
    auto bg = Sprite::create("Menu/Voidheart_menu_BG.png");
    bg->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 2));
    bg->setScale(0.85f);
    this->addChild(bg, 0);

    // 标题图片
    auto title = Sprite::create("Menu/title.png");
    title->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 270));
    title->setScale(0.85f);
    this->addChild(title, 1);

    // 菜单开始按钮
    auto startlabel = Label::createWithTTF(
        u8"开始游戏",
        "fonts/NotoSerifCJKsc-Regular.otf",
        48
    );
    startlabel->setColor(Color3B::WHITE);

    auto startItem = MenuItemLabel::create(
        startlabel,
        CC_CALLBACK_1(MainMenuScene::startGame, this)
    );
    startItem->setPosition(Vec2(visibleSize.width / 2,visibleSize.height / 2));

    auto setlabel = Label::createWithTTF(
        u8"游戏设置",
        "fonts/NotoSerifCJKsc-Regular.otf",
        48
    );
    setlabel->setColor(Color3B::WHITE);

    auto setItem = MenuItemLabel::create(
        setlabel,
        CC_CALLBACK_1(MainMenuScene::openSettings, this)
    );
    setItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 100));

    auto extrallabel = Label::createWithTTF(
        u8"额外选项",
        "fonts/NotoSerifCJKsc-Regular.otf",
        48
    );
    extrallabel->setColor(Color3B::WHITE);

    auto extralItem = MenuItemLabel::create(
        extrallabel,
        CC_CALLBACK_1(MainMenuScene::extrallabel, this)
    );
    extralItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 200));

    auto exitlabel = Label::createWithTTF(
        u8"退出游戏",
        "fonts/NotoSerifCJKsc-Regular.otf",
        48
    );
    exitlabel->setColor(Color3B::WHITE);

    auto exitItem = MenuItemLabel::create(
        exitlabel,
        CC_CALLBACK_1(MainMenuScene::exitGame, this)
    );
    exitItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 300));


    auto menu = Menu::create(startItem, setItem, extralItem, exitItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu);

    // 播放菜单背景音乐（循环）
    AudioManager::getInstance()->playMainMenuBGM();

    // 创建设置面板
    _settingsPanel = SettingsPanel::create();
    if (_settingsPanel)
    {
        this->addChild(_settingsPanel, 100);  // 高层级确保在最上层显示
    }

    return true;
}

void MainMenuScene::startGame(Ref* sender)
{
    // 播放点击音效
    AudioManager::getInstance()->playClickSound();

    if (this->getChildByName("SelectLayer") != nullptr) {
        return; // 如果存在，直接返回，避免重复添加
    }
    // 创建一个全屏遮罩层
    auto layer = LayerColor::create(Color4B(0, 0, 0, 255));
    this->addChild(layer, 10, "SelectLayer");

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 显示标题
    auto label = Label::createWithTTF(u8"选择模式", "fonts/NotoSerifCJKsc-Regular.otf", 72);
    label->setColor(Color3B::WHITE);
    label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 200));
    layer->addChild(label);

    // 创建花朵动画
    auto bg1 = Sprite::create("Menu/profile_fleur0.png");
    bg1->setPosition(Vec2(visibleSize.width / 2 - 40, visibleSize.height / 2 + 33));
    bg1->setScale(1.25f);
    layer->addChild(bg1,10);

    Vector<SpriteFrame*> bg1Frames;
    for (int i = 0; i <= 12; i++) {
        std::string frameName = "Menu/profile_fleur" + std::to_string(i) + ".png";
        auto frame = SpriteFrame::create(frameName, Rect(0, 0,
            bg1->getContentSize().width, bg1->getContentSize().height));
        if (frame) {
            bg1Frames.pushBack(frame);
        }
    }

    auto animationbg1 = Animation::createWithSpriteFrames(bg1Frames, 0.08f);
    auto animatebg1 = Animate::create(animationbg1);
    bg1->runAction(animatebg1);

    auto bg3 = Sprite::create("Menu/profile_fleur0.png");
    bg3->setPosition(Vec2(visibleSize.width / 2 - 40, visibleSize.height / 2 - 267));
    bg3->setScale(1.25f);
    layer->addChild(bg3, 10);

    Vector<SpriteFrame*> bg3Frames;
    for (int i = 0; i <= 12; i++) {
        std::string frameName = "Menu/profile_fleur" + std::to_string(i) + ".png";
        auto frame = SpriteFrame::create(frameName, Rect(0, 0,
            bg3->getContentSize().width, bg3->getContentSize().height));
        if (frame) {
            bg3Frames.pushBack(frame);
        }
    }

    auto animationbg3 = Animation::createWithSpriteFrames(bg3Frames, 0.08f);
    auto animatebg3 = Animate::create(animationbg3);
    bg3->runAction(animatebg3);

    // 创建底部花朵动画
    auto topbg = Sprite::create("Menu/pause_top_fleur0.png");
    topbg->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 200));
    layer->addChild(topbg);

    // 创建底部动画帧
    Vector<SpriteFrame*> topbgFrames;
    for (int i = 0; i <= 8; i++) {
        std::string frameName = "Menu/pause_top_fleur" + std::to_string(i) + ".png";
        auto frame = SpriteFrame::create(frameName, Rect(0, 0,
            topbg->getContentSize().width, topbg->getContentSize().height));
        if (frame) {
            topbgFrames.pushBack(frame);
        }
    }

    // 播放底部动画（不会消失）
    auto animationtop = Animation::createWithSpriteFrames(topbgFrames, 0.08f);
    auto animatetop = Animate::create(animationtop);
    topbg->runAction(animatetop);

    // 普通模式按钮
    auto bg2 = Sprite::create("Menu/Area_Dirtmouth.png");
    bg2->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    bg2->setScale(1.5f);
    bg2->setOpacity(225);
    layer->addChild(bg2);

    auto normalLabel = Label::createWithTTF(u8"普通模式", "fonts/NotoSerifCJKsc-Regular.otf", 50);
    normalLabel->setColor(Color3B::WHITE);
    auto normalItem = MenuItemLabel::create(normalLabel, [=](Ref*) {
        AudioManager::getInstance()->playClickSound();
        // 在进入游戏前停止菜单音乐
        AudioManager::getInstance()->stopBGM();

        // 创建全黑过渡场景
        auto blackScene = Scene::create();
        auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 255));
        blackScene->addChild(blackLayer);
        
        // 先切换到过渡场景，延迟后再进入游戏场景
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, blackScene));
        
        // 延迟后创建并进入游戏场景
        blackLayer->runAction(Sequence::create(
            DelayTime::create(1.0f),
            CallFunc::create([]() {
                auto scene = GameScene::createScene();
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
            }),
            nullptr
        ));
    });
    normalItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

    // Boss模式按钮
    auto bg4 = Sprite::create("Menu/Area_Green_Path.png");
    bg4->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 300));
    bg4->setScale(1.5f);
    bg4->setOpacity(225);
    layer->addChild(bg4);

    auto hardLabel = Label::createWithTTF(u8"Boss战", "fonts/NotoSerifCJKsc-Regular.otf", 50);
    hardLabel->setColor(Color3B::WHITE);
    auto hardItem = MenuItemLabel::create(hardLabel, [=](Ref*) {
        AudioManager::getInstance()->playClickSound();
        // 在进入 Boss 场景前停止菜单音乐
        AudioManager::getInstance()->stopBGM();

        // 创建全黑过渡场景
        auto blackScene = Scene::create();
        auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 255));
        blackScene->addChild(blackLayer);
        
        // 先切换到过渡场景，延迟后再进入Boss场景
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, blackScene));
        
        // 延迟后创建并进入Boss场景
        blackLayer->runAction(Sequence::create(
            DelayTime::create(1.0f),
            CallFunc::create([]() {
                auto scene = BossScene::createScene();
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
            }),
            nullptr
        ));
    });
    hardItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 300));

    auto menu = Menu::create(normalItem, hardItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    layer->addChild(menu);
}

void MainMenuScene::showQPanel()
{
    // 主菜单不再显示护符面板，该功能已移至游戏场景中
    CCLOG("护符面板只能在游戏场景中使用");
}

void MainMenuScene::openSettings(Ref* sender)
{
    // 播放点击音效
    AudioManager::getInstance()->playClickSound();
    if (_settingsPanel)
    {
        _settingsPanel->show();
    }

    CCLOG("设置界面");
}

void MainMenuScene::exitGame(Ref* sender)
{
    // 播放点击音效
    AudioManager::getInstance()->playClickSound();

    Director::getInstance()->end();
}

void MainMenuScene::extrallabel(cocos2d::Ref* sender)
{
    // 播放点击音效
    AudioManager::getInstance()->playClickSound();

    CCLOG("额外菜单");
}


