#include "GruzzerMonster.h"
#include "NextScene.h"
#include "TheKnight.h"
#include "SimpleAudioEngine.h"  // 添加音效头文件
#include "GeoManager.h"

USING_NS_CC;

// 添加析构函数，释放 retain 的动画对象
GruzzerMonster::~GruzzerMonster() {
    CC_SAFE_RELEASE(_flyAnim);
    CC_SAFE_RELEASE(_deathAnim);
}

GruzzerMonster* GruzzerMonster::createAndSpawn(Node* parent, Vec2 startPos, Rect moveRange, float speed) {
    auto pRet = new (std::nothrow) GruzzerMonster();
    // 修改：直接使用第一帧图片初始化，确保 Sprite 有正确的 ContentSize
    if (pRet && pRet->initWithFile("Monster/Gruzzer_0.png")) {
        pRet->autorelease();
        pRet->initMonster(startPos, moveRange, speed);
        
        // 【新增】从场景获取平台数据
        auto nextScene = dynamic_cast<NextScene*>(parent);
        if (nextScene) {
            pRet->_platforms = nextScene->getPlatforms();
            CCLOG("Gruzzer 获取到 %zu 个碰撞平台", pRet->_platforms.size());
        }
        
        parent->addChild(pRet);
        return pRet;
    }
    else {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

bool GruzzerMonster::initMonster(Vec2 startPos, Rect moveRange, float speed) {
    this->setPosition(startPos);
    this->_limitRange = moveRange;
    this->_speed = speed;
    this->_state = State::FLYING;
    this->_health = 2;
    this->_isStunned = false;

    // 初始速度：向左下 45 度飞行实现 Z 字型初速
    _velocity = Vec2(-_speed, -_speed);

    initAnimations();
    this->scheduleUpdate();

    // 默认播放飞行循环
    if (_flyAnim) {
        auto flyAction = RepeatForever::create(Animate::create(_flyAnim));
        flyAction->setTag((int)GruzzerMonsterActionTag::FLY_ANIMATION_TAG);
        this->runAction(flyAction);
    }

    return true;
}

void GruzzerMonster::initAnimations() {
    // --- 修改：直接从文件加载飞行图片 (Gruzzer_0.png 到 Gruzzer_3.png) ---
    auto flyFrames = Vector<SpriteFrame*>();
    for (int i = 0; i <= 3; ++i) {
        std::string filename = StringUtils::format("Monster/Gruzzer_%d.png", i);
        // 加载纹理
        auto texture = Director::getInstance()->getTextureCache()->addImage(filename);
        if (texture) {
            auto size = texture->getContentSize();
            // 使用纹理创建 SpriteFrame
            auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, size.width, size.height));
            flyFrames.pushBack(frame);
        }
    }
    
    if (!flyFrames.empty()) {
        _flyAnim = Animation::createWithSpriteFrames(flyFrames, 0.15f);
        _flyAnim->retain();
    } else {
        _flyAnim = nullptr; // 标记为空，防止后续崩溃
    }

    // --- 修改：直接从文件加载死亡图片 (Gruzzer_5.png 到 Gruzzer_8.png) ---
    auto deathFrames = Vector<SpriteFrame*>();
    for (int i = 5; i <= 8; ++i) {
        std::string filename = StringUtils::format("Monster/Gruzzer_%d.png", i);
        auto texture = Director::getInstance()->getTextureCache()->addImage(filename);
        if (texture) {
            auto size = texture->getContentSize();
            auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, size.width, size.height));
            deathFrames.pushBack(frame);
        }
    }
    
    if (!deathFrames.empty()) {
        _deathAnim = Animation::createWithSpriteFrames(deathFrames, 0.1f);
        _deathAnim->retain();
    } else {
        _deathAnim = nullptr;
    }
}

void GruzzerMonster::update(float dt) {
    if (_state != State::FLYING) return;

    Vec2 currentPos = this->getPosition();
    Vec2 nextPos = currentPos + _velocity * dt;

    // 【新增】检测墙体碰撞（参考 Knight 的逻辑）
    bool hitWall = false;
    
    // 检测水平方向的墙体碰撞
    if (_velocity.x != 0) {
        Rect futureBox(
            nextPos.x - getContentSize().width / 2,
            currentPos.y - getContentSize().height / 2,
            getContentSize().width,
            getContentSize().height
        );
        
        for (const auto& platform : _platforms) {
            if (futureBox.intersectsRect(platform.rect)) {
                // 碰到墙体，反转X方向
                _velocity.x *= -1;
                _isFacingRight = (_velocity.x > 0);
                this->setFlippedX(_isFacingRight);
                this->setTexture("Monster/Gruzzer_4.png");
                hitWall = true;
                break;
            }
        }
    }
    
    // 检测垂直方向的墙体碰撞
    if (!hitWall && _velocity.y != 0) {
        Rect futureBox(
            currentPos.x - getContentSize().width / 2,
            nextPos.y - getContentSize().height / 2,
            getContentSize().width,
            getContentSize().height
        );
        
        for (const auto& platform : _platforms) {
            if (futureBox.intersectsRect(platform.rect)) {
                // 碰到墙体，反转Y方向
                _velocity.y *= -1;
                hitWall = true;
                break;
            }
        }
    }
    
    // 如果没碰到墙体，继续检测边界
    if (!hitWall) {
        // 触碰左右边界：反转 X 轴速度并处理转向帧
        if (nextPos.x < _limitRange.getMinX() || nextPos.x > _limitRange.getMaxX()) {
            _velocity.x *= -1;
            _isFacingRight = (_velocity.x > 0);
            this->setFlippedX(_isFacingRight);
            this->setTexture("Monster/Gruzzer_4.png");
        }

        // 触碰上下边界：反转 Y 轴速度（形成 Z 字型）
        if (nextPos.y < _limitRange.getMinY() || nextPos.y > _limitRange.getMaxY()) {
            _velocity.y *= -1;
        }
    }

    this->setPosition(nextPos);
}

// 【新增】受击接口 - 与 Crawlid/Tiktik 保持一致
void GruzzerMonster::takeDamage(int damage, float knockbackPower, int knockbackDirection) {
    if (_state == State::DEAD || _isStunned) {
        return;
    }

    _health -= damage;

    // 播放受击音效
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Music/enemy_damage.wav");

    if (_health <= 0) {
        // 死亡
        _state = State::DEAD;
        _isStunned = false;
        this->stopAllActions();
        die(knockbackPower, knockbackDirection);
    }
    else {
        // 受击硬直
        _state = State::HURT;
        _isStunned = true;
        this->stopAllActions();

        // 击退效果
        float stunDuration = 0.2f;
        float horizontalDistance = knockbackPower;
        Vec2 targetPos = Vec2(
            this->getPositionX() + (float)knockbackDirection * horizontalDistance, 
            this->getPositionY()
        );

        auto knockback = MoveTo::create(stunDuration, targetPos);
        auto delay = DelayTime::create(0.3f);
        
        auto resumeFunc = CallFunc::create([this]() {
            _state = State::FLYING;
            _isStunned = false;
            
            // 恢复飞行动画
            if (_flyAnim) {
                auto flyAction = RepeatForever::create(Animate::create(_flyAnim));
                flyAction->setTag((int)GruzzerMonsterActionTag::FLY_ANIMATION_TAG);
                this->runAction(flyAction);
            }
        });

        auto stunSequence = Sequence::create(knockback, delay, resumeFunc, nullptr);
        stunSequence->setTag((int)GruzzerMonsterActionTag::STUN_ACTION_TAG);
        this->runAction(stunSequence);
    }
}

// 【修改】死亡函数 - 参考 Crawlid/Tiktik 的死亡逻辑
void GruzzerMonster::die(float knockbackPower, int knockbackDirection) {
    _state = State::DEAD;
    _isStunned = false;
    this->stopAllActions();
    this->unscheduleUpdate();

    // 播放死亡音效
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Music/enemy_death.wav");

    // 播放死亡动画
    if (_deathAnim) {
        auto dieAnimate = Animate::create(_deathAnim);
        
        // 击退效果
        float knockbackDuration = 0.3f;
        float knockbackDistance = knockbackPower * 1.5f;
        Vec2 knockbackTarget = Vec2(
            this->getPositionX() + (float)knockbackDirection * knockbackDistance,
            this->getPositionY()
        );
        auto knockbackAction = MoveTo::create(knockbackDuration, knockbackTarget);
        
        // 淡出并移除
        auto delayBeforeFade = DelayTime::create(1.0f);
        auto fadeOut = FadeOut::create(0.5f);
        auto removeSelf = RemoveSelf::create();

        auto deathSequence = Sequence::create(
            Spawn::create(dieAnimate, knockbackAction, nullptr),
            delayBeforeFade,
            fadeOut,
            removeSelf,
            nullptr
        );
        
        GeoManager::getInstance()->addGeo(2);

        this->runAction(deathSequence);
    } else {
        // 如果没有动画，直接移除
        this->removeFromParent();
    }
}

// 【新增】获取碰撞盒
cocos2d::Rect GruzzerMonster::getBoundingBox() const {
    return Sprite::getBoundingBox();
}