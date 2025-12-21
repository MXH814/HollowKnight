/**
 * @file Enemy.h
 * @brief 敌人类头文件 - 红色移动小方块
 */

#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "cocos2d.h"

USING_NS_CC;

class Enemy : public Node
{
public:
    static Enemy* create();
    
    virtual bool init() override;
    
    void update(float dt) override;
    
    // 获取碰撞盒
    Rect getBoundingBox() const;
    
    // 受到伤害
    void takeDamage(int damage);
    
    // 是否死亡
    bool isDead() const { return _hp <= 0; }
    
    // 获取血量
    int getHP() const { return _hp; }
    
    // 设置移动边界
    void setMoveBounds(float minX, float maxX);
    
    // 是否可以受到伤害（有受击冷却)
    bool canTakeDamage() const { return !_isHitCooldown; }
    
private:
    LayerColor* _sprite;    // 红色方块精灵
    int _hp;                // 血量
    float _moveSpeed;       // 移动速度
    bool _movingRight;      // 是否向右移动
    float _minX;            // 移动左边界
    float _maxX;            // 移动右边界
    
    float _hitFlashTimer;   // 受击闪烁计时器
    bool _isFlashing;       // 是否正在闪烁
    
    bool _isHitCooldown;    // 是否处于受击冷却
    float _hitCooldownTimer;// 受击冷却计时器
    float _hitCooldownDuration; // 受击冷却时间
};

#endif // __ENEMY_H__
