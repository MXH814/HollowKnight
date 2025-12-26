// VengeflyMonster.h (整合与修改后版本)

#ifndef __VENGEFLY_MONSTER_H__
#define __VENGEFLY_MONSTER_H__

#include "cocos2d.h"
#include "TheKnight.h"  // 【修改】包含完整定义而非前向声明
#include <random>

// Vengefly 状态机枚举
enum class VengeflyState
{
    PATROL,   // 巡逻
    CHASE,    // 追击
    STUN,     // 硬直
    DEAD      // 死亡
};

// Vengefly 动作Tag枚举
enum class VengeflyMonsterActionTag
{
    NORMAL_FLY_ANIMATION_TAG = 101,
    PATROL_MOVEMENT_TAG = 102,
    CHASE_MOVEMENT_TAG = 103,
    STUN_ACTION_TAG = 104,
    CURRENT_MOVEMENT_TAG = 105
};

class VengeflyMonster : public cocos2d::Sprite
{
public:
    // 静态创建函数
    static VengeflyMonster* createMonster();

    // 【新增】标准化创建和生成接口
    static VengeflyMonster* createAndSpawn(cocos2d::Node* parent, const cocos2d::Vec2& spawnPosition, float patrolRange);

    // 初始化
    virtual bool init();

    CREATE_FUNC(VengeflyMonster);

    // 设置巡逻区域
    void setupPatrolArea(const cocos2d::Vec2& center, float patrolRange);

    // 开始巡逻
    void startPatrol();

    // 开始追击
    void startChase();

    // 每帧更新 (追击移动 + 碰撞检测)
    void update(float dt) override;

    // 设置玩家位置 (用于距离检测)
    void setPlayerPosition(const cocos2d::Vec2& playerPos);

    // 【新增】受击接口 - 与 Gruzzer 一致
    void takeDamage(int damage, float knockbackPower, int knockbackDirection);

    // 【新增】获取碰撞盒
    cocos2d::Rect getBoundingBox() const;

    // 【新增】公开成员变量（用于碰撞检测）
    int _health = 3;
    bool _isStunned = false;
    
    // 【新增】平台碰撞数据（公开访问权限，参考 Gruzzer）
    std::vector<Platform> _platforms;

protected:
    // === 移动相关参数 ===
    float _patrolSpeed = 80.0f;      // 巡逻速度
    float _chaseSpeed = 180.0f;      // 追击速度
    float _patrolRange = 100.0f;     // 【修改】巡逻范围（从200缩小到100）
    float _chaseRadius = 400.0f;     // 【修改】追击检测半径（从300扩大到400）
    
private:
    // 动画创建辅助函数
    cocos2d::SpriteFrame* createFrame(const char* filename, const cocos2d::Size& size);
    cocos2d::ActionInterval* createNormalFlyAnimate();
    cocos2d::ActionInterval* createAttackFlyAnimate();

    // 死亡序列 - 改为private
    void deathSequence(float knockbackPower, int knockbackDirection);

    // Vengefly 状态
    VengeflyState _currentState;

    // 巡逻相关
    cocos2d::Vec2 _initialPosition;
    
    // 【新增】玩家位置（用于追击和距离检测）
    cocos2d::Vec2 _playerPosition;
    
    // 【新增】飞行范围限制（参考 Gruzzer 的 _limitRange）
    cocos2d::Rect _flyRange;
    cocos2d::Vec2 _velocity;  // 当前速度向量
};

#endif // __VENGEFLY_MONSTER_H__