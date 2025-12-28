#include "HornetBoss.h"
#include "AudioManager.h"

USING_NS_CC;

HornetBoss* HornetBoss::createWithFolder(const std::string& folderPath) {
    auto boss = new (std::nothrow) HornetBoss();
    if (boss && boss->init(folderPath)) {
        boss->autorelease();
        return boss;
    }
    CC_SAFE_DELETE(boss);
    return nullptr;
}

bool HornetBoss::init(const std::string& folderPath) {
    _folderPath = folderPath;

    // 初始显示
    if (!Sprite::initWithFile(_folderPath + "/Hornet_hardland_5.png")) {
        return false;
    }

    // 设置锚点为底部中心，方便处理地面逻辑
    this->setAnchorPoint(Vec2(0.5f, 0.0f));

    // 开启每帧更新
    this->scheduleUpdate();
    
    _currentPhase = 1;
    _phaseHitCount = 0;
    _defeatedHitCount = 0;

    return true;
}

void HornetBoss::setBoundaries(float groundY, float minX, float maxX) {
    _groundY = groundY;
    _minX = minX;
    _maxX = maxX;
}

void HornetBoss::updateTargetPosition(Vec2 targetPos) {
    _targetPos = targetPos;
}

// 辅助：根据目标位置调整朝向
void HornetBoss::lookAtTarget() {
    float bossX = this->getPositionX();
    float targetX = _targetPos.x;

    // 假设：stand原图朝右 (default FlippedX = false)
    // 假设：walk原图朝左 (default FlippedX = false)

    // 我们改用简单的 setFlippedX 来处理，逻辑更清晰
    if (targetX > bossX) {
        // 目标在右
        // 如果当前是 stand (原图朝右)，则不翻转；如果是 walk (原图朝左)，则需要翻转
        // 为了统一，我们直接根据当前动作名或状态来判断，
        // 但最简单的方法是统一 Scale，这里我根据你的描述修正：
        this->setScaleX(-1.0f); // 强制镜像
    }
    else {
        this->setScaleX(1.0f);  // 原始方向
    }

    // 如果发现 stand 对了 walk 反了，请将上面的 -1.0f 和 1.0f 对调即可
}

Animate* HornetBoss::createAnimate(const std::string& prefix, int startIdx, int endIdx, float delay, bool reverse) {
    Vector<SpriteFrame*> frames;
    auto build = [&](int i) {
        std::string path = StringUtils::format("%s/%s_%d.png", _folderPath.c_str(), prefix.c_str(), i);
        auto tex = Director::getInstance()->getTextureCache()->addImage(path);
        if (tex) {
            auto size = tex->getContentSize();
            frames.pushBack(SpriteFrame::createWithTexture(tex, Rect(0, 0, size.width, size.height)));
        }
        };
    if (!reverse) { for (int i = startIdx; i <= endIdx; i++) build(i); }
    else { for (int i = startIdx; i >= endIdx; i--) build(i); }
    return Animate::create(Animation::createWithSpriteFrames(frames, delay));
}

void HornetBoss::update(float dt) {
    // 1. 全局转向参考（非强制，仅更新目标点数值）
    if (_player) {
        // TODO: 未来如果 Player 类有专门的获取中心点方法（如 getCenterPosition），应替换 getPosition()
        _targetPos = _player->getPosition();
    }

    // 2. 如果动作被锁定（比如正在释放大招），则跳过当前的物理更新
    if (_isActionLocked) return;

    // 3. 执行当前动作对应的物理逻辑（如走路位移、跳跃曲线）
    if (_currentPhysicsUpdate) {
        _currentPhysicsUpdate(dt);
    }
}
// --- 组合动画实现 ---

void HornetBoss::playEntryAnimation(float startX, float startY) {
    this->stopActionByTag(10);
    _currentPhysicsUpdate = nullptr;
    _isActionLocked = true; // 明确锁定

    this->setPosition(Vec2(startX, startY));

    // 下落动作
    auto fallToGround = MoveTo::create(0.4f, Vec2(startX, _groundY));

    // 落地动画：5帧到0帧 (reverse=true)
    auto landAnimate = createAnimate("Hornet_hardland", 5, 0, 0.06f, true);

    // 关键修改：确保回调在 landAnimate 之后执行
    auto seq = Sequence::create(
        fallToGround,
        landAnimate,
        CallFunc::create([this]() {
            this->_isActionLocked = false; // 1. 先解锁
            this->decideNextAction();      // 2. 启动思考逻辑
            }),
        nullptr
    );

    seq->setTag(10);
    this->runAction(seq);
}
void HornetBoss::playIdleAnimation() {
    this->stopActionByTag(10);

    _currentPhysicsUpdate = [this](float dt) {
        // stand原图朝右。目标在右(distance>0)则不翻转(1)，目标在左则翻转(-1)
        // 注意：如果这里方向反了，请把 1.0f 和 -1.0f 对调
        if (_targetPos.x > this->getPositionX()) this->setScaleX(1.0f);
        else this->setScaleX(-1.0f);
        };

    auto idleAnim = createAnimate("Hornet_stand", 0, 3, 0.15f);
    auto repeat = RepeatForever::create(idleAnim);
    repeat->setTag(10);
    this->runAction(repeat);
}
// 修改 HornetBoss.cpp 中的 playWalkAnimation 物理部分
void HornetBoss::playWalkAnimation() {
    this->stopActionByTag(10);
    _currentPhysicsUpdate = nullptr;

    // 1. 播放走路动画帧
    auto animate = createAnimate("Hornet_walk", 0, 5, 0.1f);
    auto repeat = RepeatForever::create(animate);
    repeat->setTag(10);
    this->runAction(repeat);

    // 2. 物理寻敌逻辑
    _currentPhysicsUpdate = [this](float dt) {
        if (!_player) return;

        float moveSpeed = 180.0f;
        float currentX = this->getPositionX();
        // TODO: 未来替换为 Player::getPositionX() 或 Player::getCenterPosition().x
        float playerX = _player->getPositionX();

        // --- 计算方向：始终朝向玩家 ---
        float direction = (playerX > currentX) ? 1.0f : -1.0f;

        // --- 更新朝向 (ScaleX) ---
        // 根据你之前的逻辑：目标在右则镜像 (-1.0)，在左则原始 (1.0)
        this->setScaleX(direction > 0 ? -1.0f : 1.0f);

        // --- 执行位移 ---
        float nextX = currentX + (direction * moveSpeed * dt);

        // --- 边界检查与距离停止 ---
        // 如果离玩家非常近（例如 50 像素以内），则停止移动
        if (std::abs(playerX - currentX) < 50.0f) {
            return;
        }

        // 确保不超出地图边界
        nextX = std::max(_minX, std::min(nextX, _maxX));
        this->setPositionX(nextX);
        this->setPositionY(_groundY); // 确保高度固定在地面
        };
}
void HornetBoss::playJumpAnimation(cocos2d::Vec2 targetPos) {
    this->stopActionByTag(10);

    // 1. 物理参数初始化
    float startX = this->getPositionX();
    float startY = this->getPositionY();

    // 计算水平位移：跳向目标点（可以是玩家位置，模拟躲避）
    float distanceX = (targetPos.x - startX) * 1.2f;

    // 设定物理量
    float jumpHeight = 350.0f;       // 跳跃高度
    float hangTime = 0.7f;          // 预计空中的总时间
    _gravity = -2000.0f;            // 增加重力感，让动作更凌厉

    // 初速度计算 v = sqrt(2gh)
    _jumpVelocityY = sqrtf(2.0f * fabsf(_gravity) * jumpHeight);
    // 水平速度 v = s / t
    _jumpSpeedX = distanceX / hangTime;

    // 2. 物理更新闭包
    _currentPhysicsUpdate = [this](float dt) {
        // 垂直位移
        _jumpVelocityY += _gravity * dt;
        float nextY = this->getPositionY() + _jumpVelocityY * dt;

        // 水平位移
        float nextX = this->getPositionX() + _jumpSpeedX * dt;
        nextX = std::max(_minX, std::min(nextX, _maxX)); // 边界限制

        // 落地检测
        if (nextY <= _groundY) {
            nextY = _groundY;
            this->setPosition(Vec2(nextX, nextY));
            _currentPhysicsUpdate = nullptr; // 停止物理更新
            this->playIdleAnimation();      // 落地切待机

            // --- 修复点：落地后延迟一小会儿，然后继续AI决策 ---
            this->runAction(Sequence::create(
                DelayTime::create(0.2f),
                CallFunc::create([this]() { this->decideNextAction(); }),
                nullptr
            ));
            return;
        }

        this->setPosition(Vec2(nextX, nextY));

        // 朝向：跳向左侧时 setScaleX(1.0)，右侧时 (-1.0)
        if (_jumpSpeedX > 0) this->setScaleX(-1.0f);
        else this->setScaleX(1.0f);
        };

    // 3. 动画表现
    // 上升 0-8 帧，下降 8-0 帧
    auto riseAnim = createAnimate("Hornet_fly", 0, 8, 0.04f);
    auto fallAnim = createAnimate("Hornet_fly", 8, 0, 0.04f, true);

    auto seq = Sequence::create(riseAnim, fallAnim, nullptr);
    seq->setTag(10);
    this->runAction(seq);
}
void HornetBoss::playAttack1Animation(cocos2d::Vec2 targetPos) {
    // 1. 停止所有动作和物理逻辑，防止回调冲突
    this->stopActionByTag(10);
    _isActionLocked = true;

    // 播放攻击1音效
    AudioManager::getInstance()->playHornetAttack1Sound();

    // 2. 实时寻敌，根据传入的 targetPos 确定方向
    float currentX = this->getPositionX();
    float direction = (targetPos.x > currentX) ? 1.0f : -1.0f; // 1为右，-1为左

    // 修正图像朝向：根据此前逻辑，目标在右则镜像 (-1.0f)，在左则原始 (1.0f)
    this->setScaleX(direction > 0 ? -1.0f : 1.0f);

    // 辅助获取帧的方法
    auto getFrame = [this](const std::string& prefix, int idx) -> SpriteFrame* {
        std::string path = StringUtils::format("%s/%s_%d.png", _folderPath.c_str(), prefix.c_str(), idx);
        auto tex = Director::getInstance()->getTextureCache()->addImage(path);
        if (tex) return SpriteFrame::createWithTexture(tex, Rect(0, 0, tex->getContentSize().width, tex->getContentSize().height));
        return nullptr;
        };

    // --- A. 蓄力动画阶段 (2,3,2,3,2,3,1,4,5,6,7,8,9) ---
    Vector<SpriteFrame*> chargeFrames;
    int seqIdx[] = { 2,3,2,3,2,3,1,4,5,6,7,8,9 };
    for (int i : seqIdx) {
        auto frame = getFrame("Hornet_attack1", i);
        if (frame) chargeFrames.pushBack(frame);
    }
    auto chargeAnimate = Animate::create(Animation::createWithSpriteFrames(chargeFrames, 0.06f));

    // --- B. 局部特效：动态创建与自动销毁 ---
    auto playEffectAction = CallFunc::create([this]() {
        auto effect = Sprite::create();
        // 设置在人物中心偏上的相对位置
        effect->setPosition(Vec2(this->getContentSize().width / 2, 100.0f));
        this->addChild(effect, 1);

        // 播放 0-3 帧白光特效
        auto lightAnim = createAnimate("Hornet_attack1_light", 0, 3, 0.07f);

        // 执行完动画后自动从父节点移除，释放内存
        effect->runAction(Sequence::create(lightAnim, RemoveSelf::create(), nullptr));
        });

    // 将蓄力动作与特效爆发组合（特效在蓄力中段触发）
    auto chargeWithLight = Spawn::create(
        chargeAnimate,
        Sequence::create(DelayTime::create(0.6f), playEffectAction, nullptr),
        nullptr
    );

    // --- C. 动态冲刺位移计算 ---
    float dashDistance = 550.0f; // 固定冲刺距离
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 计算基于当前方向的目标 X 坐标
    float targetDashX = currentX + (direction * dashDistance);

    // 边界硬约束：参考 setBoundaries 逻辑
    if (targetDashX < _minX) targetDashX = _minX;
    if (targetDashX > _maxX) targetDashX = _maxX;

    // --- D. 动作组装 ---
    auto setDashFrame = CallFunc::create([this, getFrame]() {
        auto f11 = getFrame("Hornet_attack1", 11); // 切换为长条状的冲刺帧
        if (f11) this->setSpriteFrame(f11);
        });

    auto dashMove = MoveTo::create(0.12f, Vec2(targetDashX, _groundY));

    auto sequence = Sequence::create(
        chargeWithLight,        // 1. 蓄力并播放特效
        setDashFrame,           // 2. 变换为冲刺姿态
        dashMove,               // 3. 执行物理位移
        DelayTime::create(0.4f),// 4. 攻击后的硬直阶段
        CallFunc::create([this]() {
            _isActionLocked = false;
            this->decideNextAction();
            }),
        nullptr
    );

    sequence->setTag(10);
    this->runAction(sequence);
}
void HornetBoss::playAttack2Animation(cocos2d::Vec2 targetPos) {
    this->stopActionByTag(10);
    _isActionLocked = true;

    // 播放攻击2音效
    AudioManager::getInstance()->playHornetAttack2Sound();

    // 1. 方向判断与镜像 (根据当前鼠标方向)
    float currentX = this->getPositionX();
    bool isTargetOnRight = (targetPos.x > currentX);
    if (isTargetOnRight) {
        this->setScaleX(1.0f);
    }
    else {
        this->setScaleX(-1.0f);
    }

    auto getFrame = [this](const std::string& prefix, int idx) -> SpriteFrame* {
        std::string path = StringUtils::format("%s/%s_%d.png", _folderPath.c_str(), prefix.c_str(), idx);
        auto tex = Director::getInstance()->getTextureCache()->addImage(path);
        if (tex) return SpriteFrame::createWithTexture(tex, Rect(0, 0, tex->getContentSize().width, tex->getContentSize().height));
        return nullptr;
        };

    // --- A. Boss 本体动作 ---
    auto bodyAnim = createAnimate("Hornet_attack2", 0, 6, 0.08f);

    // --- B. 武器投掷逻辑 ---
    auto throwWeapon = CallFunc::create([this, getFrame]() {
        auto weapon = Sprite::create(_folderPath + "/Hornet_weapon1.png");
        weapon->setAnchorPoint(Vec2(0.5f, 0.5f));

        // --- 调整点 1: 初始高度下移 ---
        // 将初始 Y 从 80.0f 下移到 65.0f，让针从手部更低的位置飞出
        float startY = 65.0f;
        weapon->setPosition(Vec2(this->getContentSize().width / 2, startY));
        this->addChild(weapon, 2);

        // --- 调整点 2: 丝线位置 (在武器后面) ---
        auto threadEffect = Sprite::create();
        threadEffect->setPosition(Vec2(weapon->getContentSize().width / 2-400.0f, weapon->getContentSize().height / 2));
        // z-order 为 -1 确保丝线在武器贴图下方
        weapon->addChild(threadEffect, -1);
        threadEffect->runAction(RepeatForever::create(createAnimate("Hornet_weapon1", 1, 6, 0.05f)));

        // 白光特效
        auto light = Sprite::create(_folderPath + "/Hornet_attack2_light.png");
        light->setPosition(Vec2(this->getContentSize().width / 2, startY));
        this->addChild(light, 3);
        light->runAction(Sequence::create(FadeOut::create(0.8f), RemoveSelf::create(), nullptr));

        // --- 调整点 3: 飞行轨迹 ---
        // --- 原代码 ---
        float throwDist = 700.0f;
        float flyOutTime = 0.45f;
        float flyBackTime = 0.35f;

        // 将抛出的垂直高度也降低（从 40 降到 20），让轨迹更平直、更写实
        auto moveOut = MoveBy::create(flyOutTime, Vec2(throwDist, 20.0f));
        auto moveBack = MoveTo::create(flyBackTime, Vec2(this->getContentSize().width / 2, startY));

        weapon->runAction(Sequence::create(
            EaseCubicActionOut::create(moveOut),
            DelayTime::create(0.15f),
            EaseCubicActionIn::create(moveBack),
            RemoveSelf::create(),
            nullptr
        ));
        });

    // --- C. 动作总链 ---
    auto sequence = Sequence::create(
        Spawn::create(
            bodyAnim,
            Sequence::create(DelayTime::create(0.16f), throwWeapon, nullptr),
            nullptr
        ),
        DelayTime::create(0.85f), // 对应武器飞行的总时长
        CallFunc::create([this]() {
            _isActionLocked = false;
            this->decideNextAction();
            }),
        nullptr
    );

    sequence->setTag(10);
    this->runAction(sequence);
}
void HornetBoss::playAttack3Animation(cocos2d::Vec2 targetPos) {
    // 1. 准备阶段：停止旧动作和物理逻辑
    this->stopActionByTag(10);
    _isActionLocked = true;

    // 播放攻击3音效
    AudioManager::getInstance()->playHornetAttack3Sound();

    float currentX = this->getPositionX();
    bool isTargetOnRight = (targetPos.x > currentX);

    // 保持原有逻辑：上升阶段朝向玩家
    this->setScaleX(isTargetOnRight ? -1.0f : 1.0f);

    float jumpHeight = 350.0f;
    auto jumpToTop = MoveBy::create(0.4f, Vec2(0, jumpHeight));

    // 2. 蓄力阶段：执行你原有的反转逻辑和动画
    auto doCharge = CallFunc::create([this, isTargetOnRight]() {
        // 保持你原注释中的调试位：如果 1.0 反，就改 -1.0
        this->setScaleX(isTargetOnRight ? -1.0f : 1.0f);
        });
    auto chargeAnim = createAnimate("Hornet_attack3", 0, 2, 0.15f);
    auto longCharge = Repeat::create(chargeAnim, 2);

    // 3. 俯冲阶段：这是最需要修改逻辑的地方（合并了 dive 和结束回调）
    auto doDiveAndCleanup = CallFunc::create([this, targetPos]() {
        float curX = this->getPositionX();
        float idealTargetX = targetPos.x;
        float minSpan = 50.0f;

        if (std::abs(idealTargetX - curX) < minSpan) {
            float direction = (this->getScaleX() > 0) ? -1.0f : 1.0f;
            idealTargetX = curX + (direction * minSpan);
        }

        float safeTargetX = std::max(_minX, std::min(idealTargetX, _maxX));
        this->setScaleX((safeTargetX > curX) ? -1.0f : 1.0f);

        // --- 保持原有俯冲动画和位移数值 ---
        auto diveAnim = createAnimate("Hornet_attack3", 4, 6, 0.05f);
        float distance = std::abs(safeTargetX - curX);
        float diveTime = 0.1f;
        auto diveMove = MoveTo::create(diveTime, Vec2(safeTargetX, _groundY));

        // --- 关键修改：创建一个局部的 Sequence 确保落地后解锁 ---
        auto landSequence = Sequence::create(
            Spawn::create(diveAnim, EaseIn::create(diveMove, 2.0f), nullptr),
            CallFunc::create([this]() {
                // 执行你原有的落地特效逻辑
                auto light = Sprite::create();
                light->setPosition(Vec2(this->getContentSize().width / 2, 150));
                this->addChild(light, 3);
                light->runAction(Sequence::create(
                    createAnimate("Hornet_attack3_light", 0, 2, 0.15f),
                    RemoveSelf::create(),
                    nullptr
                ));
                }),
            DelayTime::create(0.6f), // 保持原有的落地僵直时间
            CallFunc::create([this]() {
                // --- 只有在这里才解锁并进入下一招 ---
                _isActionLocked = false;
                this->decideNextAction();
                }),
            nullptr
        );
        landSequence->setTag(10); // 统一 Tag
        this->runAction(landSequence);
        });

    // 4. 组装总序列
    auto fullSequence = Sequence::create(
        jumpToTop,
        doCharge,
        longCharge,
        doDiveAndCleanup, // 逻辑链条在这里会通过内部的 landSequence 延续下去
        nullptr
    );

    fullSequence->setTag(10);
    this->runAction(fullSequence);
}
void HornetBoss::playAttack4Animation(cocos2d::Vec2 targetPos) {
    this->stopActionByTag(10);
    _isActionLocked = true;

    // 播放攻击4音效
    AudioManager::getInstance()->playHornetAttack4Sound();

    // 1. 转向逻辑
    float currentX = this->getPositionX();
    bool isTargetOnRight = (targetPos.x > currentX);
    this->setScaleX(isTargetOnRight ? -1.0f : 1.0f);

    // 2. 升空与前摇
    auto jumpToAir = MoveBy::create(0.25f, Vec2(0, 100.0f)); 
    auto prepAnim = createAnimate("Hornet_attack5", 0, 1, 0.1f);

    // 3. 核心乱舞逻辑
    auto startStorm = CallFunc::create([this]() {
        // --- 直接定义往返帧顺序 ---
        std::vector<int> frameOrder = { 2, 3, 4, 5, 6, 5, 4, 3 };
        Vector<SpriteFrame*> pingPongFrames;

        for (int idx : frameOrder) {
            std::string path = StringUtils::format("%s/Hornet_attack5_%d.png", _folderPath.c_str(), idx);
            auto tex = Director::getInstance()->getTextureCache()->addImage(path);
            if (tex) {
                pingPongFrames.pushBack(SpriteFrame::createWithTexture(tex, Rect(0, 0, tex->getContentSize().width, tex->getContentSize().height)));
            }
        }

        auto animation = Animation::createWithSpriteFrames(pingPongFrames, 0.04f);
        auto bodyLoop = RepeatForever::create(Animate::create(animation));
        bodyLoop->setTag(11);
        this->runAction(bodyLoop);

        // --- 丝线特效 ---
        if (_attack4Effect) {
            _attack4Effect->removeFromParent();
            _attack4Effect = nullptr;
        }
        auto stormEffect = Sprite::create();
        stormEffect->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
        stormEffect->setScale(1.3f);
        this->addChild(stormEffect, 2);
        _attack4Effect = stormEffect;

        auto weaponAnim = createAnimate("Hornet_weapon2", 0, 3, 0.04f);
        stormEffect->runAction(RepeatForever::create(weaponAnim));

        // --- 3秒后的收招逻辑 ---
        auto finishSeq = Sequence::create(
            DelayTime::create(3.0f),
            CallFunc::create([this, stormEffect]() {
                this->stopActionByTag(11); // 停止 23456543 循环
                stormEffect->removeFromParent();
                if (_attack4Effect == stormEffect) {
                    _attack4Effect = nullptr;
                }
                }),
            EaseIn::create(MoveTo::create(0.15f, Vec2(this->getPositionX(), _groundY)), 2.0f),
            CallFunc::create([this]() {
                _isActionLocked = false;
                this->decideNextAction();
                }),
            nullptr
        );
        this->runAction(finishSeq);
        });

    auto fullSeq = Sequence::create(jumpToAir, prepAnim, startStorm, nullptr);
    fullSeq->setTag(10);
    this->runAction(fullSeq);
}
// 受击动画 1：硬直 2 秒 (injured_0 - 1)
void HornetBoss::playInjuredAction1() {
    // 强制清理所有可能导致位移的物理逻辑
    _currentPhysicsUpdate = nullptr;
    this->stopAllActions();

    _isActionLocked = true;
    _aiState = AIState::STUNNED;

    auto anim = createAnimate("Hornet_injured", 0, 1, 0.3f);
    auto loop = Repeat::create(anim, 4); // 循环4次而不是 Forever，保证能退出

    auto restore = Sequence::create(
        loop,
        CallFunc::create([this]() {
            _isActionLocked = false;
            _aiState = AIState::IDLE;
            this->decideNextAction();
            }),
        nullptr
    );
    this->runAction(restore);
}

// 受击动画 2：硬直 3 秒 (injured2_0 - 1)
void HornetBoss::playInjuredAction2() {
    this->stopActionByTag(10);
    _isActionLocked = true;

    // 创建受伤 2 动画
    auto anim = createAnimate("Hornet_injured2", 0, 1, 0.3f);
    auto loop = RepeatForever::create(anim);
    loop->setTag(10);
    this->runAction(loop);

    // 3 秒后恢复
    auto restore = Sequence::create(
        DelayTime::create(3.0f),
        CallFunc::create([this]() {
            this->stopActionByTag(10);
            _isActionLocked = false;
            this->decideNextAction();
            }),
        nullptr
    );
    restore->setTag(11);
    this->runAction(restore);
}

// 持续受伤状态（不自动恢复）
void HornetBoss::playInjuredAction2Loop() {
    this->stopAllActions();
    _isActionLocked = true;
    _aiState = AIState::DEFEATED;
    _currentPhysicsUpdate = nullptr;

    // 创建受伤 2 动画
    auto anim = createAnimate("Hornet_injured2", 0, 1, 0.3f);
    auto loop = RepeatForever::create(anim);
    loop->setTag(10);
    this->runAction(loop);
}

void HornetBoss::playFallAnimation(cocos2d::Vec2 attackSourcePos) {
    // 1. 初始化：停止当前动作并锁定
    this->stopActionByTag(10);
    _isActionLocked = true;

    // 2. 判定方向
    // 如果攻击源在 Boss 右侧，Boss 应该向左飞
    float currentX = this->getPositionX();
    bool attackFromRight = (attackSourcePos.x > currentX);

    // Hornet_fall 原图是向右飞。
    // 如果从右往左打（向左飞），setScaleX(-1.0) 镜像。
    // 如果从左往右打（向右飞），setScaleX(1.0) 保持原样。
    this->setScaleX(attackFromRight ? -1.0f : 1.0f);

    // 3. 设置击飞姿态帧
    // 这里假设只有一帧 fall.png，直接设置纹理
    std::string fallPath = StringUtils::format("%s/Hornet_fall.png", _folderPath.c_str());
    this->setTexture(fallPath);

    // 4. 创建白光特效 (Hornet_fall_light.png)
    auto light = Sprite::create(StringUtils::format("%s/Hornet_fall_light.png", _folderPath.c_str()));
    if (light) {
        light->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
        light->setOpacity(255);
        light->setScale(1.0f); // 从原图大小开始
        this->addChild(light, 5);

        // 白光效果：逐渐放大并消失
        auto lightAction = Spawn::create(
            ScaleTo::create(0.8f, 2.5f), // 放大到 2.5 倍
            FadeOut::create(1.5f),      // 逐渐变透明
            nullptr
        );
        light->runAction(Sequence::create(lightAction, RemoveSelf::create(), nullptr));
    }

    // 5. 抛物线位移 (JumpBy)
    // 根据方向决个位移正负
    float jumpDistance = attackFromRight ? -150.0f : 150.0f;
    // 参数：时长, 位移向量, 跳跃高度, 跳跃次数
    auto jumpAction = JumpBy::create(0.5f, Vec2(jumpDistance, 0), 80.0f, 1);

    // 6. 执行序列
    auto fallSequence = Sequence::create(
        jumpAction,
        DelayTime::create(0.5f), // 落地后的短暂僵直
        CallFunc::create([this]() {
            // 修改：如果是击败状态，落地后直接离场
            if (_aiState == AIState::DEFEATED) {
                this->playLeaveAnimation();
            } else {
                _isActionLocked = false;
                this->decideNextAction();
            }
            }),
        nullptr
    );

    fallSequence->setTag(10);
    this->runAction(fallSequence);
}
void HornetBoss::playLeaveAnimation() {
    this->stopActionByTag(10);
    _isActionLocked = true;

    // 1. 定义终点（屏幕上方中心，考虑可见区域原点）
    auto director   = Director::getInstance();
    auto visibleSize = director->getVisibleSize();
    auto origin      = director->getVisibleOrigin();
    Vec2 exitPos = Vec2(origin.x + visibleSize.width / 2.0f,
                        origin.y + visibleSize.height + 200.0f);

    // --- 阶段 A: 原地跳起并悬停 ---
    auto jumpUp = MoveBy::create(0.3f, Vec2(0, 350.0f));

    auto leaveLogic = CallFunc::create([this, exitPos]() {
        // 先左右镜像，再上下镜像，重置并旋转 180 度让头部朝上
        this->setScaleX(-1.0f);
        this->setScaleY(-1.0f);
        this->setRotation(180.0f);

        // 1. 转身动画 (leave4 -> 5)
        auto turnAndHold = createAnimate("Hornet_leave", 4, 5, 0.08f);

        // 2. 丝线逻辑
        auto silkLogic = CallFunc::create([this]() {
            auto silk = Sprite::create();
            silk->setPosition(Vec2(this->getContentSize().width / 2,
                                   this->getContentSize().height / 2));
            silk->setRotation(180.0f);
            this->addChild(silk, -1);

            auto silkFlying = createAnimate("Hornet_leave_silk", 0, 2, 0.05f);
            auto silkTension = CallFunc::create([this]() {
                this->setTexture(StringUtils::format("%s/Hornet_leave_6.png", _folderPath.c_str()));
            });
            auto silkFrame3   = createAnimate("Hornet_leave_silk", 3, 3, 0.1f);
            auto silkDisappear = CallFunc::create([silk]() { silk->removeFromParent(); });

            silk->runAction(Sequence::create(silkFlying, silkTension, silkFrame3, silkDisappear, nullptr));
        });

        // 3. 人物弹射冲出（飞向场景上方中央）
        auto shootOut = CallFunc::create([this, exitPos]() {
            auto flyLoop = RepeatForever::create(createAnimate("Hornet_leave", 0, 2, 0.05f));
            this->runAction(flyLoop);

            auto move = MoveTo::create(0.4f, exitPos);
            this->runAction(Sequence::create(
                EaseIn::create(move, 3.0f),
                CallFunc::create([this]() { this->removeFromParent(); }),
                nullptr
            ));
        });

        // 延长悬停时间：原 0.25f -> 0.60f
        this->runAction(Sequence::create(
            turnAndHold,
            silkLogic,
            DelayTime::create(0.60f),
            shootOut,
            nullptr
        ));
    });

    auto fullSeq = Sequence::create(jumpUp, leaveLogic, nullptr);
    fullSeq->setTag(10);
    this->runAction(fullSeq);
}

void HornetBoss::resetState() {
    this->stopAllActions(); // 停止所有动作，防止 Tag 漏掉
    _isActionLocked = false;
    _isJumping = false;
    _currentPhysicsUpdate = nullptr;
    _aiState = AIState::IDLE;
    this->setRotation(0);   // 重置旋转（防止 Leave 动画残留）
}

// 补全缺失定义，修复链接错误
void HornetBoss::moveToSafetyAndAttack(std::function<void()> onComplete) {
    this->stopActionByTag(10);
    _currentPhysicsUpdate = nullptr;
    _isActionLocked = true;
    _isFacingLocked = true;
    _aiState = AIState::EVADING;

    // 向场地中心靠拢
    float centerX = (_minX + _maxX) / 2.0f;
    float currentX = this->getPositionX();
    float jumpDir = (centerX > currentX) ? 1.0f : -1.0f;

    // 背对跳跃方向
    this->setScaleX(-jumpDir);

    // 动画帧
    auto frame0 = StringUtils::format("%s/Hornet_backjump_0.png", _folderPath.c_str());
    auto frame1 = StringUtils::format("%s/Hornet_backjump_1.png", _folderPath.c_str());
    auto animation = Animation::create();
    animation->addSpriteFrameWithFile(frame0);
    animation->addSpriteFrameWithFile(frame1);
    animation->setDelayPerUnit(0.4f);
    animation->setRestoreOriginalFrame(false);
    auto animate = Animate::create(animation);

    // 物理跳跃
    auto jumpAction = JumpBy::create(0.8f, Vec2(jumpDir * 450.0f, 0), 240.0f, 1);

    auto seq = Sequence::create(
        Spawn::create(animate, jumpAction, nullptr),
        CallFunc::create([this, onComplete]() {
            _isActionLocked = false;
            _isFacingLocked = false;

            if (onComplete) {
                onComplete();
            }
            else {
                if (this->_player) {
                    this->playAttack2Animation(this->_player->getPosition());
                }
                else {
                    this->decideNextAction();
                }
            }
        }),
        nullptr
    );

    seq->setTag(10);
    this->runAction(seq);
}

void HornetBoss::onDamaged() {
    this->stopActionByTag(999);

    float currentScaleX = this->getScaleX();
    this->removeChildByTag(998);

    auto currentFrame = this->getSpriteFrame();
    if (currentFrame) {
        auto whiteMask = Sprite::createWithSpriteFrame(currentFrame);
        whiteMask->setAnchorPoint(Vec2::ZERO);
        whiteMask->setPosition(Vec2::ZERO);
        whiteMask->setColor(Color3B::WHITE);
        BlendFunc blend = { GL_SRC_ALPHA, GL_ONE };
        whiteMask->setBlendFunc(blend);
        whiteMask->setOpacity(0);
        whiteMask->setTag(998);
        this->addChild(whiteMask, 10);

        auto flashSeq = Sequence::create(
            FadeTo::create(0.05f, 200),
            FadeTo::create(0.05f, 0),
            RemoveSelf::create(),
            nullptr
        );
        whiteMask->runAction(flashSeq);
    }

    this->blinkWhite();

    float absScaleX = fabsf(currentScaleX);
    float signX = (currentScaleX >= 0) ? 1.0f : -1.0f;
    auto scaleUp = ScaleTo::create(0.03f, signX * absScaleX * 1.1f, 1.1f);
    auto scaleReset = ScaleTo::create(0.05f, currentScaleX, 1.0f);

    auto seq = Sequence::create(
        scaleUp,
        DelayTime::create(0.04f),
        scaleReset,
        nullptr
    );
    seq->setTag(999);
    this->runAction(seq);

    // 击败状态：仅计次以便离场
    if (_aiState == AIState::DEFEATED) {
        _defeatedHitCount++;
        if (_defeatedHitCount >= 3) {
            Vec2 sourcePos = _player ? _player->getPosition() : Vec2::ZERO;
            this->playFallAnimation(sourcePos);
        }
        return;
    }

    // 受击硬直状态不计入攻击次数
    if (_aiState == AIState::STUNNED) {
        return;
    }

    // 正常战斗阶段受击逻辑
    _hitCount++;
    _phaseHitCount++;

    if (_phaseHitCount >= 10) {
        _phaseHitCount = 0;
        if (_currentPhase < 4) {
            _currentPhase++;
            this->playInjuredAction1();
        }
        else {
            this->playInjuredAction2Loop();
        }
    }
}
void HornetBoss::blinkWhite() {
    // 停止之前的闪烁动作
    this->stopActionByTag(997);

    // 使用透明度快速闪烁模拟受击反馈
    auto blink = Sequence::create(
        FadeTo::create(0.03f, 0),   
        FadeTo::create(0.03f, 255),   // 恢复
        FadeTo::create(0.03f, 0),   
        FadeTo::create(0.03f, 255),   // 恢复正常
        nullptr
    );
    blink->setTag(997);
    this->runAction(blink);
}
Rect HornetBoss::getBossHitRect() {
    // 使用 RectApplyTransform 获取更精确的世界坐标矩形，支持翻转和旋转
    Size size = this->getContentSize();
    Rect rect = Rect(0, 0, size.width, size.height);
    return RectApplyTransform(rect, this->getNodeToWorldTransform());
}

Rect HornetBoss::getWeaponRect() {
    // 遍历子节点查找武器精灵（Hornet_weapon1.png，不包括丝线动画帧）
    for (auto child : this->getChildren()) {
        auto sprite = dynamic_cast<Sprite*>(child);
        if (sprite) {
            std::string resourceName = sprite->getResourceName();
            // 只匹配 Hornet_weapon1.png（武器本体），排除丝线帧（Hornet_weapon1_1到6是丝线）
            if (resourceName.find("Hornet_weapon1.png") != std::string::npos) {
                Size size = sprite->getContentSize();
                Rect rect = Rect(0, 0, size.width, size.height);
                return RectApplyTransform(rect, sprite->getNodeToWorldTransform());
            }
        }
    }
    return Rect(0, 0, 0, 0);
}

Rect HornetBoss::getAttack4Rect() {
    // Attack4（乱舞攻击）使用 Hornet_weapon2 系列
    if (_attack4Effect) {
        Size size = _attack4Effect->getContentSize();
        Rect rect = Rect(0, 0, size.width, size.height);
        return RectApplyTransform(rect, _attack4Effect->getNodeToWorldTransform());
    }
    return Rect(0, 0, 0, 0);
}