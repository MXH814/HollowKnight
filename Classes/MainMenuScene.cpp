#include "MainMenuScene.h"
#include "LoadingScene.h" 
#include <GameScene.h>
#include <BossScene.h>
#include "audio/include/SimpleAudioEngine.h"

USING_NS_CC;
using namespace CocosDenshion;

// 全局 Charm 状态（0 = 未装备，1 = 已装备）
static int _charmWaywardCompass = 0;
static int _charmShamanStone = 0;
static int _charmStalwartShell = 0;
static int _charmSteadyBody = 0;
static int _charmSoulCatcher = 0;
static int _charmSprintMaster = 0;

// 凹槽系统
static const int MAX_NOTCHES = 6;  // 凹槽上限
static int usedNotches = 0;        // 已使用凹槽数

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
        u8"开始游戏",
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
        u8"游戏设置",
        "fonts/ZCOOLXiaoWei-Regular.ttf",
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
        "fonts/ZCOOLXiaoWei-Regular.ttf",
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
        "fonts/ZCOOLXiaoWei-Regular.ttf",
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
    SimpleAudioEngine::getInstance()->playBackgroundMusic("Music/Title.wav", true);

    // 注意：血量和灵魂UI已移至GameScene中显示
    // 主菜单不显示这些UI，因为没有角色

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
    // 播放点击音效
    SimpleAudioEngine::getInstance()->playEffect("Music/click.wav");

    if (this->getChildByName("SelectLayer") != nullptr) {
        return; // 如果存在，直接返回，避免重复创建
    }
    // 创建一个全黑遮罩层
    auto layer = LayerColor::create(Color4B(0, 0, 0, 255));
    this->addChild(layer, 10, "SelectLayer");

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 提示文字
    auto label = Label::createWithTTF(u8"选择模式", "fonts/ZCOOLXiaoWei-Regular.ttf", 72);
    label->setColor(Color3B::WHITE);
    label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 200));
    layer->addChild(label);

    // 创建过渡动画
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

    // 创建底部过渡动画
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

    // 播放底部动画（不消失）
    auto animationtop = Animation::createWithSpriteFrames(topbgFrames, 0.08f);
    auto animatetop = Animate::create(animationtop);
    topbg->runAction(animatetop);

    // 普通模式按钮
    auto bg2 = Sprite::create("Menu/Area_Dirtmouth.png");
    bg2->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    bg2->setScale(1.5f);
    bg2->setOpacity(225);
    layer->addChild(bg2);

    auto normalLabel = Label::createWithTTF(u8"剧情模式", "fonts/ZCOOLXiaoWei-Regular.ttf", 50);
    normalLabel->setColor(Color3B::WHITE);
    auto normalItem = MenuItemLabel::create(normalLabel, [=](Ref*) {
        SimpleAudioEngine::getInstance()->playEffect("Music/click.wav");
        // 在进入游戏前停止菜单音乐
        SimpleAudioEngine::getInstance()->stopBackgroundMusic(true);

        // 创建全黑过渡场景
        auto blackScene = Scene::create();
        auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 255));
        blackScene->addChild(blackLayer);
        
        // 先切换到黑屏，延迟后再进入游戏场景
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, blackScene));
        
        // 延迟后进入真正的游戏场景
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

    auto hardLabel = Label::createWithTTF(u8"Boss战", "fonts/ZCOOLXiaoWei-Regular.ttf", 50);
    hardLabel->setColor(Color3B::WHITE);
    auto hardItem = MenuItemLabel::create(hardLabel, [=](Ref*) {
        SimpleAudioEngine::getInstance()->playEffect("Music/click.wav");
        // 在进入 Boss 场景前停止菜单音乐
        SimpleAudioEngine::getInstance()->stopBackgroundMusic(true);

        // 创建全黑过渡场景
        auto blackScene = Scene::create();
        auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 255));
        blackScene->addChild(blackLayer);
        
        // 先切换到黑屏，延迟后再进入Boss场景
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, blackScene));
        
        // 延迟后进入真正的Boss场景
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
    // 检查是否已经存在 QPanel
    auto existingPanel = this->getChildByName("QPanel");
    if (existingPanel != nullptr) {
        existingPanel->removeFromParent();
        return;
    }

    // 创建一个半透明遮罩层
    auto layer = LayerColor::create(Color4B(0, 0, 0, 200));
    layer->setName("QPanel");
    this->addChild(layer, 20);

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建 Charm_Prompt 图片
    auto charmBg = Sprite::create("Charm/Charm_Prompt.png");
    charmBg->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    charmBg->setScale(1.5f);
    layer->addChild(charmBg);

    auto saveIcon = Sprite::create("Charm/Save_Icon_1.png");
    saveIcon->setPosition(Vec2(visibleSize.width - 100, 100));
    layer->addChild(saveIcon);

    // 创建动画帧
    Vector<SpriteFrame*> saveFrames;
    for (int i = 1; i <= 20; i++) {
        std::string frameName = "Charm/Save_Icon_" + std::to_string(i) + ".png";
        auto frame = SpriteFrame::create(frameName, Rect(0, 0, 
            saveIcon->getContentSize().width, saveIcon->getContentSize().height));
        if (frame) {
            saveFrames.pushBack(frame);
        }
    }

    // 播放动画
    auto animation = Animation::createWithSpriteFrames(saveFrames, 0.1f);
    auto animate = Animate::create(animation);
    auto repeatAnimate = Repeat::create(animate, 1);
    saveIcon->runAction(Sequence::create(
        repeatAnimate,
        RemoveSelf::create(),
        nullptr
    ));

    // 动作：延迟 1 秒后移除 Charm_Prompt，并显示其他内容
    charmBg->runAction(
        Sequence::create(
            DelayTime::create(2.0f),
            RemoveSelf::create(),
            CallFunc::create([=]() {
                // 显示 Charm 标题和边框
                auto label = Label::createWithTTF(u8"护符", "fonts/ZCOOLXiaoWei-Regular.ttf", 48);
                label->setColor(Color3B::WHITE);
                label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 40));
                layer->addChild(label);

                auto click = Label::createWithTTF(u8"点击装备", "fonts/ZCOOLXiaoWei-Regular.ttf", 36);
                click->setColor(Color3B::WHITE);
                click->setPosition(Vec2(visibleSize.width - 300, 180));
                layer->addChild(click);

                auto aocao = Label::createWithTTF(u8"凹槽", "fonts/ZCOOLXiaoWei-Regular.ttf", 28);
                aocao->setColor(Color3B::WHITE);
                aocao->setPosition(Vec2(365, visibleSize.height / 2 + 60));
                layer->addChild(aocao);

                auto zhuangbei = Label::createWithTTF(u8"已装备", "fonts/ZCOOLXiaoWei-Regular.ttf", 44);
                zhuangbei->setColor(Color3B::WHITE);
                zhuangbei->setPosition(Vec2(400, visibleSize.height / 2 + 220));
                layer->addChild(zhuangbei);

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

                auto line = Sprite::create("Charm/line.png");
                line->setPosition(Vec2(735, visibleSize.height / 2 - 58));
                line->setScaleX(1.44f);
                layer->addChild(line);

                // ===== 凹槽系统 =====
                auto notchSprites = std::make_shared<std::vector<Sprite*>>();
                
                // 凹槽位置：屏幕中心偏左上
                float notchStartX = visibleSize.width / 2 - 600;
                float notchY = visibleSize.height / 2 + 15;

                // 计算当前已使用的凹槽数
                usedNotches = 0;
                if (_charmWaywardCompass == 1) usedNotches += 1;
                if (_charmShamanStone == 1) usedNotches += 3;
                if (_charmStalwartShell == 1) usedNotches += 2;
                if (_charmSteadyBody == 1) usedNotches += 1;
                if (_charmSoulCatcher == 1) usedNotches += 2;
                if (_charmSprintMaster == 1) usedNotches += 1;

                // 创建凹槽图片
                for (int i = 0; i < MAX_NOTCHES; i++) {
                    std::string notchImage = (i < usedNotches) ? 
                        "Charm/charm_cost_1.png" : "Charm/charm_cost_0.png";
                    auto notch = Sprite::create(notchImage);
                    notch->setPosition(Vec2(notchStartX + i * 50, notchY));
                    layer->addChild(notch);
                    notchSprites->push_back(notch);
                }

                // 更新凹槽显示的函数
                auto updateNotchDisplay = [=]() {
                    for (int i = 0; i < MAX_NOTCHES; i++) {
                        std::string notchImage = (i < usedNotches) ? 
                            "Charm/charm_cost_1.png" : "Charm/charm_cost_0.png";
                        (*notchSprites)[i]->setTexture(notchImage);
                    }
                };

                // ===== Charm 系统 =====
                
                // Charm 配置信息
                struct CharmInfo {
                    std::string imagePath;
                    Vec2 originalPos;
                    Sprite* sprite;
                    Sprite* darkSprite;
                    int* statePtr;
                    int cost;  // 凹槽消耗
                    int equippedSlot;
                };

                auto charms = std::make_shared<std::vector<CharmInfo>>();

                // 原始位置配置：屏幕中下部分，居中，水平排列
                int charmCount = 6;
                float originalGap = 150;  // 间隔大一点
                float originalStartX = visibleSize.width / 2 - 8 * originalGap / 2;
                float originalY = visibleSize.height / 2 - 100;  // 屏幕中下部分

                // 装备槽位置配置：凹槽上方
                float equippedStartX = notchStartX;
                float equippedY = notchY + 125;  // 凹槽上方
                float equippedGap = 100;

                int rows = 3;
                int cols = 6;
                float charmBackGap = originalGap;  // 背景图片间隔
                float charmBackStartX = originalStartX;
                float charmBackStartY = originalY;  // 起始Y位置
                float rowGap = 100;  // 行间距

                for (int row = 0; row < rows; row++) {
                    for (int col = 0; col < cols; col++) {
                        auto charmBack = Sprite::create("Charm/charm_back.png");
                        charmBack->setScale(0.6f);
                        charmBack->setPosition(Vec2(
                            charmBackStartX + col * charmBackGap,
                            charmBackStartY - row * rowGap
                        ));
                        layer->addChild(charmBack);
                    }
                }

                // 初始化 6 个 Charm（图片路径、状态指针、凹槽消耗）
                struct CharmConfig {
                    std::string imagePath;
                    int* statePtr;
                    int cost;
                };

                std::vector<CharmConfig> charmConfigs = {
                    {"Charm/Charm_WaywardCompass.png", &_charmWaywardCompass, 1},
                    {"Charm/Charm_ShamanStone.png", &_charmShamanStone, 3},
                    {"Charm/Charm_StalwartShell.png", &_charmStalwartShell, 2},
                    {"Charm/Charm_SteadyBody.png", &_charmSteadyBody, 1},
                    {"Charm/Charm_SoulCatcher.png", &_charmSoulCatcher, 2},
                    {"Charm/Charm_SprintMaster.png", &_charmSprintMaster, 1}
                };

                for (int i = 0; i < charmCount; i++) {
                    CharmInfo info;
                    info.imagePath = charmConfigs[i].imagePath;
                    info.statePtr = charmConfigs[i].statePtr;
                    info.cost = charmConfigs[i].cost;
                    // 原始位置：水平居中排列
                    info.originalPos = Vec2(originalStartX + i * originalGap, originalY);
                    info.equippedSlot = -1;

                    // 创建 Charm 图片
                    info.sprite = Sprite::create(info.imagePath);
                    info.sprite->setPosition(info.originalPos);
                    layer->addChild(info.sprite);

                    // 创建暗色占位图片
                    info.darkSprite = Sprite::create(info.imagePath);
                    info.darkSprite->setPosition(info.originalPos);
                    info.darkSprite->setColor(Color3B(100, 100, 100));
                    info.darkSprite->setVisible(*info.statePtr == 1);
                    layer->addChild(info.darkSprite);

                    charms->push_back(info);
                }

                // 更新装备槽位置的函数
                auto updateEquippedPositions = [=]() {
                    int slot = 0;
                    for (auto& charm : *charms) {
                        if (*charm.statePtr == 1) {
                            charm.equippedSlot = slot;
                            Vec2 newPos = Vec2(equippedStartX + slot * equippedGap, equippedY);
                            charm.sprite->setPosition(newPos);
                            slot++;
                        }
                    }
                };

                // 初始化时更新已装备 Charm 的位置
                updateEquippedPositions();

                // ===== 右侧信息面板 =====
                float infoPanelX = visibleSize.width - 450;
                float infoPanelY = visibleSize.height / 2 + 100;

                // Charm 图标（大图）
                auto infoCharmIcon = Sprite::create("Charm/Charm_WaywardCompass.png");
                infoCharmIcon->setPosition(Vec2(infoPanelX, infoPanelY - 10));
                infoCharmIcon->setScale(1.5f);
                infoCharmIcon->setVisible(false);
                infoCharmIcon->setName("InfoCharmIcon");
                layer->addChild(infoCharmIcon);

                // Charm 名称
                auto infoCharmName = Label::createWithTTF(u8"", "fonts/ZCOOLXiaoWei-Regular.ttf", 48);
                infoCharmName->setColor(Color3B::WHITE);
                infoCharmName->setPosition(Vec2(infoPanelX, infoPanelY + 150));
                infoCharmName->setName("InfoCharmName");
                layer->addChild(infoCharmName);

                // Charm 描述
                auto infoCharmDesc = Label::createWithTTF(u8"", "fonts/ZCOOLXiaoWei-Regular.ttf", 32);
                infoCharmDesc->setColor(Color3B::WHITE);
                infoCharmDesc->setPosition(Vec2(infoPanelX, infoPanelY - 150));
                infoCharmDesc->setMaxLineWidth(400);
                infoCharmDesc->setName("InfoCharmDesc");
                layer->addChild(infoCharmDesc);

                // 凹槽消耗图片容器
                auto costContainer = Node::create();
                costContainer->setPosition(Vec2(infoPanelX + 45, infoPanelY + 100));
                costContainer->setName("InfoCharmCostContainer");
                layer->addChild(costContainer);

                auto infoCharmCost = Label::createWithTTF(u8"", "fonts/ZCOOLXiaoWei-Regular.ttf", 32);
                infoCharmCost->setColor(Color3B::WHITE);
                infoCharmCost->setPosition(Vec2(infoPanelX - 50, infoPanelY + 100));
                infoCharmCost->setName("InfoCharmCost");
                layer->addChild(infoCharmCost);

                // 更新信息面板的函数
                auto updateInfoPanel = [=](int charmIndex, bool show) {
                    auto icon = static_cast<Sprite*>(layer->getChildByName("InfoCharmIcon"));
                    auto nameLabel = static_cast<Label*>(layer->getChildByName("InfoCharmName"));
                    auto descLabel = static_cast<Label*>(layer->getChildByName("InfoCharmDesc"));
                    auto costContainer = layer->getChildByName("InfoCharmCostContainer");
                    auto costLabel = static_cast<Label*>(layer->getChildByName("InfoCharmCost"));

                    // 清除之前的凹槽图片
                    costContainer->removeAllChildren();

                    if (!show) {
                        icon->setVisible(false);
                        nameLabel->setString(u8"");
                        descLabel->setString(u8"");
                        costLabel->setString(u8"");
                        return;
                    }

                    icon->setVisible(true);

                    int costCount = 0;
                    if (charmIndex == 0) {
                        icon->setTexture("Charm/Charm_WaywardCompass.png");
                        nameLabel->setString(u8"流浪者罗盘");
                        costLabel->setString(u8"花费");
                        descLabel->setString(u8"当地图打开时，将其位置告诉佩戴者，允许漫游者进行精确定位。");
                        costCount = 1;
                    }
                    else if (charmIndex == 1) {
                        icon->setTexture("Charm/Charm_ShamanStone.png");
                        nameLabel->setString(u8"萨满之石");
                        costLabel->setString(u8"花费");
                        descLabel->setString(u8"提高法术的威力，对敌人造成更多伤害。");
                        costCount = 3;
                    }
                    else if (charmIndex == 2) {
                        icon->setTexture("Charm/Charm_StalwartShell.png");
                        nameLabel->setString(u8"坚硬外壳");
                        costLabel->setString(u8"花费");
                        descLabel->setString(u8"加强韧性。当从伤害中恢复时，延长持有者保持不受伤害的时间。");
                        costCount = 2;
                    }
                    else if (charmIndex == 3) {
                        icon->setTexture("Charm/Charm_SteadyBody.png");
                        nameLabel->setString(u8"稳定之躯");
                        costLabel->setString(u8"花费");
                        descLabel->setString(u8"防止持有者用骨钉劈砍敌人时产生后坐力。");
                        costCount = 1;
                    }
                    else if (charmIndex == 4) {
                        icon->setTexture("Charm/Charm_SoulCatcher.png");
                        nameLabel->setString(u8"灵魂捕手");
                        costLabel->setString(u8"花费");
                        descLabel->setString(u8"增加用骨钉攻击敌人时获得的灵魂数量。");
                        costCount = 2;
                    }
                    else if (charmIndex == 5) {
                        icon->setTexture("Charm/Charm_SprintMaster.png");
                        nameLabel->setString(u8"冲刺大师");
                        costLabel->setString(u8"花费");
                        descLabel->setString(u8"持有者将能够更频繁地冲刺，也能向下冲刺。");
                        costCount = 1;
                    }

                    // 创建凹槽消耗图片
                    float costGap = 40;
                    float costStartX = -(costCount - 1) * costGap / 2;
                    for (int j = 0; j < costCount; j++) {
                        auto costIcon = Sprite::create("Charm/charm_cost_1.png");
                        costIcon->setPosition(Vec2(costStartX + j * costGap, 0));
                        costContainer->addChild(costIcon);
                    }
                };

                // 为每个 Charm 添加触摸监听
                for (int i = 0; i < charmCount; i++) {
                    auto listener = EventListenerTouchOneByOne::create();
                    listener->setSwallowTouches(true);
                    listener->onTouchBegan = [=](Touch* touch, Event* event) mutable {
                        auto target = static_cast<Sprite*>(event->getCurrentTarget());
                        if (target->getBoundingBox().containsPoint(touch->getLocation())) {
                            auto& charm = (*charms)[i];
                            
                            if (*charm.statePtr == 0) {
                                // 检查凹槽是否足够
                                if (usedNotches + charm.cost > MAX_NOTCHES) {
                                    // 凹槽不足，无法装备
                                    CCLOG("凹槽不足，无法装备 Charm");
                                    return true;
                                }
                                
                                // 装备 Charm
                                *charm.statePtr = 1;
                                usedNotches += charm.cost;
                                charm.darkSprite->setVisible(true);
                                updateEquippedPositions();
                                updateNotchDisplay();
                                updateInfoPanel(i, true);
                            }
                            else {
                                // 卸下 Charm
                                *charm.statePtr = 0;
                                usedNotches -= charm.cost;
                                charm.equippedSlot = -1;
                                charm.darkSprite->setVisible(false);
                                charm.sprite->setPosition(charm.originalPos);
                                updateEquippedPositions();
                                updateNotchDisplay();
                                updateInfoPanel(-1, false);
                            }
                            return true;
                        }
                        return false;
                    };
                    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, (*charms)[i].sprite);
                }
            }),
            nullptr
        )
    );
}

void MainMenuScene::openSettings(Ref* sender)
{
    // 播放点击音效
    SimpleAudioEngine::getInstance()->playEffect("Music/click.wav");

    CCLOG("设置界面");
}

void MainMenuScene::exitGame(Ref* sender)
{
    // 播放点击音效
    SimpleAudioEngine::getInstance()->playEffect("Music/click.wav");

    Director::getInstance()->end();
}

void MainMenuScene::extrallabel(cocos2d::Ref* sender)
{
    // 播放点击音效
    SimpleAudioEngine::getInstance()->playEffect("Music/click.wav");

    CCLOG("额外菜单项");
}


