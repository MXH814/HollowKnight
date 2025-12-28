#include "VengeflyMonster.h"
#include "SimpleAudioEngine.h"
#include "NextScene.h"  // 用于获取平台数据
#include "TheKnight.h"  // 获取 Platform 定义
#include "AudioManager.h"

USING_NS_CC;

// --- 文件名常量 ---
const char* VENGEFLY_NORMAL_FRAMES[] = {
    "Monster/Vengefly-0.png",
    "Monster/Vengefly-1.png",
    "Monster/Vengefly-2.png",
    "Monster/Vengefly-3.png",
    "Monster/Vengefly-4.png"
};
const char* VENGEFLY_ATTACK_FRAMES[] = {
    "Monster/Vengefly-10.png",
    "Monster/Vengefly-11.png",
    "Monster/Vengefly-12.png",
    "Monster/Vengefly-13.png"
};
const char* VENGEFLY_STUN_FRAME = "Monster/Vengefly-14.png";
const char* VENGEFLY_FALLING_FRAME = "Monster/Vengefly-8.png";
const char* VENGEFLY_DEAD_FRAME = "Monster/Vengefly-9.png";
const char* VENGEFLY_INITIAL_FRAME = "Monster/Vengefly-0.png";


// 辅助函数:根据文件名创建 SpriteFrame
cocos2d::SpriteFrame* VengeflyMonster::createFrame(const char* filename, const cocos2d::Size& size)
{
    return SpriteFrame::create(filename, Rect(0, 0, size.width, size.height));
}

// 静态创建函数
VengeflyMonster* VengeflyMonster::createMonster()
{
    VengeflyMonster* pRet = new (std::nothrow) VengeflyMonster();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

// 初始化
bool VengeflyMonster::init()
{
    // 1. super init first (使用 initWithFile 确保尺寸正确)
    if (!Sprite::initWithFile(VENGEFLY_INITIAL_FRAME))
    {
        CCLOGERROR("Failed to load initial Vengefly frame: %s. Check resource files.", VENGEFLY_INITIAL_FRAME);
        return false;
    }

    // 2. 初始化小怪状态
    _health = 3;
    _isStunned = false;
    _currentState = VengeflyState::PATROL;
    _initialPosition = Vec2::ZERO;
    _patrolRange = 100.0f;  // 【修改】从200.0f改为100.0f - 缩小巡逻范围
    _playerPosition = Vec2::ZERO;
    _velocity = Vec2::ZERO;

    // 3. 启动巡逻动画
    auto flyAnim = createNormalFlyAnimate();
    if (flyAnim) {
        this->runAction(flyAnim);
    }

    // 4. 启用 update,用于状态机逻辑
    this->scheduleUpdate();

    return true;
}

// setupPatrolArea
void VengeflyMonster::setupPatrolArea(const Vec2& center, float patrolRange)
{
    this->_initialPosition = center;
    this->_patrolRange = patrolRange;

    // 设置飞行范围限制(以中心为基准的矩形范围)
    _flyRange = Rect(
        center.x - patrolRange,
        center.y - patrolRange,
        patrolRange * 2,
        patrolRange * 2
    );

    // 立即启动巡逻移动序列
    startPatrol();
}

// startPatrol
void VengeflyMonster::startPatrol()
{
    if (_currentState != VengeflyState::PATROL) return;

    this->stopActionByTag((int)VengeflyMonsterActionTag::CURRENT_MOVEMENT_TAG);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distribX(-_patrolRange, _patrolRange);
    std::uniform_real_distribution<> distribY(-_patrolRange, _patrolRange);

    Vec2 targetPos = _initialPosition + Vec2(distribX(gen), distribY(gen));

    // 计算距离和速度
    float distance = this->getPosition().distance(targetPos);
    float duration = distance / _patrolSpeed;

    // 创建移动动作
    auto moveTo = MoveTo::create(duration, targetPos);

    // 移动完成后,再次调用 startPatrol
    auto callFunc = CallFunc::create(CC_CALLBACK_0(VengeflyMonster::startPatrol, this));

    // 组合序列:移动 -> 停顿 -> 再次巡逻
    auto sequence = Sequence::create(
        moveTo,
        DelayTime::create(0.5f), // 停顿0.5秒
        callFunc,
        nullptr
    );
    sequence->setTag((int)VengeflyMonsterActionTag::PATROL_MOVEMENT_TAG);

    this->runAction(sequence);
    sequence->setTag((int)VengeflyMonsterActionTag::CURRENT_MOVEMENT_TAG);
}

void VengeflyMonster::startChase()
{
    if (_currentState == VengeflyState::CHASE) return;

    this->stopActionByTag((int)VengeflyMonsterActionTag::CURRENT_MOVEMENT_TAG);
    this->stopActionByTag((int)VengeflyMonsterActionTag::NORMAL_FLY_ANIMATION_TAG);

    auto attackAnim = createAttackFlyAnimate();
    if (attackAnim) {
        this->runAction(attackAnim);
    }

    _currentState = VengeflyState::CHASE;
    CCLOG("Vengefly %s entered CHASE state!", this->getName().c_str());
}

void VengeflyMonster::update(float dt)
{
    if (_currentState == VengeflyState::DEAD || _currentState == VengeflyState::STUN)
    {
        return;
    }

    if (_currentState == VengeflyState::CHASE)
    {
        Vec2 currentPos = this->getPosition();
        Vec2 direction = _playerPosition - currentPos;

        float lengthSq = direction.x * direction.x + direction.y * direction.y;
        if (lengthSq < 10.0f * 10.0f)
        {
            return;
        }

        direction.normalize();
        _velocity = direction * _chaseSpeed;
        Vec2 movement = _velocity * dt;
        Vec2 nextPos = currentPos + movement;

        // 墙体碰撞检测
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
                    // 碰到墙体,停止水平移动
                    nextPos.x = currentPos.x;
                    hitWall = true;
                    break;
                }
            }
        }

        // 检测垂直方向的墙体碰撞
        if (!hitWall && _velocity.y != 0) {
            Rect futureBox(
                nextPos.x - getContentSize().width / 2,
                nextPos.y - getContentSize().height / 2,
                getContentSize().width,
                getContentSize().height
            );

            for (const auto& platform : _platforms) {
                if (futureBox.intersectsRect(platform.rect)) {
                    // 碰到墙体,停止垂直移动
                    nextPos.y = currentPos.y;
                    hitWall = true;
                    break;
                }
            }
        }

        // 【修改】边界限制 - 追击时也不能离巡逻中心太远
        if (!hitWall) {
            // 限制追击时最大距离为巡逻范围的1.5倍（从2.5倍改为1.5倍）
            float maxChaseDistance = _patrolRange * 1.5f;
            if (nextPos.x < _flyRange.getMinX() - maxChaseDistance) nextPos.x = _flyRange.getMinX() - maxChaseDistance;
            if (nextPos.x > _flyRange.getMaxX() + maxChaseDistance) nextPos.x = _flyRange.getMaxX() + maxChaseDistance;
            if (nextPos.y < _flyRange.getMinY() - maxChaseDistance) nextPos.y = _flyRange.getMinY() - maxChaseDistance;
            if (nextPos.y > _flyRange.getMaxY() + maxChaseDistance) nextPos.y = _flyRange.getMaxY() + maxChaseDistance;
        }

        this->setPosition(nextPos);

        // 翻转精灵朝向
        if (movement.x < 0) {
            this->setFlippedX(true);
        }
        else if (movement.x > 0) {
            this->setFlippedX(false);
        }
    }
    else if (_currentState == VengeflyState::PATROL)
    {
        // 巡逻时的边界检测
        Vec2 currentPos = this->getPosition();

        // 如果超出飞行范围,慢慢飞回中心
        if (!_flyRange.containsPoint(currentPos))
        {
            Vec2 direction = _initialPosition - currentPos;
            if (direction.length() > 1.0f) {
                direction.normalize();
                Vec2 movement = direction * _patrolSpeed * 0.5f * dt;
                this->setPosition(currentPos + movement);
            }
        }
    }
}

void VengeflyMonster::setPlayerPosition(const Vec2& playerPos)
{
    _playerPosition = playerPos;

    if (_currentState == VengeflyState::DEAD || _currentState == VengeflyState::STUN)
    {
        return;
    }

    Vec2 distanceVec = playerPos - this->getPosition();
    float distanceSq = distanceVec.x * distanceVec.x + distanceVec.y * distanceVec.y;
    float chaseRadiusSq = _chaseRadius * _chaseRadius;

    if (_currentState == VengeflyState::PATROL)
    {
        // Knight 靠近时进入追击状态
        if (distanceSq <= chaseRadiusSq)
        {
            startChase();
        }
    }
    else if (_currentState == VengeflyState::CHASE)
    {
        // Knight 远离时返回巡逻状态
        if (distanceSq > (chaseRadiusSq * 4.0f))
        {
            _currentState = VengeflyState::PATROL;
            this->stopActionByTag((int)VengeflyMonsterActionTag::CURRENT_MOVEMENT_TAG);
            this->stopActionByTag((int)VengeflyMonsterActionTag::CHASE_MOVEMENT_TAG);

            auto normalAnim = createNormalFlyAnimate();
            if (normalAnim) {
                this->runAction(normalAnim);
            }
            startPatrol();
            CCLOG("Vengefly %s returned to PATROL state.", this->getName().c_str());
        }
    }
}

cocos2d::ActionInterval* VengeflyMonster::createNormalFlyAnimate()
{
    Vector<SpriteFrame*> frames;
    Size size = this->getContentSize();

    if (size.width == 0 || size.height == 0) {
        size = Size(64, 64);
    }

    for (int i = 0; i < 5; i++) {
        auto frame = createFrame(VENGEFLY_NORMAL_FRAMES[i], size);
        if (frame) {
            frames.pushBack(frame);
        }
    }

    if (frames.empty()) return nullptr;

    auto animation = Animation::createWithSpriteFrames(frames, 0.1f);
    auto animate = Animate::create(animation);
    auto repeat = RepeatForever::create(animate);
    repeat->setTag((int)VengeflyMonsterActionTag::NORMAL_FLY_ANIMATION_TAG);
    return repeat;
}

cocos2d::ActionInterval* VengeflyMonster::createAttackFlyAnimate()
{
    Vector<SpriteFrame*> frames;
    Size size = this->getContentSize();

    if (size.width == 0 || size.height == 0) {
        size = Size(64, 64);
    }

    for (int i = 0; i < 4; i++) {
        auto frame = createFrame(VENGEFLY_ATTACK_FRAMES[i], size);
        if (frame) {
            frames.pushBack(frame);
        }
    }

    if (frames.empty()) return nullptr;

    auto animation = Animation::createWithSpriteFrames(frames, 0.08f);
    auto animate = Animate::create(animation);
    auto repeat = RepeatForever::create(animate);
    repeat->setTag((int)VengeflyMonsterActionTag::CHASE_MOVEMENT_TAG);
    return repeat;
}

// 受击接口
void VengeflyMonster::takeDamage(int damage, float knockbackPower, int knockbackDirection)
{
    if (_currentState == VengeflyState::DEAD || _isStunned)
    {
        return;
    }

    _health -= damage;

    // 播放受击音效
    AudioManager::getInstance()->playEnemyTakeDamageSound();

    if (_health <= 0)
    {
        // 死亡
        _currentState = VengeflyState::DEAD;
        _isStunned = false;
        this->stopAllActions();
        deathSequence(knockbackPower, knockbackDirection);
    }
    else
    {
        // 受击硬直
        _currentState = VengeflyState::STUN;
        _isStunned = true;
        this->stopAllActions();

        Size size = this->getContentSize();
        auto frame = createFrame(VENGEFLY_STUN_FRAME, size);
        if (frame) {
            this->setSpriteFrame(frame);
        }

        float stunDuration = 0.3f;
        float horizontalDistance = knockbackPower * 0.5f;

        Vec2 targetPos = Vec2(
            this->getPositionX() + (float)knockbackDirection * horizontalDistance,
            this->getPositionY()
        );

        auto knockback = MoveTo::create(stunDuration, targetPos);
        auto delay = DelayTime::create(0.5f);

        auto resumeFunc = CallFunc::create([this]() {
            _currentState = VengeflyState::PATROL;
            _isStunned = false;
            auto normalAnim = createNormalFlyAnimate();
            if (normalAnim) {
                this->runAction(normalAnim);
            }
            this->startPatrol();
            });

        auto stunSequence = Sequence::create(knockback, delay, resumeFunc, nullptr);
        stunSequence->setTag((int)VengeflyMonsterActionTag::STUN_ACTION_TAG);
        this->runAction(stunSequence);
    }
}

void VengeflyMonster::deathSequence(float knockbackPower, int knockbackDirection)
{
    this->unscheduleUpdate();
    Size size = this->getContentSize();

    auto setFlyFrame = CallFunc::create([this, size]() {
        auto frame = createFrame(VENGEFLY_FALLING_FRAME, size);
        if (frame) {
            this->setSpriteFrame(frame);
            this->setRotation(0.0f);
            this->setFlippedY(false);
        }
        });

    float flyHeight = 150.0f;
    float jumpDuration = 0.6f;
    float horizontalDistance = knockbackPower * 2.0f;
    Vec2 landingPos = Vec2(
        this->getPositionX() + (float)knockbackDirection * horizontalDistance,
        this->getPositionY()
    );

    const float groundY = 50.0f;
    if (landingPos.y < groundY) {
        landingPos.y = groundY;
    }

    auto jump = JumpTo::create(jumpDuration, landingPos, flyHeight, 1);

    auto setDeadFrame = CallFunc::create([this, size]() {
        auto frame = createFrame(VENGEFLY_DEAD_FRAME, size);
        if (frame) {
            this->setSpriteFrame(frame);
        }
        this->setRotation(0.0f);
        });

    auto delayBeforeFade = DelayTime::create(2.0f);
    auto fadeOut = FadeOut::create(0.5f);

    auto removeSelf = CallFunc::create([this]() {
        this->removeFromParentAndCleanup(true);
        });

    auto deathSequence = Sequence::create(
        setFlyFrame,
        jump,
        setDeadFrame,
        delayBeforeFade,
        fadeOut,
        removeSelf,
        nullptr
    );

    this->runAction(deathSequence);
}

// 标准化创建和生成接口
VengeflyMonster* VengeflyMonster::createAndSpawn(cocos2d::Node* parent, const cocos2d::Vec2& spawnPosition, float patrolRange)
{
    VengeflyMonster* vengefly = VengeflyMonster::createMonster();

    if (vengefly)
    {
        vengefly->setPosition(spawnPosition);
        vengefly->setupPatrolArea(spawnPosition, patrolRange);

        // 从场景获取平台数据
        auto nextScene = dynamic_cast<NextScene*>(parent);
        if (nextScene) {
            vengefly->_platforms = nextScene->getPlatforms();
            CCLOG("Vengefly 获取到 %zu 个碰撞平台", vengefly->_platforms.size());
        }

        parent->addChild(vengefly);
        return vengefly;
    }
    return nullptr;
}

// 获取碰撞盒
cocos2d::Rect VengeflyMonster::getBoundingBox() const {
    return Sprite::getBoundingBox();
}