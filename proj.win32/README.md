HollowKnight（空洞骑士复刻）
本项目基于 cocos2d-x-3.17.2，使用 C++14 实现《Hollow Knight》核心玩法的复刻 Demo：横版动作、怪物/Boss 战斗、场景探索与切换、UI（血量/灵魂/暂停/地图）以及死亡重开与 Shade（影子）机制。

团队3人：马祥珲、胡知鱼、张茗博

1. 项目概述
题目：空洞骑士（Hollow Knight）
类型：类银河恶魔城 2D 横版动作冒险 + Boss 战斗 + 地下王国探索
引擎：cocos2d-x-3.17.2
语言标准：C++14
平台：Windows（proj.win32）

1.1 游戏描述
玩家操控主角 The Knight 在遗忘十字路等区域移动与探索，完成跳跃/冲刺/攻击/施法，与多种虫类怪物及 Boss 进行战斗，并支持地图显示、护符系统、Geo 收集与死亡重开等。

2. 已实现功能
2.1 角色系统（The Knight）
主角实体：TheKnight（继承 cocos2d::Sprite）
支持动作/能力：
  行走 / 奔跑
  跳跃 / 落地（重落地屏幕震动反馈）
  冲刺
  基础攻击（Slash 判定）
  法术技能（示例：Vengeful Spirit 施法特效 + 多段伤害）
基础状态切换：Idle / 攻击 / 受击 / 死亡 / SpikeDeath / HazardRespawn

2.2 战斗系统
玩家生命值（面具系统）（聚集或椅子恢复）、灵魂槽（攻击敌人获取）
敌人生命值（怪物持有 _health）
受击硬直与击退
战斗判定：
  近战：getSlashEffectBoundingBox() 与敌人 getBoundingBox() 相交
  法术：使用法术特效 Sprite 的矩形进行判定
  使用 update() 主动检测，避免遍历 children 时删除导致崩溃（先收集列表再处理）
Boss 血条：BossScene 内实现

2.3 敌人与 Boss
普通怪物（4 种）：
  CrawlidMonster
  TiktikMonster
  GruzzerMonster
  VengeflyMonster
怪物 AI（基础追击/移动/触碰伤害，部分支持眩晕 _isStunned）
击杀获取货币
Boss（1 个）：
  HornetBoss（含 HornetAI）
  支持多阶段/狂暴的扩展设计（由 Boss AI 控制状态机）

2.4 场景功能
横版 2D 场景移动（TMX 地图拼接）
场景内容：战斗区、跳台、危险区（荆棘）、切换点（Exit）
区域背景音乐：例如 Crossroads BGM

2.5 UI 功能
标题菜单：MainMenuScene
玩家 HUD：血量（面具）、灵魂槽（动画）、Geo 数量
暂停/继续：PauseMenu（ESC）
地图显示：TAB 按住显示、松开隐藏
死亡重开：黑屏淡入后切场景重生

2.6 扩展
护符系统：CharmManager（影响回魂、法术增强等）
屏幕震动：shakeScreen(duration, intensity)
Shade（影子）机制：死亡点保存与重生后生成，回收后重置
Geo 交互：攻击地图 Geo对象获取货币
NPC 交互：对话及购买地图

3. 操作说明（键位）
见菜单键位说明，下举几例：
ASDW：移动
J：攻击
K：跳跃
L：冲刺
ESC：打开/关闭暂停菜单
TAB：显示地图（按下显示，松开隐藏）
W：靠近出口时触发场景切换、NPC交互
Q：打开/关闭护符面板


4. 核心实现举例

4.1 安全的战斗碰撞检测（避免遍历 children 中途删除）
文件：Classes/NextScene.cpp
// 先收集，再处理（避免在遍历 children 的过程中 removeFromParent 导致崩溃）
std::vector<CrawlidMonster*> crawlids;
for (auto child : this->getChildren()) {
    auto crawlid = dynamic_cast<CrawlidMonster*>(child);
    if (crawlid && crawlid->_health > 0) {
        crawlids.push_back(crawlid);
    }
}


4.2 死亡动画计时结束后统一触发死亡回调（确保动画完整播放）
文件：Classes/NextScene.cpp
static bool isPlayingDeathAnim = false;
static float deathAnimTimer = 0.0f;
static Vec2 savedDeathPos = Vec2::ZERO;

if (knight->isDead() && !_isInSpikeDeath) {
    if (!isPlayingDeathAnim) {
        isPlayingDeathAnim = true;
        deathAnimTimer = 0.0f;
        savedDeathPos = knightPos;
        s_shadePosition = knightPos; // 保存 Shade 出生点
    }

    deathAnimTimer += dt;
    if (deathAnimTimer >= 1.3f) {
        isPlayingDeathAnim = false;
        deathAnimTimer = 0.0f;
        onKnightDeath(savedDeathPos);
        return;
    }
}

4.3 Shade 生成与回收
文件：Classes/NextScene.cpp
// 重生时延迟一帧生成 Shade，确保 _player 已设置
this->scheduleOnce([this](float) {
    if (s_shadePosition != Vec2::ZERO) {
        this->spawnShade(s_shadePosition);
    }
    s_isRespawning = false;
}, 0.1f, "respawn_delay");
 
4.4 TheKnight：攻击特效与命中判定（Slash HitBox）
设计：攻击不直接用角色碰撞体，而是生成“斩击特效精灵”，由特效的包围盒作为攻击判定区域。
好处：
  判定形状与动画更贴合（可根据 UpSlash/DownSlash 选择不同资源与偏移）
  场景侧只需要取 Rect 做 intersectsRect，实现简单且稳定

文件：Classes/TheKnightCombat.cpp
bool TheKnight::getSlashEffectBoundingBox(Rect& outRect) const
{
    if (!_slashEffect || !_isAttacking)
    {
        return false;
    }

    auto effectSize = _slashEffect->getContentSize();
    auto effectPos = _slashEffect->getPosition();

    outRect = Rect(effectPos.x effectSize.width / 2,
                   effectPos.y effectSize.height / 2,
                   effectSize.width,
                   effectSize.height);
    return true;
}

 4.5 TheKnight：受击/无敌帧与“取消动作”（战斗手感）
设计：takeDamage() 内集中处理“死亡判定、受击无敌帧、取消攻击/冲刺/Focus、退出地图模式”等。
要点：
  _isInvincible + _invincibleTimer 负责无敌帧
  StalwartShell 护符可延长无敌时间

文件：Classes/TheKnightCombat.cpp
void TheKnight::takeDamage(int damage)
{
    // 如果已经死亡或处于无敌状态，不受伤
    if (_state == KnightState::DEAD || _isInvincible)
    {
        return;
    }

    _hp -= damage;
    if (_hp < 0) _hp = 0;

    // 如果处于地图模式，强制退出
    if (_isMapMode)
    {
        exitMapMode();
    }

    // 清理攻击特效
    _isAttacking = false;
    removeSlashEffect();

    // 清理冲刺特效
    if (_dashEffect)
    {
        _dashEffect->removeFromParent();
        _dashEffect = nullptr;
    }

    // 生命值归零，触发死亡
    if (_hp <= 0)
    {
        startDeath();
    }
    else
    {
        _isInvincible = true;
        // 计算无敌时长（考虑StalwartShell护符）
        _invincibleTimer = _invincibleDuration;
        if (_charmStalwartShell)
        {
            _invincibleTimer += 0.4f;  // 增加0.4秒无敌时间
        }
        changeState(KnightState::GET_ATTACKED);
    }
}


 4.6 TheKnight：跳跃/下落（含重落地判定）
设计：跳跃与下落采用显式速度积分，并通过 checkGroundCollision/checkWallCollision 实现平台碰撞。
重落地：记录 _fallStartY 与落地高度差决定 LANDING/HARD_LANDING，用于触发震屏等反馈。

文件：Classes/TheKnightMovement.cpp
float fallDistance = _fallStartY groundY;
if (fallDistance >= _hardLandThreshold)
{
    changeState(KnightState::HARD_LANDING);
}
else
{
    changeState(KnightState::LANDING);
}


 4.7 Monster：VengeflyMonster 追击 AI（PATROL/CHASE 状态机）
设计：飞行怪物具备 PATROL/CHASE/STUN/DEAD 状态。
追击触发：玩家进入 _chaseRadius 后追击；超出一定距离后回到巡逻。
文件：Classes/Monster/VengeflyMonster.cpp
void VengeflyMonster::setPlayerPosition(const Vec2& playerPos)
{
    _playerPosition = playerPos;

    if (_currentState == VengeflyState::DEAD || _currentState == VengeflyState::STUN)
    {
        return;
    }

    Vec2 distanceVec = playerPos this->getPosition();
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
            startPatrol();
        }
    }
}


 4.8 Monster：GruzzerMonster Z 字飞行 + 墙体反弹
设计：Gruzzer 初速度为对角方向飞行，并对“平台碰撞/边界”做反弹，形成 Z 字轨迹。
文件：Classes/Monster/GruzzerMonster.cpp
Rect futureBox(
    nextPos.x getContentSize().width / 2,
    currentPos.y getContentSize().height / 2,
    getContentSize().width,
    getContentSize().height
);

for (const auto& platform : _platforms) {
    if (futureBox.intersectsRect(platform.rect)) {
        _velocity.x *= -1;
        hitWall = true;
        break;
    }
}


 4.9 Boss：HornetAI 决策（概率选择 + 攻击去重 + 强制硬直）
设计：decideNextAction() 统一选招。
关键点：
  攻击后强制进入 IDLE/WALK 休息窗口（随阶段加速）
  针对重复招式做去重（Attack3 允许最多连续两次）

文件：Classes/boss/HornetAI.cpp
// 攻击去重：Attack3最多允许连续两次，其余重复则强制走路过渡
if (currentAttack == _lastAttackType) {
    if (currentAttack == 3 && _attack3RepeatCount < 1) {
        _attack3RepeatCount++;
    }
    else {
        this->playWalkAnimation();
        this->runAction(Sequence::create(
            DelayTime::create(0.8f),
            CallFunc::create([this]() {
                this->_currentPhysicsUpdate = nullptr;
                this->decideNextAction();
            }),
            nullptr));
        return;
    }
}


 4.10 BossScene：Boss/玩家双向伤害判定（含武器/技能判定）
Hornet → Knight：检测 Boss 本体矩形 / 投掷武器矩形 / 乱舞范围矩形。
Knight → Hornet：复用 Slash 特效包围盒与法术特效包围盒，并用冷却 _knightAttackCooldown/_spellAttackCooldown 防止多次结算。

文件：Classes/BossScene.cpp
Rect bossHurtRect = _hornet->getBossHitRect();

if (_knightAttackCooldown <= 0)
{
    Rect slashRect;
    if (_knight->getSlashEffectBoundingBox(slashRect))
    {
        if (slashRect.intersectsRect(bossHurtRect))
        {
            _hornet->onDamaged();
            _knightAttackCooldown = 0.3f;
        }
    }
}





5. 架构与目录结构
> 目录尽量保持“模块归类清晰”，便于 3 人协作和代码审查。
Classes/
  TheKnight*.cpp/.h：主角移动/战斗/动画/灵魂等逻辑
  Monster/：普通怪物、刷怪器 MonsterSpawner
  boss/：Boss（HornetBoss / HornetAI 等）
  GameScene / NextScene / BossScene / MainMenuScene：场景层
  CharmManager / GeoManager：管理器模块
  GameCommon.h：模板工具、异常封装、通用函数



6. 版本控制与协作（GitHub 使用规范）
6.1 分支策略（推荐）
main：可演示、稳定版本
dev：日常集成分支（可选）
feature/<module>-<desc>：功能分支，例如：
  feature/shade-respawn
  feature/boss-hornet-phase
  feature/ui-map

6.2 Commit 规范（记录清晰）

6.3 协作与分工（合理分工）

马祥珲：角色系统（移动/战斗/动画/状态机）
胡知鱼：敌人/Boss（AI、阶段、技能）
张茗博：场景/UI（地图/暂停/切换/资源）



7. 代码质量
7.1 单元测试
项目编写过程中始终进行单元测试

7.2 异常处理与健壮性
GameCommon.h 内含异常封装（GameException / ResourceLoadException）
关键资源加载失败使用 CCASSERT 快速失败，避免空指针继续运行

7.3 内存泄漏控制
cocos2d-x 节点使用引用计数；通过 addChild/removeFromParent() 管理生命周期
关键指针移除后置 nullptr（例如 _shade）



8. C++11/14 特性使用说明（验收项）

项目中已使用（详见 Classes/CppFeaturesSummary.md）：
STL 容器：std::vector, std::string
迭代器：对容器遍历（含 cbegin/cend 风格）
类与多态：继承、虚函数、dynamic_cast 用于运行时类型识别
模板：Clamp/Lerp/SafeCast（GameCommon.h）
异常处理：基于 std::runtime_error 的业务异常封装
Lambda/捕获：scheduleOnce([this](float){...})
现代语法：override, nullptr, = delete, 初始化列表等
函数 & 操作符重载
// 构造函数重载
GameException(const std::string& message);
GameException(const std::string& message, const std::string& file, int line);

// Platform 结构体构造函数重载
struct Platform {
    Platform() : node(nullptr) {}
    Platform(const Rect& r, Node* n) : rect(r), node(n) {}
};

// 删除的操作符（禁止赋值）
CharmManager& operator=(const CharmManager&) = delete;



9. 构建与运行（Windows / proj.win32）

1. 使用 Visual Studio 打开 Windows 工程（proj.win32）
2. 选择 Debug/Release + Win32
3. 构建并运行

10. 声明
本项目为课程/学习性质复刻 Demo，仅用于教学展示与技术验证。资源与设定归原作所有方。
