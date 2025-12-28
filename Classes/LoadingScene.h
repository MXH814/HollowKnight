#ifndef __LOADING_SCENE_H__
#define __LOADING_SCENE_H__

#include "cocos2d.h"

class LoadingScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC(LoadingScene);

private:
    void onLoadingFinished();
    
    // 预加载所有游戏音效
    void preloadAllAudio();
};

#endif // __LOADING_SCENE_H__

