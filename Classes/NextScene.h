#ifndef __NEXT_SCENE_H__
#define __NEXT_SCENE_H__

#include "cocos2d.h"
#include "TheKnight.h"

// 出口对象结构体
struct ExitObject {
    cocos2d::Vec2 position;
    float radius;
};

// 尖刺对象结构体
struct ThornObject {
    cocos2d::Vec2 position;
    cocos2d::Size size;
};

class NextScene : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    virtual void update(float dt) override;
    
    CREATE_FUNC(NextScene);

private:
    void createCollisionFromTMX(cocos2d::TMXTiledMap* map, 
                                 const std::string& layerName, 
                                 float scale, 
                                 const cocos2d::Vec2& mapOffset);
    
    // 创建陷阱背景精灵
    void createTrapSprites(cocos2d::TMXTiledMap* map,
                           const std::string& layerName,
                           const std::string& trapType,
                           const std::string& spritePath,
                           float scale,
                           const cocos2d::Vec2& mapOffset);
    
    // 加载出口对象
    void loadExitObjects(cocos2d::TMXTiledMap* map, float scale, const cocos2d::Vec2& mapOffset);
    
    // 加载尖刺对象
    void loadThornObjects(cocos2d::TMXTiledMap* map, float scale, const cocos2d::Vec2& mapOffset);
    
    // 加载前景对象（bg类，显示在角色上层）
    void loadForegroundObjects(cocos2d::TMXTiledMap* map, float scale, const cocos2d::Vec2& mapOffset);
    
    // 检测交互
    void checkInteractions();
    
    // 屏幕震动效果
    void shakeScreen(float duration, float intensity);
    
    // HP和Soul UI相关
    void createHPAndSoulUI();
    void updateHPAndSoulUI(float dt);
    
    // 物理碰撞回调
    bool onContactBegin(cocos2d::PhysicsContact& contact);
    
    std::vector<Platform> _platforms;  // 碰撞平台列表
    std::vector<ExitObject> _exitObjects;   // 出口对象列表
    std::vector<ThornObject> _thornObjects; // 尖刺对象列表
    
    cocos2d::Label* _exitLabel = nullptr;   // 出口提示标签
    cocos2d::Label* _thornLabel = nullptr;  // 尖刺警告标签
    
    bool _hasLandedOnce = false;       // 是否已经落地
    bool _isTransitioning = false;     // 是否正在场景切换
    bool _isNearExit = false;          // 是否靠近出口
    bool _isNearThorn = false;         // 是否靠近尖刺
    
    // 屏幕震动相关
    bool _isShaking = false;           // 是否正在震动
    float _shakeDuration = 0.0f;       // 震动持续时间
    float _shakeElapsed = 0.0f;        // 震动已经过时间
    float _shakeIntensity = 0.0f;      // 震动强度
    cocos2d::Vec2 _shakeOffset;        // 震动偏移量
    
    // HP和Soul UI
    cocos2d::Node* _uiLayer = nullptr;
    cocos2d::Sprite* _hpBg = nullptr;
    cocos2d::Sprite* _soulBg = nullptr;
    std::vector<cocos2d::Sprite*> _hpBars;
    cocos2d::Sprite* _hpLose = nullptr;
    int _lastDisplayedHP = 0;
    int _lastDisplayedSoul = 0;
};

#endif // __NEXT_SCENE_H__
