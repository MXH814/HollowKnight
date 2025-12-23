#include "HornetBoss.h"
USING_NS_CC;

void HornetBoss::startBehaviorLogic1() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 停止之前可能存在的逻辑动作
    this->stopActionByTag(20);

    // 1. 立即执行登场动画 (下落 + 落地)
    this->playEntryAnimation(visibleSize.width / 2, visibleSize.height + 200);

    // 2. 逻辑时间轴
    // entryDuration: 0.4s(下落) + 0.06s*6(落地帧) = 约0.8s
    auto entryDuration = DelayTime::create(0.8f);
    auto waitFiveSecs = DelayTime::create(5.0f);

    auto mainSequence = Sequence::create(
        entryDuration,
        CallFunc::create([this]() {
            CCLOG("Logic: Switching to Idle");
            this->playIdleAnimation();
            }),
        waitFiveSecs,
        CallFunc::create([this]() {
            CCLOG("Logic: Switching to Walk");
            this->playWalkAnimation();
            }),
        nullptr
    );

    mainSequence->setTag(20); // 逻辑专用 Tag
    this->runAction(mainSequence);
}
void HornetBoss::startBehaviorLogic2() {
    // 停止所有之前的逻辑
    this->stopActionByTag(20);

    // 初始位置直接设为地面中心
    auto visibleSize = Director::getInstance()->getVisibleSize();
    this->setPosition(Vec2(visibleSize.width / 2, _groundY));
    //this->playIdleAnimation();

    // 演示逻辑：等待 1 秒后，执行跳跃（跳向当前鼠标位置）
    auto delay = DelayTime::create(0.0f);
    auto doJump = CallFunc::create([this]() {
        CCLOG("BehaviorLogic2: Execution Jump");
        // 传入当前鼠标记录的坐标
        this->playJumpAnimation(_targetPos);
        });

    // 如果你想测试连续跳跃，可以用 RepeatForever
    auto seq = Sequence::create(delay, doJump, DelayTime::create(2.0f), nullptr);
    auto repeat = RepeatForever::create(seq);

    repeat->setTag(20);
    this->runAction(repeat);
}
void HornetBoss::startBehaviorLogic3() {
    // 1. 清理之前的逻辑动作，防止多重计时器冲突
    this->stopActionByTag(20);

    // 2. 初始化状态
    auto visibleSize = Director::getInstance()->getVisibleSize();
    // 如果是第一次启动，可以设定一个初始位置
    if (this->getPositionY() <= 0) {
        this->setPosition(Vec2(visibleSize.width / 2, _groundY));
    }
    this->playIdleAnimation();

    // 3. 定义循环行为：等待 -> 锁定并攻击 -> 恢复
    auto decideAndAttack = CallFunc::create([this]() {
        if (!_isActionLocked) {
            // 直接调用你刚修改好的带寻敌功能的 Attack1
            // 此时 _targetPos 已经在鼠标监听中被实时更新了
            this->playAttack1Animation(_targetPos);
        }
        });

    // 4. 创建一个更紧凑的循环，方便测试
    // 流程：等待 2 秒(观察寻敌转向) -> 发动攻击 -> 等待攻击完成(ActionLocked) -> 循环
    auto seq = Sequence::create(
        DelayTime::create(2.5f),
        decideAndAttack,
        nullptr
    );

    auto repeat = RepeatForever::create(seq);
    repeat->setTag(20);
    this->runAction(repeat);
}
void HornetBoss::startBehaviorLogic4() {
    this->stopActionByTag(20);

    // 出生在地面靠左
    float desiredX = _minX + 150.0f;
    float safeX = std::max(_minX, std::min(desiredX, _maxX));
    this->setPosition(Vec2(safeX, _groundY));

    _isActionLocked = false;
    this->playIdleAnimation();

    auto doAttack = CallFunc::create([this]() {
        if (!_isActionLocked) {
            // 使用当前记录的 _targetPos 启动攻击
            this->playAttack2Animation(_targetPos);
        }
        });

    // 循环：每 3.5 秒攻击一次
    auto seq = Sequence::create(
        DelayTime::create(1.0f),
        doAttack,
        DelayTime::create(3.5f),
        nullptr
    );

    auto repeat = RepeatForever::create(seq);
    repeat->setTag(20);
    this->runAction(repeat);
}
void HornetBoss::startBehaviorLogic5() {
    this->stopActionByTag(20);

    // --- 修复出生点超出左边框的问题 ---
    // 确保 X 坐标至少为 _minX + 偏移量，且不超过 _maxX
    float startX = _minX + 200.0f;
    float safeStartX = std::max(_minX, std::min(startX, _maxX));

    this->setPosition(Vec2(safeStartX, _groundY));
    // ---------------------------------

    _isActionLocked = false;
    this->playIdleAnimation();

    auto doAttack = CallFunc::create([this]() {
        if (!_isActionLocked) {
            this->playAttack3Animation(_targetPos);
        }
        });

    auto seq = Sequence::create(DelayTime::create(1.0f), doAttack, DelayTime::create(4.0f), nullptr);
    auto repeat = RepeatForever::create(seq);
    repeat->setTag(20);
    this->runAction(repeat);
}
void HornetBoss::startBehaviorLogic6() {
    this->stopActionByTag(20);

    // 初始化位置
    this->setPosition(Vec2(_minX + (_maxX - _minX) * 0.5f, _groundY));
    _isActionLocked = false;
    this->playIdleAnimation();

    auto doAttack = CallFunc::create([this]() {
        if (!_isActionLocked) {
            // _targetPos 应该是你在场景 update 中实时更新的成员变量
            this->playAttack4Animation(_targetPos);
        }
        });

    auto seq = Sequence::create(DelayTime::create(1.0f), doAttack, DelayTime::create(5.0f), nullptr);
    auto repeat = RepeatForever::create(seq);
    repeat->setTag(20);
    this->runAction(repeat);
}
void HornetBoss::startBehaviorLogic7() {
    this->stopActionByTag(20); // 停止逻辑循环
    this->stopActionByTag(10); // 停止动作执行

    // 重置位置到地面中心
    this->setPosition(Vec2(_minX + (_maxX - _minX) * 0.5f, _groundY));
    _isActionLocked = false;
    this->playIdleAnimation();

    auto testInjured = CallFunc::create([this]() {
        // 随机选择受击 1 或 2
        if (cocos2d::random(0, 1) == 0) {
            CCLOG("Testing Injured Action 1 (2s)");
            this->playInjuredAction1();
        }
        else {
            CCLOG("Testing Injured Action 2 (3s)");
            this->playInjuredAction2();
        }
        });

    // 逻辑循环：等待 1 秒 -> 受击硬直 -> 等待一段时间 -> 再次受击
    auto seq = Sequence::create(
        DelayTime::create(1.0f),
        testInjured,
        DelayTime::create(5.0f), // 给足够的时间观察恢复到 Idle 的状态
        nullptr
    );

    auto repeat = RepeatForever::create(seq);
    repeat->setTag(20);
    this->runAction(repeat);
}
void HornetBoss::startBehaviorLogic8() {
    this->stopActionByTag(20);

    // 初始化位置：地面中心
    this->setPosition(Vec2(_minX + (_maxX - _minX) * 0.5f, _groundY));
    _isActionLocked = false;
    this->playIdleAnimation();

    // 记录测试状态，用来交替改变攻击方向
    static bool testLeft = true;

    auto doTest = CallFunc::create([this]() {
        if (!_isActionLocked) {
            // 构造一个模拟的攻击源位置
            float offsetX = testLeft ? -100.0f : 100.0f;
            Vec2 fakeAttackPos = Vec2(this->getPositionX() + offsetX, _groundY);

            CCLOG("Testing Fall: Attack from %s", testLeft ? "LEFT" : "RIGHT");
            this->playFallAnimation(fakeAttackPos);

            testLeft = !testLeft; // 下次反向测试
        }
        });

    auto seq = Sequence::create(
        DelayTime::create(1.0f),
        doTest,
        DelayTime::create(3.0f),
        nullptr
    );

    auto repeat = RepeatForever::create(seq);
    repeat->setTag(20);
    this->runAction(repeat);
}
void HornetBoss::startBehaviorLogic9() {
    this->stopActionByTag(20);

    // 放在左侧地面
    this->setPosition(Vec2(_minX + 150.0f, _groundY));
    this->setScaleX(1.0f); // 初始面向左
    _isActionLocked = false;
    this->playIdleAnimation();

    auto doLeave = CallFunc::create([this]() {
        CCLOG("Hornet is leaving the stage...");
        this->playLeaveAnimation();
        });

    this->runAction(Sequence::create(DelayTime::create(2.0f), doLeave, nullptr));
}