/**
 * @file TheKnightAnimation.cpp
 * @brief 小骑士（TheKnight）角色类 - 动画相关实现
 */

#include "TheKnight.h"

Animation* TheKnight::createAnimation(const std::string& path, const std::string& prefix, int startFrame, int endFrame, float delay)
{
    Vector<SpriteFrame*> frames;
    
    for (int i = startFrame; i <= endFrame; i++)
    {
        std::string filename = path + prefix + std::to_string(i) + ".png";
        
        auto texture = Director::getInstance()->getTextureCache()->addImage(filename);
        if (texture)
        {
            auto size = texture->getContentSize();
            auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, size.width, size.height));
            if (frame)
            {
                frames.pushBack(frame);
            }
        }
    }
    
    if (frames.empty())
    {
        return nullptr;
    }
    
    auto animation = Animation::createWithSpriteFrames(frames, delay);
    return animation;
}

void TheKnight::loadAnimations()
{
    // Idle动画 - 9帧
    _idleAnim = createAnimation("TheKnight/Idle/", "Idle", 1, 9, 0.1f);
    if (_idleAnim)
    {
        _idleAnim->retain();
        AnimationCache::getInstance()->addAnimation(_idleAnim, "idle");
    }
    
    // Run起步动画 - 前4帧
    _runStartAnim = createAnimation("TheKnight/Run/", "Run", 1, 4, 0.05f);
    if (_runStartAnim)
    {
        _runStartAnim->retain();
        AnimationCache::getInstance()->addAnimation(_runStartAnim, "runStart");
    }
    
    // Run循环动画 - 第5-13帧
    _runAnim = createAnimation("TheKnight/Run/", "Run", 5, 13, 0.05f);
    if (_runAnim)
    {
        _runAnim->retain();
        AnimationCache::getInstance()->addAnimation(_runAnim, "run");
    }
    
    // Turn动画 - 前2帧
    _turnAnim = createAnimation("TheKnight/Turn/", "Turn", 1, 3, 0.005f);
    if (_turnAnim)
    {
        _turnAnim->retain();
        AnimationCache::getInstance()->addAnimation(_turnAnim, "turn");
    }
    
    // RunToIdle动画 - 6帧
    _runToIdleAnim = createAnimation("TheKnight/Run/", "RunToIdle", 1, 6, 0.05f);
    if (_runToIdleAnim)
    {
        _runToIdleAnim->retain();
        AnimationCache::getInstance()->addAnimation(_runToIdleAnim, "runToIdle");
    }
    
    // 跳跃上升动画 - 1-6帧
    _jumpUpAnim = createAnimation("TheKnight/Jump/Airborne/", "Airborne", 1, 6, 0.05f);
    if (_jumpUpAnim)
    {
        _jumpUpAnim->retain();
        AnimationCache::getInstance()->addAnimation(_jumpUpAnim, "jumpUp");
    }
    
    // 跳跃最高点动画 - 第7帧
    _jumpPeakAnim = createAnimation("TheKnight/Jump/Airborne/", "Airborne", 7, 7, 0.1f);
    if (_jumpPeakAnim)
    {
        _jumpPeakAnim->retain();
        AnimationCache::getInstance()->addAnimation(_jumpPeakAnim, "jumpPeak");
    }
    
    // 跳跃下落动画 - 8-12帧
    _jumpFallAnim = createAnimation("TheKnight/Jump/Airborne/", "Airborne", 8, 12, 0.05f);
    if (_jumpFallAnim)
    {
        _jumpFallAnim->retain();
        AnimationCache::getInstance()->addAnimation(_jumpFallAnim, "jumpFall");
    }
    
    // 落地动画 - 3帧
    _landAnim = createAnimation("TheKnight/Land/", "Land", 1, 3, 0.05f);
    if (_landAnim)
    {
        _landAnim->retain();
        AnimationCache::getInstance()->addAnimation(_landAnim, "land");
    }
    
    // 冲刺动画 - 12帧
    _dashAnim = createAnimation("TheKnight/Dash/", "Dash", 1, 12, 0.02f);
    if (_dashAnim)
    {
        _dashAnim->retain();
        AnimationCache::getInstance()->addAnimation(_dashAnim, "dash");
    }
    
    // 冲刺结束动画 - 4帧
    _dashToIdleAnim = createAnimation("TheKnight/Dash/", "DashToIdle", 1, 4, 0.05f);
    if (_dashToIdleAnim)
    {
        _dashToIdleAnim->retain();
        AnimationCache::getInstance()->addAnimation(_dashToIdleAnim, "dashToIdle");
    }
    
    // 冲刺特效动画 - 8帧
    _dashEffectAnim = createAnimation("TheKnight/Dash/", "DashEffect", 1, 8, 0.03f);
    if (_dashEffectAnim)
    {
        _dashEffectAnim->retain();
        AnimationCache::getInstance()->addAnimation(_dashEffectAnim, "dashEffect");
    }
    
    // 向上看动画 - 6帧
    _lookUpAnim = createAnimation("TheKnight/LookUp/", "LookUp", 1, 6, 0.05f);
    if (_lookUpAnim)
    {
        _lookUpAnim->retain();
        AnimationCache::getInstance()->addAnimation(_lookUpAnim, "lookUp");
    }
    
    // 向上看结束动画 - 3帧
    _lookUpEndAnim = createAnimation("TheKnight/LookUp/", "LookUpEnd", 1, 3, 0.05f);
    if (_lookUpEndAnim)
    {
        _lookUpEndAnim->retain();
        AnimationCache::getInstance()->addAnimation(_lookUpEndAnim, "lookUpEnd");
    }
    
    // 向下看动画 - 6帧
    _lookDownAnim = createAnimation("TheKnight/LookDown/", "LookDown", 1, 6, 0.05f);
    if (_lookDownAnim)
    {
        _lookDownAnim->retain();
        AnimationCache::getInstance()->addAnimation(_lookDownAnim, "lookDown");
    }
    
    // 向下看结束动画 - 2帧
    _lookDownEndAnim = createAnimation("TheKnight/LookDown/", "LookDownEnd", 1, 2, 0.05f);
    if (_lookDownEndAnim)
    {
        _lookDownEndAnim->retain();
        AnimationCache::getInstance()->addAnimation(_lookDownEndAnim, "lookDownEnd");
    }
    
    // 贴墙下滑动画 - 4帧
    _wallSlideAnim = createAnimation("TheKnight/Wall/WallSlide/", "WallSlide", 1, 4, 0.08f);
    if (_wallSlideAnim)
    {
        _wallSlideAnim->retain();
        AnimationCache::getInstance()->addAnimation(_wallSlideAnim, "wallSlide");
    }
    
    // 贴墙攻击动画 - 5帧
    _wallSlashAnim = createAnimation("TheKnight/Wall/WallSlash/", "WallSlash", 1, 5, 0.03f);
    if (_wallSlashAnim)
    {
        _wallSlashAnim->retain();
        AnimationCache::getInstance()->addAnimation(_wallSlashAnim, "wallSlash");
    }
    
    // 蹬墙跳动画 - 9帧
    _wallJumpAnim = createAnimation("TheKnight/Wall/WallJump/", "WallJump", 1, 9, 0.04f);
    if (_wallJumpAnim)
    {
        _wallJumpAnim->retain();
        AnimationCache::getInstance()->addAnimation(_wallJumpAnim, "wallJump");
    }
    
    // 蹬墙跳烟雾特效动画 - 6帧
    _wallJumpPuffAnim = createAnimation("TheKnight/Wall/WallJump/", "WallJumpPuff", 1, 6, 0.05f);
    if (_wallJumpPuffAnim)
    {
        _wallJumpPuffAnim->retain();
        AnimationCache::getInstance()->addAnimation(_wallJumpPuffAnim, "wallJumpPuff");
    }
    
    // 二段跳动画 - 8帧
    _doubleJumpAnim = createAnimation("TheKnight/Jump/DoubleJump/", "DoubleJump", 1, 8, 0.06f);
    if (_doubleJumpAnim)
    {
        _doubleJumpAnim->retain();
        AnimationCache::getInstance()->addAnimation(_doubleJumpAnim, "doubleJump");
    }
    
    // 水平攻击动画 - 5帧
    _slashAnim = createAnimation("TheKnight/Slash/Slash/", "Slash", 1, 5, 0.03f);
    if (_slashAnim)
    {
        _slashAnim->retain();
        AnimationCache::getInstance()->addAnimation(_slashAnim, "slash");
    }
    
    // 向上攻击动画 - 5帧
    _upSlashAnim = createAnimation("TheKnight/Slash/UpSlash/", "UpSlash", 1, 5, 0.03f);
    if (_upSlashAnim)
    {
        _upSlashAnim->retain();
        AnimationCache::getInstance()->addAnimation(_upSlashAnim, "upSlash");
    }
    
    // 向下攻击动画 - 5帧
    _downSlashAnim = createAnimation("TheKnight/Slash/DownSlash/", "DownSlash", 1, 5, 0.03f);
    if (_downSlashAnim)
    {
        _downSlashAnim->retain();
        AnimationCache::getInstance()->addAnimation(_downSlashAnim, "downSlash");
    }
    
    // 受击动画 - 8帧
    _getAttackedAnim = createAnimation("TheKnight/GetAttacked/", "GetAttacked", 1, 8, 0.03f);
    if (_getAttackedAnim)
    {
        _getAttackedAnim->retain();
        AnimationCache::getInstance()->addAnimation(_getAttackedAnim, "getAttacked");
    }
    
    // 死亡动画 - 13帧
    _deadAnim = createAnimation("TheKnight/Dead/Dead/", "Dead", 1, 13, 0.1f);
    if (_deadAnim)
    {
        _deadAnim->retain();
        AnimationCache::getInstance()->addAnimation(_deadAnim, "dead");
    }
    
    // 法术释放动画 - 9帧
    _vengefulSpiritAnim = createAnimation("TheKnight/VengefulSpirit/", "VengefulSpirit", 1, 9, 0.03f);
    if (_vengefulSpiritAnim)
    {
        _vengefulSpiritAnim->retain();
        AnimationCache::getInstance()->addAnimation(_vengefulSpiritAnim, "vengefulSpirit");
    }
    
    // 法术特效动画 - 7帧（循环）
    _vengefulSpiritEffectAnim = createAnimation("TheKnight/VengefulSpirit/", "VengefulSpiritEffect", 1, 7, 0.05f);
    if (_vengefulSpiritEffectAnim)
    {
        _vengefulSpiritEffectAnim->retain();
        AnimationCache::getInstance()->addAnimation(_vengefulSpiritEffectAnim, "vengefulSpiritEffect");
    }
    
    // 回复动画 - 6帧
    _recoverAnim = createAnimation("TheKnight/Recover/", "Recover", 1, 6, 0.2f);
    if (_recoverAnim)
    {
        _recoverAnim->retain();
        AnimationCache::getInstance()->addAnimation(_recoverAnim, "recover");
    }
    
    // 设置初始纹理
    if (_idleAnim && _idleAnim->getFrames().size() > 0)
    {
        this->setSpriteFrame(_idleAnim->getFrames().at(0)->getSpriteFrame());
    }
}

void TheKnight::playAnimation(const std::string& animName, bool loop)
{
    this->stopAllActions();
    
    auto animation = AnimationCache::getInstance()->getAnimation(animName);
    if (animation)
    {
        if (loop)
        {
            auto animate = Animate::create(animation);
            this->runAction(RepeatForever::create(animate));
        }
        else
        {
            auto animate = Animate::create(animation);
            this->runAction(animate);
        }
    }
}

void TheKnight::changeState(KnightState newState)
{
    if (_state == newState)
    {
        return;
    }
    
    // 如果从攻击状态切换出去，清理攻击特效
    if ((_state == KnightState::SLASHING || 
         _state == KnightState::UP_SLASHING || 
         _state == KnightState::DOWN_SLASHING ||
         _state == KnightState::WALL_SLASHING) &&
        newState != KnightState::SLASHING &&
        newState != KnightState::UP_SLASHING &&
        newState != KnightState::DOWN_SLASHING &&
        newState != KnightState::WALL_SLASHING)
    {
        _isAttacking = false;
        removeSlashEffect();
    }
    
    _state = newState;
    
    switch (_state)
    {
        case KnightState::IDLE:
            playAnimation("idle", true);
            break;
            
        case KnightState::RUNNING:
        {
            _isRunStartFinished = false;
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("runStart");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onRunStartFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                playAnimation("run", true);
            }
            break;
        }
            
        case KnightState::TURNING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("turn");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onTurnFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            break;
        }
            
        case KnightState::RUN_TO_IDLE:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("runToIdle");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onRunToIdleFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                _state = KnightState::IDLE;
                playAnimation("idle", true);
            }
            break;
        }
            
        case KnightState::JUMPING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("jumpUp");
            if (animation)
            {
                auto animate = Animate::create(animation);
                this->runAction(animate);
            }
            break;
        }
            
        case KnightState::FALLING:
        {
            this->stopAllActions();
            // 先播放最高点帧，再播放下落动画，然后回调开始循环
            auto peakAnim = AnimationCache::getInstance()->getAnimation("jumpPeak");
            auto fallAnim = AnimationCache::getInstance()->getAnimation("jumpFall");
            if (peakAnim && fallAnim)
            {
                auto animatePeak = Animate::create(peakAnim);
                auto animateFall = Animate::create(fallAnim);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onFallAnimFinished, this));
                auto sequence = Sequence::create(animatePeak, animateFall, callback, nullptr);
                this->runAction(sequence);
            }
            else if (fallAnim)
            {
                playAnimation("jumpFall", true);
            }
            break;
        }
            
        case KnightState::LANDING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("land");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onLandFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onLandFinished();
            }
            break;
        }
            
        case KnightState::DASHING:
        {
            this->stopAllActions();
            _dashTimer = 0.0f;
            auto animation = AnimationCache::getInstance()->getAnimation("dash");
            if (animation)
            {
                auto animate = Animate::create(animation);
                this->runAction(animate);
            }
            break;
        }
            
        case KnightState::DASH_TO_IDLE:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("dashToIdle");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onDashToIdleFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onDashToIdleFinished();
            }
            break;
        }
            
        case KnightState::LOOKING_UP:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("lookUp");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onLookUpAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            break;
        }
            
        case KnightState::LOOK_UP_END:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("lookUpEnd");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onLookUpEndFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onLookUpEndFinished();
            }
            break;
        }
            
        case KnightState::LOOKING_DOWN:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("lookDown");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onLookDownAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            break;
        }
            
        case KnightState::LOOK_DOWN_END:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("lookDownEnd");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onLookDownEndFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onLookDownEndFinished();
            }
            break;
        }
            
        case KnightState::WALL_SLIDING:
        {
            this->stopAllActions();
            // 循环播放贴墙下滑动画
            auto animation = AnimationCache::getInstance()->getAnimation("wallSlide");
            if (animation)
            {
                auto animate = Animate::create(animation);
                this->runAction(RepeatForever::create(animate));
            }
            break;
        }
        
        case KnightState::WALL_SLASHING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("wallSlash");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onWallSlashAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onWallSlashAnimFinished();
            }
            break;
        }
            
        case KnightState::WALL_JUMPING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("wallJump");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onWallJumpAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            break;
        }
            
        case KnightState::DOUBLE_JUMPING:
        {
            this->stopAllActions();
            // 播放二段跳动画（8帧），然后播放Airborne7，最后切换到下落
            auto doubleJumpAnim = AnimationCache::getInstance()->getAnimation("doubleJump");
            auto peakAnim = AnimationCache::getInstance()->getAnimation("jumpPeak");
            if (doubleJumpAnim)
            {
                auto animateDoubleJump = Animate::create(doubleJumpAnim);
                if (peakAnim)
                {
                    auto animatePeak = Animate::create(peakAnim);
                    auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onDoubleJumpAnimFinished, this));
                    auto sequence = Sequence::create(animateDoubleJump, animatePeak, callback, nullptr);
                    this->runAction(sequence);
                }
                else
                {
                    auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onDoubleJumpAnimFinished, this));
                    this->runAction(Sequence::create(animateDoubleJump, callback, nullptr));
                }
            }
            break;
        }
            
        case KnightState::SLASHING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("slash");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onSlashAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onSlashAnimFinished();
            }
            break;
        }
            
        case KnightState::UP_SLASHING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("upSlash");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onUpSlashAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onUpSlashAnimFinished();
            }
            break;
        }
            
        case KnightState::DOWN_SLASHING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("downSlash");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onDownSlashAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onDownSlashAnimFinished();
            }
            break;
        }
            
        case KnightState::GET_ATTACKED:
        {
            this->stopAllActions();
            _knockbackTimer = 0.0f;
            // 素材面朝左，需要根据后退方向决定翻转
            // 后退方向为背面方向，如果从右侧受击则向左后退（面朝右），需要翻转
            this->setFlippedX(_knockbackFromRight);
            auto animation = AnimationCache::getInstance()->getAnimation("getAttacked");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onGetAttackedFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onGetAttackedFinished();
            }
            break;
        }
            
        case KnightState::DEAD:
        {
            this->stopAllActions();
            // 禁用键盘输入
            _eventDispatcher->removeEventListenersForTarget(this);
            // 播放死亡动画
            auto animation = AnimationCache::getInstance()->getAnimation("dead");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onDeathAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onDeathAnimFinished();
            }
            break;
        }
            
        case KnightState::CASTING_SPELL:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("vengefulSpirit");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onCastSpellAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onCastSpellAnimFinished();
            }
            break;
        }
            
        case KnightState::RECOVERING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("recover");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onRecoverAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onRecoverAnimFinished();
            }
            break;
        }
    }
}

void TheKnight::onRunStartFinished()
{
    _isRunStartFinished = true;
    if (_state == KnightState::RUNNING)
    {
        playAnimation("run", true);
    }
}

void TheKnight::onTurnFinished()
{
    _facingRight = !_facingRight;
    this->setFlippedX(_facingRight);
    
    if ((_isMovingLeft && !_facingRight) || (_isMovingRight && _facingRight))
    {
        changeState(KnightState::RUNNING);
    }
    else
    {
        changeState(KnightState::IDLE);
    }
}

void TheKnight::onRunToIdleFinished()
{
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

void TheKnight::onFallAnimFinished()
{
    // 下落动画播放完后开始循环
    if (_state == KnightState::FALLING)
    {
        playAnimation("jumpFall", true);
    }
}

void TheKnight::onLandFinished()
{
    // 落地时重置二段跳
    _hasDoubleJumped = false;
    
    // 落地动画播放完毕，根据输入状态决定下一个状态
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

void TheKnight::onDashToIdleFinished()
{
    // 冲刺结束后根据输入状态决定下一个状态
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

void TheKnight::onLookUpAnimFinished()
{
    // 向上看动画播放完毕，如果还在按住W键，停留在第6帧
    if (_isLookingUp && _state == KnightState::LOOKING_UP)
    {
        // 停留在最后一帧
        auto animation = AnimationCache::getInstance()->getAnimation("lookUp");
        if (animation && animation->getFrames().size() > 0)
        {
            auto lastFrame = animation->getFrames().back()->getSpriteFrame();
            this->setSpriteFrame(lastFrame);
        }
    }
    else
    {
        // 松开了，播放结束动画
        changeState(KnightState::LOOK_UP_END);
    }
}

void TheKnight::onLookUpEndFinished()
{
    changeState(KnightState::IDLE);
}

void TheKnight::onLookDownAnimFinished()
{
    // 向下看动画播放完毕，如果还在按住S键，停留在第6帧
    if (_isLookingDown && _state == KnightState::LOOKING_DOWN)
    {
        // 停留在最后一帧
        auto animation = AnimationCache::getInstance()->getAnimation("lookDown");
        if (animation && animation->getFrames().size() > 0)
        {
            auto lastFrame = animation->getFrames().back()->getSpriteFrame();
            this->setSpriteFrame(lastFrame);
        }
    }
    else
    {
        // 松开了，播放结束动画
        changeState(KnightState::LOOK_DOWN_END);
    }
}

void TheKnight::onLookDownEndFinished()
{
    changeState(KnightState::IDLE);
}

void TheKnight::onWallJumpAnimFinished()
{
    // 动画播放完毕后，根据垂直速度决定状态
    if (_velocityY > 0)
    {
        changeState(KnightState::JUMPING);
    }
    else
    {
        changeState(KnightState::FALLING);
    }
}

void TheKnight::onDoubleJumpAnimFinished()
{
    // 二段跳动画（含Airborne7）播放完毕后，直接播放下落动画
    // 不再判断速度，因为动画已经包含了最高点帧
    this->stopAllActions();
    auto fallAnim = AnimationCache::getInstance()->getAnimation("jumpFall");
    if (fallAnim)
    {
        auto animateFall = Animate::create(fallAnim);
        auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onFallAnimFinished, this));
        this->runAction(Sequence::create(animateFall, callback, nullptr));
    }
    _state = KnightState::FALLING;
}
