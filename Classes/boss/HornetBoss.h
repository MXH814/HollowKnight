#ifndef __HORNET_BOSS_H__
#define __HORNET_BOSS_H__

#include "cocos2d.h"
#include "AudioSettings.h"
#include "SimpleAudioEngine.h"

class HornetBoss : public cocos2d::Sprite {
public:
    static HornetBoss* createWithFolder(const std::string& folderPath);
    virtual bool init(const std::string& folderPath);

    void setBoundaries(float groundY, float minX, float maxX);
    void updateTargetPosition(cocos2d::Vec2 targetPos);

    enum class AIState {
        IDLE,
        WALKING,
        ATTACKING,
        EVADING, 
        STUNNED,  
        DEFEATED 
    };

    void playEntryAnimation(float startX, float startY);
    void playIdleAnimation();
    void playWalkAnimation();
    void playJumpAnimation(cocos2d::Vec2 targetPos);

    void playAttack1Animation(cocos2d::Vec2 targetPos);
    void playAttack2Animation(cocos2d::Vec2 targetPos);
    void playAttack3Animation(cocos2d::Vec2 targetPos);
    void playAttack4Animation(cocos2d::Vec2 targetPos); 

    void playInjuredAction1();
    void playInjuredAction2();
    void playInjuredAction2Loop(); 
    void playFallAnimation(cocos2d::Vec2 attackSourcePos);
    void playLeaveAnimation();
    void playBackjumpAnimation();

    void startBehaviorLogic1();
    void startBehaviorLogic2();
    void startBehaviorLogic3();
    void startBehaviorLogic4();
    void startBehaviorLogic5();
    void startBehaviorLogic6();
    void startBehaviorLogic7();
    void startBehaviorLogic8();
    void startBehaviorLogic9();

    // GameScene 调用的唯一接口
    // TODO: 传入正式 Player 节点后，可在外部用玩家类的真实 hurtbox / attackbox 取代模拟矩形
    // 目前使用 cocos2d::Node* 作为占位符，未来应替换为具体的 Player* 类型
    void startAI(cocos2d::Node* playerTarget);

    void resetState();

    // 增加 std::function<void()> 回调：用于位移到安全位置后继续攻击
    // NOTE: 此逻辑不依赖玩家判定矩形，可与正式玩家类共存，无需修改
    void moveToSafetyAndAttack(std::function<void()> onComplete = nullptr);

    // 供外部（如 GameScene）调用的受击接口
    // TODO: 玩家类接入后，应由玩家类的攻击逻辑（如 Player::attack()）触发此接口
    void onDamaged(); 

    // 返回 Boss 本体世界系矩形
    // TODO: 玩家类接入后，可直接用于与 Player::getAttackBox() 进行交集判定
    cocos2d::Rect getBossHitRect();   

    // 返回投掷武器世界系矩形
    // TODO: 玩家类接入后，可直接用于与 Player::getHurtBox() 进行交集判定
    cocos2d::Rect getWeaponRect();    

    // 返回乱舞丝线世界系矩形
    // TODO: 玩家类接入后，可直接用于与 Player::getHurtBox() 进行交集判定
    cocos2d::Rect getAttack4Rect();   

private:
    cocos2d::Sprite* _attack4Effect = nullptr; // 乱舞武器（丝线）特效节点；用于提供 getAttack4Rect 的世界系判定
    cocos2d::Sprite* _effectSprite; 
    float _jumpVelocityY;      
    float _gravity = -1200.0f; 
    float _jumpSpeedX;         
    bool _isJumping = false;   

    std::string _folderPath;
    float _groundY, _minX, _maxX;
    cocos2d::Vec2 _targetPos;

    cocos2d::Animate* createAnimate(const std::string& prefix, int startIdx, int endIdx, float delay, bool reverse = false);

    // 转向逻辑辅助
    void lookAtTarget();

    // 重点：物理位移的控制开关，改由具体的动画函数控制
    std::function<void(float)> _currentPhysicsUpdate = nullptr;
    void update(float dt) override;
    bool _isActionLocked = false; // 是否锁定转向和物理逻辑
    
    // 新增：朝向锁定标志
    bool _isFacingLocked = false; 

    // 保存玩家引用
    // TODO: 替换为正式 Player* 后，可直接访问玩家类的 hurtbox / attackbox
    // 例如：_player->getHurtBox()
    cocos2d::Node* _player = nullptr; 
    void decideNextAction();          // 随机决策下一个技能

    AIState _aiState = AIState::IDLE;
    int _hitCount = 0; // 受击计数
    void blinkWhite(); // 新增：变白闪烁逻辑

    int _lastAttackType = 0; // 记录上一次使用的攻击类型 (1-4)
    int _attack3RepeatCount = 0; // 记录攻击3连续使用的次数

    int _currentPhase = 1; // 当前阶段 1-4
    int _phaseHitCount = 0; // 当前阶段受击计数
    int _defeatedHitCount = 0; // 击败后受击计数
};

#endif