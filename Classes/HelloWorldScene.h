#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "TheKnight.h"
#include "Enemy.h"

class HelloWorld : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    void update(float dt) override;
    
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    CREATE_FUNC(HelloWorld);

private:
    void createPlatforms();
    void createRandomPlatforms();
    void createEnemies();
    void updateCamera();
    void updateCombat(float dt);
    void updateHPLabel();
    void updateSoulLabel();
    
    TheKnight* _knight;
    std::vector<Platform> _platforms;
    std::vector<Enemy*> _enemies;
    
    cocos2d::Node* _gameLayer;      // 游戏层（包含所有游戏对象）
    cocos2d::Size _mapSize;         // 地图大小
    
    float _cameraOffsetY;           // 镜头Y偏移（用于向上/下看）
    float _targetCameraOffsetY;     // 目标镜头Y偏移
    
    cocos2d::Label* _hpLabel;       // HP显示标签
    cocos2d::Label* _soulLabel;     // Soul显示标签
};

#endif // __HELLOWORLD_SCENE_H__
