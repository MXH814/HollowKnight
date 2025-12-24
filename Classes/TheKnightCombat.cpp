/**
 * @file TheKnightCombat.cpp
 * @brief 小骑士（TheKnight）角色类 - 战斗相关实现
 */

#include "TheKnight.h"

void TheKnight::startSlash()
{
    _isAttacking = true;
    _currentSlashType = 0;  // 水平攻击
    _slashEffectTimer = 0.0f;
    _slashEffectPhase = 1;
    createSlashEffect(1);
    
    // 水平攻击后坐力（向身后位移），SteadyBody护符可以消除
    if (!_charmSteadyBody)
    {
        float recoilDistance = 30.0f;  // 后坐力距离
        float recoilDir = _facingRight ? -1.0f : 1.0f;  // 向身后
        
        Vec2 pos = this->getPosition();
        pos.x += recoilDir * recoilDistance;
        
        // 检查墙壁碰撞，防止穿墙
        float correctedX;
        if (recoilDir > 0 && checkWallCollision(correctedX, true))
        {
            pos.x = correctedX;
        }
        else if (recoilDir < 0 && checkWallCollision(correctedX, false))
        {
            pos.x = correctedX;
        }
        
        this->setPosition(pos);
    }
    
    changeState(KnightState::SLASHING);
}

void TheKnight::startUpSlash()
{
    _isAttacking = true;
    _currentSlashType = 1;  // 向上攻击
    _slashEffectTimer = 0.0f;
    _slashEffectPhase = 1;
    createSlashEffect(1);
    changeState(KnightState::UP_SLASHING);
}

void TheKnight::startDownSlash()
{
    _isAttacking = true;
    _currentSlashType = 2;  // 向下攻击
    _slashEffectTimer = 0.0f;
    _slashEffectPhase = 1;
    createSlashEffect(1);
    changeState(KnightState::DOWN_SLASHING);
}

void TheKnight::createSlashEffect(int effectNum)
{
    // 移除旧特效
    removeSlashEffect();
    
    // 根据攻击类型选择特效图片路径
    std::string effectPath;
    switch (_currentSlashType)
    {
        case 0:  // 水平攻击
            effectPath = "TheKnight/Slash/SlashEffect/SlashEffect" + std::to_string(effectNum) + ".png";
            break;
        case 1:  // 向上攻击
            effectPath = "TheKnight/Slash/UpSlashEffect/UpSlashEffect" + std::to_string(effectNum) + ".png";
            break;
        case 2:  // 向下攻击
            effectPath = "TheKnight/Slash/DownSlashEffect/DownSlashEffect" + std::to_string(effectNum) + ".png";
            break;
    }
    
    _slashEffect = Sprite::create(effectPath);
    if (_slashEffect && this->getParent())
    {
        _slashEffect->setAnchorPoint(Vec2(0.5f, 0.5f));
        this->getParent()->addChild(_slashEffect, this->getLocalZOrder() + 1);
        updateSlashEffectPosition();
    }
}

void TheKnight::updateSlashEffectPosition()
{
    if (!_slashEffect) return;
    
    Vec2 pos = this->getPosition();
    auto knightSize = this->getContentSize();
    auto effectSize = _slashEffect->getContentSize();
    
    float effectX = pos.x;
    float effectY = pos.y + knightSize.height / 2;  // 默认垂直中心
    
    if (_slashEffectPhase == 1)
    {
        switch (_currentSlashType)
        {
            case 0:  // 水平攻击
            {
                float offsetX = (knightSize.width * 0.1f + effectSize.width / 2);
                if (_facingRight)
                {
                    effectX = pos.x + offsetX;
                }
                else
                {
                    effectX = pos.x - offsetX;
                }
                effectY = pos.y + knightSize.height / 2;
                _slashEffect->setFlippedX(_facingRight);
                break;
            }
            case 1:  // 向上攻击
            {
                effectX = pos.x;
                effectY = pos.y + knightSize.height * 0.4f + effectSize.height / 2;
                _slashEffect->setFlippedX(_facingRight);
                break;
            }
            case 2:  // 向下攻击
            {
                effectX = pos.x;
                effectY = pos.y + knightSize.height * 0.3f - effectSize.height / 2;
                _slashEffect->setFlippedX(_facingRight);
                break;
            }
        }
    }
    else if (_slashEffectPhase == 2)
    {
        switch (_currentSlashType)
        {
            case 0:  // 水平攻击收刀位置
            {
                float offsetX = (knightSize.width * 0.1f + effectSize.width / 2);
                if (_facingRight)
                {
                    effectX = pos.x + offsetX;
                }
                else
                {
                    effectX = pos.x - offsetX;
                }
                effectY = pos.y + knightSize.height / 2;
                _slashEffect->setFlippedX(_facingRight);
                break;
            }
            case 1:  // 向上攻击收刀位置
            {
                float offsetX = (-knightSize.width * 0.5f + effectSize.width / 2);
                if (_facingRight)
                {
                    effectX = pos.x - offsetX;
                }
                else
                {
                    effectX = pos.x + offsetX;
                }
                effectY = pos.y + knightSize.height * 0.2f + effectSize.height / 2;
                _slashEffect->setFlippedX(_facingRight);
                break;
            }
            case 2:  // 向下攻击收刀位置
            {
                float offsetX = (-knightSize.width * 0.5f + effectSize.width / 2);
                if (_facingRight)
                {
                    effectX = pos.x - offsetX;
                }
                else
                {
                    effectX = pos.x + offsetX;
                }
                effectY = pos.y + knightSize.height * 0.8f - effectSize.height / 2;
                _slashEffect->setFlippedX(_facingRight);
                break;
            }
        }
    }
    
    _slashEffect->setPosition(Vec2(effectX, effectY));
}

void TheKnight::removeSlashEffect()
{
    if (_slashEffect)
    {
        _slashEffect->removeFromParent();
        _slashEffect = nullptr;
    }
}

void TheKnight::updateSlash(float dt)
{
    _slashEffectTimer += dt;
    
    if (_slashEffectPhase == 1 && _slashEffectTimer >= 0.09f)
    {
        _slashEffectPhase = 2;
        createSlashEffect(2);
    }
    
    updateSlashEffectPosition();
    
    if (!_isOnGround)
    {
        _velocityY -= _gravity * dt;
        
        if (_velocityY < -1600.0f)
        {
            _velocityY = -1600.0f;
        }
        
        Vec2 pos = this->getPosition();
        pos.y += _velocityY * dt;
        this->setPosition(pos);
        
        float ceilingY;
        if (checkCeilingCollision(ceilingY))
        {
            pos.y = ceilingY;
            _velocityY = 0;
            this->setPosition(pos);
        }
        
        float groundY;
        if (checkGroundCollision(groundY))
        {
            pos.y = groundY;
            _velocityY = 0;
            _isOnGround = true;
            this->setPosition(pos);
        }
        
        if (_isMovingLeft)
        {
            pos.x -= _moveSpeed * dt;
            this->setPosition(pos);
        }
        if (_isMovingRight)
        {
            pos.x += _moveSpeed * dt;
            this->setPosition(pos);
        }
        
        float correctedX;
        if (_isMovingRight && checkWallCollision(correctedX, true))
        {
            pos.x = correctedX;
            this->setPosition(pos);
        }
        else if (_isMovingLeft && checkWallCollision(correctedX, false))
        {
            pos.x = correctedX;
            this->setPosition(pos);
        }
    }
}

void TheKnight::updateUpSlash(float dt)
{
    _slashEffectTimer += dt;
    
    if (_slashEffectPhase == 1 && _slashEffectTimer >= 0.09f)
    {
        _slashEffectPhase = 2;
        createSlashEffect(2);
    }
    
    updateSlashEffectPosition();
    
    // 上劈时不再有向上移动的效果，只处理重力和水平移动
    if (!_isOnGround)
    {
        _velocityY -= _gravity * dt;
        
        if (_velocityY < -1600.0f)
        {
            _velocityY = -1600.0f;
        }
        
        Vec2 pos = this->getPosition();
        pos.y += _velocityY * dt;
        this->setPosition(pos);
        
        float ceilingY;
        if (checkCeilingCollision(ceilingY))
        {
            pos.y = ceilingY;
            _velocityY = 0;
            this->setPosition(pos);
        }
        
        float groundY;
        if (checkGroundCollision(groundY))
        {
            pos.y = groundY;
            _velocityY = 0;
            _isOnGround = true;
            this->setPosition(pos);
        }
        
        if (_isMovingLeft)
        {
            pos.x -= _moveSpeed * dt;
            this->setPosition(pos);
        }
        if (_isMovingRight)
        {
            pos.x += _moveSpeed * dt;
            this->setPosition(pos);
        }
        
        float correctedX;
        if (_isMovingRight && checkWallCollision(correctedX, true))
        {
            pos.x = correctedX;
            this->setPosition(pos);
        }
        else if (_isMovingLeft && checkWallCollision(correctedX, false))
        {
            pos.x = correctedX;
            this->setPosition(pos);
        }
    }
}

void TheKnight::updateDownSlash(float dt)
{
    _slashEffectTimer += dt;
    
    if (_slashEffectPhase == 1 && _slashEffectTimer >= 0.09f)
    {
        _slashEffectPhase = 2;
        createSlashEffect(2);
    }
    
    updateSlashEffectPosition();
    
    if (!_isOnGround)
    {
        _velocityY -= _gravity * dt;
        
        if (_velocityY < -1600.0f)
        {
            _velocityY = -1600.0f;
        }
        
        Vec2 pos = this->getPosition();
        pos.y += _velocityY * dt;
        this->setPosition(pos);
        
        float ceilingY;
        if (checkCeilingCollision(ceilingY))
        {
            pos.y = ceilingY;
            _velocityY = 0;
            this->setPosition(pos);
        }
        
        float groundY;
        if (checkGroundCollision(groundY))
        {
            pos.y = groundY;
            _velocityY = 0;
            _isOnGround = true;
            this->setPosition(pos);
        }
        
        if (_isMovingLeft)
        {
            pos.x -= _moveSpeed * dt;
            this->setPosition(pos);
        }
        if (_isMovingRight)
        {
            pos.x += _moveSpeed * dt;
            this->setPosition(pos);
        }
        
        float correctedX;
        if (_isMovingRight && checkWallCollision(correctedX, true))
        {
            pos.x = correctedX;
            this->setPosition(pos);
        }
        else if (_isMovingLeft && checkWallCollision(correctedX, false))
        {
            pos.x = correctedX;
            this->setPosition(pos);
        }
    }
}

void TheKnight::onSlashAnimFinished()
{
    _isAttacking = false;
    removeSlashEffect();
    
    if (!_isOnGround)
    {
        if (_velocityY > 0)
        {
            changeState(KnightState::JUMPING);
        }
        else
        {
            changeState(KnightState::FALLING);
        }
    }
    else if (_isMovingLeft || _isMovingRight)
    {
        if ((_isMovingLeft && _facingRight) || (_isMovingRight && !_facingRight))
        {
            changeState(KnightState::TURNING);
        }
        else
        {
            changeState(KnightState::RUNNING);
        }
    }
    else
    {
        changeState(KnightState::IDLE);
    }
}

void TheKnight::onUpSlashAnimFinished()
{
    _isAttacking = false;
    removeSlashEffect();
    
    if (!_isOnGround)
    {
        if (_velocityY > 0)
        {
            changeState(KnightState::JUMPING);
        }
        else
        {
            changeState(KnightState::FALLING);
        }
    }
    else if (_isMovingLeft || _isMovingRight)
    {
        if ((_isMovingLeft && _facingRight) || (_isMovingRight && !_facingRight))
        {
            changeState(KnightState::TURNING);
        }
        else
        {
            changeState(KnightState::RUNNING);
        }
    }
    else
    {
        changeState(KnightState::IDLE);
    }
}

void TheKnight::onDownSlashAnimFinished()
{
    _isAttacking = false;
    removeSlashEffect();
    
    if (_velocityY > 0)
    {
        changeState(KnightState::JUMPING);
    }
    else
    {
        changeState(KnightState::FALLING);
    }
}

// ========== 贴墙攻击相关 ==========

void TheKnight::startWallSlash()
{
    _isAttacking = true;
    _currentSlashType = 0;  // 使用水平攻击的特效
    _wallSlashEffectTimer = 0.0f;
    _wallSlashEffectPhase = 1;
    createWallSlashEffect(1);
    
    changeState(KnightState::WALL_SLASHING);
}

void TheKnight::updateWallSlash(float dt)
{
    _wallSlashEffectTimer += dt;
    
    // 切换特效阶段
    if (_wallSlashEffectPhase == 1 && _wallSlashEffectTimer >= 0.09f)
    {
        _wallSlashEffectPhase = 2;
        createWallSlashEffect(2);
    }
    
    updateWallSlashEffectPosition();
    
    // 贴墙攻击时继续下滑
    Vec2 pos = this->getPosition();
    float slideDistance = _wallSlideSpeed * dt;
    pos.y -= slideDistance;
    this->setPosition(pos);
    
    // 检查地面碰撞
    float groundY;
    if (checkGroundCollision(groundY))
    {
        pos.y = groundY;
        _velocityY = 0;
        _isOnGround = true;
        _isOnWall = false;
        this->setPosition(pos);
        // 落地后继续攻击动画，结束后会处理状态
    }
}

void TheKnight::onWallSlashAnimFinished()
{
    _isAttacking = false;
    removeSlashEffect();
    
    // 如果已经落地
    if (_isOnGround)
    {
        _facingRight = !_wallOnRight;  // 恢复朝向
        this->setFlippedX(_facingRight);
        
        if (_isMovingLeft || _isMovingRight)
        {
            if ((_isMovingLeft && _facingRight) || (_isMovingRight && !_facingRight))
            {
                changeState(KnightState::TURNING);
            }
            else
            {
                changeState(KnightState::RUNNING);
            }
        }
        else
        {
            changeState(KnightState::IDLE);
        }
    }
    // 还在墙上
    else if (checkWallSlideCollision(_wallOnRight))
    {
        changeState(KnightState::WALL_SLIDING);
    }
    // 不在墙上也不在地面
    else
    {
        _isOnWall = false;
        _facingRight = !_wallOnRight;
        this->setFlippedX(_facingRight);
        changeState(KnightState::FALLING);
    }
}

void TheKnight::createWallSlashEffect(int effectNum)
{
    // 移除旧特效
    removeSlashEffect();
    
    // 使用水平攻击的特效（SlashEffect）
    std::string effectPath = "TheKnight/Slash/SlashEffect/SlashEffect" + std::to_string(effectNum) + ".png";
    
    _slashEffect = Sprite::create(effectPath);
    if (_slashEffect && this->getParent())
    {
        _slashEffect->setAnchorPoint(Vec2(0.5f, 0.5f));
        _slashEffectPhase = effectNum;  // 同步阶段给通用特效位置更新使用
        this->getParent()->addChild(_slashEffect, this->getLocalZOrder() + 1);
        updateWallSlashEffectPosition();
    }
}

void TheKnight::updateWallSlashEffectPosition()
{
    if (!_slashEffect) return;
    
    Vec2 pos = this->getPosition();
    auto knightSize = this->getContentSize();
    auto effectSize = _slashEffect->getContentSize();
    
    // 贴墙时角色面向与墙相反方向（素材图片在右墙上Slide面朝左）
    // 所以攻击特效应该在离墙的那一侧
    float offsetX = (knightSize.width * 0.1f + effectSize.width / 2);
    float effectX, effectY;
    
    // 墙在右边时角色面朝左，特效在左边
    // 墙在左边时角色面朝右，特效在右边
    if (_wallOnRight)
    {
        effectX = pos.x - offsetX;  // 特效在左边
        _slashEffect->setFlippedX(false);  // 面朝左
    }
    else
    {
        effectX = pos.x + offsetX;  // 特效在右边
        _slashEffect->setFlippedX(true);   // 面朝右
    }
    effectY = pos.y + knightSize.height / 2;
    
    _slashEffect->setPosition(Vec2(effectX, effectY));
}

bool TheKnight::getSlashEffectBoundingBox(Rect& outRect) const
{
    if (!_slashEffect || !_isAttacking)
    {
        return false;
    }
    
    auto effectSize = _slashEffect->getContentSize();
    auto effectPos = _slashEffect->getPosition();
    
    outRect = Rect(effectPos.x - effectSize.width / 2,
                   effectPos.y - effectSize.height / 2,
                   effectSize.width,
                   effectSize.height);
    return true;
}

void TheKnight::takeDamage(int damage)
{
    if (_isInvincible || _state == KnightState::GET_ATTACKED || _state == KnightState::DEAD)
    {
        return;
    }
    
    _hp -= damage;
    if (_hp < 0) _hp = 0;
    
    // 如果正在恢复，取消恢复（灵魂已消耗，不回血）
    if (_state == KnightState::RECOVERING)
    {
        _isRecovering = false;
        // 不调用 cancelRecover，直接进入受击状态
    }
    
    // 如果正在Focus，取消Focus（灵魂已消耗，不回血）
    if (_state == KnightState::FOCUSING || _state == KnightState::FOCUS_GET || _state == KnightState::FOCUS_END)
    {
        cancelFocus();
    }
    
    // 如果处于地图模式，强制退出
    if (_isMapMode)
    {
        exitMapMode();
    }
    
    // 清理攻击特效（如果正在攻击确保特效被清理）
    _isAttacking = false;
    removeSlashEffect();
    
    // 清理冲刺特效
    if (_dashEffect)
    {
        _dashEffect->removeFromParent();
        _dashEffect = nullptr;
    }
    
    // 如果在冲刺中受击，重置冲刺状态
    if (_state == KnightState::DASHING)
    {
        _dashTimer = 0.0f;
    }
    
    // 生命值归零，触发死亡
    if (_hp <= 0)
    {
        startDeath();
    }
    else
    {
        _isInvincible = true;
        // 计算无敌时长（考虑StalwartShell护符）
        _invincibleTimer = _invincibleDuration;
        if (_charmStalwartShell)
        {
            _invincibleTimer += 0.4f;  // 增加0.4秒无敌时间
        }
        changeState(KnightState::GET_ATTACKED);
    }
}

void TheKnight::setKnockbackDirection(bool fromRight)
{
    _knockbackFromRight = fromRight;
}

void TheKnight::updateGetAttacked(float dt)
{
    _knockbackTimer += dt;
    
    float knockbackDir = _knockbackFromRight ? -1.0f : 1.0f;
    
    if (_knockbackTimer < _knockbackDuration)
    {
        Vec2 pos = this->getPosition();
        pos.x += knockbackDir * _knockbackSpeed * dt;
        
        float correctedX;
        if (knockbackDir > 0 && checkWallCollision(correctedX, true))
        {
            pos.x = correctedX;
        }
        else if (knockbackDir < 0 && checkWallCollision(correctedX, false))
        {
            pos.x = correctedX;
        }
        
        this->setPosition(pos);
    }
    
    if (!_isOnGround)
    {
        _velocityY -= _gravity * dt;
        
        if (_velocityY < -1600.0f)
        {
            _velocityY = -1600.0f;
        }
        
        Vec2 pos = this->getPosition();
        pos.y += _velocityY * dt;
        this->setPosition(pos);
        
        float groundY;
        if (checkGroundCollision(groundY))
        {
            pos.y = groundY;
            _velocityY = 0;
            _isOnGround = true;
            this->setPosition(pos);
        }
    }
}

void TheKnight::onGetAttackedFinished()
{
    this->setFlippedX(_facingRight);
    
    if (!_isOnGround)
    {
        changeState(KnightState::FALLING);
    }
    else if (_isMovingLeft || _isMovingRight)
    {
        if ((_isMovingLeft && _facingRight) || (_isMovingRight && !_facingRight))
        {
            changeState(KnightState::TURNING);
        }
        else
        {
            changeState(KnightState::RUNNING);
        }
    }
    else
    {
        changeState(KnightState::IDLE);
    }
}

void TheKnight::startDeath()
{
    changeState(KnightState::DEAD);
}

void TheKnight::onDeathAnimFinished()
{
    // 死亡动画播放完毕，角色消失
    this->setVisible(false);
    // 停止更新
    this->unscheduleUpdate();
}

void TheKnight::bounceFromDownSlash()
{
    // 下劈命中敌人后弹起
    // 只有在下劈状态才能弹反
    if (_state != KnightState::DOWN_SLASHING) return;
    
    // 设置向上的速度，实现弹起效果
    _velocityY = 800.0f;  // 弹起力度
    
    // 确保处于空中状态
    _isOnGround = false;
    
    // 重置二段跳（弹反后可以再次二段跳）
    _hasDoubleJumped = false;
}
