/**
 * @file Enemy.cpp
 * @brief 敌人类实现文件
 * @details 实现了基础敌人的行为逻辑
 * 
 * C++ 特性使用说明：
 * - 异常安全：使用 std::nothrow 进行内存分配
 * - const 正确性：getBoundingBox() 使用 const
 */

#include "Enemy.h"

// ============================================================================
// 工厂方法
// ============================================================================

Enemy* Enemy::create() {
    // 使用 std::nothrow 进行异常安全的内存分配
    Enemy* enemy = new (std::nothrow) Enemy();
    if (enemy != nullptr && enemy->init()) {
        enemy->autorelease();
        return enemy;
    }
    CC_SAFE_DELETE(enemy);
    return nullptr;
}

// ============================================================================
// 初始化
// ============================================================================

bool Enemy::init() {
    if (!Node::init()) {
        return false;
    }
    
    // 初始化属性
    _hp = 2;
    _moveSpeed = 100.0f;
    _movingRight = true;
    _minX = 0.0f;
    _maxX = 1000.0f;
    _hitFlashTimer = 0.0f;
    _isFlashing = false;
    _isHitCooldown = false;
    _hitCooldownTimer = 0.0f;
    _hitCooldownDuration = 0.3f;  // 0.3秒受击冷却
    
    // 创建红色方块 (40x40 大小)
    _sprite = LayerColor::create(Color4B(255, 50, 50, 255), 40, 40);
    _sprite->setAnchorPoint(Vec2(0.5f, 0.0f));
    _sprite->ignoreAnchorPointForPosition(false);
    this->addChild(_sprite);
    
    // 设置节点大小
    this->setContentSize(Size(40, 40));
    this->setAnchorPoint(Vec2(0.5f, 0.0f));
    
    // 启用 update
    this->scheduleUpdate();
    
    return true;
}

// ============================================================================
// 边界设置
// ============================================================================

void Enemy::setMoveBounds(float minX, float maxX) {
    _minX = minX;
    _maxX = maxX;
}

// ============================================================================
// 碰撞检测
// ============================================================================

Rect Enemy::getBoundingBox() const {
    const auto size = this->getContentSize();
    const auto pos = this->getPosition();
    // 锚点在底部中心 (0.5, 0)
    return Rect(pos.x - size.width / 2.0f, pos.y, size.width, size.height);
}

// ============================================================================
// 战斗系统
// ============================================================================

void Enemy::takeDamage(int damage) {
    // 冷却中不受伤害
    if (_isHitCooldown) {
        return;
    }
    
    _hp -= damage;
    
    // 进入受击冷却
    _isHitCooldown = true;
    _hitCooldownTimer = _hitCooldownDuration;
    
    // 受击闪烁效果
    _isFlashing = true;
    _hitFlashTimer = 0.15f;
    _sprite->setColor(Color3B(255, 255, 255));  // 变白
}

// ============================================================================
// 每帧更新
// ============================================================================

void Enemy::update(float dt) {
    // 处理受击冷却
    if (_isHitCooldown) {
        _hitCooldownTimer -= dt;
        if (_hitCooldownTimer <= 0.0f) {
            _isHitCooldown = false;
        }
    }
    
    // 处理闪烁效果
    if (_isFlashing) {
        _hitFlashTimer -= dt;
        if (_hitFlashTimer <= 0.0f) {
            _isFlashing = false;
            _sprite->setColor(Color3B(255, 50, 50));  // 恢复红色
        }
    }
    
    // 如果死亡则不移动
    if (isDead()) {
        return;
    }
    
    // 左右移动
    Vec2 pos = this->getPosition();
    
    if (_movingRight) {
        pos.x += _moveSpeed * dt;
        if (pos.x >= _maxX) {
            pos.x = _maxX;
            _movingRight = false;
        }
    } else {
        pos.x -= _moveSpeed * dt;
        if (pos.x <= _minX) {
            pos.x = _minX;
            _movingRight = true;
        }
    }
    
    this->setPosition(pos);
}
