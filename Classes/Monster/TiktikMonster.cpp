// TiktikMonster.cpp

#include "TiktikMonster.h"
#include "SimpleAudioEngine.h"
#include "GeoManager.h"

USING_NS_CC;

// --- 文件名常量 (保持不变) ---
const char* TIKTIK_WALK_FRAMES[] = {
    "Monster/Tiktik-4.png", // Walk 1
    "Monster/Tiktik-5.png", // Walk 2
    "Monster/Tiktik-6.png", // Walk 3
    "Monster/Tiktik-7.png"  // Walk 4
};
const char* TIKTIK_TURN_FRAMES[] = {
    "Monster/Tiktik-0.png", // Turn 1
    "Monster/Tiktik-1.png", // Turn 2
    "Monster/Tiktik-2.png"  // Turn 3
};
const char* TIKTIK_STUN_FRAME = "Monster/Tiktik-3.png";
const char* TIKTIK_FLY_FRAME = "Monster/Tiktik-8.png";
const char* TIKTIK_DEAD_FRAME = "Monster/Tiktik-9.png";


TiktikMonster* TiktikMonster::createMonster()
{
    TiktikMonster* pRet = new (std::nothrow) TiktikMonster();
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

bool TiktikMonster::init()
{
    if (!Sprite::initWithFile(TIKTIK_WALK_FRAMES[0]))
    {
        CCLOGERROR("Failed to load initial Tiktik frame: %s. Check resource files.", TIKTIK_WALK_FRAMES[0]);
        return false;
    }

    // 【关键】锚点设置在图片底部中心 (0.5, 0)，这样Tiktik的脚就在精灵底部
    this->setAnchorPoint(Vec2(0.5f, 0.0f));

    _health = 3;
    _isStunned = false;
    _isPatrolling = false;
    _currentPatrolIndex = 0;
    _patrolSpeed = 80.0f;

    return true;
}

// 【核心修复】setupPatrolArea - 让 Tiktik 的脚紧贴岩石边缘爬行
void TiktikMonster::setupPatrolArea(const Vec2& center, float halfWidth, float halfHeight)
{
    CCLOG("[Tiktik] setupPatrolArea: center(%.1f, %.1f), halfWidth=%.1f, halfHeight=%.1f",
          center.x, center.y, halfWidth, halfHeight);
    
    // 获取精灵尺寸
    Size spriteSize = this->getContentSize();
    CCLOG("[Tiktik] 精灵尺寸: %.1f x %.1f", spriteSize.width, spriteSize.height);
    
    // === 【关键】：计算岩石边缘，Tiktik的脚将贴着这些边缘爬行 ===
    // 
    // 因为锚点在底部 (0.5, 0)，所以 Tiktik 的 position.y 就是脚的位置
    // 
    // 岩石块定义（这是碰撞检测矩形）：
    //   上边：y = center.y + halfHeight
    //   下边：y = center.y - halfHeight
    //   右边：x = center.x + halfWidth
    //   左边：x = center.x - halfWidth
    //
    // Tiktik 贴着岩石边缘爬行时：
    //   - 在岩石**上边**爬行时，Tiktik 的脚 (position.y) = 岩石上边
    //   - 在岩石**下边**爬行时，Tiktik 的脚 (position.y) = 岩石下边
    //   - 在岩石**左右边**爬行时，Tiktik 需要旋转 90度，脚贴着左/右边
    
    float topEdge = center.y + halfHeight;
    float bottomEdge = center.y - halfHeight;
    float rightEdge = center.x + halfWidth;
    float leftEdge = center.x - halfWidth;
    
    // 定义四个角点 + 巡逻方向（顺时针：上边向右 → 右边向下 → 下边向左 → 左边向上）
    // 
    // 巡逻路径示意图：
    //
    //   P3 (左上角) ← ← ← ← ← ← P0 (右上角)
    //      ↓                      ↑
    //      ↓ (左边向下)     (右边向上) ↑
    //      ↓                      ↑
    //   P2 (左下角) → → → → → → P1 (右下角)
    //
    // 注意：Tiktik 在四条边上爬行时，需要不同的旋转角度：
    //   - 上边（P0→P3）：头朝左，rotation = 180°
    //   - 右边（P0→P1）：头朝下，rotation = 90°
    //   - 下边（P1→P2）：头朝左，rotation = 180°
    //   - 左边（P2→P3）：头朝上，rotation = 270°
    
    Vec2 P0 = Vec2(rightEdge, topEdge);      // 右上角
    Vec2 P1 = Vec2(rightEdge, bottomEdge);   // 右下角
    Vec2 P2 = Vec2(leftEdge, bottomEdge);    // 左下角
    Vec2 P3 = Vec2(leftEdge, topEdge);       // 左上角
    
    _patrolPoints.clear();
    _patrolPoints.push_back(P0); // Index 0
    _patrolPoints.push_back(P1); // Index 1
    _patrolPoints.push_back(P2); // Index 2
    _patrolPoints.push_back(P3); // Index 3

    // 定义每段路径的旋转角度
    // 注意：这里的旋转是让 Tiktik 始终"头朝运动方向"
    _patrolRotations.clear();
    _patrolRotations.push_back(90.0f);   // P0 → P1 (右边向下)
    _patrolRotations.push_back(180.0f);  // P1 → P2 (下边向左)
    _patrolRotations.push_back(270.0f);  // P2 → P3 (左边向上)
    _patrolRotations.push_back(0.0f);    // P3 → P0 (上边向右，但此时需要翻转)

    // 设置初始位置：从右下角 (P1) 开始，沿下边向左爬
    this->setPosition(P1);
    this->setRotation(_patrolRotations[1]);  // 180° 向左
    _currentPatrolIndex = 1;
    
    CCLOG("[Tiktik] 巡逻路径设置完成:");
    CCLOG("  P0 右上角: (%.1f, %.1f)", P0.x, P0.y);
    CCLOG("  P1 右下角: (%.1f, %.1f)", P1.x, P1.y);
    CCLOG("  P2 左下角: (%.1f, %.1f)", P2.x, P2.y);
    CCLOG("  P3 左上角: (%.1f, %.1f)", P3.x, P3.y);
    CCLOG("  初始位置: P%d (%.1f, %.1f), 旋转: %.1f°", 
          _currentPatrolIndex, P1.x, P1.y, _patrolRotations[1]);
}


cocos2d::ActionInterval* TiktikMonster::createWalkAnimate()
{
    Vector<SpriteFrame*> walkFrames;
    const Size& size = this->getContentSize();

    for (int i = 0; i < 4; i++) {
        auto frame = SpriteFrame::create(TIKTIK_WALK_FRAMES[i], Rect(0, 0, size.width, size.height));
        if (frame) {
            walkFrames.pushBack(frame);
        }
        else {
            CCLOGERROR("Tiktik walk frame NOT FOUND: %s. Check resource path.", TIKTIK_WALK_FRAMES[i]);
        }
    }

    if (walkFrames.empty()) return nullptr;

    float frameDelay = 0.1f;
    auto animation = Animation::createWithSpriteFrames(walkFrames, frameDelay, 1);

    return Animate::create(animation);
}

cocos2d::ActionInterval* TiktikMonster::createTurnAnimate()
{
    Vector<SpriteFrame*> turnFrames;
    const Size& size = this->getContentSize();

    for (int i = 0; i < 3; i++) {
        auto frame = SpriteFrame::create(TIKTIK_TURN_FRAMES[i], Rect(0, 0, size.width, size.height));
        if (frame) {
            turnFrames.pushBack(frame);
        }
    }

    if (turnFrames.empty()) return DelayTime::create(0.1f);

    float frameDelay = 0.1f;
    auto animation = Animation::createWithSpriteFrames(turnFrames, frameDelay, 1);

    return Animate::create(animation);
}

void TiktikMonster::startPatrol()
{
    if (_isPatrolling) return;

    if (_patrolPoints.empty()) {
        CCLOGERROR("Patrol points not set! Call setupPatrolArea() first.");
        return;
    }

    _isPatrolling = true;
    this->executeNextPatrolStep();
}

void TiktikMonster::stopAllMonsterActions()
{
    this->stopActionByTag((int)TiktikMonsterActionTag::PATROL_MOVEMENT_TAG);
    this->stopActionByTag((int)TiktikMonsterActionTag::WALK_ANIMATION_TAG);
    this->stopActionByTag((int)TiktikMonsterActionTag::PATROL_SEQUENCE_TAG);
    this->stopActionByTag((int)TiktikMonsterActionTag::STUN_ACTION_TAG);
}

void TiktikMonster::executeNextPatrolStep()
{
    if (_isStunned) return;

    this->stopAllMonsterActions();

    int nextIndex = (_currentPatrolIndex + 1) % _patrolPoints.size();
    Vec2 startPos = _patrolPoints[_currentPatrolIndex];
    Vec2 endPos = _patrolPoints[nextIndex];
    float startRotation = _patrolRotations[_currentPatrolIndex];
    float endRotation = _patrolRotations[nextIndex];

    this->setRotation(startRotation);
    this->setFlippedX(false);
    this->setFlippedY(false);

    float distance = startPos.distance(endPos);
    float duration = distance / _patrolSpeed;
    float turnDuration = 0.3f;

    auto movePhase = Sequence::create(
        CallFunc::create([this]() {
            auto walkAnim = this->createWalkAnimate();
            if (walkAnim) {
                auto repeat = RepeatForever::create(walkAnim);
                repeat->setTag((int)TiktikMonsterActionTag::WALK_ANIMATION_TAG);
                this->runAction(repeat);
            }
        }),
        MoveTo::create(duration, endPos),
        CallFunc::create([this]() {
            this->stopActionByTag((int)TiktikMonsterActionTag::WALK_ANIMATION_TAG);
        }),
        nullptr
    );
    movePhase->setTag((int)TiktikMonsterActionTag::PATROL_MOVEMENT_TAG);

    auto rotateAction = RotateTo::create(turnDuration, endRotation);
    auto turnAnimate = this->createTurnAnimate();
    auto turnAction = Spawn::createWithTwoActions(turnAnimate, rotateAction);

    auto nextStep = CallFunc::create([this, nextIndex]() {
        _currentPatrolIndex = nextIndex;
        this->executeNextPatrolStep();
    });

    auto patrolSequence = Sequence::create(
        movePhase,
        turnAction,
        nextStep,
        nullptr
    );

    patrolSequence->setTag((int)TiktikMonsterActionTag::PATROL_SEQUENCE_TAG);
    this->runAction(patrolSequence);
}

void TiktikMonster::resumePatrol()
{
    if (_health > 0 && !_isPatrolling && _isStunned) {

        this->stopAllMonsterActions();

        _isStunned = false;
        _isPatrolling = true;

        auto initialFrame = SpriteFrame::create(TIKTIK_WALK_FRAMES[0], Rect(0, 0, this->getContentSize().width, this->getContentSize().height));
        if (initialFrame) this->setSpriteFrame(initialFrame);

        Vec2 currentPos = this->getPosition();
        float minDistance = -1.0f;
        int closestIndex = _currentPatrolIndex;

        for (size_t i = 0; i < _patrolPoints.size(); ++i) {
            float dist = currentPos.distance(_patrolPoints[i]);
            if (minDistance < 0.0f || dist < minDistance) {
                minDistance = dist;
                closestIndex = (int)i;
            }
        }

        if (minDistance < 20.0f) {
            CCLOG("Tiktik: Resuming patrol near point %d", closestIndex);
            this->setPosition(_patrolPoints[closestIndex]);
            _currentPatrolIndex = closestIndex;
        }
        else {
            CCLOG("Tiktik: Resuming patrol from current position toward next point.");
        }

        this->executeNextPatrolStep();
    }
}

void TiktikMonster::takeDamage(int damage, float knockbackPower, int knockbackDirection)
{
    if (_isStunned || _health <= 0) return;

    _health -= damage;

    // 播放受击音效
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Music/enemy_damage.wav");

    if (_health <= 0)
    {
        this->die(knockbackPower, knockbackDirection);
        return;
    }

    this->stopAllMonsterActions();
    _isPatrolling = false;
    _isStunned = true;

    auto stunFrame = SpriteFrame::create(TIKTIK_STUN_FRAME, Rect(0, 0, this->getContentSize().width, this->getContentSize().height));
    if (stunFrame) {
        this->setSpriteFrame(stunFrame);
        this->setFlippedX(false);
        this->setFlippedY(false);
    }
    else {
        CCLOGERROR("Failed to load Tiktik stun frame: %s. Check resource files.", TIKTIK_STUN_FRAME);
    }

    float stopDuration = 1.0f;
    auto resumeCall = CallFunc::create(CC_CALLBACK_0(TiktikMonster::resumePatrol, this));

    auto sequence = Sequence::create(
        DelayTime::create(stopDuration),
        resumeCall,
        nullptr
    );
    sequence->setTag((int)TiktikMonsterActionTag::STUN_ACTION_TAG);
    this->runAction(sequence);
}

void TiktikMonster::die(float knockbackPower, int knockbackDirection)
{
    if (_isStunned) return;

    _isStunned = true;
    _isPatrolling = false;
    this->stopAllActions();

    // 播放死亡音效
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Music/enemy_death.wav");

    float flyHeight = 150.0f;
    float jumpDuration = 0.8f;
    float horizontalDistance = knockbackPower * 2.0f;
    Vec2 currentPos = this->getPosition();

    Vec2 landingPos = Vec2(currentPos.x + (float)knockbackDirection * horizontalDistance, currentPos.y);
    
    float mapWidth = 120.0f * 16.0f * 2.61f;
    float minX = 200.0f;
    float maxX = mapWidth - 200.0f;
    
    landingPos.x = clampf(landingPos.x, minX, maxX);
    landingPos.y = MAX(landingPos.y, currentPos.y);
    
    CCLOG("[Tiktik Death] 当前位置: (%.1f, %.1f), 落地位置: (%.1f, %.1f)", 
          currentPos.x, currentPos.y, landingPos.x, landingPos.y);

    auto jump = JumpTo::create(jumpDuration, landingPos, flyHeight, 1);

    auto setFlyFrame = CallFunc::create([this]() {
        auto frame = SpriteFrame::create(TIKTIK_FLY_FRAME, Rect(0, 0, this->getContentSize().width, this->getContentSize().height));
        if (frame) {
            this->setSpriteFrame(frame);
            this->setRotation(0.0f);
            this->setFlippedY(false);
        }
    });

    auto setDeadFrame = CallFunc::create([this]() {
        auto frame = SpriteFrame::create(TIKTIK_DEAD_FRAME, Rect(0, 0, this->getContentSize().width, this->getContentSize().height));
        if (frame) {
            this->setSpriteFrame(frame);
        }
        this->setRotation(0.0f);
    });

    auto delayBeforeFade = DelayTime::create(2.0f);
    auto fadeOut = FadeOut::create(0.5f);
    auto removeSelf = RemoveSelf::create(true);

    auto deathSequence = Sequence::create(
        setFlyFrame,
        jump,
        setDeadFrame,
        delayBeforeFade,
        fadeOut,
        removeSelf,
        nullptr
    );

    GeoManager::getInstance()->addGeo(2);

    this->runAction(deathSequence);
}

TiktikMonster* TiktikMonster::createAndSpawn(cocos2d::Node* parent, const cocos2d::Vec2& centerPosition, float halfWidth, float halfHeight)
{
    CCLOG("=== TiktikMonster::createAndSpawn 开始 ===");
    CCLOG("  参数: center(%.1f, %.1f), halfWidth=%.1f, halfHeight=%.1f", 
          centerPosition.x, centerPosition.y, halfWidth, halfHeight);
    
    TiktikMonster* tiktik = TiktikMonster::createMonster();

    if (tiktik)
    {
        CCLOG("  [✓] Tiktik实例创建成功");
        
        tiktik->setupPatrolArea(centerPosition, halfWidth, halfHeight);
        CCLOG("  [✓] 巡逻区域设置完成");
        CCLOG("      初始位置: (%.1f, %.1f)", tiktik->getPosition().x, tiktik->getPosition().y);
        
        tiktik->startPatrol();
        CCLOG("  [✓] 巡逻AI已启动");

        if (parent) {
            parent->addChild(tiktik, 100);
            CCLOG("  [✓] 已添加到父节点, Z轴=100");
            CCLOG("      父节点名称: %s", parent->getName().c_str());
            
            auto verifyParent = tiktik->getParent();
            if (verifyParent) {
                CCLOG("  [✓] 验证: Tiktik已成功关联到父节点");
            } else {
                CCLOG("  [✗] 警告: Tiktik未关联到父节点!");
            }
        }
        else {
            CCLOGERROR("  [✗] TiktikMonster::createAndSpawn failed: Parent node is null.");
            return nullptr;
        }
    }
    else
    {
        CCLOGERROR("  [✗] Tiktik实例创建失败! 检查资源文件: %s", TIKTIK_WALK_FRAMES[0]);
        return nullptr;
    }
    
    CCLOG("=== TiktikMonster::createAndSpawn 完成 ===\n");
    return tiktik;
}