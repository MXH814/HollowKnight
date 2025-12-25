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
    void loadForegroundObjects(cocos2d::TMXTiledMap* map,
        float scale,
        const cocos2d::Vec2& mapOffset);
    void checkInteractions();
    
    // 摄像机相关
    void updateCamera();
    
    // HP和Soul UI相关
    void createHPAndSoulUI();
    void updateHPAndSoulUI(float dt);
    void startHPRecoveryAnimation();  // 开始血量恢复动画
    void updateHPRecoveryAnimation(float dt);  // 更新血量恢复动画

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
    
    // UI 显示标签
    cocos2d::Label* _interactionLabel = nullptr;
    
    // 禁止重复切换场景
    bool _isTransitioning = false;
    
    // 地图信息
    float scale = 1.8f;
    cocos2d::Size _mapSize;
    
    // 摄像机偏移（上看/下看）
    float _cameraOffsetY = 0.0f;
    float _targetCameraOffsetY = 0.0f;
    
    // HP和Soul UI
    cocos2d::Node* _uiLayer = nullptr;           // UI层（跟随摄像机）
    cocos2d::Sprite* _hpBg = nullptr;            // 血条背景
    cocos2d::Sprite* _soulBg = nullptr;          // 灵魂背景
    std::vector<cocos2d::Sprite*> _hpBars;       // 血量图标
    cocos2d::Sprite* _hpLose = nullptr;          // 失去血量图标
    int _lastDisplayedHP = 0;                    // 上次显示的HP
    int _lastDisplayedSoul = 0;                  // 上次显示的灵魂值
    
    // 血量恢复动画相关
    bool _isHPRecovering = false;                // 是否正在恢复血量
    int _hpRecoverTarget = 0;                    // 目标血量
    int _hpRecoverCurrent = 0;                   // 当前恢复到的血量
    float _hpRecoverTimer = 0.0f;                // 恢复计时器
    float _hpRecoverInterval = 0.3f;             // 每个血量恢复间隔
    
    // 坐下状态追踪
    bool _wasSitting = false;                    // 上一帧是否坐着
};

#endif // __GAME_SCENE_H__
