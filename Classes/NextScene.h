#ifndef __NEXT_SCENE_H__
#define __NEXT_SCENE_H__

#include "cocos2d.h"
#include "TheKnight.h"
#include "CorniferNPC.h"
#include "ShadowEnemy.h"  // 添加 Shadow 头文件
#include "PauseMenu.h"

// 【修改】ExitObject 结构体 - 根据 NextScene.cpp 的使用方式定义
struct ExitObject {
    cocos2d::Sprite* sprite;           // 出口精灵
    cocos2d::Vec2 position;            // 出口位置
    float radius;                       // 触发半径
    std::string targetScene;            // 目标场景名称
    
    ExitObject() : sprite(nullptr), position(cocos2d::Vec2::ZERO), radius(0.0f), targetScene("") {}
};

// 【修改】ThornObject 结构体 - 根据 NextScene.cpp 的使用方式定义
struct ThornObject {
    cocos2d::Sprite* sprite;           // 尖刺精灵
    cocos2d::Vec2 position;            // 尖刺位置
    cocos2d::Size size;                // 尖刺尺寸
    int damage;                         // 伤害值
    
    ThornObject() : sprite(nullptr), position(cocos2d::Vec2::ZERO), size(cocos2d::Size::ZERO), damage(1) {}
};

class NextScene : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
    
    // 新增：带重生标志的场景创建方法
    static cocos2d::Scene* createSceneWithRespawn();
    
    virtual bool init() override;
    virtual void update(float dt) override;
    
    // 新增：Knight 死亡回调
    void onKnightDeath(const cocos2d::Vec2& deathPos);
    
    CREATE_FUNC(NextScene);
    
    // 【修改】获取碰撞平台数据（使用 TheKnight.h 中的 Platform 定义）
    const std::vector<Platform>& getPlatforms() const { return _platforms; }
    
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
    
    // 加载前景对象(bg类，显示在角色上层)
    void loadForegroundObjects(cocos2d::TMXTiledMap* map, float scale, const cocos2d::Vec2& mapOffset);
    
    // 检测交互
    void checkInteractions();
    
    // 尖刺死亡流程
    void startSpikeDeath(TheKnight* knight);
    void updateSpikeDeath(float dt, TheKnight* knight);
    
    // 屏幕震动效果
    void shakeScreen(float duration, float intensity);
    
    // HP和Soul UI相关
    void createHPAndSoulUI();
    void updateHPAndSoulUI(float dt);
    
    // 【新增】战斗碰撞检测 (参考BossScene)
    void checkCombatCollisions();
    
    // 物理碰撞回调
    bool onContactBegin(cocos2d::PhysicsContact& contact);
    
    std::vector<Platform> _platforms;         // 碰撞平台列表（使用 TheKnight.h 中的 Platform）
    std::vector<ExitObject> _exitObjects;     // 出口对象列表
    std::vector<ThornObject> _thornObjects;   // 尖刺对象列表
    
    cocos2d::Label* _exitLabel = nullptr;   // 出口提示标签
    cocos2d::Label* _thornLabel = nullptr;  // 尖刺警告标签
    
    bool _hasLandedOnce = false;       // 是否已经着陆
    bool _isTransitioning = false;     // 是否正在场景切换
    bool _isNearExit = false;          // 是否靠近出口
    bool _isNearThorn = false;         // 是否靠近尖刺
    
    // 尖刺伤害相关
    bool _isInSpikeDeath = false;      // 是否正在尖刺死亡流程
    cocos2d::Vec2 _lastSafePosition;   // 最后的安全位置
    float _spikeDeathTimer = 0.0f;     // 尖刺死亡计时器
    int _spikeDeathPhase = 0;          // 尖刺死亡阶段(0=未开始, 1=播放动画, 2=黑屏, 3=重生)
    cocos2d::LayerColor* _blackScreen = nullptr;  // 黑屏遮罩
    
    // 屏幕震动相关
    bool _isShaking = false;           // 是否正在震动
    float _shakeDuration = 0.0f;       // 震动持续时间
    float _shakeElapsed = 0.0f;        // 已经过时间
    float _shakeIntensity = 0.0f;      // 震动强度
    cocos2d::Vec2 _shakeOffset;        // 震动偏移量
    
    // 【新增】攻击冷却时间 (参考BossScene)
    float _knightAttackCooldown = 0.0f;  // Knight普通攻击冷却
    float _spellAttackCooldown = 0.0f;   // Knight法术攻击冷却
    
    // HP和Soul UI
    cocos2d::Node* _uiLayer = nullptr;
    cocos2d::Sprite* _hpBg = nullptr;
    cocos2d::Sprite* _soulBg = nullptr;
    std::vector<cocos2d::Sprite*> _hpBars;
    cocos2d::Sprite* _hpLose = nullptr;
    int _lastDisplayedHP = 0;
    int _lastDisplayedSoul = 0;
    
    // 【新增】Cornifer NPC 引用
    CorniferNPC* _cornifer = nullptr;
    
    // 【新增】TheKnight 引用
    TheKnight* _player = nullptr;
    
    // 新增：Shade 实例
    ShadowEnemy* _shade = nullptr;
    
    // 新增：重生相关静态变量
    static bool s_isRespawning;
    static cocos2d::Vec2 s_shadePosition;
    
    // 新增：Shade 相关方法
    void spawnShade(const cocos2d::Vec2& position);
    void removeShade();
    void updateShade(float dt);

    PauseMenu* _pauseMenu = nullptr;

    cocos2d::Sprite* _geoIcon = nullptr;
    cocos2d::Label* _geoLabel = nullptr;
    int _lastDisplayedGeo = 0;
};

#endif // __NEXT_SCENE_H__
