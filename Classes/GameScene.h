#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include "TheKnight.h"

class GameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    virtual void update(float dt) override;
    
    CREATE_FUNC(GameScene);

private:
    void createCollisionFromTMX(cocos2d::TMXTiledMap* map, 
                                 const std::string& layerName, 
                                 float scale, 
                                 const cocos2d::Vec2& mapOffset);
    
    void loadInteractiveObjects(cocos2d::TMXTiledMap* map, 
                                float scale, 
                                const cocos2d::Vec2& mapOffset);
    
    void checkInteractions();
    
    // 摄像机更新
    void updateCamera();

    // 交互对象
    struct InteractiveObject {
        cocos2d::Vec2 position;
        std::string name;
        float radius;
    };
    std::vector<InteractiveObject> _interactiveObjects;
    
    // 平台列表（碰撞检测）
    std::vector<Platform> _platforms;
    
    // 玩家
    TheKnight* _knight = nullptr;
    
    // UI 提示标签
    cocos2d::Label* _interactionLabel = nullptr;
    
    // 防止重复切换场景
    bool _isTransitioning = false;
    
    // 地图信息
    float _mapScale = 1.8f;
    cocos2d::Size _mapSize;
    
    // 摄像机偏移（看向上/下）
    float _cameraOffsetY = 0.0f;
    float _targetCameraOffsetY = 0.0f;
};

#endif // __GAME_SCENE_H__
