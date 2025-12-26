// CrawlidMonster.h (完整修正版)

#ifndef __CRAWLID_MONSTER_H__
#define __CRAWLID_MONSTER_H__

#include "cocos2d.h"

// 定义动作的Tag常量
enum class CrawlidMonsterActionTag
{
    WALK_ANIMATION_TAG = 101, // 包含在 PATROL_ACTION_TAG 内部的行走动画的 Tag
    PATROL_ACTION_TAG = 102,  // 整个巡逻循环序列的 Tag
    STUN_ACTION_TAG = 103     // 硬直/受击动作的 Tag
};

class CrawlidMonster : public cocos2d::Sprite
{
public:
    static CrawlidMonster* createMonster();//

    virtual bool init();

    CREATE_FUNC(CrawlidMonster);

    // --- 【新增接口】 ---
    /**
     * 设置 Crawlid 水平巡逻的范围。
     */
    void setupPatrolRange(float patrolRange);
    // -------------------
    // 【新增接口】：静态生成函数，负责创建、配置和添加到父节点
    static CrawlidMonster* createAndSpawn(cocos2d::Node* parent, const cocos2d::Vec2& spawnPosition, float patrolRange);
    // --- 核心逻辑函数 ---
    cocos2d::ActionInterval* playWalkAnimation();

    /**
     * 启动小怪的自动巡逻逻辑（包含移动、停止和转向）。
     */
    void startPatrol();

    /**
     * 模拟小怪受到攻击。
     */
    void takeDamage(int damage, float knockbackPower, int knockbackDirection);

    /**
     * 死亡处理：现在带有击退参数
     */
    void die(float knockbackPower, int knockbackDirection);

    // 成员变量
    int _health;
    bool _isStunned;

private:
    bool _isPatrolling;

    // 【修正/新增】私有成员声明
    void resumePatrol(); // 原始文件中缺失的声明
    float _patrolRange;
    cocos2d::Vec2 _initialPosition;
};

#endif // __CRAWLID_MONSTER_H__