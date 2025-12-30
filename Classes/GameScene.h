#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include "TheKnight.h"
#include "PauseMenu.h" 

class GameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    
    // 带自定义出生点和朝向的场景创建方法
    static cocos2d::Scene* createSceneWithSpawn(const cocos2d::Vec2& spawnPos, bool facingRight);
    static Scene* createSceneForRespawn();  // 【新增】从 NextScene 死亡后返回的场景创建方法
    
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
    void startHPRecoveryAnimation();
    void updateHPRecoveryAnimation(float dt);

    // 交互对象
    struct InteractiveObject {
        cocos2d::Vec2 position;
        std::string name;
        float radius;
    };
    std::vector<InteractiveObject> _interactiveObjects;
    
    // 平台列表（碰撞检测）
    std::vector<Platform> _platforms;
    
    // 角色
    TheKnight* _knight = nullptr;
    
    // UI 显示标签
    cocos2d::Label* _interactionLabel = nullptr;
    
    // 防止重复切换场景
    bool _isTransitioning = false;
    
    // 地图信息
    float scale = 1.8f;
    cocos2d::Size _mapSize;
    
    // 摄像机偏移（上看/下看）
    float _cameraOffsetY = 0.0f;
    float _targetCameraOffsetY = 0.0f;
    
    // HP和Soul UI
    cocos2d::Node* _uiLayer = nullptr;
    cocos2d::Sprite* _hpBg = nullptr;
    cocos2d::Sprite* _soulBg = nullptr;
    std::vector<cocos2d::Sprite*> _hpBars;
    std::vector<cocos2d::Sprite*> _hpEmptyBars;
    cocos2d::Sprite* _hpLose = nullptr;
    int _lastDisplayedHP = 0;
    int _lastDisplayedSoul = 0;
    
    // 血量恢复动画相关
    bool _isHPRecovering = false;
    int _hpRecoverTarget = 0;
    int _hpRecoverCurrent = 0;
    float _hpRecoverTimer = 0.0f;
    float _hpRecoverInterval = 0.3f;
    
    // 坐姿状态追踪
    bool _wasSitting = false;
    
    // 静态变量（从NextScene返回时可以传参）
    static bool s_hasCustomSpawn;
    static cocos2d::Vec2 s_customSpawnPos;
    static bool s_spawnFacingRight;
    static bool s_spawnDoJump;

    PauseMenu* _pauseMenu = nullptr;

    // Geo UI
    cocos2d::Sprite* _geoIcon = nullptr;
    cocos2d::Label* _geoLabel = nullptr;
    int _lastDisplayedGeo = 0;

    void showMap();
    void hideMap();
    cocos2d::LayerColor* _mapOverlay = nullptr;    // 半透明遮罩层
    cocos2d::Sprite* _mapSprite = nullptr;         // 地图图片
    bool _isMapVisible = false;
};

#endif // __GAME_SCENE_H__
