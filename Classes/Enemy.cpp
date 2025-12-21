/**
 * @file Enemy.cpp
 * @brief 敌人类实现文件 - 红色移动小方块
 */

#include "Enemy.h"

Enemy* Enemy::create()
{
    Enemy* enemy = new (std::nothrow) Enemy();
    if (enemy && enemy->init())
    {
        enemy->autorelease();
        return enemy;
    }
    CC_SAFE_DELETE(enemy);
    return nullptr;
}

bool Enemy::init()
{
    if (!Node::init())
    {
        return false;
    }
    
    // 初始化属性
    _hp = 2;
    _moveSpeed = 100.0f;
    _movingRight = true;
    _minX = 0;
    _maxX = 1000;
    _hitFlashTimer = 0.0f;
    _isFlashing = false;
    _isHitCooldown = false;
    _hitCooldownTimer = 0.0f;
    _hitCooldownDuration = 0.3f;  // 0.3秒受击冷却，防止一次攻击多次伤害
    
    // 创建红色方块 (40x40 大小)
    _sprite = LayerColor::create(Color4B(255, 50, 50, 255), 40, 40);
    _sprite->setAnchorPoint(Vec2(0.5f, 0.0f));
    _sprite->ignoreAnchorPointForPosition(false);
    this->addChild(_sprite);
    
    // 设置节点大小
    this->setContentSize(Size(40, 40));
    this->setAnchorPoint(Vec2(0.5f, 0.0f));
    
    // 启用update
    this->scheduleUpdate();
    
    return true;
}

void Enemy::setMoveBounds(float minX, float maxX)
{
    _minX = minX;
    _maxX = maxX;
}

Rect Enemy::getBoundingBox() const
{
    auto size = this->getContentSize();
    auto pos = this->getPosition();
    // 锚点在底部中心(0.5, 0)
    return Rect(pos.x - size.width / 2, pos.y, size.width, size.height);
}

void Enemy::takeDamage(int damage)
{
    if (_isHitCooldown) return;  // 冷却中不受伤害
    
    _hp -= damage;
    
    // 进入受击冷却
    _isHitCooldown = true;
    _hitCooldownTimer = _hitCooldownDuration;
    
    // 受击闪烁效果
    _isFlashing = true;
    _hitFlashTimer = 0.15f;
    _sprite->setColor(Color3B(255, 255, 255));  // 变白
}

void Enemy::update(float dt)
{
    // 处理受击冷却
    if (_isHitCooldown)
    {
        _hitCooldownTimer -= dt;
        if (_hitCooldownTimer <= 0)
        {
            _isHitCooldown = false;
        }
    }
    
    // 处理闪烁效果
    if (_isFlashing)
    {
        _hitFlashTimer -= dt;
        if (_hitFlashTimer <= 0)
        {
            _isFlashing = false;
            _sprite->setColor(Color3B(255, 50, 50));  // 恢复红色
        }
    }
    
    // 如果死亡则不移动
    if (isDead()) return;
    
    // 左右移动
    Vec2 pos = this->getPosition();
    
    if (_movingRight)
    {
        pos.x += _moveSpeed * dt;
        if (pos.x >= _maxX)
        {
            pos.x = _maxX;
            _movingRight = false;
        }
    }
    else
    {
        pos.x -= _moveSpeed * dt;
        if (pos.x <= _minX)
        {
            pos.x = _minX;
            _movingRight = true;
        }
    }
    
    this->setPosition(pos);
}
