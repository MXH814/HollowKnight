#ifndef __BOSS_SCENE_H__
#define __BOSS_SCENE_H__

#include "cocos2d.h"
#include "TheKnight.h"
#include "boss/HornetBoss.h"
#include "AudioManager.h"

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
    
    // 更新摄像机
    void updateCamera();
    
    // HP和Soul UI相关
    void createHPAndSoulUI();
    void updateHPAndSoulUI(float dt);
    
    // 碰撞检测
    void checkCombatCollisions();
    
    // 战斗结束相关
    void onKnightDefeated();
    void onBossDefeated();
    void createRewardPickup();
    void checkRewardPickup();
    void collectReward();
    void showRewardAtPickup(const cocos2d::Vec2& pickupPos);
    void returnToMainMenu();
    
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
    
    // 摄像机相关
    float _cameraOffsetY = 0.0f;
    float _targetCameraOffsetY = 0.0f;
    
    // HP和Soul UI
    cocos2d::Node* _uiLayer = nullptr;
    cocos2d::Sprite* _hpBg = nullptr;
    cocos2d::Sprite* _soulBg = nullptr;
    std::vector<cocos2d::Sprite*> _hpBars;
    cocos2d::Sprite* _hpLose = nullptr;
    int _lastDisplayedHP = 0;
    int _lastDisplayedSoul = 0;
    
    // Boss HP标签
    cocos2d::Label* _bossHPLabel = nullptr;
    
    // 攻击命中冷却
    float _knightAttackCooldown = 0.0f;
    float _spellAttackCooldown = 0.0f;
    
    // 战斗结束状态
    bool _isBattleEnded = false;
    bool _isKnightDefeated = false;
    bool _isBossDefeated = false;
    bool _isPlayingDeathAnim = false;
    float _deathAnimTimer = 0.0f;
    
    // 奖励拾取相关
    bool _rewardCollected = false;
    bool _isNearReward = false;
    
    // 结果显示层
    cocos2d::Node* _resultLayer = nullptr;
};

#endif // __BOSS_SCENE_H__
