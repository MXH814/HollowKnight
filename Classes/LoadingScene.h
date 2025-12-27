/**
 * @file LoadingScene.h
 * @brief 加载场景类头文件
 * @details 游戏启动时的加载界面，负责预加载所有资源
 */

#ifndef LOADING_SCENE_H_
#define LOADING_SCENE_H_

#include "cocos2d.h"

/**
 * @class LoadingScene
 * @brief 加载场景类
 * @details 显示加载动画，并预加载所有游戏资源（音效、纹理等）
 */
class LoadingScene : public cocos2d::Scene {
 public:
  /**
   * @brief 创建场景
   * @return 场景指针
   */
  static cocos2d::Scene* createScene();
  
  /**
   * @brief 初始化场景
   * @return 初始化是否成功
   */
  bool init() override;
  
  CREATE_FUNC(LoadingScene);

 private:
  /** @brief 加载完成回调 */
  void OnLoadingFinished();
  
  /** @brief 预加载所有游戏音效 */
  void PreloadAllAudio();
  
  /** @brief 预加载所有游戏纹理 */
  void PreloadAllTextures();
};

#endif  // LOADING_SCENE_H_

