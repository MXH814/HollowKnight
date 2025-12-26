#include "ShadowEnemy.h"

USING_NS_CC;

bool ShadowEnemy::init() {
    if (!Node::init()) return false;

    CCLOG("ShadowEnemy::init() started");

    _hp = 3; 
    _moveSpeed = 100.0f;
    // 初始状态设为RETURN，确保第一次切换到IDLE才真正执行
    _currentState = State::RETURN; 
    _previousState = State::IDLE;
    _isFacingLeft = true;
    _detectionRange = 350.0f;
    _playerDamageCooldown = 0.0f;

    // 创建显示精灵
    _display = Sprite::create();
    if (_display) {
        _display->setTextureRect(Rect(0, 0, 80, 120));
        _display->setColor(Color3B::BLACK);
        this->addChild(_display);
    }

    // 尝试加载默认图片
    auto testSprite = Sprite::create("shadow/shadow_0.png");
    if (testSprite) {
        _display->removeFromParent();
        _display = testSprite;
        this->addChild(_display);
    }

    _walkAction = nullptr;
    _deathAction = nullptr;
    _returnAction = nullptr;

    initAnimations();
    this->scheduleUpdate();
    changeState(State::IDLE); 

    CCLOG("ShadowEnemy::init() completed");
    return true;
}

// 辅助函数：从图集创建动画
cocos2d::Animation* ShadowEnemy::createAnimation(std::string prefix, int start, int end, float delay) {
    Vector<SpriteFrame*> frames;
    for (int i = start; i <= end; i++) {
        std::string path = prefix + std::to_string(i) + ".png";
        auto sprite = Sprite::create(path);
        if (sprite) {
            frames.pushBack(sprite->getSpriteFrame());
        }
    }

    if (frames.empty()) return nullptr;
    return Animation::createWithSpriteFrames(frames, delay);
}

// 初始化动画
void ShadowEnemy::initAnimations() {
    // 1. 行走动画
    auto walkAnim = createAnimation("shadow/shadow_", 0, 5, 0.1f);
    if (walkAnim) {
        _walkAction = RepeatForever::create(Animate::create(walkAnim));
        _walkAction->retain();
    } else {
        auto scaleUp = ScaleTo::create(0.5f, 1.1f);
        auto scaleDown = ScaleTo::create(0.5f, 1.0f);
        _walkAction = RepeatForever::create(Sequence::create(scaleUp, scaleDown, nullptr));
        _walkAction->retain();
    }

    // 2. 死亡动画
    auto deathAnim = createAnimation("shadow/shadow_death_", 0, 7, 0.1f);
    if (deathAnim) {
        _deathAction = Animate::create(deathAnim);
        _deathAction->retain();
    }

    // 3. 回归动画
    auto returnAnim = createAnimation("shadow/shadow_return_", 0, 6, 0.1f);
    if (returnAnim) {
        // 修改：不再使用 RepeatForever，只播放一次
        _returnAction = Animate::create(returnAnim);
        _returnAction->retain();
    } else {
        // 备选：旋转一圈
        _returnAction = RotateBy::create(1.0f, 360);
        _returnAction->retain();
    }
}

void ShadowEnemy::setTarget(TheKnight* target) {
    _target = target;
}

void ShadowEnemy::update(float dt) {
    if (!_target) return;

    // 更新冷却时间
    if (_playerDamageCooldown > 0) _playerDamageCooldown -= dt;

    // --- 碰撞检测逻辑 ---

    // 1. 获取 Shadow 敌人的世界坐标碰撞框
    Rect localHitBox = _display->getBoundingBox();
    Vec2 p1 = this->convertToWorldSpace(localHitBox.origin);
    Vec2 p2 = this->convertToWorldSpace(localHitBox.origin + localHitBox.size);
    
    float minX = std::min(p1.x, p2.x);
    float maxX = std::max(p1.x, p2.x);
    float minY = std::min(p1.y, p2.y);
    float maxY = std::max(p1.y, p2.y);
    
    Rect worldHitBox(minX, minY, maxX - minX, maxY - minY);

    // 2. 获取 TheKnight 的世界坐标碰撞框
    Rect knightLocalBox = _target->getBoundingBox(); 
    Rect knightWorldBox = knightLocalBox; 

    // --- 情况 1: Shadow 伤害 TheKnight (TheKnight 受击) ---
    // 【修改】只有在 IDLE 和 CHASE 状态下才会对 Knight 造成伤害
    if (_playerDamageCooldown <= 0 && 
        (_currentState == State::IDLE || _currentState == State::CHASE)) {
        if (worldHitBox.intersectsRect(knightWorldBox)) {
            CCLOG("Shadow hit TheKnight! Simulating damage...");
            _target->takeDamage(1);  // 使用TheKnight的takeDamage方法
            _playerDamageCooldown = 1.5f; 
        }
    }

    // --- 情况 2: TheKnight 攻击 Shadow (Shadow 受击) ---
    if (_currentState != State::INJURED && _currentState != State::DYING && _currentState != State::RETURN) {
        // 修改：使用 getSlashEffectBoundingBox 方法
        Rect attackRect;
        if (_target->getSlashEffectBoundingBox(attackRect)) {
            // 如果攻击框有效（攻击动作进行中）
            if (attackRect.intersectsRect(worldHitBox)) {
                CCLOG("TheKnight attack hit Shadow!");
                takeDamage();
            }
        }
    }
    
    // --- 状态更新 ---
    switch (_currentState) {
        case State::IDLE:
            updateIdle(dt);
            break;
        case State::CHASE:
            updateChase(dt);
            break;
        default:
            break;
    }
}

void ShadowEnemy::updateIdle(float dt) {
    if (this->getPosition().distance(_target->getPosition()) < _detectionRange) {
        changeState(State::CHASE);
    }
}

void ShadowEnemy::updateChase(float dt) {
    Vec2 myPos = this->getPosition();
    Vec2 targetPos = _target->getPosition();
    float dist = myPos.distance(targetPos);

    if (dist > _detectionRange) {
        changeState(State::IDLE);
        return;
    }

    if (dist > 10.0f) {
        Vec2 direction = (targetPos - myPos).getNormalized();
        this->setPosition(myPos + direction * _moveSpeed * dt);

        if (direction.x > 0) {
            _display->setFlippedX(true);
        } else {
            _display->setFlippedX(false);
        }
    }
}

void ShadowEnemy::takeDamage() {
    _hp--;
    CCLOG("Shadow took damage! HP: %d", _hp);

    if (_hp <= 0) {
        changeState(State::DYING);
    } else {
        changeState(State::INJURED);
    }
}

void ShadowEnemy::changeState(State newState) {
    if (_currentState == newState) return;
    
    // 如果不是进入受伤状态，则记录当前状态以便恢复
    if (newState != State::INJURED) {
        _previousState = _currentState;
    }

    _currentState = newState;
    
    if (_display) {
        _display->stopAllActions(); 
    }

    switch (newState) {
        case State::IDLE:
        case State::CHASE:
            // 恢复行走动画
            if (_display && _walkAction) {
                _display->runAction(_walkAction);
            }
            break;

        case State::INJURED:
            if (_display) {
                // 1. 显示受击图片
                _display->setTexture("shadow/shadow_injured.png");
                
                // 2. 0.4秒后恢复到之前的状态
                auto recover = CallFunc::create([this]() {
                    this->changeState(_previousState);
                });
                
                // 运行一个延时动作来控制受击时长
                this->runAction(Sequence::create(DelayTime::create(0.4f), recover, nullptr));
            }
            break;

        case State::DYING:
            if (_display && _deathAction) {
                auto finishDeath = CallFunc::create([this]() {
                    this->changeState(State::RETURN);
                });
                _display->runAction(Sequence::create(static_cast<FiniteTimeAction*>(_deathAction->clone()), finishDeath, nullptr));
            } else {
                changeState(State::RETURN);
            }
            break;

        case State::RETURN:
            // 1. 播放一次回归动画
            if (_display && _returnAction) {
                _display->runAction(_returnAction);
            }
            
            if (_target) {
                // 2. 飞向玩家并消失
                float duration = 1.0f; // 缩短飞行时间以匹配动画长度
                auto moveTo = MoveTo::create(duration, _target->getPosition());
                auto fadeOut = FadeOut::create(duration);
                
                auto finish = CallFunc::create([this]() {
                    this->removeFromParent();
                });
                
                auto spawn = Spawn::create(moveTo, fadeOut, nullptr);
                this->runAction(Sequence::create(spawn, finish, nullptr));
            } else {
                this->removeFromParent();
            }
            break;

        default:
            break;
    }
}

cocos2d::Rect ShadowEnemy::getHitBox() {
    if (_display) {
        Rect rect = _display->getBoundingBox();
        rect.origin += this->getPosition(); 
        return rect;
    }
    return cocos2d::Rect::ZERO;
}