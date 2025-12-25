/**
 * @file TheKnightMovement.cpp
 * @brief 小骑士（TheKnight）角色类 - 移动相关实现
 */

#include "TheKnight.h"

void TheKnight::startJump()
{
    if (!_isOnGround) return;
    
    _isOnGround = false;
    _velocityY = _minJumpForce;
    _jumpKeyHoldTime = 0.0f;
    _hasDoubleJumped = false;  // 跳跃时重置二段跳
    _fallStartY = this->getPositionY();  // 记录起跳位置
    changeState(KnightState::JUMPING);
}

void TheKnight::updateJump(float dt)
{
    // 如果还在按住跳跃键且未超过最大时间，持续增加上升速度
    if (_isJumpKeyPressed && _jumpKeyHoldTime < _maxJumpHoldTime)
    {
        _jumpKeyHoldTime += dt;
        float holdRatio = _jumpKeyHoldTime / _maxJumpHoldTime;
        _velocityY = _minJumpForce + (_jumpForce - _minJumpForce) * holdRatio;
    }
    
    // 应用重力
    _velocityY -= _gravity * dt;
    
    // 获取当前位置
    Vec2 pos = this->getPosition();
    Vec2 newPos = pos;
    
    // 计算新的垂直位置
    newPos.y += _velocityY * dt;
    
    // 计算新的水平位置
    // 处理外部跳跃的强制移动
    if (_isExternalJump)
    {
        _externalJumpTimer += dt;
        if (_externalJumpTimer < _externalJumpDuration)
        {
            // 强制移动期间
            newPos.x += _externalJumpDirection * _moveSpeed * dt;
        }
        else
        {
            // 强制移动结束
            _isExternalJump = false;
        }
    }
    else
    {
        // 正常的玩家输入移动
        if (_isMovingLeft)
        {
            newPos.x -= _moveSpeed * dt;
        }
        if (_isMovingRight)
        {
            newPos.x += _moveSpeed * dt;
        }
    }
    
    // 先更新位置用于碰撞检测
    this->setPosition(newPos);
    
    // 更新最高点位置（用于下落距离计算）
    if (this->getPositionY() > _fallStartY)
    {
        _fallStartY = this->getPositionY();
    }
    
    // 检查天花板碰撞
    float ceilingY;
    if (checkCeilingCollision(ceilingY))
    {
        newPos.y = ceilingY;
        _velocityY = 0;
        this->setPosition(newPos);
        changeState(KnightState::FALLING);
        return;
    }
    
    // 检查地面碰撞（优先级高于墙壁）
    float groundY;
    if (checkGroundCollision(groundY))
    {
        newPos.y = groundY;
        _velocityY = 0;
        _isOnGround = true;
        _isExternalJump = false;  // 落地时结束外部跳跃
        this->setPosition(newPos);
        
        // 计算下落距离，判断是普通落地还是重落地
        float fallDistance = _fallStartY - groundY;
        if (fallDistance >= _hardLandThreshold)
        {
            changeState(KnightState::HARD_LANDING);
        }
        else
        {
            changeState(KnightState::LANDING);
        }
        return;
    }
    
    // 检查墙壁碰撞和贴墙
    float correctedX;
    bool movingRight = _isMovingRight || (_isExternalJump && _externalJumpDirection > 0);
    bool movingLeft = _isMovingLeft || (_isExternalJump && _externalJumpDirection < 0);
    
    if (movingRight && checkWallCollision(correctedX, true))
    {
        newPos.x = correctedX;
        this->setPosition(newPos);
        if (checkWallSlideCollision(true))
        {
            _isExternalJump = false;
            startWallSlide(true);
            return;
        }
    }
    else if (movingLeft && checkWallCollision(correctedX, false))
    {
        newPos.x = correctedX;
        this->setPosition(newPos);
        if (checkWallSlideCollision(false))
        {
            _isExternalJump = false;
            startWallSlide(false);
            return;
        }
    }
    
    // 空中转向（只在非外部跳跃时）
    if (!_isExternalJump)
    {
        if (_isMovingLeft && _facingRight)
        {
            _facingRight = false;
            this->setFlippedX(false);
        }
        else if (_isMovingRight && !_facingRight)
        {
            _facingRight = true;
            this->setFlippedX(true);
        }
    }
    
    // 如果速度变为负数，切换到下落状态
    if (_velocityY < 0)
    {
        changeState(KnightState::FALLING);
    }
}

void TheKnight::updateFall(float dt)
{
    // 应用重力
    _velocityY -= _gravity * dt;
    
    // 限制最大下落速度
    if (_velocityY < -1600.0f)
    {
        _velocityY = -1600.0f;
    }
    
    // 获取当前位置
    Vec2 pos = this->getPosition();
    Vec2 newPos = pos;
    
    // 计算新的垂直位置
    newPos.y += _velocityY * dt;
    
    // 计算新的水平位置
    // 处理外部跳跃的强制移动
    if (_isExternalJump)
    {
        _externalJumpTimer += dt;
        if (_externalJumpTimer < _externalJumpDuration)
        {
            // 强制移动期间
            newPos.x += _externalJumpDirection * _moveSpeed * dt;
        }
        else
        {
            // 强制移动结束
            _isExternalJump = false;
        }
    }
    else
    {
        // 正常的玩家输入移动
        if (_isMovingLeft)
        {
            newPos.x -= _moveSpeed * dt;
        }
        if (_isMovingRight)
        {
            newPos.x += _moveSpeed * dt;
        }
    }
    
    // 先更新位置用于碰撞检测
    this->setPosition(newPos);
    
    // 检查地面碰撞（最高优先级）
    float groundY;
    if (checkGroundCollision(groundY))
    {
        newPos.y = groundY;
        _velocityY = 0;
        _isOnGround = true;
        _isExternalJump = false;  // 落地时结束外部跳跃
        this->setPosition(newPos);
        
        // 计算下落距离，判断是普通落地还是重落地
        float fallDistance = _fallStartY - groundY;
        if (fallDistance >= _hardLandThreshold)
        {
            changeState(KnightState::HARD_LANDING);
        }
        else
        {
            changeState(KnightState::LANDING);
        }
        return;
    }
    
    // 检查墙壁碰撞和贴墙
    float correctedX;
    bool movingRight = _isMovingRight || (_isExternalJump && _externalJumpDirection > 0);
    bool movingLeft = _isMovingLeft || (_isExternalJump && _externalJumpDirection < 0);
    
    if (movingRight && checkWallCollision(correctedX, true))
    {
        newPos.x = correctedX;
        this->setPosition(newPos);
        if (checkWallSlideCollision(true))
        {
            _isExternalJump = false;
            startWallSlide(true);
            return;
        }
    }
    else if (movingLeft && checkWallCollision(correctedX, false))
    {
        newPos.x = correctedX;
        this->setPosition(newPos);
        if (checkWallSlideCollision(false))
        {
            _isExternalJump = false;
            startWallSlide(false);
            return;
        }
    }
    
    // 空中转向
    if (_isMovingLeft && _facingRight)
    {
        _facingRight = false;
        this->setFlippedX(false);
    }
    else if (_isMovingRight && !_facingRight)
    {
        _facingRight = true;
        this->setFlippedX(true);
    }
}

void TheKnight::startDoubleJump()
{
    _hasDoubleJumped = true;
    _velocityY = _doubleJumpForce;
    _jumpKeyHoldTime = _maxJumpHoldTime;  // 设置为最大值，防止长按增加跳跃力度
    _isJumpKeyPressed = false;  // 重置跳跃键状态，防止后续误判
    _fallStartY = this->getPositionY();  // 记录二段跳起始位置
    changeState(KnightState::DOUBLE_JUMPING);
}

void TheKnight::updateDoubleJump(float dt)
{
    // 应用重力
    _velocityY -= _gravity * dt;
    
    // 获取当前位置
    Vec2 pos = this->getPosition();
    Vec2 newPos = pos;
    
    // 计算新的垂直位置
    newPos.y += _velocityY * dt;
    
    // 计算新的水平位置
    if (_isMovingLeft)
    {
        newPos.x -= _moveSpeed * dt;
    }
    if (_isMovingRight)
    {
        newPos.x += _moveSpeed * dt;
    }
    
    // 先更新位置用于碰撞检测
    this->setPosition(newPos);
    
    // 更新最高点位置（用于下落距离计算）
    if (this->getPositionY() > _fallStartY)
    {
        _fallStartY = this->getPositionY();
    }
    
    // 检查天花板碰撞
    float ceilingY;
    if (checkCeilingCollision(ceilingY))
    {
        newPos.y = ceilingY;
        _velocityY = 0;
        this->setPosition(newPos);
        changeState(KnightState::FALLING);
        return;
    }
    
    // 检查地面碰撞
    float groundY;
    if (checkGroundCollision(groundY))
    {
        newPos.y = groundY;
        _velocityY = 0;
        _isOnGround = true;
        this->setPosition(newPos);
        
        // 计算下落距离，判断是普通落地还是重落地
        float fallDistance = _fallStartY - groundY;
        if (fallDistance >= _hardLandThreshold)
        {
            changeState(KnightState::HARD_LANDING);
        }
        else
        {
            changeState(KnightState::LANDING);
        }
        return;
    }
    
    // 检查墙壁碰撞和贴墙
    float correctedX;
    if (_isMovingRight && checkWallCollision(correctedX, true))
    {
        newPos.x = correctedX;
        this->setPosition(newPos);
        if (checkWallSlideCollision(true))
        {
            startWallSlide(true);
            return;
        }
    }
    else if (_isMovingLeft && checkWallCollision(correctedX, false))
    {
        newPos.x = correctedX;
        this->setPosition(newPos);
        if (checkWallSlideCollision(false))
        {
            startWallSlide(false);
            return;
        }
    }
    
    // 空中转向
    if (_isMovingLeft && _facingRight)
    {
        _facingRight = false;
        this->setFlippedX(false);
    }
    else if (_isMovingRight && !_facingRight)
    {
        _facingRight = true;
        this->setFlippedX(true);
    }
}

void TheKnight::startWallSlide(bool wallOnRight)
{
    _isOnWall = true;
    _wallOnRight = wallOnRight;
    _velocityY = 0.0f;
    _isOnGround = false;
    _hasDoubleJumped = false;  // 贴墙时重置二段跳
    _fallStartY = this->getPositionY();  // 记录贴墙开始位置
    
    // 素材是角色右侧为墙（角色面向左），所以：
    // 如果墙在右侧，不翻转（素材原样，角色面向左）
    // 如果墙在左侧，翻转（角色面向右）
    this->setFlippedX(!wallOnRight);
    
    changeState(KnightState::WALL_SLIDING);
}

void TheKnight::updateWallSlide(float dt)
{
    // 保持贴墙时的翻转状态不变
    this->setFlippedX(!_wallOnRight);
    
    // 检查玩家是否主动想离开墙壁（按反方向键）
    // 只有按离开墙壁的方向键才会离开，朝向墙壁的方向键被忽略
    // 墙在右侧时，只有按左键才能离开；墙在左侧时，只有按右键才能离开
    bool wantLeaveWall = (_wallOnRight && _isMovingLeft) || (!_wallOnRight && _isMovingRight);
    
    if (wantLeaveWall)
    {
        _isOnWall = false;
        this->setFlippedX(_facingRight);
        changeState(KnightState::FALLING);
        return;
    }
    
    // 获取当前位置
    Vec2 pos = this->getPosition();
    Vec2 newPos = pos;
    
    // 计算下滑后的位置
    float slideDistance = _wallSlideSpeed * dt;
    newPos.y -= slideDistance;
    
    // 设置临时的垂直速度用于地面碰撞检测
    float savedVelocityY = _velocityY;
    _velocityY = -_wallSlideSpeed;
    
    // 先更新位置用于碰撞检测
    this->setPosition(newPos);
    
    // 检查地面碰撞
    float groundY;
    if (checkGroundCollision(groundY))
    {
        newPos.y = groundY;
        _velocityY = 0;
        _isOnGround = true;
        _isOnWall = false;
        this->setPosition(newPos);
        this->setFlippedX(_facingRight);
        
        // 计算下落距离，判断是普通落地还是重落地
        float fallDistance = _fallStartY - groundY;
        if (fallDistance >= _hardLandThreshold)
        {
            changeState(KnightState::HARD_LANDING);
        }
        else
        {
            changeState(KnightState::LANDING);
        }
        return;
    }
    
    // 恢复速度
    _velocityY = savedVelocityY;
    
    // 检查是否还在贴着墙（使用墙壁碰撞检测）
    float correctedX;
    bool stillOnWall = false;
    
    if (_wallOnRight)
    {
        Vec2 testPos = newPos;
        testPos.x += 5.0f;
        this->setPosition(testPos);
        stillOnWall = checkWallCollision(correctedX, true);
        this->setPosition(newPos);
    }
    else
    {
        Vec2 testPos = newPos;
        testPos.x -= 5.0f;
        this->setPosition(testPos);
        stillOnWall = checkWallCollision(correctedX, false);
        this->setPosition(newPos);
    }
    
    if (!stillOnWall)
    {
        _isOnWall = false;
        this->setFlippedX(_facingRight);
        changeState(KnightState::FALLING);
        return;
    }
}

void TheKnight::startWallJump()
{
    _isOnWall = false;
    _wallJumpTimer = 0.0f;
    
    // 记录起跳位置用于烟雾特效
    _wallJumpPuffPos = this->getPosition();
    // 创建蹬墙跳烟雾特效
    createWallJumpPuffEffect();
    
    // 蹬墙跳方向离开墙壁
    // 如果墙在右侧，跳向左侧（_facingRight = false）
    // 如果墙在左侧，跳向右侧（_facingRight = true）
    _facingRight = !_wallOnRight;
    this->setFlippedX(_facingRight);
    
    _velocityY = _wallJumpForceY;
    _fallStartY = this->getPositionY();  // 记录蹬墙跳起始位置
    
    changeState(KnightState::WALL_JUMPING);
}

void TheKnight::updateWallJump(float dt)
{
    _wallJumpTimer += dt;
    
    // 应用重力
    _velocityY -= _gravity * dt;
    
    // 获取当前位置
    Vec2 pos = this->getPosition();
    Vec2 newPos = pos;
    
    // 计算新的垂直位置
    newPos.y += _velocityY * dt;
    
    // 蹬墙跳期间强制水平移动（离开墙壁方向）
    bool inForcedMovement = _wallJumpTimer < _wallJumpDuration;
    if (inForcedMovement)
    {
        float jumpDir = _wallOnRight ? -1.0f : 1.0f;
        newPos.x += jumpDir * _wallJumpForceX * dt;
    }
    else
    {
        if (_isMovingLeft)
        {
            newPos.x -= _moveSpeed * dt;
        }
        if (_isMovingRight)
        {
            newPos.x += _moveSpeed * dt;
        }
    }
    
    // 先更新位置用于碰撞检测
    this->setPosition(newPos);
    
    // 更新最高点位置（用于下落距离计算）
    if (this->getPositionY() > _fallStartY)
    {
        _fallStartY = this->getPositionY();
    }
    
    // 检查天花板碰撞
    float ceilingY;
    if (checkCeilingCollision(ceilingY))
    {
        newPos.y = ceilingY;
        _velocityY = 0;
        this->setPosition(newPos);
        changeState(KnightState::FALLING);
        return;
    }
    
    // 检查地面碰撞
    float groundY;
    if (checkGroundCollision(groundY))
    {
        newPos.y = groundY;
        _velocityY = 0;
        _isOnGround = true;
        this->setPosition(newPos);
        
        // 计算下落距离，判断是普通落地还是重落地
        float fallDistance = _fallStartY - groundY;
        if (fallDistance >= _hardLandThreshold)
        {
            changeState(KnightState::HARD_LANDING);
        }
        else
        {
            changeState(KnightState::LANDING);
        }
        return;
    }
    
    // 检查墙壁碰撞（只有强制移动结束后才检测贴墙）
    float correctedX;
    if (_isMovingRight && checkWallCollision(correctedX, true))
    {
        newPos.x = correctedX;
        this->setPosition(newPos);
        // 只有在强制移动结束后才能贴墙
        if (!inForcedMovement && checkWallSlideCollision(true))
        {
            startWallSlide(true);
            return;
        }
    }
    else if (_isMovingLeft && checkWallCollision(correctedX, false))
    {
        newPos.x = correctedX;
        this->setPosition(newPos);
        // 只有在强制移动结束后才能贴墙
        if (!inForcedMovement && checkWallSlideCollision(false))
        {
            startWallSlide(false);
            return;
        }
    }
    
    // 开始下落后，切换到下落状态
    if (_velocityY <= 0)
    {
        changeState(KnightState::FALLING);
    }
}

// ========== 蹬墙跳烟雾特效相关 ==========

void TheKnight::createWallJumpPuffEffect()
{
    // 移除旧特效
    removeWallJumpPuffEffect();
    
    // 创建烟雾特效
    _wallJumpPuffEffect = Sprite::create("TheKnight/Wall/WallJump/WallJumpPuff1.png");
    if (_wallJumpPuffEffect && this->getParent())
    {
        _wallJumpPuffEffect->setAnchorPoint(Vec2(0.5f, 0.5f));
        
        // 设置位置在起跳点
        Vec2 pos = _wallJumpPuffPos;
        auto knightSize = this->getContentSize();
        
        // 烟雾在角色中心位置
        pos.y += knightSize.height / 2 + 50;
        pos.x += knightSize.width / 2;
        
        _wallJumpPuffEffect->setPosition(pos);
        
        // 根据墙的位置翻转（墙在右边时不翻转，墙在左边时翻转）
        _wallJumpPuffEffect->setFlippedX(!_wallOnRight);
        
        this->getParent()->addChild(_wallJumpPuffEffect, this->getLocalZOrder() - 1);
        
        // 初始化帧动画参数
        _wallJumpPuffTimer = 0.0f;
        _wallJumpPuffFrame = 1;
    }
}

void TheKnight::updateWallJumpPuffEffect(float dt)
{
    if (!_wallJumpPuffEffect) return;
    
    _wallJumpPuffTimer += dt;
    
    // 每0.05秒切换一帧
    if (_wallJumpPuffTimer >= 0.05f)
    {
        _wallJumpPuffTimer = 0.0f;
        _wallJumpPuffFrame++;
        
        if (_wallJumpPuffFrame <= 6)
        {
            std::string filename = "TheKnight/Wall/WallJump/WallJumpPuff" + std::to_string(_wallJumpPuffFrame) + ".png";
            auto texture = Director::getInstance()->getTextureCache()->addImage(filename);
            if (texture)
            {
                auto size = texture->getContentSize();
                auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, size.width, size.height));
                if (frame)
                {
                    _wallJumpPuffEffect->setSpriteFrame(frame);
                }
            }
        }
        else
        {
            // 动画播放完毕，移除特效
            removeWallJumpPuffEffect();
        }
    }
}

void TheKnight::removeWallJumpPuffEffect()
{
    if (_wallJumpPuffEffect)
    {
        _wallJumpPuffEffect->removeFromParent();
        _wallJumpPuffEffect = nullptr;
    }
}

void TheKnight::startDash()
{
    _canDash = false;
    _dashCooldownTimer = 0.0f;
    _velocityY = 0.0f;  // 冲刺时停止垂直运动
    _dashEffectFrame = 0;
    _dashEffectTimer = 0.0f;
    createDashEffect();
    changeState(KnightState::DASHING);
}

void TheKnight::updateDash(float dt)
{
    _dashTimer += dt;
    
    // 更新冲刺特效
    updateDashEffect(dt);
    
    // 获取当前位置
    Vec2 pos = this->getPosition();
    Vec2 newPos = pos;
    
    // 冲刺移动
    float dashDirection = _facingRight ? 1.0f : -1.0f;
    newPos.x += dashDirection * _dashSpeed * dt;
    
    // 先更新位置用于碰撞检测
    this->setPosition(newPos);
    
    // 检查墙壁碰撞
    float correctedX;
    if (_facingRight && checkWallCollision(correctedX, true))
    {
        newPos.x = correctedX;
        this->setPosition(newPos);
        onDashFinished();
        return;
    }
    else if (!_facingRight && checkWallCollision(correctedX, false))
    {
        newPos.x = correctedX;
        this->setPosition(newPos);
        onDashFinished();
        return;
    }
    
    // 冲刺时间结束
    if (_dashTimer >= _dashDuration)
    {
        onDashFinished();
    }
}

void TheKnight::onDashFinished()
{
    // 移除冲刺特效
    if (_dashEffect)
    {
        _dashEffect->removeFromParent();
        _dashEffect = nullptr;
    }
    
    // 检查是否在空中
    if (!_isOnGround)
    {
        changeState(KnightState::FALLING);
    }
    else
    {
        changeState(KnightState::DASH_TO_IDLE);
    }
}

void TheKnight::createDashEffect()
{
    // 如果已有特效，先移除
    if (_dashEffect)
    {
        _dashEffect->removeFromParent();
        _dashEffect = nullptr;
    }
    
    // 创建特效精灵
    _dashEffect = Sprite::create("TheKnight/Dash/DashEffect1.png");
    if (_dashEffect)
    {
        // 特效锚点设置为中心
        _dashEffect->setAnchorPoint(Vec2(0.5f, 0.5f));
        // 特效放在人物身后（z-order更低）
        if (this->getParent())
        {
            this->getParent()->addChild(_dashEffect, this->getLocalZOrder() - 5);
        }
        // 设置特效位置和翻转
        _dashEffect->setFlippedX(_facingRight);
        updateDashEffect(0);
    }
}

void TheKnight::updateDashEffect(float dt)
{
    if (!_dashEffect) return;
    
    // 获取人物尺寸和位置
    Vec2 pos = this->getPosition();
    auto knightSize = this->getContentSize();
    auto effectSize = _dashEffect->getContentSize();
    
    // 计算特效位置：垂直中心对齐
    // 人物锚点在底部中心(0.5, 0)，所以垂直中心是 pos.y + knightSize.height / 2
    float centerY = pos.y + knightSize.height / 2;
    
    // 水平位置：特效与人物后方部分重叠
    float overlapCenter = knightSize.width / 1.0f;  
    float offsetX = (knightSize.width / 2.0f - overlapCenter + effectSize.width / 2.0f) * (_facingRight ? -1.0f : 1.0f);
    
    _dashEffect->setPosition(Vec2(pos.x + offsetX, centerY));
    _dashEffect->setFlippedX(_facingRight);
    
    // 更新特效帧
    _dashEffectTimer += dt;
    if (_dashEffectTimer >= 0.03f)
    {
        _dashEffectTimer = 0.0f;
        _dashEffectFrame++;
        
        if (_dashEffectFrame <= 8)
        {
            std::string filename = "TheKnight/Dash/DashEffect" + std::to_string(_dashEffectFrame) + ".png";
            auto texture = Director::getInstance()->getTextureCache()->addImage(filename);
            if (texture)
            {
                auto size = texture->getContentSize();
                auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, size.width, size.height));
                if (frame)
                {
                    _dashEffect->setSpriteFrame(frame);
                }
            }
        }
        else
        {
            // 特效播放完毕，移除
            _dashEffect->removeFromParent();
            _dashEffect = nullptr;
        }
    }
}

void TheKnight::triggerJumpFromExternal(float horizontalSpeed)
{
    // 从外部触发跳跃（用于场景切换时的跳跃效果）
    _isOnGround = false;
    _velocityY = _jumpForce;  // 使用完整的跳跃力度
    _jumpKeyHoldTime = _maxJumpHoldTime;  // 设置为最大值，防止继续增加跳跃力度
    _isJumpKeyPressed = false;  // 防止继续累积
    _hasDoubleJumped = false;  // 允许二段跳
    _fallStartY = this->getPositionY();  // 记录起始位置
    
    // 不设置_isMovingRight/_isMovingLeft，而是使用外部跳跃专用变量
    _isMovingRight = false;
    _isMovingLeft = false;
    
    // 设置外部跳跃参数
    _isExternalJump = true;
    _externalJumpTimer = 0.0f;
    _externalJumpDuration = 0.5f;  // 强制移动0.5秒
    _externalJumpDirection = (horizontalSpeed > 0) ? 1.0f : -1.0f;
    
    // 设置朝向
    if (horizontalSpeed > 0)
    {
        _facingRight = true;
        this->setFlippedX(true);
    }
    else if (horizontalSpeed < 0)
    {
        _facingRight = false;
        this->setFlippedX(false);
    }
    
    // 切换到跳跃状态
    changeState(KnightState::JUMPING);
    
    CCLOG("外部触发跳跃！速度Y: %.1f, 朝向: %s, 强制移动时间: %.1f秒", 
          _velocityY, _facingRight ? "右" : "左", _externalJumpDuration);
}
