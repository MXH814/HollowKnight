// CrawlidMonster.cpp (增强击退效果)

#include "CrawlidMonster.h"
#include "AudioManager.h"
#include "GeoManager.h"

USING_NS_CC;

// 静态创建函数
CrawlidMonster* CrawlidMonster::createMonster()
{
    CrawlidMonster* pRet = new (std::nothrow) CrawlidMonster();
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
bool CrawlidMonster::init()
{
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("Monster/Crawlid - atlas0 #0070052.plist");

    if (!Sprite::initWithSpriteFrameName("Crawlid - atlas0 #0070052_0.png"))
    {
        return false;
    }

    _health = 2;
    _isStunned = false;
    _isPatrolling = false;
    _patrolRange = 0.0f;
    _initialPosition = Vec2::ZERO;

    return true;
}

void CrawlidMonster::setupPatrolRange(float patrolRange)
{
    this->_patrolRange = patrolRange;
    this->_initialPosition = this->getPosition();
    CCLOG("Crawlid set up with range: %f at position: (%f, %f)",
        patrolRange, _initialPosition.x, _initialPosition.y);
}

cocos2d::ActionInterval* CrawlidMonster::playWalkAnimation()
{
    Vector<SpriteFrame*> walkFrames;

    int frames[] = { 0, 6, 7, 10 };
    for (int frameIndex : frames) {
        std::string frameName = StringUtils::format("Crawlid - atlas0 #0070052_%d.png", frameIndex);
        auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
        if (frame) {
            walkFrames.pushBack(frame);
        }
    }

    if (walkFrames.empty()) return nullptr;

    Animation* walkAnimation = Animation::createWithSpriteFrames(walkFrames, 0.10f);

    return Animate::create(walkAnimation);
}

ActionInterval* createCrawlidTurnRightAction()
{
    Vector<SpriteFrame*> turnFrames;
    turnFrames.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("Crawlid - atlas0 #0070052_1.png"));
    turnFrames.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("Crawlid - atlas0 #0070052_3.png"));

    Animation* turnAnimation = Animation::createWithSpriteFrames(turnFrames, 0.15f);

    return Animate::create(turnAnimation);
}

ActionInterval* createCrawlidTurnLeftAction()
{
    Vector<SpriteFrame*> turnFrames;
    turnFrames.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("Crawlid - atlas0 #0070052_2.png"));
    turnFrames.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("Crawlid - atlas0 #0070052_4.png"));

    Animation* turnAnimation = Animation::createWithSpriteFrames(turnFrames, 0.15f);

    return Animate::create(turnAnimation);
}

void CrawlidMonster::startPatrol()
{
    if (_isStunned || _isPatrolling) return;

    if (_patrolRange <= 0.0f || _initialPosition == Vec2::ZERO) return;

    _isPatrolling = true;

    float speed = 150.0f;
    float distance = _patrolRange * 2;
    float moveDuration = distance / speed;

    float leftX = _initialPosition.x - _patrolRange;
    float rightX = _initialPosition.x + _patrolRange;

    auto startWalkAnim = CallFunc::create([this]() {
        auto walkAnim = this->playWalkAnimation();
        if (walkAnim) {
            auto repeat = RepeatForever::create(static_cast<ActionInterval*>(walkAnim));
            repeat->setTag((int)CrawlidMonsterActionTag::WALK_ANIMATION_TAG);
            this->runAction(repeat);
        }
    });

    auto stopWalkAnim = CallFunc::create([this]() {
        this->stopActionByTag((int)CrawlidMonsterActionTag::WALK_ANIMATION_TAG);
    });

    auto moveRightSeq = Sequence::create(
        CallFunc::create([this]() {
            this->setFlippedX(false);
            auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName("Crawlid - atlas0 #0070052_0.png");
            if (frame) this->setSpriteFrame(frame);
        }),
        startWalkAnim,
        MoveTo::create(moveDuration, Vec2(rightX, this->getPositionY())),
        stopWalkAnim,
        nullptr
    );

    auto turnRightSeq = Sequence::create(
        createCrawlidTurnRightAction(), 
        nullptr
    );

    auto moveLeftSeq = Sequence::create(
        CallFunc::create([this]() {
            this->setFlippedX(true);
            auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName("Crawlid - atlas0 #0070052_0.png");
            if (frame) this->setSpriteFrame(frame);
        }),
        startWalkAnim,
        MoveTo::create(moveDuration, Vec2(leftX, this->getPositionY())),
        stopWalkAnim,
        nullptr
    );

    auto turnLeftSeq = Sequence::create(
        CallFunc::create([this]() {
            this->setFlippedX(false); 
        }),
        createCrawlidTurnLeftAction(), 
        nullptr
    );

    auto patrolSequence = Sequence::create(
        moveRightSeq,
        turnRightSeq,
        moveLeftSeq,
        turnLeftSeq,
        nullptr
    );

    auto repeatPatrol = RepeatForever::create(patrolSequence);
    repeatPatrol->setTag((int)CrawlidMonsterActionTag::PATROL_ACTION_TAG);

    this->runAction(repeatPatrol);
}

// === 增强：更明显的击退效果 ===
void CrawlidMonster::takeDamage(int damage, float knockbackPower, int knockbackDirection)
{
    if (_isStunned) return;

    _health -= damage;
    _isStunned = true;

    // 播放受击音效
    AudioManager::getInstance()->playEnemyTakeDamageSound();

    // 1. 停止巡逻动作
    this->stopActionByTag((int)CrawlidMonsterActionTag::PATROL_ACTION_TAG);
    _isPatrolling = false;

    this->stopActionByTag((int)CrawlidMonsterActionTag::WALK_ANIMATION_TAG);
    this->stopActionByTag((int)CrawlidMonsterActionTag::STUN_ACTION_TAG);

    // 2. 死亡处理
    if (_health <= 0)
    {
        this->die(knockbackPower, knockbackDirection);
        return;
    }

    // 3. 非死亡受击：显示硬直帧
    auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName("Crawlid - atlas0 #0070052_9.png");
    if (frame) {
        this->setSpriteFrame(frame);
    }

    // === 4. 增强击退效果 ===
    
    // 增加击退距离
    float knockbackDistance = 150.0f;  // 从80增加到150像素
    
    // 延长击退时间，使效果更明显
    float knockbackDuration = 0.25f;   // 从0.15增加到0.25秒
    float stunDuration = 0.6f;         // 总硬直时间也延长
    
    Vec2 currentPos = this->getPosition();
    Vec2 targetPos = Vec2(currentPos.x + (float)knockbackDirection * knockbackDistance, currentPos.y);

    // 限制在巡逻范围内
    if (_initialPosition != Vec2::ZERO && _patrolRange > 0) {
        float leftLimit = _initialPosition.x - _patrolRange - 80.0f;   // 允许超出更多
        float rightLimit = _initialPosition.x + _patrolRange + 80.0f;
        targetPos.x = clampf(targetPos.x, leftLimit, rightLimit);
    }

    // 使用 EaseOut 缓动效果，开始快后来慢，更有冲击感
    auto knockback = EaseOut::create(MoveTo::create(knockbackDuration, targetPos), 2.0f);
    
    // === 5. 添加视觉反馈效果 ===
    
    // 5.1 受击闪烁效果 (红色闪烁)
    auto blinkRed = TintTo::create(0.05f, 255, 100, 100);  // 变红
    auto blinkNormal = TintTo::create(0.05f, 255, 255, 255); // 恢复
    auto blinkSeq = Sequence::create(blinkRed, blinkNormal, nullptr);
    auto blink = Repeat::create(blinkSeq, 3);  // 闪烁3次
    
    // 5.2 轻微旋转效果 (可选，增加冲击感)
    auto rotate1 = RotateTo::create(0.08f, knockbackDirection * 15.0f);  // 旋转15度
    auto rotate2 = RotateTo::create(0.12f, 0.0f);  // 恢复
    auto rotateSeq = Sequence::create(rotate1, rotate2, nullptr);
    
    // 5.3 组合击退动作
    auto knockbackWithEffects = Spawn::create(
        knockback,
        blink,
        rotateSeq,
        nullptr
    );

    // 6. 组合硬直和击退动作
    auto resumeCall = CallFunc::create(CC_CALLBACK_0(CrawlidMonster::resumePatrol, this));

    auto sequence = Sequence::create(
        knockbackWithEffects,  // 带视觉效果的击退
        DelayTime::create(stunDuration - knockbackDuration),
        resumeCall,
        nullptr
    );
    sequence->setTag((int)CrawlidMonsterActionTag::STUN_ACTION_TAG);
    this->runAction(sequence);
    
    CCLOG("Crawlid 受击! 击退距离: %.1f, 方向: %d", knockbackDistance, knockbackDirection);
}

void CrawlidMonster::resumePatrol()
{
    this->stopActionByTag((int)CrawlidMonsterActionTag::STUN_ACTION_TAG);

    _isStunned = false;

    this->startPatrol();
}

// === 增强：更夸张的死亡效果 ===
void CrawlidMonster::die(float knockbackPower, int knockbackDirection)
{
    this->stopAllActions();

    _isStunned = true;

    // 播放死亡音效
    AudioManager::getInstance()->playEnemyDeathSound();

    GeoManager::getInstance()->addGeo(2);
    CCLOG("Crawlid died! +2 Geo");

    CCLOG("Crawlid Monster Died! Starting death animation.");

    // 1. 增强死亡抛物线参数
    float flyHeight = 150.0f;          // 从100增加到150像素，飞得更高
    float jumpDuration = 0.7f;         // 稍微延长飞行时间
    float horizontalDistance = 180.0f; // 从120增加到180像素，飞得更远
    
    Vec2 currentPos = this->getPosition();
    Vec2 landingPos = Vec2(currentPos.x + (float)knockbackDirection * horizontalDistance, currentPos.y);

    // 限制在巡逻范围附近
    if (_initialPosition != Vec2::ZERO && _patrolRange > 0) {
        float leftLimit = _initialPosition.x - _patrolRange - 150.0f;
        float rightLimit = _initialPosition.x + _patrolRange + 150.0f;
        landingPos.x = clampf(landingPos.x, leftLimit, rightLimit);
    }

    auto jump = JumpTo::create(jumpDuration, landingPos, flyHeight, 1);

    // 2. 抛物线过程显示动作帧 _12 + 旋转效果
    auto setFlyFrame = CallFunc::create([this]() {
        auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName("Crawlid - atlas0 #0070052_12.png");
        if (frame) this->setSpriteFrame(frame);
    });
    
    // 添加旋转效果，增加死亡的戏剧性
    auto rotate = RotateBy::create(jumpDuration, 360.0f * 2);  // 旋转2圈
    
    // 添加闪烁效果
    auto blinkRed = TintTo::create(0.1f, 255, 100, 100);
    auto blinkNormal = TintTo::create(0.1f, 255, 255, 255);
    auto blinkSeq = Sequence::create(blinkRed, blinkNormal, nullptr);
    auto blink = Repeat::create(blinkSeq, 3);

    // 3. 落地后显示帧 _8 并垂直翻转
    auto setDeadFrame = CallFunc::create([this]() {
        auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName("Crawlid - atlas0 #0070052_8.png");
        if (frame) this->setSpriteFrame(frame);
        this->setFlippedY(true);
        this->setRotation(0);  // 重置旋转
    });

    // 4. 延迟后消失
    auto delayBeforeFade = DelayTime::create(1.0f);
    auto fadeOut = FadeOut::create(0.5f);
    auto remove = RemoveSelf::create(true);

    auto sequence = Sequence::create(
        setFlyFrame,
        Spawn::create(jump, rotate, blink, nullptr),  // 同时进行跳跃、旋转和闪烁
        setDeadFrame,
        delayBeforeFade,
        fadeOut,
        remove,
        nullptr
    );

    this->runAction(sequence);
}

CrawlidMonster* CrawlidMonster::createAndSpawn(cocos2d::Node* parent, const cocos2d::Vec2& spawnPosition, float patrolRange)
{
    CrawlidMonster* crawlid = CrawlidMonster::createMonster();

    if (crawlid)
    {
        crawlid->setPosition(spawnPosition);
        crawlid->setupPatrolRange(patrolRange);
        crawlid->startPatrol();

        if (parent) {
            parent->addChild(crawlid, 10);
        }
        else {
            CCLOGERROR("CrawlidMonster::createAndSpawn failed: Parent node is null.");
        }
    }
    return crawlid;
}