/**
 * @file Enemy.h
 * @brief 敌人基类头文件
 * @details 定义了游戏中敌人的基本行为和属性
 * 
 * C++ 特性使用说明：
 * - 类与多态：继承自 cocos2d::Node，支持派生类继承
 * - const 成员函数：所有状态查询方法
 * - override 关键字：重写虚函数
 */

#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "cocos2d.h"

USING_NS_CC;

/**
 * @class Enemy
 * @brief 敌人基类
 * @details 实现了基本的敌人行为，包括移动、受击、死亡等
 * 
 * 设计说明：
 * - 可继承该类创建不同类型的敌人
 * - 重写 init() 设置外观
 * - 重写 update() 实现自定义 AI
 */
class Enemy : public Node {
public:
    // ========================================================================
    // 工厂方法与生命周期
    // ========================================================================
    
    /**
     * @brief 创建敌人实例
     * @return 敌人指针，失败返回 nullptr
     */
    static Enemy* create();
    
    /**
     * @brief 初始化敌人
     * @return 初始化是否成功
     */
    virtual bool init() override;
    
    /**
     * @brief 每帧更新
     * @param dt 距上一帧的时间间隔（秒）
     */
    void update(float dt) override;
    
    // ========================================================================
    // 碰撞检测
    // ========================================================================
    
    /**
     * @brief 获取碰撞盒
     * @return 碰撞矩形
     */
    Rect getBoundingBox() const;
    
    // ========================================================================
    // 战斗接口
    // ========================================================================
    
    /**
     * @brief 受到伤害
     * @param damage 伤害值
     */
    void takeDamage(int damage);
    
    /**
     * @brief 是否可以受到伤害
     * @return 是否可受击（有受击冷却）
     */
    bool canTakeDamage() const { return !_isHitCooldown; }
    
    // ========================================================================
    // 状态查询（const 成员函数）
    // ========================================================================
    
    /** @brief 是否死亡 */
    bool isDead() const { return _hp <= 0; }
    
    /** @brief 获取当前血量 */
    int getHP() const { return _hp; }
    
    // ========================================================================
    // 移动边界设置
    // ========================================================================
    
    /**
     * @brief 设置移动边界
     * @param minX 左边界
     * @param maxX 右边界
     */
    void setMoveBounds(float minX, float maxX);

private:
    // ========================================================================
    // 成员变量
    // ========================================================================
    
    LayerColor* _sprite;     ///< 敌人精灵（红色方块）
    int _hp;                 ///< 当前血量
    float _moveSpeed;        ///< 移动速度
    bool _movingRight;       ///< 是否向右移动
    float _minX;             ///< 移动左边界
    float _maxX;             ///< 移动右边界
    
    float _hitFlashTimer;    ///< 受击闪烁计时器
    bool _isFlashing;        ///< 是否正在闪烁
    
    bool _isHitCooldown;     ///< 是否处于受击冷却
    float _hitCooldownTimer; ///< 受击冷却计时器
    float _hitCooldownDuration; ///< 受击冷却时间
};

#endif // __ENEMY_H__
