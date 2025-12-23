#ifndef __BOSS_SCENE_H__
#define __BOSS_SCENE_H__

#include "cocos2d.h"
#include "TheKnight.h"
#include "boss/HornetBoss.h"

class BossScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    
    void update(float dt) override;
    
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    CREATE_FUNC(BossScene);

private:
    // 从地图碰撞层解析平台
    void parseCollisionLayer();
    
    // 更新摄像机
    void updateCamera();
    
    // 更新UI
    void updateHPLabel();
    void updateSoulLabel();
    
    // 碰撞检测
    void checkCombatCollisions();
    
    // 玩家
    TheKnight* _knight;
    
    // Boss
    HornetBoss* _hornet = nullptr;
    
    // 平台列表
    std::vector<Platform> _platforms;
    
    // 地图相关
    cocos2d::TMXTiledMap* _map;
    float _mapScale;
    cocos2d::Size _mapSize;
    
    // 摄像机相关
    float _cameraOffsetY;
    float _targetCameraOffsetY;
    
    // UI标签
    cocos2d::Label* _hpLabel;
    cocos2d::Label* _soulLabel;
    cocos2d::Label* _bossHPLabel = nullptr;
    
    // 攻击命中冷却（防止一次攻击多次伤害）
    float _knightAttackCooldown = 0.0f;
    float _spellAttackCooldown = 0.0f;
};

#endif // __BOSS_SCENE_H__
