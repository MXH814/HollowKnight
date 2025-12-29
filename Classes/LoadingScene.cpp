/**
 * @file LoadingScene.cpp
 * @brief 加载场景类实现
 * @details 实现游戏启动时的资源预加载
 * 
 * C++ 特性使用说明：
 * - STL 容器：std::vector 存储资源路径
 * - 范围 for 循环：遍历预加载列表
 * - auto 关键字：类型推导
 */

#include "LoadingScene.h"
#include "MainMenuScene.h"
#include "audio/include/SimpleAudioEngine.h"
#include <vector>
#include <string>

using namespace CocosDenshion;

USING_NS_CC;

Scene* LoadingScene::createScene() {
  return LoadingScene::create();
}

bool LoadingScene::init() {
  if (!Scene::init()) {
    return false;
  }

  auto visible_size = Director::getInstance()->getVisibleSize();
  auto origin = Director::getInstance()->getVisibleOrigin();

  // 创建黑色背景
  auto black_bg = LayerColor::create(Color4B::BLACK);
  this->addChild(black_bg);

  // 创建加载图标精灵（固定在右下角）
  auto hero = Sprite::create("Loading/loading_icon_new0000.png");
  hero->setPosition(Vec2(1700, 100));
  hero->setScale(1.5f);
  this->addChild(hero);

  // 创建加载动画（8 帧循环）
  auto animation = Animation::create();
  animation->setDelayPerUnit(1.0f / 8.0f);
  
  // 使用循环添加动画帧
  for (int i = 0; i < 8; ++i) {
    std::string frame_name = "Loading/loading_icon_new000" + std::to_string(i) + ".png";
    animation->addSpriteFrameWithFile(frame_name);
  }

  auto animate = RepeatForever::create(Animate::create(animation));
  hero->runAction(animate);

  // 预加载所有游戏资源
  PreloadAllAudio();
  PreloadAllTextures();

  // 3 秒后进入主菜单
  this->scheduleOnce([](float) {
    auto scene = MainMenuScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
  }, 3.0f, "loading_finished");

  return true;
}

void LoadingScene::OnLoadingFinished() {
  auto scene = MainMenuScene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void LoadingScene::PreloadAllAudio() {
  auto* audio = SimpleAudioEngine::getInstance();
  
  // 使用 STL 容器存储音效路径
  const std::vector<std::string> kBackgroundMusicList = {
    "Music/Title.wav",
    "Music/Dirtmouth.wav",
    "Music/Crossroads.wav",
    "Music/Greenpath.wav"
  };
  
  const std::vector<std::string> kEffectList = {
    // UI 音效
    "Music/click.wav",
    // 骑士移动音效
    "Music/hero_running.wav",
    "Music/hero_jump.wav",
    "Music/hero_land.wav",
    "Music/hero_dash.wav",
    // 骑士战斗音效
    "Music/hero_sword.wav",
    "Music/hero_damage.wav",
    "Music/hero_death.wav",
    // 法术音效
    "Music/hero_fireball.wav",
    "Music/fireball_disappear.wav",
    // 怪物音效
    "Music/enemy_damage.wav",
    "Music/enemy_death.wav",
    // Boss 音效
    "Music/1.wav",
    "Music/2.wav",
    "Music/3.wav",
    "Music/4.wav"
  };
  
  // 使用范围 for 循环预加载背景音乐
  for (const auto& music_path : kBackgroundMusicList) {
    audio->preloadBackgroundMusic(music_path.c_str());
  }
  
  // 使用范围 for 循环预加载音效
  for (const auto& effect_path : kEffectList) {
    audio->preloadEffect(effect_path.c_str());
  }
  
  CCLOG("All audio files preloaded successfully!");
}

void LoadingScene::PreloadAllTextures() {
  auto* texture_cache = Director::getInstance()->getTextureCache();
  
  // 预加载常用纹理（使用 STL 容器和迭代器）
  const std::vector<std::string> kTextureList = {
    "TheKnight/Idle/Idle1.png",
    "Menu/Voidheart_menu_BG.png",
    "Menu/title.png",
    "Hp/hpbg.png"
  };
  
  // 使用迭代器遍历纹理列表
  for (auto it = kTextureList.cbegin(); it != kTextureList.cend(); ++it) {
    texture_cache->addImage(*it);
  }
  
  CCLOG("Key textures preloaded successfully!");
}
