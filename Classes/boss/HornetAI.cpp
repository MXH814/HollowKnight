// HornetAI.cpp
#include "HornetBoss.h"
USING_NS_CC;

void HornetBoss::startAI(Node* playerTarget) {
    // TODO: 参数类型应改为 Player*，并保存到 _player 成员变量中
    this->_player = playerTarget;

    this->scheduleUpdate();
    float midX = (_minX + _maxX) / 2.0f;
    _isActionLocked = true;
    this->playEntryAnimation(midX, _groundY + 800);
    
}
void HornetBoss::decideNextAction() {
    if (_isActionLocked || !_player) return;
        
    // 如果处于击败状态，不再进行任何AI决策
    if (_aiState == AIState::DEFEATED) return;

    // 获取当前距离
    // TODO: 未来替换为 Player::getPositionX()
    float distance = std::abs(this->getPositionX() - _player->getPositionX());
    int randVal = cocos2d::random(1, 100);

    // --- 强制间隔逻辑 ---
    // 如果上一个状态是 ATTACKING，则这次必须强制进入 IDLE 或 WALKING 
    if (_aiState == AIState::ATTACKING) {
        _aiState = AIState::IDLE;

        // 根据阶段调整休息时间
        float minRest, maxRest;
        switch (_currentPhase) {
            case 1: minRest = 0.8f; maxRest = 1.5f; break; // 阶段1：最慢
            case 2: minRest = 0.5f; maxRest = 1.2f; break; // 阶段2：原速度
            case 3: minRest = 0.3f; maxRest = 0.8f; break; // 阶段3：加快
            case 4: minRest = 0.1f; maxRest = 0.4f; break; // 阶段4：极快
            default: minRest = 0.5f; maxRest = 1.2f; break;
        }
            
        float restTime = cocos2d::random(minRest, maxRest);

        // 50% 概率原地待机，50% 概率走动一下
        if (cocos2d::random(0, 1) == 0) {
                this->playIdleAnimation();
                this->runAction(Sequence::create(
                    DelayTime::create(restTime),
                    CallFunc::create([this]() { this->decideNextAction(); }),
                    nullptr));
            }
            else {
                this->playWalkAnimation();
                this->runAction(Sequence::create(
                    DelayTime::create(restTime),
                    CallFunc::create([this]() {
                        this->_currentPhysicsUpdate = nullptr; // 停止走路物理
                        this->decideNextAction();
                        }), nullptr));
            }
            return; // 结束当前决策，等待 DelayTime 后再次进入
        }

    // --- 正式攻击决策逻辑 ---
    _aiState = AIState::ATTACKING;
    int currentAttack = 0;

    if (distance < 250.0f) {
        // 近距离：删除单独后撤步，提高攻击1概率
        if (randVal <= 95) {
            // 95%概率使用攻击1
            currentAttack = 1;
        }
        else {
            // 5%概率使用攻击4（乱舞）
            currentAttack = 4;
        }
    }
    else {
        // 远距离：提高攻击2(投掷武器)概率，降低攻击4概率到5%
        if (randVal <= 20) {
            // 20%概率使用攻击3（俯冲）
            currentAttack = 3;
        }
        else if (randVal <= 80) {
            // 60%概率使用攻击2（投掷武器）- 大幅提高
            currentAttack = 2;
        }
        else if (randVal <= 85) {
            // 5%概率使用攻击4（乱舞）- 极低概率，10次出现0-1次
            currentAttack = 4;
        }
        else {
            // 15%概率走路
            this->playWalkAnimation();
            _lastAttackType = 0;
            // 走路后的回调
            this->runAction(Sequence::create(DelayTime::create(1.0f), CallFunc::create([this]() {
                this->_currentPhysicsUpdate = nullptr;
                this->decideNextAction();
                }), nullptr));
            return;
        }
    }

    // --- 攻击去重检查 ---
    if (currentAttack != 0) {
        // 如果和上次一样
        if (currentAttack == _lastAttackType) {
            // 特殊处理攻击3：允许连续两次
            if (currentAttack == 3 && _attack3RepeatCount < 1) {
                _attack3RepeatCount++;
            }
            else {
                // 其他攻击或攻击3已重复过：强制切换到另一种动作（例如：如果不让重复，就改成走路）
                this->playWalkAnimation();
                this->runAction(Sequence::create(DelayTime::create(0.8f), CallFunc::create([this]() {
                    this->_currentPhysicsUpdate = nullptr;
                    this->decideNextAction();
                    }), nullptr));
                return;
            }
        }
        else {
            _attack3RepeatCount = 0; // 重置攻击3计数
        }

        _lastAttackType = currentAttack;

        // 执行最终选定的攻击
        switch (currentAttack) {
            case 1: 
            {
                // 修改：Attack 1 增加与 Attack 2 相同的安全检查逻辑
                // 如果前方空间不足以施展冲刺（假设冲刺距离较长），则先调整位置
                float currentX = this->getPositionX();
                // TODO: 未来替换为 Player::getPositionX()
                float direction = (this->_player->getPositionX() > currentX) ? 1.0f : -1.0f;
                // 检查前方 550 像素（Attack1 冲刺距离）是否有空间，预留 100 缓冲
                if (currentX + (direction * 650.0f) < _minX || currentX + (direction * 650.0f) > _maxX) {
                    // 空间不足，先移动到安全位置，完成后回调执行 Attack 1
                    this->moveToSafetyAndAttack([this]() {
                        // TODO: 未来替换为 Player::getPosition()
                        this->playAttack1Animation(this->_player->getPosition());
                    });
                }
                else {
                    // TODO: 未来替换为 Player::getPosition()
                    this->playAttack1Animation(_player->getPosition());
                }
            }
            break;
            case 2:
            {
                float currentX = this->getPositionX();
                // TODO: 未来替换为 Player::getPositionX()
                float direction = (this->_player->getPositionX() > currentX) ? 1.0f : -1.0f;
                // Attack 2 投掷距离约 700，预留 250 缓冲
                if (currentX + (direction * 750.0f) < _minX + 250.0f || currentX + (direction * 750.0f) > _maxX - 250.0f) {
                    // 空间不足，先移动到安全位置，完成后回调执行 Attack 2
                    this->moveToSafetyAndAttack([this]() {
                        // TODO: 未来替换为 Player::getPosition()
                        this->playAttack2Animation(this->_player->getPosition());
                    });
                }
                else {
                    // TODO: 未来替换为 Player::getPosition()
                    this->playAttack2Animation(_player->getPosition());
                }
            }
            break;
            // TODO: 未来替换为 Player::getPosition()
            case 3: this->playAttack3Animation(_player->getPosition()); break;
            // TODO: 未来替换为 Player::getPosition()
            case 4: this->playAttack4Animation(_player->getPosition()); break;
        }
    }
}