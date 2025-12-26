#ifndef __GRUZZER_MONSTER_H__
#define __GRUZZER_MONSTER_H__

#include "cocos2d.h"
#include "TheKnight.h"  // 【修改】包含完整定义而非前向声明

// 【新增】动作Tag枚举
enum class GruzzerMonsterActionTag
{
    FLY_ANIMATION_TAG = 101,
    MOVEMENT_TAG = 102,
    STUN_ACTION_TAG = 103
};

class GruzzerMonster : public cocos2d::Sprite {
public:
    static GruzzerMonster* createWithFrames();

    // Static helper to create and spawn the monster (Standardized interface)
    static GruzzerMonster* createAndSpawn(cocos2d::Node* parent, cocos2d::Vec2 startPos, cocos2d::Rect moveRange, float speed);

    // 析构函数释放资源
    virtual ~GruzzerMonster();

    // 初始化接口：位置、巡行范围（Rect）、飞行速度
    bool initMonster(cocos2d::Vec2 startPos, cocos2d::Rect moveRange, float speed);

    void update(float dt) override;
    
    // 【新增】受击接口 - 与 Crawlid/Tiktik 一致
    void takeDamage(int damage, float knockbackPower, int knockbackDirection);
    
    // 【新增】获取碰撞盒
    cocos2d::Rect getBoundingBox() const;

    // 【新增】公开成员变量（用于碰撞检测）
    int _health = 2;
    bool _isStunned = false;

private:
    void initAnimations();
    void updateAnimation(float dt);
    void die(float knockbackPower, int knockbackDirection);

    enum class State { FLYING, HURT, DEAD } _state;

    cocos2d::Rect _limitRange;
    cocos2d::Vec2 _velocity;
    float _speed;

    // 【新增】平台碰撞数据
    std::vector<Platform> _platforms;

    // 动画资源
    cocos2d::Animation* _flyAnim;
    cocos2d::Animation* _deathAnim;
    bool _isFacingRight = false;
};

#endif // __GRUZZER_MONSTER_H__