/**
 * @file TheKnightSoul.cpp
 * @brief 小骑士（TheKnight）角色类 - 灵魂系统实现
 */

#include "TheKnight.h"
#include "audio/include/SimpleAudioEngine.h"

using namespace CocosDenshion;

void TheKnight::addSoul(int amount)
{
    _soul += amount;
    if (_soul > _maxSoul)
    {
        _soul = _maxSoul;
    }
}

bool TheKnight::useSoul(int amount)
{
    if (_soul >= amount)
    {
        _soul -= amount;
        return true;
    }
    return false;
}

void TheKnight::startCastSpell()
{
    if (!useSoul(_spellCost))
    {
        return;
    }
    
    _castSpellAnimTimer = 0.0f;
    _spellEffectCreated = false;
    _vengefulSpiritFacingRight = _facingRight;
    
    // 清除攻击特效
    if (_isAttacking)
    {
        _isAttacking = false;
        removeSlashEffect();
    }
    
    changeState(KnightState::CASTING_SPELL);
}

void TheKnight::updateCastSpell(float dt)
{
    _castSpellAnimTimer += dt;
    
    // 在第3帧（约0.09秒，每帧0.03秒）时创建法术特效
    if (!_spellEffectCreated && _castSpellAnimTimer >= 0.09f)
    {
        _spellEffectCreated = true;
        createVengefulSpiritEffect();
    }
    
    // 更新物理下落
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

void TheKnight::onCastSpellAnimFinished()
{
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

void TheKnight::createVengefulSpiritEffect()
{
    // 移除旧特效
    removeVengefulSpiritEffect();
    
    // 创建法术特效
    _vengefulSpiritEffect = Sprite::create("TheKnight/VengefulSpirit/VengefulSpiritEffect1.png");
    if (_vengefulSpiritEffect && this->getParent())
    {
        _vengefulSpiritEffect->setAnchorPoint(Vec2(0.5f, 0.5f));
        
        // 设置初始位置（在角色正前方）
        Vec2 pos = this->getPosition();
        auto knightSize = this->getContentSize();
        auto effectSize = _vengefulSpiritEffect->getContentSize();
        
        float offsetX = knightSize.width / 2 + effectSize.width / 2;
        if (_vengefulSpiritFacingRight)
        {
            pos.x += offsetX;
        }
        else
        {
            pos.x -= offsetX;
        }
        pos.y += knightSize.height / 2;
        
        _vengefulSpiritEffect->setPosition(pos);
        _vengefulSpiritEffect->setFlippedX(_vengefulSpiritFacingRight);
        
        this->getParent()->addChild(_vengefulSpiritEffect, this->getLocalZOrder() + 1);
        
        // 播放循环动画
        auto animation = AnimationCache::getInstance()->getAnimation("vengefulSpiritEffect");
        if (animation)
        {
            auto animate = Animate::create(animation);
            _vengefulSpiritEffect->runAction(RepeatForever::create(animate));
        }
    }
}

void TheKnight::updateVengefulSpiritEffect(float dt)
{
    if (!_vengefulSpiritEffect) return;
    
    // 移动特效
    Vec2 pos = _vengefulSpiritEffect->getPosition();
    float moveDir = _vengefulSpiritFacingRight ? 1.0f : -1.0f;
    pos.x += moveDir * _vengefulSpiritSpeed * dt;
    _vengefulSpiritEffect->setPosition(pos);
    
    // 检查墙壁碰撞
    auto effectSize = _vengefulSpiritEffect->getContentSize();
    Rect effectRect(pos.x - effectSize.width / 2, pos.y - effectSize.height / 2,
                    effectSize.width, effectSize.height);
    
    for (const auto& platform : _platforms)
    {
        // 检查是否与平台有垂直重叠
        if (effectRect.getMaxY() > platform.rect.getMinY() &&
            effectRect.getMinY() < platform.rect.getMaxY())
        {
            if (_vengefulSpiritFacingRight)
            {
                // 向右移动，检查是否碰到平台左边
                if (effectRect.getMaxX() >= platform.rect.getMinX() &&
                    effectRect.getMinX() < platform.rect.getMinX())
                {
                    // 播放法术消失音效
                    SimpleAudioEngine::getInstance()->playEffect("Music/fireball_disappear.wav", false);
                    removeVengefulSpiritEffect();
                    return;
                }
            }
            else
            {
                // 向左移动，检查是否碰到平台右边
                if (effectRect.getMinX() <= platform.rect.getMaxX() &&
                    effectRect.getMaxX() > platform.rect.getMaxX())
                {
                    // 播放法术消失音效
                    SimpleAudioEngine::getInstance()->playEffect("Music/fireball_disappear.wav", false);
                    removeVengefulSpiritEffect();
                    return;
                }
            }
        }
    }
}

void TheKnight::removeVengefulSpiritEffect()
{
    if (_vengefulSpiritEffect)
    {
        _vengefulSpiritEffect->removeFromParent();
        _vengefulSpiritEffect = nullptr;
    }
}

void TheKnight::startRecover()
{
    if (!useSoul(_recoverCost))
    {
        return;
    }
    
    _isRecovering = true;
    _recoverConsumed = true;  // 标记灵魂已消耗
    
    // 清理攻击特效
    if (_isAttacking)
    {
        _isAttacking = false;
        removeSlashEffect();
    }
    
    changeState(KnightState::RECOVERING);
}

void TheKnight::updateRecover(float dt)
{
    // 更新物理下落
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

void TheKnight::onRecoverAnimFinished()
{
    // 恢复动画播放完成，恢复生命值
    if (_isRecovering && _recoverConsumed)
    {
        _hp += 1;
        if (_hp > _maxHP)
        {
            _hp = _maxHP;
        }
    }
    
    _isRecovering = false;
    _recoverConsumed = false;
    
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

void TheKnight::cancelRecover()
{
    // 取消恢复（不会恢复血量）
    _isRecovering = false;
    // _recoverConsumed 保持为 true，表示灵魂已被消耗
    
    // 切换到合适的状态
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

// Focus系统实现

void TheKnight::startFocus()
{
    if (!useSoul(_focusCost))
    {
        return;
    }
    
    _isFocusing = true;
    _focusConsumed = true;  // 标记灵魂已消耗
    
    // 清理攻击特效
    if (_isAttacking)
    {
        _isAttacking = false;
        removeSlashEffect();
    }
    
    changeState(KnightState::FOCUSING);
}

void TheKnight::updateFocus(float dt)
{
    // 更新物理下落
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

void TheKnight::onFocusAnimFinished()
{
    // Focus动画播放完成，检查是否还在按住空格键
    if (_isFocusing && _isSpaceKeyPressed)
    {
        // 进入FocusGet状态（回血）
        changeState(KnightState::FOCUS_GET);
    }
    else
    {
        // 松开了空格键，播放FocusEnd动画
        _isFocusing = false;
        _focusConsumed = false;
        changeState(KnightState::FOCUS_END);
    }
}

void TheKnight::onFocusGetAnimFinished()
{
    // FocusGet动画播放完成，回复1点血
    _hp += 1;
    if (_hp > _maxHP)
    {
        _hp = _maxHP;
    }
    
    // 检查是否还在按住空格键且灵魂充足
    if (_isSpaceKeyPressed && _soul >= _focusCost)
    {
        // 消耗灵魂并继续Focus循环
        if (useSoul(_focusCost))
        {
            _focusConsumed = true;
            changeState(KnightState::FOCUSING);
        }
        else
        {
            // 灵魂不足，播放FocusEnd
            _isFocusing = false;
            _focusConsumed = false;
            changeState(KnightState::FOCUS_END);
        }
    }
    else
    {
        // 松开了空格键或灵魂不足，播放FocusEnd动画
        _isFocusing = false;
        _focusConsumed = false;
        changeState(KnightState::FOCUS_END);
    }
}

void TheKnight::onFocusEndAnimFinished()
{
    // FocusEnd动画播放完成，切换到适合的状态
    _isFocusing = false;
    _focusConsumed = false;
    
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

void TheKnight::cancelFocus()
{
    // 被打断（受击等），不回血，直接结束Focus
    _isFocusing = false;
    // _focusConsumed 保持为 true，表示灵魂已被消耗但未回血
    
    // 不播放FocusEnd动画，由受击等状态处理
}
