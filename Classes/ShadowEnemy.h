#ifndef __SHADOW_ENEMY_H__
#define __SHADOW_ENEMY_H__

#include "cocos2d.h"
#include "TheKnight.h" // 修改：包含TheKnight头文件

class ShadowEnemy : public cocos2d::Node {
public:
    // 状态机
    enum class State {
        IDLE,
        CHASE,
        INJURED,
        DYING,
        RETURN
    };

    virtual bool init() override;
    CREATE_FUNC(ShadowEnemy);

    void setTarget(TheKnight* target); // 修改：目标类型改为TheKnight*
    void takeDamage();
    
    // 获取碰撞框
    cocos2d::Rect getHitBox();

protected:
    void update(float dt) override;
    void updateIdle(float dt);
    void updateChase(float dt);
    void initAnimations();
    void changeState(State newState);
    cocos2d::Animation* createAnimation(std::string prefix, int start, int end, float delay);

    cocos2d::Sprite* _display;
    TheKnight* _target; // 修改：目标类型改为TheKnight*
    State _currentState;
    State _previousState; // 用于保存受击前的状态
    int _hp;
    float _moveSpeed;
    bool _isFacingLeft;
    float _detectionRange; // 侦测范围

    // 内部冷却计时器
    float _playerDamageCooldown;

    // 动画动作
    cocos2d::Action* _walkAction;
    cocos2d::Action* _deathAction;
    cocos2d::Action* _returnAction;
};

#endif // __SHADOW_ENEMY_H__