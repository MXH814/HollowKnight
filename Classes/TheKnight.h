/**
 * @file TheKnight.h
 * @brief 小骑士（TheKnight）主角类头文件
 * @details 实现了空洞骑士游戏中主角的所有行为，包括移动、跳跃、攻击、
 *          法术释放、护符系统等。
 * 
 * @author HollowKnight Team
 * @date 2024
 * 
 * C++ 特性使用说明：
 * - 类与多态：继承自 cocos2d::Sprite，重写 init() 和 update()
 * - STL 容器：std::vector 存储平台列表
 * - enum class：强类型枚举 KnightState
 * - const 成员函数：所有 getter 方法
 * - C++11：override 关键字、enum class、nullptr、auto
 */

#ifndef __THE_KNIGHT_H__
#define __THE_KNIGHT_H__

#include "cocos2d.h"
#include "GameCommon.h"
#include <vector>
#include <string>

USING_NS_CC;

/**
 * @enum KnightState
 * @brief 骑士动画状态枚举（强类型枚举 - C++11）
 * @details 使用 enum class 确保类型安全，避免隐式转换
 */
enum class KnightState {
    IDLE,           ///< 静止状态
    RUNNING,        ///< 跑动状态
    TURNING,        ///< 转向状态
    RUN_TO_IDLE,    ///< 从跑动到静止的过渡
    JUMPING,        ///< 跳跃上升状态
    FALLING,        ///< 下落状态
    LANDING,        ///< 落地状态
    HARD_LANDING,   ///< 重落地状态（高处落下）
    DASHING,        ///< 冲刺状态
    DASH_TO_IDLE,   ///< 冲刺结束状态
    LOOKING_UP,     ///< 向上看状态
    LOOK_UP_END,    ///< 向上看结束状态
    LOOKING_DOWN,   ///< 向下看状态
    LOOK_DOWN_END,  ///< 向下看结束状态
    WALL_SLIDING,   ///< 贴墙下滑状态
    WALL_SLASHING,  ///< 贴墙攻击状态
    WALL_JUMPING,   ///< 蹬墙跳状态
    DOUBLE_JUMPING, ///< 二段跳状态
    SLASHING,       ///< 水平攻击状态
    UP_SLASHING,    ///< 向上攻击状态
    DOWN_SLASHING,  ///< 向下攻击状态
    GET_ATTACKED,   ///< 受击状态（硬直）
    DEAD,           ///< 死亡状态
    SPIKE_DEATH,    ///< 尖刺死亡状态
    HAZARD_RESPAWN, ///< 危险区域重生状态
    CASTING_SPELL,  ///< 释放法术状态
    RECOVERING,     ///< 回复生命状态
    FOCUSING,       ///< Focus 聚气状态（长按空格）
    FOCUS_GET,      ///< Focus 回血状态
    FOCUS_END,      ///< Focus 结束状态（松开空格）
    MAP_OPENING,    ///< 打开地图状态
    MAP_IDLE,       ///< 地图模式静止状态
    MAP_WALKING,    ///< 地图模式行走状态
    MAP_TURNING,    ///< 地图模式转向状态
    MAP_CLOSING,    ///< 关闭地图状态
    // 椅子坐下相关状态
    SITTING,        ///< 坐下动作
    SIT_IDLE,       ///< 坐着静止
    SIT_FALL_ASLEEP,///< 坐着入睡
    SITTING_ASLEEP, ///< 坐着睡觉
    WAKE_TO_SIT,    ///< 醒来
    GET_OFF,        ///< 起身离开
    SIT_MAP_OPEN,   ///< 坐着打开地图
    SIT_MAP_CLOSE   ///< 坐着关闭地图
};

/**
 * @struct Platform
 * @brief 平台碰撞数据结构
 */
struct Platform {
    Rect rect;      ///< 平台的碰撞矩形
    Node* node;     ///< 平台节点指针
    
    Platform() : node(nullptr) {}
    Platform(const Rect& r, Node* n) : rect(r), node(n) {}
};

/**
 * @class TheKnight
 * @brief 游戏主角类
 * @details 继承自 cocos2d::Sprite，实现了完整的角色控制系统
 * 
 * 设计说明：
 * - 使用状态机模式管理角色状态
 * - 支持多种移动方式：行走、跳跃、二段跳、冲刺、贴墙
 * - 实现完整的战斗系统：普通攻击、方向攻击、法术释放
 * - 集成护符系统，可通过装备护符获得特殊能力
 */
class TheKnight : public Sprite {
public:
    // ========================================================================
    // 工厂方法与生命周期
    // ========================================================================
    
    /**
     * @brief 创建骑士实例（工厂方法）
     * @return 骑士指针，失败返回 nullptr
     */
    static TheKnight* create();
    
    /**
     * @brief 初始化骑士
     * @return 初始化是否成功
     */
    virtual bool init() override;
    
    /**
     * @brief 每帧更新回调
     * @param dt 距上一帧的时间间隔（秒）
     */
    void update(float dt) override;
    
    // ========================================================================
    // 碰撞检测接口
    // ========================================================================
    
    /**
     * @brief 设置平台列表（用于碰撞检测）
     * @param platforms 平台列表的常量引用
     */
    void setPlatforms(const std::vector<Platform>& platforms);
    
    /**
     * @brief 获取碰撞盒
     * @return 角色的碰撞矩形
     */
    Rect getBoundingBox() const;
    
    /**
     * @brief 获取攻击特效的碰撞盒
     * @param[out] outRect 输出的碰撞矩形
     * @return 是否存在攻击特效
     */
    bool getSlashEffectBoundingBox(Rect& outRect) const;
    
    // ========================================================================
    // 状态查询接口（所有 getter 使用 const 修饰）
    // ========================================================================
    
    /** @brief 是否正在向上看 */
    bool isLookingUp() const { return _state == KnightState::LOOKING_UP; }
    
    /** @brief 是否正在向下看 */
    bool isLookingDown() const { return _state == KnightState::LOOKING_DOWN; }
    
    /** @brief 是否处于无敌状态 */
    bool isInvincible() const { return _isInvincible; }
    
    /** @brief 是否处于硬直状态 */
    bool isStunned() const { return _state == KnightState::GET_ATTACKED; }
    
    /** @brief 是否已死亡 */
    bool isDead() const { return _state == KnightState::DEAD; }
    
    /** @brief 是否正在重落地 */
    bool isHardLanding() const { return _state == KnightState::HARD_LANDING; }
    
    /** @brief 是否处于尖刺死亡状态 */
    bool isSpikeDeathState() const { return _state == KnightState::SPIKE_DEATH; }
    
    /** @brief 是否处于危险区域重生状态 */
    bool isHazardRespawnState() const { return _state == KnightState::HAZARD_RESPAWN; }
    
    /** @brief 是否靠近椅子 */
    bool isNearChair() const { return _isNearChair; }
    
    /** @brief 是否正在坐着 */
    bool isSitting() const;
    
    // ========================================================================
    // 生命值与灵魂值接口
    // ========================================================================
    
    /** @brief 获取当前生命值 */
    int getHP() const { return _hp; }
    
    /** @brief 获取最大生命值 */
    int getMaxHP() const { return _maxHP; }
    
    /** 
     * @brief 设置生命值
     * @param hp 新的生命值（自动限制在最大值以内）
     */
    void setHP(int hp) { _hp = std::min(hp, _maxHP); }
    
    /** @brief 获取当前灵魂值 */
    int getSoul() const { return _soul; }
    
    /** @brief 获取最大灵魂值 */
    int getMaxSoul() const { return _maxSoul; }
    
    /**
     * @brief 设置灵魂值
     * @param soul 新的灵魂值（自动限制在 [0, maxSoul] 范围内）
     */
    void setSoul(int soul) { _soul = std::max(0, std::min(soul, _maxSoul)); }
    
    /**
     * @brief 增加灵魂值（攻击命中时调用）
     * @param amount 增加量
     */
    void addSoul(int amount);
    
    /**
     * @brief 受到伤害
     * @param damage 伤害值
     */
    void takeDamage(int damage);
    
    // ========================================================================
    // 护符系统接口
    // ========================================================================
    
    // Getters (const)
    int getCharmSoulCatcher() const { return _charmSoulCatcher; }
    int getCharmShamanStone() const { return _charmShamanStone; }
    int getCharmStalwartShell() const { return _charmStalwartShell; }
    int getCharmSprintmaster() const { return _charmSprintmaster; }
    int getCharmSteadyBody() const { return _charmSteadyBody; }
    
    // Setters
    void setCharmSoulCatcher(int value) { _charmSoulCatcher = value; }
    void setCharmShamanStone(int value) { _charmShamanStone = value; }
    void setCharmStalwartShell(int value) { _charmStalwartShell = value; }
    void setCharmSprintmaster(int value) { _charmSprintmaster = value; }
    void setCharmSteadyBody(int value) { _charmSteadyBody = value; }
    
    // ========================================================================
    // 特殊动作接口
    // ========================================================================
    
    /**
     * @brief 设置受击后退方向
     * @param fromRight true 表示从右侧受击，向左后退
     */
    void setKnockbackDirection(bool fromRight);
    
    /** @brief 下劈弹反（下劈命中敌人后弹起） */
    void bounceFromDownSlash();
    
    /**
     * @brief 从外部触发跳跃（用于场景切换时的跳跃效果）
     * @param horizontalSpeed 水平方向速度
     */
    void triggerJumpFromExternal(float horizontalSpeed);
    
    /** @brief 开始尖刺死亡动画 */
    void startSpikeDeath();
    
    /**
     * @brief 开始危险区域重生
     * @param respawnPos 重生位置
     */
    void startHazardRespawn(const Vec2& respawnPos);
    
    /** @brief 设置最后安全位置 */
    void setLastSafePosition(const Vec2& pos) { _lastSafePosition = pos; }
    
    /** @brief 获取最后安全位置 */
    Vec2 getLastSafePosition() const { return _lastSafePosition; }
    
    /** @brief 设置是否靠近椅子 */
    void setNearChair(bool isNear) { _isNearChair = isNear; }
    
    /** @brief 开始坐下 */
    void startSitting();
    
    /** @brief 获取法术特效（用于碰撞检测） */
    Sprite* getVengefulSpiritEffect() const { return _vengefulSpiritEffect; }

private:
    // ========================================================================
    // 动画相关私有方法
    // ========================================================================
    
    /**
     * @brief 创建动画
     * @param path 资源路径
     * @param prefix 文件名前缀
     * @param startFrame 起始帧
     * @param endFrame 结束帧
     * @param delay 帧间隔（秒）
     * @return 动画指针，失败返回 nullptr
     */
    Animation* createAnimation(
        const std::string& path,
        const std::string& prefix,
        int startFrame,
        int endFrame,
        float delay);
    
    /** @brief 加载所有动画资源 */
    void loadAnimations();
    
    /**
     * @brief 播放动画
     * @param animName 动画名称
     * @param loop 是否循环播放
     */
    void playAnimation(const std::string& animName, bool loop = true);
    
    // ========================================================================
    // 输入处理
    // ========================================================================
    
    void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
    void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
    
    // ========================================================================
    // 状态机
    // ========================================================================
    
    /**
     * @brief 切换状态
     * @param newState 新状态
     */
    void changeState(KnightState newState);
    
    // 状态转换回调
    void onTurnFinished();
    void onRunToIdleFinished();
    void onRunStartFinished();
    void onLandFinished();
    void onHardLandFinished();
    void onFallAnimFinished();
    void onDashFinished();
    void onDashToIdleFinished();
    void onLookUpAnimFinished();
    void onLookUpEndFinished();
    void onLookDownAnimFinished();
    void onLookDownEndFinished();
    void onWallJumpAnimFinished();
    void onMapOpenFinished();
    void onMapTurnFinished();
    void onMapCloseFinished();
    void exitMapMode();
    
    // 椅子相关回调
    void onSitAnimFinished();
    void onSitFallAsleepFinished();
    void onWakeToSitFinished();
    void onGetOffFinished();
    void onSitMapOpenFinished();
    void onSitMapCloseFinished();
    void exitSitting(bool pressedLeft);
    void updateSitting(float dt);
    
    // ========================================================================
    // 移动系统
    // ========================================================================
    
    void startJump();
    void updateJump(float dt);
    void updateFall(float dt);
    bool checkGroundCollision(float& groundY);
    bool checkCeilingCollision(float& ceilingY);
    bool checkWallCollision(float& newX, bool movingRight);
    bool checkStillOnGround();
    
    // 二段跳
    void startDoubleJump();
    void updateDoubleJump(float dt);
    void onDoubleJumpAnimFinished();
    
    // 贴墙
    bool checkWallSlideCollision(bool checkRight);
    void startWallSlide(bool wallOnRight);
    void updateWallSlide(float dt);
    void startWallJump();
    void updateWallJump(float dt);
    
    // 冲刺
    void startDash();
    void updateDash(float dt);
    void createDashEffect();
    void updateDashEffect(float dt);
    
    // ========================================================================
    // 战斗系统
    // ========================================================================
    
    void startSlash();
    void startUpSlash();
    void startDownSlash();
    void onSlashAnimFinished();
    void onUpSlashAnimFinished();
    void onDownSlashAnimFinished();
    void updateSlash(float dt);
    void updateUpSlash(float dt);
    void updateDownSlash(float dt);
    void createSlashEffect(int effectNum);
    void updateSlashEffectPosition();
    void removeSlashEffect();
    
    // 贴墙攻击
    void startWallSlash();
    void updateWallSlash(float dt);
    void onWallSlashAnimFinished();
    void createWallSlashEffect(int effectNum);
    void updateWallSlashEffectPosition();
    
    // 蹬墙跳特效
    void createWallJumpPuffEffect();
    void updateWallJumpPuffEffect(float dt);
    void removeWallJumpPuffEffect();
    
    // 受击与死亡
    void updateGetAttacked(float dt);
    void onGetAttackedFinished();
    void startDeath();
    void onDeathAnimFinished();
    void onSpikeDeathAnimFinished();
    void onHazardRespawnAnimFinished();
    
    // ========================================================================
    // 灵魂与法术系统
    // ========================================================================
    
    bool useSoul(int amount);
    void startCastSpell();
    void updateCastSpell(float dt);
    void onCastSpellAnimFinished();
    void createVengefulSpiritEffect();
    void updateVengefulSpiritEffect(float dt);
    void removeVengefulSpiritEffect();
    void startRecover();
    void updateRecover(float dt);
    void onRecoverAnimFinished();
    void cancelRecover();
    
    // Focus 系统
    void startFocus();
    void updateFocus(float dt);
    void onFocusAnimFinished();
    void onFocusGetAnimFinished();
    void onFocusEndAnimFinished();
    void cancelFocus();

private:
    // ========================================================================
    // 状态相关成员变量
    // ========================================================================
    
    KnightState _state;          ///< 当前状态
    bool _facingRight;           ///< 是否面向右边
    bool _isMovingLeft;          ///< 是否正在向左移动
    bool _isMovingRight;         ///< 是否正在向右移动
    float _moveSpeed;            ///< 移动速度
    bool _isRunStartFinished;    ///< 跑步起步动画是否完成
    
    // ========================================================================
    // 生命值相关
    // ========================================================================
    
    int _hp;                     ///< 当前生命值
    int _maxHP;                  ///< 最大生命值
    
    // ========================================================================
    // 受击相关
    // ========================================================================
    
    bool _isInvincible;          ///< 是否无敌
    float _invincibleTimer;      ///< 无敌时间计时器
    float _invincibleDuration;   ///< 无敌持续时间
    bool _knockbackFromRight;    ///< 后退方向（true=从右侧受击）
    float _knockbackSpeed;       ///< 后退速度
    float _knockbackDuration;    ///< 后退持续时间
    float _knockbackTimer;       ///< 后退计时器
    
    // ========================================================================
    // 跳跃相关
    // ========================================================================
    
    bool _isJumpKeyPressed;      ///< 跳跃键是否按下
    float _jumpKeyHoldTime;      ///< 跳跃键按住时间
    float _maxJumpHoldTime;      ///< 最大跳跃按键时间
    float _velocityY;            ///< 垂直速度
    float _gravity;              ///< 重力加速度
    float _jumpForce;            ///< 跳跃力度
    float _minJumpForce;         ///< 最小跳跃力度（点按）
    bool _isOnGround;            ///< 是否在地面上
    float _groundY;              ///< 地面Y坐标
    
    // 二段跳
    bool _canDoubleJump;         ///< 是否可以二段跳
    bool _hasDoubleJumped;       ///< 是否已经二段跳过
    float _doubleJumpForce;      ///< 二段跳力度
    
    // 下落距离追踪
    float _fallStartY;           ///< 开始下落时的Y坐标
    float _hardLandThreshold;    ///< 触发重落地的下落距离阈值
    
    // ========================================================================
    // 攻击相关
    // ========================================================================
    
    bool _isAttacking;               ///< 是否正在攻击
    KnightState _stateBeforeAttack;  ///< 攻击前的状态
    bool _wasOnGroundBeforeAttack;   ///< 攻击前是否在地面
    Sprite* _slashEffect;            ///< 攻击特效精灵
    int _currentSlashType;           ///< 当前攻击类型
    float _slashEffectTimer;         ///< 攻击特效计时器
    int _slashEffectPhase;           ///< 攻击特效阶段

    // ========================================================================
    // 贴墙相关
    // ========================================================================
    
    bool _isOnWall;              ///< 是否贴在墙上
    bool _wallOnRight;           ///< 墙壁是否在右侧
    float _wallSlideSpeed;       ///< 贴墙下滑速度
    float _wallJumpForceX;       ///< 蹬墙跳水平力度
    float _wallJumpForceY;       ///< 蹬墙跳垂直力度
    float _wallJumpTimer;        ///< 蹬墙跳计时器
    float _wallJumpDuration;     ///< 蹬墙跳持续时间
    
    // 贴墙攻击
    float _wallSlashEffectTimer;     ///< 贴墙攻击特效计时器
    int _wallSlashEffectPhase;       ///< 贴墙攻击特效阶段
    
    // 蹬墙跳烟雾特效
    Sprite* _wallJumpPuffEffect;     ///< 蹬墙跳烟雾特效精灵
    float _wallJumpPuffTimer;        ///< 烟雾特效计时器
    int _wallJumpPuffFrame;          ///< 当前烟雾特效帧
    Vec2 _wallJumpPuffPos;           ///< 烟雾特效位置
    
    // ========================================================================
    // 外部跳跃
    // ========================================================================
    
    bool _isExternalJump;            ///< 是否处于外部触发的跳跃
    float _externalJumpTimer;        ///< 外部跳跃计时器
    float _externalJumpDuration;     ///< 外部跳跃强制移动持续时间
    float _externalJumpDirection;    ///< 外部跳跃水平方向
    
    // ========================================================================
    // 冲刺相关
    // ========================================================================
    
    float _dashSpeed;            ///< 冲刺速度
    float _dashDuration;         ///< 冲刺持续时间
    float _dashTimer;            ///< 冲刺计时器
    float _dashCooldown;         ///< 冲刺冷却时间
    float _dashCooldownTimer;    ///< 冲刺冷却计时器
    bool _canDash;               ///< 是否可以冲刺
    Sprite* _dashEffect;         ///< 冲刺特效精灵
    int _dashEffectFrame;        ///< 当前冲刺特效帧
    float _dashEffectTimer;      ///< 冲刺特效计时器
    
    // ========================================================================
    // 看向相关
    // ========================================================================
    
    bool _isLookingUp;           ///< 是否按住向上看
    bool _isLookingDown;         ///< 是否按住向下看
    
    // ========================================================================
    // 地图模式
    // ========================================================================
    
    bool _isMapMode;             ///< 是否处于地图模式
    bool _isMapKeyPressed;       ///< Tab键是否按住
    
    // ========================================================================
    // 椅子相关
    // ========================================================================
    
    bool _isNearChair;           ///< 是否靠近椅子
    bool _isSitting;             ///< 是否正在坐着
    float _sitIdleTimer;         ///< 坐着静止计时器
    float _sitIdleTimeout;       ///< 入睡超时时间
    bool _isAsleep;              ///< 是否已经睡着
    EventKeyboard::KeyCode _exitKey;  ///< 退出时按的键
    
    // ========================================================================
    // 护符系统（0=未装备，1=已装备）
    // ========================================================================
    
    int _charmStalwartShell;     ///< 坚硬外壳：受击无敌时长+0.4s
    int _charmSoulCatcher;       ///< 灵魂捕手：攻击获得Soul+1
    int _charmShamanStone;       ///< 萨满之石：法术伤害+1
    int _charmSprintmaster;      ///< 飞毛腿：移动速度+20%
    int _charmSteadyBody;        ///< 稳定之体：攻击无后坐力
    
    // ========================================================================
    // 灵魂系统
    // ========================================================================
    
    int _soul;                   ///< 当前灵魂值
    int _maxSoul;                ///< 最大灵魂值
    int _spellCost;              ///< 法术消耗灵魂
    int _recoverCost;            ///< 回复消耗灵魂
    bool _isSpaceKeyPressed;     ///< 空格键是否按下
    float _spaceKeyHoldTime;     ///< 空格键按住时间
    float _recoverHoldThreshold; ///< 判定长按的时间阈值
    bool _isRecovering;          ///< 是否正在回复
    bool _recoverConsumed;       ///< 回复是否已消耗灵魂
    
    // Focus 系统
    bool _isFocusing;            ///< 是否正在Focus
    bool _focusConsumed;         ///< Focus是否已消耗灵魂
    int _focusCost;              ///< Focus消耗灵魂
    float _castSpellAnimTimer;   ///< 法术动画计时器
    bool _spellEffectCreated;    ///< 法术特效是否已创建
    Sprite* _vengefulSpiritEffect; ///< 法术特效精灵
    float _vengefulSpiritSpeed;  ///< 法术移动速度
    bool _vengefulSpiritFacingRight; ///< 法术朝向
    
    // ========================================================================
    // 平台与位置
    // ========================================================================
    
    std::vector<Platform> _platforms;  ///< 平台列表（STL 容器）
    Vec2 _lastSafePosition;      ///< 最后安全位置
    Vec2 _respawnPosition;       ///< 重生目标位置
    
    // ========================================================================
    // 动画缓存
    // ========================================================================
    
    Animation* _idleAnim;
    Animation* _runStartAnim;
    Animation* _runAnim;
    Animation* _turnAnim;
    Animation* _runToIdleAnim;
    Animation* _jumpUpAnim;
    Animation* _jumpPeakAnim;
    Animation* _jumpFallAnim;
    Animation* _landAnim;
    Animation* _hardLandAnim;
    Animation* _dashAnim;
    Animation* _dashToIdleAnim;
    Animation* _dashEffectAnim;
    Animation* _lookUpAnim;
    Animation* _lookUpEndAnim;
    Animation* _lookDownAnim;
    Animation* _lookDownEndAnim;
    Animation* _wallSlideAnim;
    Animation* _wallSlashAnim;
    Animation* _wallJumpAnim;
    Animation* _doubleJumpAnim;
    Animation* _slashAnim;
    Animation* _upSlashAnim;
    Animation* _downSlashAnim;
    Animation* _getAttackedAnim;
    Animation* _deadAnim;
    Animation* _spikeDeathAnim;
    Animation* _hazardRespawnAnim;
    Animation* _vengefulSpiritAnim;
    Animation* _vengefulSpiritEffectAnim;
    Animation* _recoverAnim;
    Animation* _focusAnim;
    Animation* _focusGetAnim;
    Animation* _focusEndAnim;
    Animation* _wallJumpPuffAnim;
    Animation* _mapOpenAnim;
    Animation* _mapIdleAnim;
    Animation* _mapWalkAnim;
    Animation* _mapTurnAnim;
    Animation* _mapAwayAnim;
    
    // 椅子动画
    Animation* _sitAnim;
    Animation* _sitIdleAnim;
    Animation* _sitFallAsleepAnim;
    Animation* _sittingAsleepAnim;
    Animation* _wakeToSitAnim;
    Animation* _getOffAnim;
    Animation* _sitMapOpenAnim;
    Animation* _sitMapCloseAnim;

    // ========================================================================
    // 音效相关
    // ========================================================================
    
    int _runningSoundId;         ///< 跑步音效ID
    int _jumpSoundId;            ///< 跳跃音效ID
};

#endif // __THE_KNIGHT_H__
