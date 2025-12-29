#ifndef __BOSS_SCENE_H__
#define __BOSS_SCENE_H__

#include "cocos2d.h"
#include "TheKnight.h"
#include "boss/HornetBoss.h"
#include "PauseMenu.h"

class BossScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    
    void update(float dt) override;
    
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    CREATE_FUNC(BossScene);

private:
    // 从地图解析碰撞平台
    void parseCollisionLayer();
    
    // 摄像机跟随
    void updateCamera();
    
    // HP和Soul UI相关
    void createHPAndSoulUI();
    void updateHPAndSoulUI(float dt);
    
    // 碰撞检测
    void checkCombatCollisions();
    
    // 战斗结束处理
    void onKnightDefeated();                    // 骑士被击败
    void onBossDefeated();                      // Boss被击败
    void createRewardPickup();                  // 创建奖励拾取物
    void checkRewardPickup();                   // 检测奖励拾取
    void showRewardUI();                        // 显示奖励界面
    void returnToMainMenu();                    // 返回主菜单
    
    // 玩家
    TheKnight* _knight = nullptr;
    
    // Boss
    HornetBoss* _hornet = nullptr;
    
    // 平台列表
    std::vector<Platform> _platforms;
    
    // 地图相关
    cocos2d::TMXTiledMap* _map = nullptr;
    float scale = 1.0f;
    cocos2d::Size _mapSize;
    
    // 摄像机偏移
    float _cameraOffsetY = 0.0f;
    float _targetCameraOffsetY = 0.0f;
    
    // HP和Soul UI
    cocos2d::Node* _uiLayer = nullptr;
    cocos2d::Sprite* _hpBg = nullptr;
    cocos2d::Sprite* _soulBg = nullptr;
    std::vector<cocos2d::Sprite*> _hpBars;
    std::vector<cocos2d::Sprite*> _hpEmptyBars;  // 【新增】空血槽图标数组
    cocos2d::Sprite* _hpLose = nullptr;
    int _lastDisplayedHP = 0;
    int _lastDisplayedSoul = 0;
    
    // Boss HP标签
    cocos2d::Label* _bossHPLabel = nullptr;
    
    // 攻击冷却（防止一次攻击造成多次伤害）
    float _knightAttackCooldown = 0.0f;
    float _spellAttackCooldown = 0.0f;
    
    // 暂停菜单（保留备用）
    PauseMenu* _pauseMenu = nullptr;

    cocos2d::Sprite* _geoIcon = nullptr;
    cocos2d::Label* _geoLabel = nullptr;
    int _lastDisplayedGeo = 0;
    
    // 战斗结束状态
    bool _isBattleEnded = false;                // 战斗是否已结束
    bool _isKnightDefeated = false;             // 骑士是否被击败
    bool _isBossDefeated = false;               // Boss是否被击败
    float _deathAnimTimer = 0.0f;               // 死亡动画计时器
    bool _isPlayingDeathAnim = false;           // 是否正在播放死亡动画
    
    // 奖励拾取物
    cocos2d::Sprite* _rewardPickup = nullptr;   // 奖励拾取物精灵
    bool _rewardCollected = false;              // 奖励是否已被拾取

    // 结果UI层
    cocos2d::Node* _resultLayer = nullptr;      // 结果显示层

    // 是否在奖励拾取范围内
    bool _isNearReward = false;  
    
    void collectReward();  // 拾取奖励
    void showRewardAtPickup(const cocos2d::Vec2& pickupPos);  // 在拾取位置显示奖励UI
};

#endif // __BOSS_SCENE_H__
