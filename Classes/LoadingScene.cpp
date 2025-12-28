#include "LoadingScene.h"
#include "MainMenuScene.h"
#include "AudioManager.h"

USING_NS_CC;

Scene* LoadingScene::createScene()
{
    return LoadingScene::create();
}

bool LoadingScene::init()
{
    if (!Scene::init())
        return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 背景
    auto blackBg = LayerColor::create(Color4B::BLACK);
    this->addChild(blackBg);

    // 小人精灵（固定在右下角）
    auto hero = Sprite::create("Loading/loading_icon_new0000.png");
    hero->setPosition(Vec2(1700, 100));
    hero->setScale(1.5f);
    this->addChild(hero);

    // 小人动画帧（手动添加 8 帧循环）
    auto animation = Animation::create();
    animation->setDelayPerUnit(1.0f / 8.0f);
    animation->addSpriteFrameWithFile("Loading/loading_icon_new0000.png");
    animation->addSpriteFrameWithFile("Loading/loading_icon_new0001.png");
    animation->addSpriteFrameWithFile("Loading/loading_icon_new0002.png");
    animation->addSpriteFrameWithFile("Loading/loading_icon_new0003.png");
    animation->addSpriteFrameWithFile("Loading/loading_icon_new0004.png");
    animation->addSpriteFrameWithFile("Loading/loading_icon_new0005.png");
    animation->addSpriteFrameWithFile("Loading/loading_icon_new0006.png");
    animation->addSpriteFrameWithFile("Loading/loading_icon_new0007.png");

    auto animate = RepeatForever::create(Animate::create(animation));
    hero->runAction(animate);

    // 预加载所有音频资源
    AudioManager::getInstance()->preloadAllAudio();

    // 模拟加载 3 秒后进入主菜单
    this->scheduleOnce([this](float) {
        auto scene = MainMenuScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        }, 3.0f, "loading_finished");

    return true;
}
