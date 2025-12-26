#ifndef __Tiktik_MONSTER_H__
#define __Tiktik_MONSTER_H__

#include "cocos2d.h"
#include <vector>

USING_NS_CC;

// 定义动作的Tag常量
enum class TiktikMonsterActionTag
{
    WALK_ANIMATION_TAG = 101,   // 行走动画的 Tag (RepeatForever内部)
    PATROL_MOVEMENT_TAG = 102,  // 移动动作的 Tag (MoveTo)
    PATROL_SEQUENCE_TAG = 103,  // 整个巡逻序列的 Tag (Sequence)
    STUN_ACTION_TAG = 104       // 硬直/受击动作的 Tag
};

class TiktikMonster : public cocos2d::Sprite
{
public:
    static TiktikMonster* createMonster();

    virtual bool init();

    CREATE_FUNC(TiktikMonster);

    // 【新增接口】：静态生成函数，负责创建、配置和添加到父节点
    static TiktikMonster* createAndSpawn(cocos2d::Node* parent, const cocos2d::Vec2& centerPosition, float halfWidth, float halfHeight);
    // --- 核心逻辑函数 ---

    /**
     * 创建行走动画的动作实例 (Animate 动作)，不立即运行。
     */
    cocos2d::ActionInterval* createWalkAnimate();

    /**
     * 创建转弯动画的动作实例 (Animate 动作)，不立即运行。
     */
    cocos2d::ActionInterval* createTurnAnimate();

    /**
     * 【新增】设置巡逻区域的中心点和半边长，并初始化巡逻路径。
     * 小怪将根据这些参数计算四个巡逻点，并初始化在右下角。
     * @param center 巡逻区域的中心点。
     * @param halfWidth 巡逻区域宽度的一半。
     * @param halfHeight 巡逻区域高度的一半。
     */
    void setupPatrolArea(const Vec2& center, float halfWidth, float halfHeight);

    /**
     * 启动小怪的自动巡逻逻辑。
     */
    void startPatrol();

    /**
     * 执行巡逻的下一步：移动到下一个点，并播放动画。
     */
    void executeNextPatrolStep();

    /**
     * 恢复巡逻（在受击停顿后继续）。
     */
    void resumePatrol();

    /**
     * 模拟小怪受到攻击。
     */
    void takeDamage(int damage, float knockbackPower, int knockbackDirection);

    /**
     * 死亡处理：带有击退参数
     */
    void die(float knockbackPower, int knockbackDirection);

    // 成员变量
    int _health;        // 初始为 3
    bool _isStunned;    // 是否处于硬直/死亡状态

private:
    // 巡逻点的集合 (角点)
    std::vector<Vec2> _patrolPoints;
    // 当前巡逻段的索引 (从当前点移动到下一个点)
    int _currentPatrolIndex;
    // 巡逻是否激活
    bool _isPatrolling;

    // 巡逻段的旋转角度（模拟脚贴着岩石行走）
    std::vector<float> _patrolRotations;

    // 巡逻速度
    float _patrolSpeed = 80.0f; // 像素/秒

    // 停止所有关键动作
    void stopAllMonsterActions();
};

#endif // __Tiktik_MONSTER_H__