#include "LoadingScene.h"
#include "MainMenuScene.h"
#include "audio/include/SimpleAudioEngine.h"

using namespace CocosDenshion;

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

    // 小人动画帧（这里是 8 帧循环）
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

    // 预加载所有游戏音效（消除首次播放延迟）
    preloadAllAudio();

    // 模拟加载 3 秒后进入菜单
    this->scheduleOnce([this](float) {
        auto scene = MainMenuScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        }, 3.0f, "loading_finished");

    return true;
}

void LoadingScene::preloadAllAudio()
{
    auto audio = SimpleAudioEngine::getInstance();
    
    // ========== 背景音乐预加载 ==========
    audio->preloadBackgroundMusic("Music/Title.wav");
    audio->preloadBackgroundMusic("Music/Dirtmouth.wav");
    audio->preloadBackgroundMusic("Music/Crossroads.wav");
    audio->preloadBackgroundMusic("Music/Hornet.wav");
    
    // ========== UI 音效 ==========
    audio->preloadEffect("Music/click.wav");
    
    // ========== 骑士音效 ==========
    // 移动相关
    audio->preloadEffect("Music/hero_running.wav");
    audio->preloadEffect("Music/hero_jump.wav");
    audio->preloadEffect("Music/hero_land.wav");
    audio->preloadEffect("Music/hero_dash.wav");
    
    // 攻击相关
    audio->preloadEffect("Music/hero_sword.wav");
    
    // 受伤与死亡
    audio->preloadEffect("Music/hero_damage.wav");
    audio->preloadEffect("Music/hero_death.wav");
    
    // 法术相关
    audio->preloadEffect("Music/hero_fireball.wav");
    audio->preloadEffect("Music/fireball_disappear.wav");
    
    // ========== Boss (Hornet) 音效 ==========
    audio->preloadEffect("Music/1.wav");
    audio->preloadEffect("Music/2.wav");
    audio->preloadEffect("Music/3.wav");
    audio->preloadEffect("Music/4.wav");
    
    CCLOG("All audio files preloaded successfully!");
}
