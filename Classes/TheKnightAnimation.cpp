/**
 * @file TheKnightAnimation.cpp
 * @brief 小骑士（TheKnight）角色类 - 动画相关实现
 */

#include "TheKnight.h"
#include "audio/include/SimpleAudioEngine.h"

using namespace CocosDenshion;

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
    
    // 重落地动画 - 10帧
    _hardLandAnim = createAnimation("TheKnight/Land/HardLand/", "HardLand", 1, 10, 0.05f);
    if (_hardLandAnim)
    {
        _hardLandAnim->retain();
        AnimationCache::getInstance()->addAnimation(_hardLandAnim, "hardLand");
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
    
    // 尖刺死亡动画 - 8帧
    _spikeDeathAnim = createAnimation("TheKnight/Dead/SpikeDeath/", "SpikeDeath", 1, 8, 0.08f);
    if (_spikeDeathAnim)
    {
        _spikeDeathAnim->retain();
        AnimationCache::getInstance()->addAnimation(_spikeDeathAnim, "spikeDeath");
    }
    
    // 危险区域重生动画 - 20帧
    _hazardRespawnAnim = createAnimation("TheKnight/HazardRespawn/", "HazardRespawn", 1, 20, 0.1f);
    if (_hazardRespawnAnim)
    {
        _hazardRespawnAnim->retain();
        AnimationCache::getInstance()->addAnimation(_hazardRespawnAnim, "hazardRespawn");
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
    
    // Focus聚气动画 - 7帧
    _focusAnim = createAnimation("TheKnight/Focus/", "Focus", 1, 7, 0.1f);
    if (_focusAnim)
    {
        _focusAnim->retain();
        AnimationCache::getInstance()->addAnimation(_focusAnim, "focus");
    }
    
    // Focus回血动画 - 6帧
    _focusGetAnim = createAnimation("TheKnight/Focus/", "FocusGet", 1, 6, 0.08f);
    if (_focusGetAnim)
    {
        _focusGetAnim->retain();
        AnimationCache::getInstance()->addAnimation(_focusGetAnim, "focusGet");
    }
    
    // Focus结束动画 - 3帧
    _focusEndAnim = createAnimation("TheKnight/Focus/", "FocusEnd", 1, 3, 0.08f);
    if (_focusEndAnim)
    {
        _focusEndAnim->retain();
        AnimationCache::getInstance()->addAnimation(_focusEndAnim, "focusEnd");
    }
    
    // 打开地图动画 - 3帧
    _mapOpenAnim = createAnimation("TheKnight/Map/MapOpen/", "MapOpen", 1, 3, 0.05f);
    if (_mapOpenAnim)
    {
        _mapOpenAnim->retain();
        AnimationCache::getInstance()->addAnimation(_mapOpenAnim, "mapOpen");
    }
    
    // 地图模式静止动画 - 7帧
    _mapIdleAnim = createAnimation("TheKnight/Map/MapIdle/", "MapIdle", 1, 7, 0.1f);
    if (_mapIdleAnim)
    {
        _mapIdleAnim->retain();
        AnimationCache::getInstance()->addAnimation(_mapIdleAnim, "mapIdle");
    }
    
    // 地图模式行走动画 - 8帧
    _mapWalkAnim = createAnimation("TheKnight/Map/MapWalk/", "MapWalk", 1, 8, 0.05f);
    if (_mapWalkAnim)
    {
        _mapWalkAnim->retain();
        AnimationCache::getInstance()->addAnimation(_mapWalkAnim, "mapWalk");
    }
    
    // 地图模式转向动画 - 2帧
    _mapTurnAnim = createAnimation("TheKnight/Map/MapTurn/", "MapTurn", 1, 2, 0.05f);
    if (_mapTurnAnim)
    {
        _mapTurnAnim->retain();
        AnimationCache::getInstance()->addAnimation(_mapTurnAnim, "mapTurn");
    }
    
    // 关闭地图动画 - 2帧
    _mapAwayAnim = createAnimation("TheKnight/Map/MapAway/", "MapAway", 1, 2, 0.05f);
    if (_mapAwayAnim)
    {
        _mapAwayAnim->retain();
        AnimationCache::getInstance()->addAnimation(_mapAwayAnim, "mapAway");
    }
    
    // Chair animations
    // 坐下动画 - 3帧
    _sitAnim = createAnimation("TheKnight/Chair/Sit/", "Sit", 1, 3, 0.1f);
    if (_sitAnim)
    {
        _sitAnim->retain();
        AnimationCache::getInstance()->addAnimation(_sitAnim, "sit");
    }
    
    // 坐着静止动画 - 1帧
    _sitIdleAnim = createAnimation("TheKnight/Chair/SitIdle/", "SitIdle", 1, 1, 0.1f);
    if (_sitIdleAnim)
    {
        _sitIdleAnim->retain();
        AnimationCache::getInstance()->addAnimation(_sitIdleAnim, "sitIdle");
    }
    
    // 坐着入睡动画 - 2帧
    _sitFallAsleepAnim = createAnimation("TheKnight/Chair/SitFallAsleep/", "SitFallAsleep", 1, 2, 0.2f);
    if (_sitFallAsleepAnim)
    {
        _sitFallAsleepAnim->retain();
        AnimationCache::getInstance()->addAnimation(_sitFallAsleepAnim, "sitFallAsleep");
    }
    
    // 坐着睡觉动画 - 1帧
    _sittingAsleepAnim = createAnimation("TheKnight/Chair/SittingAsleep/", "SittingAsleep", 1, 1, 0.1f);
    if (_sittingAsleepAnim)
    {
        _sittingAsleepAnim->retain();
        AnimationCache::getInstance()->addAnimation(_sittingAsleepAnim, "sittingAsleep");
    }
    
    // 醒来动画 - 9帧
    _wakeToSitAnim = createAnimation("TheKnight/Chair/WakeToSit/", "WakeToSit", 1, 9, 0.06f);
    if (_wakeToSitAnim)
    {
        _wakeToSitAnim->retain();
        AnimationCache::getInstance()->addAnimation(_wakeToSitAnim, "wakeToSit");
    }
    
    // 起身离开动画 - 5帧
    _getOffAnim = createAnimation("TheKnight/Chair/GetOff/", "GetOff", 1, 5, 0.06f);
    if (_getOffAnim)
    {
        _getOffAnim->retain();
        AnimationCache::getInstance()->addAnimation(_getOffAnim, "getOff");
    }
    
    // 坐着打开地图动画 - 4帧
    _sitMapOpenAnim = createAnimation("TheKnight/Chair/SitMapOpen/", "SitMapOpen", 1, 4, 0.08f);
    if (_sitMapOpenAnim)
    {
        _sitMapOpenAnim->retain();
        AnimationCache::getInstance()->addAnimation(_sitMapOpenAnim, "sitMapOpen");
    }
    
    // 坐着关闭地图动画 - 5帧
    _sitMapCloseAnim = createAnimation("TheKnight/Chair/SitMapClose/", "SitMapClose", 1, 5, 0.06f);
    if (_sitMapCloseAnim)
    {
        _sitMapCloseAnim->retain();
        AnimationCache::getInstance()->addAnimation(_sitMapCloseAnim, "sitMapClose");
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
    
    // 如果离开RUNNING状态，停止跑步音效
    if (_state == KnightState::RUNNING && newState != KnightState::RUNNING)
    {
        if (_runningSoundId != -1)
        {
            SimpleAudioEngine::getInstance()->stopEffect(_runningSoundId);
            _runningSoundId = -1;
        }
    }
    
    // 如果从攻击状态切换出去，清除攻击特效
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
            
            // 播放跑步音效（循环播放）
            _runningSoundId = SimpleAudioEngine::getInstance()->playEffect("Music/hero_running.wav", true);
            
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
            // 播放跳跃音效
            SimpleAudioEngine::getInstance()->playEffect("Music/hero_jump.wav", false);
            
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("Airborne");
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
            // 播放落地音效
            SimpleAudioEngine::getInstance()->playEffect("Music/hero_land.wav", false);
            
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("land");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onLandFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            break;
        }
        
        case KnightState::HARD_LANDING:
        {
            // 播放重落地音效
            SimpleAudioEngine::getInstance()->playEffect("Music/hero_land.wav", false);
            
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("hardLand");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onHardLandFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            break;
        }
        
        case KnightState::DASHING:
        {
            // 播放冲刺音效
            SimpleAudioEngine::getInstance()->playEffect("Music/hero_dash.wav", false);
            
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
            // 蹬墙跳也播放跳跃音效
            SimpleAudioEngine::getInstance()->playEffect("Music/hero_jump.wav", false);
            
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
            // 二段跳也播放跳跃音效
            SimpleAudioEngine::getInstance()->playEffect("Music/hero_jump.wav", false);
            
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("doubleJump");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onDoubleJumpAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            break;
        }
        
        case KnightState::SLASHING:
        {
            // 播放攻击音效
            SimpleAudioEngine::getInstance()->playEffect("Music/hero_sword.wav", false);
            
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
            // 播放攻击音效
            SimpleAudioEngine::getInstance()->playEffect("Music/hero_sword.wav", false);
            
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
            // 播放攻击音效
            SimpleAudioEngine::getInstance()->playEffect("Music/hero_sword.wav", false);
            
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
            // 播放受击音效
            SimpleAudioEngine::getInstance()->playEffect("Music/hero_damage.wav", false);
            
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
            // 播放死亡音效
            SimpleAudioEngine::getInstance()->playEffect("Music/hero_death.wav", false);
            
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
        
        case KnightState::SPIKE_DEATH:
        {
            this->stopAllActions();
            // 播放尖刺死亡动画
            auto animation = AnimationCache::getInstance()->getAnimation("spikeDeath");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onSpikeDeathAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onSpikeDeathAnimFinished();
            }
            break;
        }
        
        case KnightState::HAZARD_RESPAWN:
        {
            this->stopAllActions();
            // 设置位置到重生点
            this->setPosition(_respawnPosition);
            // 播放危险重生动画
            auto animation = AnimationCache::getInstance()->getAnimation("hazardRespawn");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onHazardRespawnAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onHazardRespawnAnimFinished();
            }
            break;
        }
        
        case KnightState::CASTING_SPELL:
        {
            // 播放法术音效
            SimpleAudioEngine::getInstance()->playEffect("Music/hero_fireball.wav", false);
            
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
        
        case KnightState::FOCUSING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("focus");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onFocusAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onFocusAnimFinished();
            }
            break;
        }
        
        case KnightState::FOCUS_GET:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("focusGet");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onFocusGetAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onFocusGetAnimFinished();
            }
            break;
        }
        
        case KnightState::FOCUS_END:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("focusEnd");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onFocusEndAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onFocusEndAnimFinished();
            }
            break;
        }
        
        case KnightState::MAP_OPENING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("mapOpen");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onMapOpenFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onMapOpenFinished();
            }
            break;
        }
        
        case KnightState::MAP_IDLE:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("mapIdle");
            if (animation)
            {
                auto animate = Animate::create(animation);
                this->runAction(RepeatForever::create(animate));
            }
            break;
        }
        
        case KnightState::MAP_WALKING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("mapWalk");
            if (animation)
            {
                auto animate = Animate::create(animation);
                this->runAction(RepeatForever::create(animate));
            }
            break;
        }
        
        case KnightState::MAP_TURNING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("mapTurn");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onMapTurnFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onMapTurnFinished();
            }
            break;
        }
        
        case KnightState::MAP_CLOSING:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("mapAway");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onMapCloseFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onMapCloseFinished();
            }
            break;
        }
        
        // Chair sitting states
        case KnightState::SITTING:
        {
            this->stopAllActions();
            _isSitting = true;
            _sitIdleTimer = 0.0f;
            _isAsleep = false;
            auto animation = AnimationCache::getInstance()->getAnimation("sit");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onSitAnimFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onSitAnimFinished();
            }
            break;
        }
        
        case KnightState::SIT_IDLE:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("sitIdle");
            if (animation)
            {
                auto animate = Animate::create(animation);
                this->runAction(RepeatForever::create(animate));
            }
            break;
        }
        
        case KnightState::SIT_FALL_ASLEEP:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("sitFallAsleep");
            if (animation)
            {
                // 调整位置偏移（向右上）
                Vec2 originalPos = this->getPosition();
                this->setPosition(originalPos + Vec2(5, 10)); // 向右5，向上10
                
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create([this, originalPos]() {
                    // 恢复位置
                    this->setPosition(originalPos);
                    this->onSitFallAsleepFinished();
                });
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onSitFallAsleepFinished();
            }
            break;
        }
        
        case KnightState::SITTING_ASLEEP:
        {
            this->stopAllActions();
            _isAsleep = true;
            
            // 调整位置偏移（向右上），与SitFallAsleep保持一致
            // 注意：需要在退出此状态时恢复位置，或者在WakeToSit开始时恢复
            // 由于SITTING_ASLEEP是循环状态，我们在这里设置偏移
            // 并在切换到WAKE_TO_SIT时处理恢复（或者在WAKE_TO_SIT里也保持偏移直到动画结束）
            
            // 这里我们假设进入SITTING_ASLEEP前位置是正常的（因为SIT_FALL_ASLEEP结束时恢复了）
            // 所以我们需要再次应用偏移
            this->setPosition(this->getPosition() + Vec2(5, 10));
            
            auto animation = AnimationCache::getInstance()->getAnimation("sittingAsleep");
            if (animation)
            {
                auto animate = Animate::create(animation);
                this->runAction(RepeatForever::create(animate));
            }
            break;
        }
        
        case KnightState::WAKE_TO_SIT:
        {
            this->stopAllActions();
            
            // 如果是从SITTING_ASLEEP过来的，位置是有偏移的
            // WAKE_TO_SIT动画应该也是基于这个偏移位置的（假设素材一致）
            // 所以我们保持偏移，直到动画结束
            
            // 如果不是从SITTING_ASLEEP过来的（理论上不会），可能需要处理
            // 但为了安全起见，我们记录当前位置作为"偏移后的位置"
            // 并在动画结束恢复到 "当前位置 - 偏移量"
            
            Vec2 currentPos = this->getPosition();
            Vec2 originalPos = currentPos - Vec2(5, 10); // 假设当前是偏移后的位置
            
            auto animation = AnimationCache::getInstance()->getAnimation("wakeToSit");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create([this, originalPos]() {
                    // 动画结束，恢复位置
                    this->setPosition(originalPos);
                    this->onWakeToSitFinished();
                });
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                // 如果没有动画，直接恢复位置并结束
                this->setPosition(originalPos);
                onWakeToSitFinished();
            }
            break;
        }
        
        case KnightState::GET_OFF:
        {
            this->stopAllActions();
            // GetOff素材面朝左，根据后退方向决定翻转
            // 如果按的是A键（左），则面朝左（不翻转）
            // 其他键面朝右（翻转）
            bool faceLeft = (_exitKey == EventKeyboard::KeyCode::KEY_A || 
                             _exitKey == EventKeyboard::KeyCode::KEY_CAPITAL_A);
            this->setFlippedX(!faceLeft);
            _facingRight = !faceLeft;
            
            auto animation = AnimationCache::getInstance()->getAnimation("getOff");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onGetOffFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onGetOffFinished();
            }
            break;
        }
        
        case KnightState::SIT_MAP_OPEN:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("sitMapOpen");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onSitMapOpenFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onSitMapOpenFinished();
            }
            break;
        }
        
        case KnightState::SIT_MAP_CLOSE:
        {
            this->stopAllActions();
            auto animation = AnimationCache::getInstance()->getAnimation("sitMapClose");
            if (animation)
            {
                auto animate = Animate::create(animation);
                auto callback = CallFunc::create(CC_CALLBACK_0(TheKnight::onSitMapCloseFinished, this));
                this->runAction(Sequence::create(animate, callback, nullptr));
            }
            else
            {
                onSitMapCloseFinished();
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

void TheKnight::onHardLandFinished()
{
    // 重落地时重置二段跳
    _hasDoubleJumped = false;
    
    // 重落地动画播放完毕，根据输入状态决定下一个状态
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
    // 二段跳动画（包括Airborne7）播放完后直接播放下落动画
    // 不再判断速度，因为我们已经播放了最高点帧
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

void TheKnight::onMapOpenFinished()
{
    // 打开地图动画完成后，进入地图模式静止或行走状态
    _isMapMode = true;
    if (_isMovingLeft || _isMovingRight)
    {
        if ((_isMovingLeft && _facingRight) || (_isMovingRight && !_facingRight))
        {
            changeState(KnightState::MAP_TURNING);
        }
        else
        {
            changeState(KnightState::MAP_WALKING);
        }
    }
    else
    {
        changeState(KnightState::MAP_IDLE);
    }
}

void TheKnight::onMapTurnFinished()
{
    _facingRight = !_facingRight;
    this->setFlippedX(_facingRight);
    
    if ((_isMovingLeft && !_facingRight) || (_isMovingRight && _facingRight))
    {
        changeState(KnightState::MAP_WALKING);
    }
    else
    {
        changeState(KnightState::MAP_IDLE);
    }
}

void TheKnight::onMapCloseFinished()
{
    _isMapMode = false;
    // 关闭地图后根据移动状态决定下一状态
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

void TheKnight::exitMapMode()
{
    // 强制退出地图模式（用于受击或高度变化时）
    if (!_isMapMode) return;
    
    _isMapMode = false;
    // 不播放关闭动画，直接切换状态
}

// Chair sitting methods
void TheKnight::onSitAnimFinished()
{
    if (_state == KnightState::SITTING)
    {
        changeState(KnightState::SIT_IDLE);
    }
}

void TheKnight::onSitFallAsleepFinished()
{
    if (_state == KnightState::SIT_FALL_ASLEEP)
    {
        changeState(KnightState::SITTING_ASLEEP);
    }
}

void TheKnight::onWakeToSitFinished()
{
    // Wake to sit finished, now play GetOff
    changeState(KnightState::GET_OFF);
}

void TheKnight::onGetOffFinished()
{
    _isSitting = false;
    _isAsleep = false;
    
    // GetOff后直接进入Idle状态
    changeState(KnightState::IDLE);
}

void TheKnight::onSitMapOpenFinished()
{
    // Stay in map open state, wait for Tab release
    // Keep the last frame
    auto animation = AnimationCache::getInstance()->getAnimation("sitMapOpen");
    if (animation && animation->getFrames().size() > 0)
    {
        auto lastFrame = animation->getFrames().back()->getSpriteFrame();
        this->setSpriteFrame(lastFrame);
    }
}

void TheKnight::onSitMapCloseFinished()
{
    // Return to sit idle or sitting asleep
    if (_isAsleep)
    {
        changeState(KnightState::SITTING_ASLEEP);
    }
    else
    {
        changeState(KnightState::SIT_IDLE);
    }
}

bool TheKnight::isSitting() const
{
    return _isSitting || 
           _state == KnightState::SITTING ||
           _state == KnightState::SIT_IDLE ||
           _state == KnightState::SIT_FALL_ASLEEP ||
           _state == KnightState::SITTING_ASLEEP ||
           _state == KnightState::WAKE_TO_SIT ||
           _state == KnightState::GET_OFF ||
           _state == KnightState::SIT_MAP_OPEN ||
           _state == KnightState::SIT_MAP_CLOSE;
}

void TheKnight::startSitting()
{
    if (_isOnGround && !_isSitting && 
        (_state == KnightState::IDLE || _state == KnightState::LOOKING_UP))
    {
        changeState(KnightState::SITTING);
    }
}

void TheKnight::exitSitting(bool pressedLeft)
{
    if (!_isSitting) return;
    
    _exitKey = pressedLeft ? EventKeyboard::KeyCode::KEY_A : EventKeyboard::KeyCode::KEY_D;
    
    if (_isAsleep)
    {
        // If asleep, play wake animation first
        changeState(KnightState::WAKE_TO_SIT);
    }
    else
    {
        // If not asleep, directly get off
        changeState(KnightState::GET_OFF);
    }
}

void TheKnight::updateSitting(float dt)
{
    if (!_isSitting) return;
    
    // Only count timer in SIT_IDLE state
    if (_state == KnightState::SIT_IDLE)
    {
        _sitIdleTimer += dt;
        
        // After 8 seconds of no input, fall asleep
        if (_sitIdleTimer >= _sitIdleTimeout)
        {
            changeState(KnightState::SIT_FALL_ASLEEP);
        }
    }
}
