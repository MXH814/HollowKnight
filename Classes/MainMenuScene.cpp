#include "MainMenuScene.h"
#include "LoadingScene.h" 
#include <GameScene.h>

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

    // 菜单文字按钮
    auto startlabel = Label::createWithTTF(
        "Start Game",
        "fonts/ZCOOLXiaoWei-Regular.ttf",
        48
    );
    startlabel->setColor(Color3B::WHITE);

    auto startItem = MenuItemLabel::create(
        startlabel,
        CC_CALLBACK_1(MainMenuScene::startGame, this)
    );
    startItem->setPosition(Vec2(visibleSize.width / 2,visibleSize.height / 2));

    auto setlabel = Label::createWithTTF(
        "Settings",
        "fonts/ZCOOLXiaoWei-Regular.ttf",
        48
    );
    setlabel->setColor(Color3B::WHITE);

    auto setItem = MenuItemLabel::create(
        setlabel,
        CC_CALLBACK_1(MainMenuScene::openSettings, this)
    );
    setItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 100));

    auto exitlabel = Label::createWithTTF(
        "Quit Game",
        "fonts/ZCOOLXiaoWei-Regular.ttf",
        48
    );
    exitlabel->setColor(Color3B::WHITE);

    auto exitItem = MenuItemLabel::create(
        exitlabel,
        CC_CALLBACK_1(MainMenuScene::exitGame, this)
    );
    exitItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 200));

    auto menu = Menu::create(startItem, setItem, exitItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu);


    // ===== 血量初始化 =====
    int maxHp = 5;
    int currentHp = 5;

    // 血条背景
    auto hpBg = Sprite::create("Hp/hpbg.png");
    hpBg->setPosition(Vec2(200, 950));
    this->addChild(hpBg);

    auto soul = Sprite::create("Hp/soul.png");
    soul->setScale(0.9f);
    soul->setPosition(Vec2(152, 935));
    this->addChild(soul);

    // 当前血量条
    float startX = 20;
    float gap = 50;

    for (int i = 0; i < maxHp; i++)
    {
        auto hpBar = Sprite::create("Hp/hp1.png");
        hpBar->setPosition(Vec2(260 + i * gap, 980));
        hpBar->setScale(0.5f);
        this->addChild(hpBar);
    }

    int lost = currentHp - maxHp;
    if (lost < 0)
    {
        auto hplose = Sprite::create("Hp/hp8.png");
        hplose->setPosition(Vec2(260 + currentHp * gap, 978));
        hplose->setScale(0.5f);
        this->addChild(hplose);
    }

    auto coin = Sprite::create("Hp/coin.png");
    coin->setPosition(Vec2(260, 900));
    this->addChild(coin);

    int coins;
    coins = 14;

    cocos2d::Label* coinsnum;
    coinsnum = Label::createWithTTF(
        std::to_string(coins),
        "fonts/Perpetua.ttf",
        50
    );

    coinsnum->setColor(Color3B::WHITE);
    coinsnum->setPosition(Vec2(350, 900));
    this->addChild(coinsnum);

    // 添加键盘事件监听
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event) {
        if (keyCode == EventKeyboard::KeyCode::KEY_Q) {
            this->showQPanel();
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void MainMenuScene::startGame(Ref* sender)
{
    if (this->getChildByName("SelectLayer") != nullptr) {
        return; // 如果存在，直接返回，避免重复创建
    }
    // 创建一个全黑遮罩层
    auto layer = LayerColor::create(Color4B(0, 0, 0, 255));
    this->addChild(layer, 10, "SelectLayer");

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 提示文字
    auto label = Label::createWithTTF("Choose", "fonts/ZCOOLXiaoWei-Regular.ttf", 64);
    label->setColor(Color3B::WHITE);
    label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 280));
    layer->addChild(label);

    // 普通模式按钮
    auto bg1 = Sprite::create("Menu/selector.png");
    bg1->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    bg1->setScale(1.5f);
    bg1->setScaleX(2.9f);
    this->addChild(bg1, 10);

    auto bg2 = Sprite::create("Menu/Area_Dirtmouth.png");
    bg2->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    bg2->setScale(1.5f);
    bg2->setOpacity(245);
    this->addChild(bg2,11);

    auto normalLabel = Label::createWithTTF("Normal", "fonts/ZCOOLXiaoWei-Regular.ttf", 50);
    normalLabel->setColor(Color3B::WHITE);
    auto normalItem = MenuItemLabel::create(normalLabel, [=](Ref*) {
        auto scene = GameScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        });
    normalItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

    // 挑战模式按钮
    auto bg3 = Sprite::create("Menu/selector.png");
    bg3->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 300));
    bg3->setScale(1.5f);
    bg3->setScaleX(2.9f);
    this->addChild(bg3, 10);

    auto bg4 = Sprite::create("Menu/Area_Green_Path.png");
    bg4->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 300));
    bg4->setScale(1.5f);
    bg4->setOpacity(245);
    this->addChild(bg4, 11);
    auto hardLabel = Label::createWithTTF("Boss", "fonts/ZCOOLXiaoWei-Regular.ttf", 50);
    hardLabel->setColor(Color3B::WHITE);
    auto hardItem = MenuItemLabel::create(hardLabel, [=](Ref*) {
        auto scene = GameScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        });

    hardItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 300));

    auto menu = Menu::create(normalItem, hardItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu,20);
}

void MainMenuScene::showQPanel()
{
    // 检查是否已经存在 QPanel
    auto existingPanel = this->getChildByName("QPanel");
    if (existingPanel != nullptr) {
        // 如果 QPanel 存在，移除它
        existingPanel->removeFromParent();
        return;
    }

    // 创建一个半透明遮罩层
    auto layer = LayerColor::create(Color4B(0, 0, 0, 180));
    layer->setName("QPanel"); // 设置唯一名称
    this->addChild(layer, 20);

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建 Charm_Prompt 图片
    auto charmBg = Sprite::create("Charm/Charm_Prompt.png");
    charmBg->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    charmBg->setScale(1.5f);
    layer->addChild(charmBg);

    // 动作：延迟 1 秒后移除 Charm_Prompt，并显示其他内容
    charmBg->runAction(
        Sequence::create(
            DelayTime::create(1.0f), // 延迟 1 秒
            RemoveSelf::create(),    // 移除 Charm_Prompt
            CallFunc::create([=]() { // 回调函数，显示其他内容
                // 显示 Charm
                auto label = Label::createWithTTF("Charm", "fonts/ZCOOLXiaoWei-Regular.ttf", 48);
                label->setColor(Color3B::WHITE);
                label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 40));
                layer->addChild(label);

                auto top = Sprite::create("Charm/top.png");
                top->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 70));
                layer->addChild(top);

                auto edging1 = Sprite::create("Charm/edging1.png");
                edging1->setPosition(Vec2(200, visibleSize.height - 160));
                layer->addChild(edging1);

                auto edging2 = Sprite::create("Charm/edging2.png");
                edging2->setPosition(Vec2(visibleSize.width - 200, visibleSize.height - 160));
                layer->addChild(edging2);

                auto edging3 = Sprite::create("Charm/edging3.png");
                edging3->setPosition(Vec2(200, 160));
                layer->addChild(edging3);

                auto edging4 = Sprite::create("Charm/edging4.png");
                edging4->setPosition(Vec2(visibleSize.width - 200, 160));
                layer->addChild(edging4);

                auto smalledge1 = Sprite::create("Charm/smalledge1.png");
                smalledge1->setPosition(Vec2(100, visibleSize.height / 2));
                layer->addChild(smalledge1);

                auto smalledge2 = Sprite::create("Charm/smalledge2.png");
                smalledge2->setPosition(Vec2(visibleSize.width - 100, visibleSize.height / 2));
                layer->addChild(smalledge2);

                auto bottom = Sprite::create("Charm/bottom.png");
                bottom->setPosition(Vec2(visibleSize.width / 2, 80));
                layer->addChild(bottom);

                int charmWaywardCompass;
                auto Charm_WaywardCompass = Sprite::create("Charm/Charm_WaywardCompass.png");
                Charm_WaywardCompass->setPosition(Vec2(visibleSize.width - 200, visibleSize.height / 2));
                layer->addChild(Charm_WaywardCompass);

                auto listener = EventListenerTouchOneByOne::create();
                listener->setSwallowTouches(true);
                listener->onTouchBegan = [=](Touch* touch, Event* event) {
                    auto target = static_cast<Sprite*>(event->getCurrentTarget());
                    if (target->getBoundingBox().containsPoint(touch->getLocation())) {
                        // 变暗
                        target->setColor(Color3B(100, 100, 100));
                        return true;
                    }
                    return false;
                    };
            }),
            nullptr
        )
    );
}

void MainMenuScene::openSettings(Ref* sender)
{
    CCLOG("设置界面");
    // 这里可以跳转到 SettingsScene（如果你需要我可以帮你写）
}

void MainMenuScene::exitGame(Ref* sender)
{
    Director::getInstance()->end();
}


