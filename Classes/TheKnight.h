/**
 * @file TheKnight.h
 * @brief 小骑士（TheKnight）角色类头文件
 */

#ifndef __THE_KNIGHT_H__
#define __THE_KNIGHT_H__

#include "cocos2d.h"

USING_NS_CC;

// 骑士动画状态
enum class KnightState
{
    IDLE,       // 静止状态
    RUNNING,    // 跑动状态
    TURNING,    // 转向状态
    RUN_TO_IDLE,// 从跑动到静止的过渡
    JUMPING,    // 跳跃上升状态
    FALLING,    // 下落状态
    LANDING,    // 落地状态
    HARD_LANDING, // 重落地状态（高处落下）
    DASHING,    // 冲刺状态
    DASH_TO_IDLE, // 冲刺结束状态
    LOOKING_UP,   // 向上看状态
    LOOK_UP_END,  // 向上看结束状态
    LOOKING_DOWN, // 向下看状态
    LOOK_DOWN_END, // 向下看结束状态
    WALL_SLIDING,  // 贴墙下滑状态
    WALL_SLASHING, // 贴墙攻击状态（新增）
    WALL_JUMPING,  // 蹬墙跳状态
    DOUBLE_JUMPING, // 二段跳状态
    SLASHING,      // 水平攻击状态
    UP_SLASHING,   // 向上攻击状态
    DOWN_SLASHING,  // 向下攻击状态
    GET_ATTACKED,   // 受击状态（硬直）
    DEAD,          // 死亡状态
    SPIKE_DEATH,   // 尖刺死亡状态
    HAZARD_RESPAWN, // 危险区域重生状态
    CASTING_SPELL,  // 释放法术状态
    RECOVERING,     // 回复生命状态（保留向后兼容）
    FOCUSING,       // Focus聚气状态（长按空格）
    FOCUS_GET,      // Focus回血状态
    FOCUS_END,      // Focus结束状态（松开空格）
    MAP_OPENING,    // 打开地图状态
    MAP_IDLE,       // 地图模式静止状态
    MAP_WALKING,    // 地图模式行走状态
    MAP_TURNING,    // 地图模式转向状态
    MAP_CLOSING,     // 关闭地图状态

    // Chair sitting states
    SITTING,           // 坐下动作
    SIT_IDLE,          // 坐着静止
    SIT_FALL_ASLEEP,   // 坐着入睡
    SITTING_ASLEEP,    // 坐着睡觉
    WAKE_TO_SIT,       // 醒来
    GET_OFF,           // 起身离开
    SIT_MAP_OPEN,      // 坐着打开地图
    SIT_MAP_CLOSE      // 坐着关闭地图
};

// 平台结构
struct Platform
{
    Rect rect;      // 平台的碰撞矩形
    Node* node;     // 平台节点
};

class TheKnight : public Sprite
{
public:
    static TheKnight* create();
    
    virtual bool init() override;
    
    // 每帧更新
    void update(float dt) override;
    
    // 设置平台列表（用于碰撞检测）
    void setPlatforms(const std::vector<Platform>& platforms);
    
    // 获取碰撞盒
    Rect getBoundingBox() const;
    
    // 获取看向状态
    bool isLookingUp() const { return _state == KnightState::LOOKING_UP; }
    bool isLookingDown() const { return _state == KnightState::LOOKING_DOWN; }
    
    // 获取攻击特效的碰撞盒（用于攻击判定）
    bool getSlashEffectBoundingBox(Rect& outRect) const;
    
    // 受到伤害
    void takeDamage(int damage);
    
    // 是否无敌
    bool isInvincible() const { return _isInvincible; }
    
    // 是否处于硬直状态
    bool isStunned() const { return _state == KnightState::GET_ATTACKED; }
    
    // 是否已死亡
    bool isDead() const { return _state == KnightState::DEAD; }
    
    // 是否正在重落地
    bool isHardLanding() const { return _state == KnightState::HARD_LANDING; }
    
    // 尖刺死亡相关
    bool isSpikeDeathState() const { return _state == KnightState::SPIKE_DEATH; }
    bool isHazardRespawnState() const { return _state == KnightState::HAZARD_RESPAWN; }
    void startSpikeDeath();  // 开始尖刺死亡动画
    void startHazardRespawn(const Vec2& respawnPos);  // 开始危险区域重生
    void setLastSafePosition(const Vec2& pos) { _lastSafePosition = pos; }
    Vec2 getLastSafePosition() const { return _lastSafePosition; }
    
    // 获取生命值
    int getHP() const { return _hp; }
    int getMaxHP() const { return _maxHP; }
    
    // 设置生命值（用于恢复）
    void setHP(int hp) { _hp = std::min(hp, _maxHP); }
    
    // 获取灵魂值
    int getSoul() const { return _soul; }
    int getMaxSoul() const { return _maxSoul; }
    
    // 设置灵魂值（用于重置）
    void setSoul(int soul) { _soul = std::max(0, std::min(soul, _maxSoul)); }
    
    // 增加灵魂值（攻击命中时调用）
    void addSoul(int amount);
    
    // 获取法术特效（用于碰撞检测）
    Sprite* getVengefulSpiritEffect() const { return _vengefulSpiritEffect; }
    
    // 获取护符状态
    int getCharmSoulCatcher() const { return _charmSoulCatcher; }
    int getCharmShamanStone() const { return _charmShamanStone; }
    int getCharmStalwartShell() const { return _charmStalwartShell; }
    int getCharmSprintmaster() const { return _charmSprintmaster; }
    int getCharmSteadyBody() const { return _charmSteadyBody; }
    
    // 设置护符状态
    void setCharmSoulCatcher(int value) { _charmSoulCatcher = value; }
    void setCharmShamanStone(int value) { _charmShamanStone = value; }
    void setCharmStalwartShell(int value) { _charmStalwartShell = value; }
    void setCharmSprintmaster(int value) { _charmSprintmaster = value; }
    void setCharmSteadyBody(int value) { _charmSteadyBody = value; }
    
    // 设置受击后退方向（根据敌人位置）
    void setKnockbackDirection(bool fromRight);
    
    // 下劈弹反（下劈命中敌人后弹起）
    void bounceFromDownSlash();
    
    // 从外部触发跳跃（用于场景切换时的跳跃效果）
    void triggerJumpFromExternal(float horizontalSpeed);
    
    // Chair sitting system
    bool isNearChair() const { return _isNearChair; }
    void setNearChair(bool isNear) { _isNearChair = isNear; }
    bool isSitting() const;
    void startSitting();  // Called by GameScene when near chair and press W
    
private:
    // 创建动画（指定起始帧和结束帧）
    Animation* createAnimation(const std::string& path, const std::string& prefix, int startFrame, int endFrame, float delay);
    
    // 加载所有动画
    void loadAnimations();
    
    // 播放动画
    void playAnimation(const std::string& animName, bool loop = true);
    
    // 键盘事件处理
    void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
    void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
    
    // 状态切换
    void changeState(KnightState newState);
    void onTurnFinished();
    void onRunToIdleFinished();
    void onRunStartFinished();
    void onLandFinished();
    void onHardLandFinished();   // 重落地动画完成回调
    void onFallAnimFinished();
    void onDashFinished();
    void onDashToIdleFinished();
    void onLookUpAnimFinished();
    void onLookUpEndFinished();
    void onLookDownAnimFinished();
    void onLookDownEndFinished();
    void onWallJumpAnimFinished();
    
    // 地图模式相关
    void onMapOpenFinished();
    void onMapTurnFinished();
    void onMapCloseFinished();
    void exitMapMode();  // 强制退出地图模式
    
    // Chair sitting related
    void onSitAnimFinished();
    void onSitFallAsleepFinished();
    void onWakeToSitFinished();
    void onGetOffFinished();
    void onSitMapOpenFinished();
    void onSitMapCloseFinished();
    void exitSitting(bool pressedLeft);  // Exit sitting with direction
    void updateSitting(float dt);
    
    // 跳跃相关
    void startJump();
    void updateJump(float dt);
    void updateFall(float dt);
    bool checkGroundCollision(float& groundY);
    bool checkCeilingCollision(float& ceilingY);
    bool checkWallCollision(float& newX, bool movingRight);
    bool checkStillOnGround();
    
    // 二段跳相关
    void startDoubleJump();
    void updateDoubleJump(float dt);
    void onDoubleJumpAnimFinished();
    
    // 攻击相关
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
    
    // 爬墙相关
    bool checkWallSlideCollision(bool checkRight);
    void startWallSlide(bool wallOnRight);
    void updateWallSlide(float dt);
    void startWallJump();
    void updateWallJump(float dt);
    
    // 冲刺相关
    void startDash();
    void updateDash(float dt);
    void createDashEffect();
    void updateDashEffect(float dt);
    
    // 受击相关
    void updateGetAttacked(float dt);
    void onGetAttackedFinished();
    
    // 死亡相关
    void startDeath();
    void onDeathAnimFinished();
    
    // 尖刺死亡相关
    void onSpikeDeathAnimFinished();
    void onHazardRespawnAnimFinished();
    
    // 灵魂系统相关
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
    
    // Focus系统相关
    void startFocus();
    void updateFocus(float dt);
    void onFocusAnimFinished();
    void onFocusGetAnimFinished();
    void onFocusEndAnimFinished();
    void cancelFocus();
    
    // 贴墙攻击相关
    void startWallSlash();
    void updateWallSlash(float dt);
    void onWallSlashAnimFinished();
    void createWallSlashEffect(int effectNum);
    void updateWallSlashEffectPosition();
    
    // 蹬墙跳烟雾特效
    void createWallJumpPuffEffect();
    void updateWallJumpPuffEffect(float dt);
    void removeWallJumpPuffEffect();
    
private:
    KnightState _state;          // 当前状态
    bool _facingRight;           // 是否面向右边
    bool _isMovingLeft;          // 是否正在向左移动
    bool _isMovingRight;         // 是否正在向右移动
    float _moveSpeed;            // 移动速度
    bool _isRunStartFinished;    // 跑步起步动画是否完成
    
    // 生命值相关
    int _hp;                     // 当前生命值
    int _maxHP;                  // 最大生命值
    
    // 受击相关
    bool _isInvincible;          // 是否无敌
    float _invincibleTimer;      // 无敌时间计时器
    float _invincibleDuration;   // 无敌持续时间
    bool _knockbackFromRight;    // 后退方向（true表示从右侧受击，向左后退）
    float _knockbackSpeed;       // 后退速度
    float _knockbackDuration;    // 后退持续时间
    float _knockbackTimer;       // 后退计时器
    
    // 跳跃相关
    bool _isJumpKeyPressed;      // 跳跃键是否按下
    float _jumpKeyHoldTime;      // 跳跃键按住时间
    float _maxJumpHoldTime;      // 最大跳跃按键时间
    float _velocityY;            // 垂直速度
    float _gravity;              // 重力加速度
    float _jumpForce;            // 跳跃力度
    float _minJumpForce;         // 最小跳跃力度（点按）
    bool _isOnGround;            // 是否在地面上
    float _groundY;              // 地面Y坐标
    
    // 二段跳相关
    bool _canDoubleJump;         // 是否可以二段跳
    bool _hasDoubleJumped;       // 是否已经二段跳过
    float _doubleJumpForce;      // 二段跳力度
    
    // 下落距离追踪（用于重落地判定）
    float _fallStartY;           // 开始下落时的Y坐标
    float _hardLandThreshold;    // 触发重落地的下落距离阈值
    
    // 攻击相关
    bool _isAttacking;           // 是否正在攻击
    KnightState _stateBeforeAttack; // 攻击前的状态（用于恢复）
    bool _wasOnGroundBeforeAttack;  // 攻击前是否在地面
    Sprite* _slashEffect;        // 攻击特效精灵
    int _currentSlashType;       // 当前攻击类型：0=水平, 1=向上, 2=向下
    float _slashEffectTimer;     // 攻击特效计时器
    int _slashEffectPhase;       // 攻击特效阶段：1=Effect1, 2=Effect2

    // 爬墙相关
    bool _isOnWall;              // 是否贴在墙上
    bool _wallOnRight;           // 墙壁是否在右侧
    float _wallSlideSpeed;       // 贴墙下滑速度
    float _wallJumpForceX;       // 蹬墙跳水平力度
    float _wallJumpForceY;       // 蹬墙跳垂直力度
    float _wallJumpTimer;        // 蹬墙跳计时器
    float _wallJumpDuration;     // 蹬墙跳持续时间（控制不住方向的时间）
    
    // 贴墙攻击相关
    float _wallSlashEffectTimer;     // 贴墙攻击特效计时器
    int _wallSlashEffectPhase;       // 贴墙攻击特效阶段
    
    // 蹬墙跳烟雾特效
    Sprite* _wallJumpPuffEffect;     // 蹬墙跳烟雾特效精灵
    float _wallJumpPuffTimer;        // 烟雾特效计时器
    int _wallJumpPuffFrame;          // 当前烟雾特效帧
    Vec2 _wallJumpPuffPos;           // 烟雾特效位置
    
    // 外部触发跳跃相关
    bool _isExternalJump;            // 是否处于外部触发的跳跃
    float _externalJumpTimer;        // 外部跳跃计时器
    float _externalJumpDuration;     // 外部跳跃强制移动持续时间
    float _externalJumpDirection;    // 外部跳跃水平方向（1.0或-1.0）
    
    // 冲刺相关
    float _dashSpeed;            // 冲刺速度
    float _dashDuration;         // 冲刺持续时间
    float _dashTimer;            // 冲刺计时器
    float _dashCooldown;         // 冲刺冷却时间
    float _dashCooldownTimer;    // 冲刺冷却计时器
    bool _canDash;               // 是否可以冲刺
    Sprite* _dashEffect;         // 冲刺特效精灵
    int _dashEffectFrame;        // 当前冲刺特效帧
    float _dashEffectTimer;      // 冲刺特效计时器
    
    // 看向相关
    bool _isLookingUp;           // 是否按住向上看
    bool _isLookingDown;         // 是否按住向下看
    
    // 地图模式相关
    bool _isMapMode;             // 是否处于地图模式
    bool _isMapKeyPressed;       // Tab键是否按住
    
    // Chair sitting related
    bool _isNearChair;           // 是否靠近椅子
    bool _isSitting;             // 是否正在坐着
    float _sitIdleTimer;         // 坐着静止计时器
    float _sitIdleTimeout;       // 入睡超时时间（8秒）
    bool _isAsleep;              // 是否已经睡着
    EventKeyboard::KeyCode _exitKey;  // 退出时按的键
    
    // 护符系统
    int _charmStalwartShell;     // 坚硬外壳：受击无敌时长+0.4s
    int _charmSoulCatcher;       // 灵魂捕手：攻击获得Soul+1
    int _charmShamanStone;       // 萨满之石：法术伤害+1
    int _charmSprintmaster;      // 飞毛腿  ：移动速度+20%
    int _charmSteadyBody;        // 稳定之体：攻击无后坐力
    
    // 灵魂系统相关
    int _soul;                   // 当前灵魂值
    int _maxSoul;                // 最大灵魂值
    int _spellCost;              // 法术消耗灵魂
    int _recoverCost;            // 回复消耗灵魂
    bool _isSpaceKeyPressed;     // 空格键是否按下
    float _spaceKeyHoldTime;     // 空格键按住时间
    float _recoverHoldThreshold; // 判定长按的时间阈值
    bool _isRecovering;          // 是否正在回复
    bool _recoverConsumed;       // 回复是否已消耗灵魂
    
    // Focus系统相关
    bool _isFocusing;            // 是否正在Focus
    bool _focusConsumed;         // Focus是否已消耗灵魂
    int _focusCost;              // Focus消耗灵魂
    
    float _castSpellAnimTimer;   // 法术动画计时器
    bool _spellEffectCreated;    // 法术特效是否已创建
    Sprite* _vengefulSpiritEffect; // 法术特效精灵
    float _vengefulSpiritSpeed;  // 法术移动速度
    bool _vengefulSpiritFacingRight; // 法术朝向
    
    // 平台列表
    std::vector<Platform> _platforms;
    
    // 安全位置（用于危险区域重生）
    Vec2 _lastSafePosition;
    Vec2 _respawnPosition;  // 重生目标位置
    
    // 动画缓存
    Animation* _idleAnim;
    Animation* _runStartAnim;
    Animation* _runAnim;
    Animation* _turnAnim;
    Animation* _runToIdleAnim;
    Animation* _jumpUpAnim;
    Animation* _jumpPeakAnim;
    Animation* _jumpFallAnim;
    Animation* _landAnim;
    Animation* _hardLandAnim;    // 重落地动画
    Animation* _dashAnim;        // 冲刺动画
    Animation* _dashToIdleAnim;  // 冲刺结束动画
    Animation* _dashEffectAnim;  // 冲刺特效动画
    Animation* _lookUpAnim;      // 向上看动画
    Animation* _lookUpEndAnim;   // 向上看结束动画
    Animation* _lookDownAnim;    // 向下看动画
    Animation* _lookDownEndAnim; // 谷歌翻译结果
    Animation* _wallSlideAnim;   // 贴墙下滑动画
    Animation* _wallSlashAnim;   // 贴墙攻击动画
    Animation* _wallJumpAnim;    // 蹬墙跳动画
    Animation* _doubleJumpAnim;  // 二段跳动画
    Animation* _slashAnim;       // 水平攻击动画
    Animation* _upSlashAnim;     // 向上攻击动画
    Animation* _downSlashAnim;   // 向下攻击动画
    Animation* _getAttackedAnim; // 受击动画
    Animation* _deadAnim;        // 死亡动画
    Animation* _spikeDeathAnim;  // 尖刺死亡动画
    Animation* _hazardRespawnAnim; // 危险区域重生动画
    Animation* _vengefulSpiritAnim;      // 法术释放动画
    Animation* _vengefulSpiritEffectAnim; // 法术特效动画
    Animation* _recoverAnim;     // 回复动画
    Animation* _focusAnim;       // Focus聚气动画
    Animation* _focusGetAnim;    // Focus回血动画
    Animation* _focusEndAnim;    // Focus结束动画
    Animation* _wallJumpPuffAnim; // 蹬墙跳烟雾动画
    Animation* _mapOpenAnim;     // 打开地图动画
    Animation* _mapIdleAnim;     // 地图模式静止动画
    Animation* _mapWalkAnim;     // 地图模式行走动画
    Animation* _mapTurnAnim;     // 地图模式转向动画
    Animation* _mapAwayAnim;     // 关闭地图动画
    
    // Chair animations
    Animation* _sitAnim;             // 坐下动画
    Animation* _sitIdleAnim;         // 坐着静止动画
    Animation* _sitFallAsleepAnim;   // 坐着入睡动画
    Animation* _sittingAsleepAnim;   // 坐着睡觉动画
    Animation* _wakeToSitAnim;       // 醒来动画
    Animation* _getOffAnim;          // 起身离开动画
    Animation* _sitMapOpenAnim;      // 坐着打开地图动画
    Animation* _sitMapCloseAnim;     // 坐着关闭地图动画

    // 音效相关
    int _runningSoundId;             // 跑步音效ID
    int _jumpSoundId;                // 跳跃音效ID
};

#endif // __THE_KNIGHT_H__
