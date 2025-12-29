/**
 * @file TheKnightCoreLogic.cpp
 * @brief 小骑士（TheKnight）角色类实现文件 - 核心逻辑
 */

#include "TheKnight.h"
#include "CharmManager.h"

TheKnight* TheKnight::create()
{
    TheKnight* knight = new (std::nothrow) TheKnight();
    if (knight && knight->init())
    {
        knight->autorelease();
        return knight;
    }
    CC_SAFE_DELETE(knight);
    return nullptr;
}

bool TheKnight::init()
{
    // 先用第一帧图片初始化Sprite
    if (!Sprite::initWithFile("TheKnight/Idle/Idle1.png"))
    {
        return false;
    }
    
    // 设置锚点为底部中心，方便碰撞检测
    this->setAnchorPoint(Vec2(0.5f, 0.0f));
    
    // 初始化状态
    _state = KnightState::IDLE;
    _facingRight = false;
    _isMovingLeft = false;
    _isMovingRight = false;
    _moveSpeed = 400.0f;  // 移动速度翻倍
    _isRunStartFinished = false;
    
    // 生命值初始化
    _hp = 5;
    _maxHP = 5;
    
    // 受击相关初始化
    _isInvincible = false;
    _invincibleTimer = 0.0f;
    _invincibleDuration = 1.3f;      // 无敌持续时间1.3秒
    _knockbackFromRight = false;
    _knockbackSpeed = 400.0f;        // 后退速度
    _knockbackDuration = 0.24f;      // 后退持续时间（与8帧动画同步，8*0.03=0.24秒）
    _knockbackTimer = 0.0f;
    
    // 跳跃相关初始化
    _isJumpKeyPressed = false;
    _jumpKeyHoldTime = 0.0f;
    _maxJumpHoldTime = 0.15f;
    _velocityY = 0.0f;
    _gravity = 4000.0f;           // 重力加速度
    _jumpForce = 1400.0f;         // 跳跃力度（最大跳跃高度约3倍角色高度）
    _minJumpForce = 600.0f;       // 最小跳跃力度（点按）
    _isOnGround = true;
    _groundY = 0.0f;
    
    // 二段跳相关初始化
    _canDoubleJump = true;        // 是否拥有二段跳能力
    _hasDoubleJumped = false;     // 是否已经使用了二段跳
    _doubleJumpForce = 2000.0f;   // 二段跳力度（两倍高度）
    
    // 下落距离追踪初始化
    _fallStartY = 0.0f;
    _hardLandThreshold = 1500.0f;  // 下落超过1500像素触发重落地
    
    // 冲刺相关初始化
    _dashSpeed = 1600.0f;
    _dashDuration = 0.25f;
    _dashTimer = 0.0f;
    _dashCooldown = 0.5f;
    _dashCooldownTimer = 0.0f;
    _canDash = true;
    _dashEffect = nullptr;
    _dashEffectFrame = 0;
    _dashEffectTimer = 0.0f;
    
    // 看向相关初始化
    _isLookingUp = false;
    _isLookingDown = false;
    
    // 地图模式初始化
    _isMapMode = false;
    _isMapKeyPressed = false;
    
    // Chair sitting related initialization
    _isNearChair = false;
    _isSitting = false;
    _sitIdleTimer = 0.0f;
    _sitIdleTimeout = 8.0f;  // 8秒无操作后入睡
    _isAsleep = false;
    _exitKey = EventKeyboard::KeyCode::KEY_NONE;

    // 输入禁用初始化
    _inputDisabled = false;

    // 护符系统初始化（0=未装备，1=已装备）
    _charmStalwartShell = 0;     // 坚硬外壳：受击无敌时长+0.4s
    _charmSoulCatcher = 0;       // 灵魂捕手：攻击获得Soul+1
    _charmShamanStone = 0;       // 萨满之石：法术伤害+1
    _charmSprintmaster = 0;      // 飞毛腿  ：移动速度+20%
    _charmSteadyBody = 0;        // 稳定之体：攻击无后坐力
    
    // 爬墙相关初始化
    _isOnWall = false;
    _wallOnRight = false;
    _wallSlideSpeed = 600.0f;       // 贴墙下滑速度
    _wallJumpForceX = 800.0f;       // 蹬墙跳水平力度
    _wallJumpForceY = 2000.0f;      // 蹬墙跳垂直力度
    _wallJumpTimer = 0.0f;
    _wallJumpDuration = 0.15f;      // 蹬墙跳控制不住方向的时间
    
    // 贴墙攻击相关初始化
    _wallSlashEffectTimer = 0.0f;
    _wallSlashEffectPhase = 0;
    
    // 蹬墙跳烟雾特效初始化
    _wallJumpPuffEffect = nullptr;
    _wallJumpPuffTimer = 0.0f;
    _wallJumpPuffFrame = 0;
    _wallJumpPuffPos = Vec2::ZERO;
    
    // 外部触发跳跃初始化
    _isExternalJump = false;
    _externalJumpTimer = 0.0f;
    _externalJumpDuration = 0.5f;
    _externalJumpDirection = 0.0f;
    
    // 攻击相关初始化
    _isAttacking = false;
    _stateBeforeAttack = KnightState::IDLE;
    _wasOnGroundBeforeAttack = true;
    _slashEffect = nullptr;
    _currentSlashType = 0;
    _slashEffectTimer = 0.0f;
    _slashEffectPhase = 0;
    
    // 灵魂系统初始化
    _soul = 0;
    _maxSoul = 6;
    _spellCost = 2;
    _recoverCost = 2;
    _isSpaceKeyPressed = false;
    _spaceKeyHoldTime = 0.0f;
    _recoverHoldThreshold = 0.2f;  // 长按0.2秒开始恢复
    _isRecovering = false;
    _recoverConsumed = false;
    
    // Focus系统初始化
    _isFocusing = false;
    _focusConsumed = false;
    _focusCost = 2;               // Focus消耗2点灵魂
    
    _castSpellAnimTimer = 0.0f;
    _spellEffectCreated = false;
    _vengefulSpiritEffect = nullptr;
    _vengefulSpiritSpeed = 3000.0f;
    _vengefulSpiritFacingRight = false;

    // 安全位置初始化
    _lastSafePosition = Vec2::ZERO;
    _respawnPosition = Vec2::ZERO;

    // 加载动画
    loadAnimations();
    
    // 播放初始Idle动画
    playAnimation("idle", true);
    
    // 注册键盘事件
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = CC_CALLBACK_2(TheKnight::onKeyPressed, this);
    keyboardListener->onKeyReleased = CC_CALLBACK_2(TheKnight::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
    
    // 启用update
    this->scheduleUpdate();
    
    return true;
}

void TheKnight::setPlatforms(const std::vector<Platform>& platforms)
{
    _platforms = platforms;
}

Rect TheKnight::getBoundingBox() const
{
    auto size = this->getContentSize();
    auto pos = this->getPosition();
    // 锚点在底部中心(0.5, 0)，pos.y就是底部位置
    // 缩小碰撞盒使其更精确
    float shrinkX = size.width * 0.3f;
    float shrinkY = size.height * 0.05f;
    return Rect(pos.x - size.width / 2 + shrinkX, 
                pos.y + shrinkY, 
                size.width - shrinkX * 2, 
                size.height - shrinkY * 2);
}

bool TheKnight::checkGroundCollision(float& groundY)
{
    Vec2 pos = this->getPosition();
    Rect knightRect = getBoundingBox();
    
    // 检查所有平台
    for (const auto& platform : _platforms)
    {
        // 使用角色中心点检查水平范围，与checkStillOnGround保持一致
        float centerX = pos.x;
        if (centerX > platform.rect.getMinX() &&
            centerX < platform.rect.getMaxX())
        {
            // 检查是否落在平台顶部
            float platformTop = platform.rect.getMaxY();
            float knightBottom = knightRect.getMinY();
            
            // 扩大检测范围，防止高速移动时穿透
            // 检测范围根据下落速度动态调整
            float detectionRange = 50.0f + std::abs(_velocityY) * 0.02f;
            
            // 当骑士底部接近或低于平台顶部时
            if (knightBottom <= platformTop && 
                knightBottom >= platformTop - detectionRange)
            {
                // 锚点在底部，返回平台顶部Y坐标
                groundY = platformTop;
                return true;
            }
        }
    }
    
    return false;
}

bool TheKnight::checkCeilingCollision(float& ceilingY)
{
    Rect knightRect = getBoundingBox();
    
    // 检查所有平台
    for (const auto& platform : _platforms)
    {
        // 检查水平范围是否重叠
        if (knightRect.getMaxX() > platform.rect.getMinX() &&
            knightRect.getMinX() < platform.rect.getMaxX())
        {
            // 检查是否撞到平台底部
            float platformBottom = platform.rect.getMinY();
            float knightTop = knightRect.getMaxY();
            
            // 扩大检测范围，防止高速移动时穿透
            float detectionRange = 50.0f + std::abs(_velocityY) * 0.02f;
            
            // 当骑士顶部进入平台底部时
            if (knightTop >= platformBottom && 
                knightTop <= platformBottom + detectionRange)
            {
                // 同时检查骑士是否真的在平台内部（不是站在平台上面）
                float platformTop = platform.rect.getMaxY();
                float knightBottom = knightRect.getMinY();
                if (knightBottom < platformTop)
                {
                    // 锚点在底部，返回骑士应该被推回的Y坐标
                    auto size = this->getContentSize();
                    ceilingY = platformBottom - size.height;
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool TheKnight::checkWallCollision(float& newX, bool movingRight)
{
    Rect knightRect = getBoundingBox();
    
    // 检查所有平台
    for (const auto& platform : _platforms)
    {
        // 检查垂直范围是否重叠（骑士和平台在垂直方向上有交集）
        if (knightRect.getMaxY() > platform.rect.getMinY() &&
            knightRect.getMinY() < platform.rect.getMaxY())
        {
            if (movingRight)
            {
                // 向右移动，检查是否撞到平台左侧
                float platformLeft = platform.rect.getMinX();
                // 检测条件：骑士右侧进入或超过平台左侧
                if (knightRect.getMaxX() >= platformLeft &&
                    knightRect.getMinX() < platformLeft)
                {
                    // 计算骑士应该被推回的X坐标
                    auto size = this->getContentSize();
                    float shrinkX = size.width * 0.3f;
                    newX = platformLeft - size.width / 2 + shrinkX - 1;
                    return true;
                }
            }
            else
            {
                // 向左移动，检查是否撞到平台右侧
                float platformRight = platform.rect.getMaxX();
                // 检测条件：骑士左侧进入或超过平台右侧
                if (knightRect.getMinX() <= platformRight &&
                    knightRect.getMaxX() > platformRight)
                {
                    // 计算骑士应该被推回的X坐标
                    auto size = this->getContentSize();
                    float shrinkX = size.width * 0.3f;
                    newX = platformRight + size.width / 2 - shrinkX + 1;
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool TheKnight::checkStillOnGround()
{
    if (!_isOnGround) return false;
    
    Vec2 pos = this->getPosition();
    
    // 使用角色中心点检查，与checkGroundCollision保持一致
    float centerX = pos.x;
    
    // 检查是否还站在某个平台上
    for (const auto& platform : _platforms)
    {
        // 检查角色中心点是否在平台水平范围内
        if (centerX > platform.rect.getMinX() &&
            centerX < platform.rect.getMaxX())
        {
            // 检查是否站在平台顶部（允许一点误差）
            float platformTop = platform.rect.getMaxY();
            if (std::abs(pos.y - platformTop) < 10.0f)
            {
                return true;
            }
        }
    }
    
    return false;
}

bool TheKnight::checkWallSlideCollision(bool checkRight)
{
    Rect knightRect = getBoundingBox();
    Vec2 pos = this->getPosition();
    auto size = this->getContentSize();
    
    // 计算角色底部位置（用于检测是否真的贴着墙）
    float knightBottom = knightRect.getMinY();
    
    // 检查所有平台
    for (const auto& platform : _platforms)
    {
        // 关键检查：角色底部必须在墙壁的垂直范围内
        // 即角色底部要高于平台顶部，或者角色底部在平台的垂直范围内
        // 这样可以避免在两段墙之间的空隙中检测到下方的墙
        if (knightBottom >= platform.rect.getMinY() &&
            knightRect.getMaxY() > platform.rect.getMinY())
        {
            if (checkRight)
            {
                // 检查右侧是否有墙
                float platformLeft = platform.rect.getMinX();
                float knightRight = knightRect.getMaxX();
                
                // 骑士右侧接近或接触平台左侧
                if (knightRight >= platformLeft - 10 &&
                    knightRight <= platformLeft + 30)
                {
                    return true;
                }
            }
            else
            {
                // 检查左侧是否有墙
                float platformRight = platform.rect.getMaxX();
                float knightLeft = knightRect.getMinX();
                
                // 骑士左侧接近或接触平台右侧
                if (knightLeft <= platformRight + 10 &&
                    knightLeft >= platformRight - 30)
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

void TheKnight::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
    // 如果输入被禁用（如NPC对话中），忽略所有输入
    if (_inputDisabled) {
        return;
    }
    
    // 如果护符面板打开，禁用角色控制
    if (CharmManager::getInstance()->isPanelOpen()) {
        return;
    }
    
    // If sitting and not in map mode, any key except Tab should exit sitting
    if (_isSitting && _state != KnightState::SIT_MAP_OPEN && _state != KnightState::SIT_MAP_CLOSE)
    {
        // Tab key for sit map
        if (keyCode == EventKeyboard::KeyCode::KEY_TAB)
        {
            _isMapKeyPressed = true;
            if (_state == KnightState::SIT_IDLE || _state == KnightState::SITTING_ASLEEP)
            {
                changeState(KnightState::SIT_MAP_OPEN);
            }
            return;
        }
        
        // Other functional keys exit sitting
        if (keyCode == EventKeyboard::KeyCode::KEY_A || 
            keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_A ||
            keyCode == EventKeyboard::KeyCode::KEY_D || 
            keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_D ||
            keyCode == EventKeyboard::KeyCode::KEY_W || 
            keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_W ||
            keyCode == EventKeyboard::KeyCode::KEY_S || 
            keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_S ||
            keyCode == EventKeyboard::KeyCode::KEY_K ||
            keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_K ||
            keyCode == EventKeyboard::KeyCode::KEY_L ||
            keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_L ||
            keyCode == EventKeyboard::KeyCode::KEY_J ||
            keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_J ||
            keyCode == EventKeyboard::KeyCode::KEY_SPACE)
        {
            _exitKey = keyCode;
            bool pressedLeft = (keyCode == EventKeyboard::KeyCode::KEY_A || 
                               keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_A);
            exitSitting(pressedLeft);
            
            // Reset sitting timer
            _sitIdleTimer = 0.0f;
            return;
        }
    }
    
    // 支持大小写 W/w 键 - 向上看
    if (keyCode == EventKeyboard::KeyCode::KEY_W || 
        keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_W)
    {
        // 地图模式下禁用
        if (_isMapMode) return;
        
        _isLookingUp = true;
        
        if (_state == KnightState::IDLE && _isOnGround)
        {
            changeState(KnightState::LOOKING_UP);
        }
    }
    // 支持大小写 S/s 键 - 向下看
    else if (keyCode == EventKeyboard::KeyCode::KEY_S || 
             keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_S)
    {
        // 地图模式下禁用
        if (_isMapMode) return;
        
        _isLookingDown = true;
        
        if (_state == KnightState::IDLE && _isOnGround)
        {
            changeState(KnightState::LOOKING_DOWN);
        }
    }
    // 支持大小写 A/a 键
    else if (keyCode == EventKeyboard::KeyCode::KEY_A || 
        keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_A)
    {
        _isMovingLeft = true;
        
        // 地图模式下的移动
        if (_isMapMode && _isOnGround)
        {
            if (_facingRight && _state != KnightState::MAP_TURNING)
            {
                changeState(KnightState::MAP_TURNING);
            }
            else if (!_facingRight && _state == KnightState::MAP_IDLE)
            {
                changeState(KnightState::MAP_WALKING);
            }
        }
        else if (_isOnGround && _state != KnightState::DASHING)
        {
            if (_facingRight && _state != KnightState::TURNING)
            {
                changeState(KnightState::TURNING);
            }
            else if (!_facingRight && (_state == KnightState::IDLE || _state == KnightState::RUN_TO_IDLE || _state == KnightState::LANDING || _state == KnightState::DASH_TO_IDLE))
            {
                changeState(KnightState::RUNNING);
            }
        }
    }
    // 支持大小写 D/d 键
    else if (keyCode == EventKeyboard::KeyCode::KEY_D || 
             keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_D)
    {
        _isMovingRight = true;
        
        // 地图模式下的移动
        if (_isMapMode && _isOnGround)
        {
            if (!_facingRight && _state != KnightState::MAP_TURNING)
            {
                changeState(KnightState::MAP_TURNING);
            }
            else if (_facingRight && _state == KnightState::MAP_IDLE)
            {
                changeState(KnightState::MAP_WALKING);
            }
        }
        else if (_isOnGround && _state != KnightState::DASHING)
        {
            if (!_facingRight && _state != KnightState::TURNING)
            {
                changeState(KnightState::TURNING);
            }
            else if (_facingRight && (_state == KnightState::IDLE || _state == KnightState::RUN_TO_IDLE || _state == KnightState::LANDING || _state == KnightState::DASH_TO_IDLE))
            {
                changeState(KnightState::RUNNING);
            }
        }
    }
    // 支持大小写 K/k 键 - 跳跃
    else if (keyCode == EventKeyboard::KeyCode::KEY_K ||
             keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_K)
    {
        // 地图模式下禁用
        if (_isMapMode) return;
        
        // 贴墙时按跳跃键触发蹬墙跳
        if (_state == KnightState::WALL_SLIDING)
        {
            _isJumpKeyPressed = true;
            startWallJump();
        }
        // 只要在地面上就可以跳跃，包括落地动画中，但冲刺中不能跳跃
        else if (_isOnGround && _state != KnightState::DASHING)
        {
            _isJumpKeyPressed = true;
            _hasDoubleJumped = false;  // 落地后重置二段跳
            startJump();
        }
        // 空中且未使用二段跳时，可以二段跳
        else if (!_isOnGround && _canDoubleJump && !_hasDoubleJumped && 
                 (_state == KnightState::JUMPING || _state == KnightState::FALLING || _state == KnightState::WALL_JUMPING))
        {
            _isJumpKeyPressed = true;
            startDoubleJump();
        }
    }
    // 支持大小写 L/l 键 - 冲刺
    else if (keyCode == EventKeyboard::KeyCode::KEY_L ||
             keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_L)
    {
        // 地图模式下禁用
        if (_isMapMode) return;
        
        if (_canDash && _state != KnightState::DASHING)
        {
            // 贴墙时冲刺，方向为离开墙壁
            if (_state == KnightState::WALL_SLIDING)
            {
                _isOnWall = false;
                // 冲刺方向为离开墙壁的方向
                _facingRight = !_wallOnRight;
                this->setFlippedX(_facingRight);
            }
            startDash();
        }
    }
    // 支持大小写 J/j 键 - 攻击
    else if (keyCode == EventKeyboard::KeyCode::KEY_J ||
             keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_J)
    {
        // 地图模式下禁用
        if (_isMapMode) return;
        
        // 不在攻击状态且不在冲刺状态时可以攻击
        if (!_isAttacking && _state != KnightState::DASHING)
        {
            // 贴墙时攻击 -> 贴墙攻击
            if (_state == KnightState::WALL_SLIDING)
            {
                startWallSlash();
            }
            // 判断攻击方向
            // 同时按上下或都不按-> 水平攻击
            // 只按上 -> 向上攻击
            // 只按下且在空中 -> 向下攻击
            else if (_isLookingUp && !_isLookingDown)
            {
                startUpSlash();
            }
            else if (_isLookingDown && !_isLookingUp && !_isOnGround)
            {
                startDownSlash();
            }
            else
            {
                startSlash();
            }
        }
    }
    // 空格键 - 法术/回复
    else if (keyCode == EventKeyboard::KeyCode::KEY_SPACE)
    {
        // 地图模式下禁用
        if (_isMapMode) return;
        
        _isSpaceKeyPressed = true;
        _spaceKeyHoldTime = 0.0f;
        // 按下时不做任何动作，等待判断是短按还是长按
    }
    // Tab键 - 地图模式
    else if (keyCode == EventKeyboard::KeyCode::KEY_TAB)
    {
        _isMapKeyPressed = true;
        // 只有在地面上且处于静止或移动状态时才能打开地图
        if (_isOnGround && !_isMapMode &&
            (_state == KnightState::IDLE || _state == KnightState::RUNNING || 
             _state == KnightState::RUN_TO_IDLE))
        {
            changeState(KnightState::MAP_OPENING);
        }
    }
}

void TheKnight::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
    // 如果输入被禁用（如NPC对话中），忽略所有输入
    if (_inputDisabled) {
        return;
    }
    
    // 如果护符面板打开，禁用角色控制
    if (CharmManager::getInstance()->isPanelOpen()) {
        return;
    }
    
    // 支持大小写 W/w 键 - 向上看释放
    if (keyCode == EventKeyboard::KeyCode::KEY_W || 
        keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_W)
    {
        _isLookingUp = false;
        
        if (_state == KnightState::LOOKING_UP)
        {
            changeState(KnightState::LOOK_UP_END);
        }
    }
    // 支持大小写 S/s 键 - 向下看释放
    else if (keyCode == EventKeyboard::KeyCode::KEY_S || 
             keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_S)
    {
        _isLookingDown = false;
        
        if (_state == KnightState::LOOKING_DOWN)
        {
            changeState(KnightState::LOOK_DOWN_END);
        }
    }
    // 支持大小写 A/a 键
    else if (keyCode == EventKeyboard::KeyCode::KEY_A || 
        keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_A)
    {
        _isMovingLeft = false;
        
        // 地图模式下的移动
        if (_isMapMode && _state == KnightState::MAP_WALKING)
        {
            if (!_isMovingRight)
            {
                changeState(KnightState::MAP_IDLE);
            }
            else if (_isMovingRight && !_facingRight && _state != KnightState::MAP_TURNING)
            {
                changeState(KnightState::MAP_TURNING);
            }
        }
        else if (_isOnGround && _state == KnightState::RUNNING)
        {
            if (!_isMovingRight)
            {
                changeState(KnightState::RUN_TO_IDLE);
            }
            else if (_isMovingRight && !_facingRight && _state != KnightState::TURNING)
            {
                changeState(KnightState::TURNING);
            }
        }
    }
    // 支持大小写 D/d 键
    else if (keyCode == EventKeyboard::KeyCode::KEY_D || 
             keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_D)
    {
        _isMovingRight = false;
        
        // 地图模式下的移动
        if (_isMapMode && _state == KnightState::MAP_WALKING)
        {
            if (!_isMovingLeft)
            {
                changeState(KnightState::MAP_IDLE);
            }
            else if (_isMovingLeft && _facingRight && _state != KnightState::MAP_TURNING)
            {
                changeState(KnightState::MAP_TURNING);
            }
        }
        else if (_isOnGround && _state == KnightState::RUNNING)
        {
            if (!_isMovingLeft)
            {
                changeState(KnightState::RUN_TO_IDLE);
            }
            else if (_isMovingLeft && _facingRight && _state != KnightState::TURNING)
            {
                changeState(KnightState::TURNING);
            }
        }
    }
    // 支持大小写 K/k 键 - 跳跃释放
    else if (keyCode == EventKeyboard::KeyCode::KEY_K ||
             keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_K)
    {
        _isJumpKeyPressed = false;
    }
    // 空格键释放
    else if (keyCode == EventKeyboard::KeyCode::KEY_SPACE)
    {
        // 如果正在Focus状态，松开空格键应该播放FocusEnd动画
        if (_state == KnightState::FOCUSING || _state == KnightState::FOCUS_GET)
        {
            // Focus动画完成后会检查_isSpaceKeyPressed来决定是否继续
            // 所以这里只需要标记按键已释放，让动画回调处理FocusEnd
            // 如果当前正在播放Focus动画，动画结束回调会检测到_isSpaceKeyPressed为false并播放FocusEnd
        }
        // 如果正在恢复状态且还没完成，取消恢复
        else if (_state == KnightState::RECOVERING)
        {
            cancelRecover();
        }
        // 如果按住时间小于阈值，是短按释放法术
        else if (_isSpaceKeyPressed && _spaceKeyHoldTime < _recoverHoldThreshold)
        {
            if (_soul >= _spellCost && 
                _state != KnightState::DASHING && 
                _state != KnightState::GET_ATTACKED &&
                _state != KnightState::DEAD &&
                _state != KnightState::CASTING_SPELL &&
                _state != KnightState::FOCUSING &&
                _state != KnightState::FOCUS_GET &&
                _state != KnightState::FOCUS_END &&
                !_isAttacking)
            {
                startCastSpell();
            }
        }
        _isSpaceKeyPressed = false;
    }
    // Tab键释放 - 关闭地图
    else if (keyCode == EventKeyboard::KeyCode::KEY_TAB)
    {
        _isMapKeyPressed = false;
        
        // If sitting with map open, close sit map
        if (_state == KnightState::SIT_MAP_OPEN)
        {
            changeState(KnightState::SIT_MAP_CLOSE);
        }
        // If in regular map mode, close map
        else if (_isMapMode && 
            (_state == KnightState::MAP_IDLE || _state == KnightState::MAP_WALKING || _state == KnightState::MAP_TURNING))
        {
            changeState(KnightState::MAP_CLOSING);
        }
    }
}

void TheKnight::update(float dt)
{
    // 调试：每隔1秒输出位置
    static float debugTimer = 0.0f;
    debugTimer += dt;
    if (debugTimer >= 1.0f)
    {
        Vec2 pos = this->getPosition();
        CCLOG("[TheKnight] Position: (%.1f, %.1f), Statement: %d, Face: %s, OnGround: %s", 
              pos.x, pos.y, (int)_state, 
              _facingRight ? "Right" : "Left",
              _isOnGround ? "1" : "0");
        debugTimer = 0.0f;
    }
    
    // 死亡状态不处理任何更新
    if (_state == KnightState::DEAD)
    {
        return;
    }
    
    // 处理坐下状态
    if (_isSitting)
    {
        updateSitting(dt);
        return;  // 坐着时不处理其他更新
    }
    
    // 更新空格键按住时间，并检查是否触发Focus
    if (_isSpaceKeyPressed)
    {
        _spaceKeyHoldTime += dt;
        
        // 按住超过阈值，开始Focus
        if (_spaceKeyHoldTime >= _recoverHoldThreshold && 
            _state != KnightState::FOCUSING &&
            _state != KnightState::FOCUS_GET &&
            _state != KnightState::FOCUS_END &&
            _state != KnightState::RECOVERING &&
            _state != KnightState::CASTING_SPELL)
        {
            if (_soul >= _focusCost && 
                _state != KnightState::DASHING && 
                _state != KnightState::GET_ATTACKED &&
                !_isAttacking)
            {
                startFocus();
            }
        }
    }
    
    // 更新蹬墙跳烟雾特效
    if (_wallJumpPuffEffect)
    {
        updateWallJumpPuffEffect(dt);
    }
    
    // 更新法术特效
    if (_vengefulSpiritEffect)
    {
        updateVengefulSpiritEffect(dt);
    }
    
    // 更新无敌状态
    if (_isInvincible)
    {
        _invincibleTimer -= dt;
        if (_invincibleTimer <= 0)
        {
            _isInvincible = false;
            this->setOpacity(255);  // 恢复完全不透明
        }
        else
        {
            // 无敌期间闪烁效果
            int flashPhase = (int)(_invincibleTimer * 10) % 2;
            this->setOpacity(flashPhase == 0 ? 128 : 255);
        }
    }
    
    // 处理受击状态
    if (_state == KnightState::GET_ATTACKED)
    {
        updateGetAttacked(dt);
        return;
    }
    
    // 更新冲刺冷却
    if (!_canDash)
    {
        _dashCooldownTimer += dt;
        if (_dashCooldownTimer >= _dashCooldown)
        {
            _canDash = true;
        }
    }
    
    // 处理冲刺
    if (_state == KnightState::DASHING)
    {
        updateDash(dt);
        return;  // 冲刺时不处理其他移动
    }
    
    // 处理攻击状态
    if (_state == KnightState::SLASHING)
    {
        updateSlash(dt);
        return;
    }
    else if (_state == KnightState::UP_SLASHING)
    {
        updateUpSlash(dt);
        return;
    }
    else if (_state == KnightState::DOWN_SLASHING)
    {
        updateDownSlash(dt);
        return;
    }
    // 处理贴墙攻击状态
    else if (_state == KnightState::WALL_SLASHING)
    {
        updateWallSlash(dt);
        return;
    }
    // 处理法术释放状态
    else if (_state == KnightState::CASTING_SPELL)
    {
        updateCastSpell(dt);
        return;
    }
    // 处理恢复状态
    else if (_state == KnightState::RECOVERING)
    {
        updateRecover(dt);
        return;
    }
    // 更新Focus状态
    else if (_state == KnightState::FOCUSING || _state == KnightState::FOCUS_GET)
    {
        updateFocus(dt);
        return;
    }
    
    // 更新贴墙下滑
    if (_state == KnightState::WALL_SLIDING)
    {
        updateWallSlide(dt);
        return;  // 贴墙时不处理其他移动
    }
    
    // 处理蹬墙跳
    if (_state == KnightState::WALL_JUMPING)
    {
        updateWallJump(dt);
        return;  // 蹬墙跳时不处理其他移动
    }
    
    // 处理二段跳
    if (_state == KnightState::DOUBLE_JUMPING)
    {
        updateDoubleJump(dt);
        return;  // 二段跳时不处理其他移动
    }
    
    // 处理跳跃和下落（这两个状态已经在各自的update函数中处理水平移动）
    if (_state == KnightState::JUMPING)
    {
        updateJump(dt);
        return;
    }
    else if (_state == KnightState::FALLING)
    {
        updateFall(dt);
        return;
    }
    else if (!_isOnGround)
    {
        // 如果不在地面且不是跳跃/下落状态，开始下落
        changeState(KnightState::FALLING);
        return;
    }
    
    // 只在非LANDING状态时检查是否还站在平台上
    // LANDING状态时不检查，避免边缘抽搐
    if (_isOnGround && _state != KnightState::LANDING && _state != KnightState::HARD_LANDING)
    {
        // 检查是否还站在平台上，如果走出平台边缘则开始下落
        if (!checkStillOnGround())
        {
            _isOnGround = false;
            _fallStartY = this->getPositionY();  // 记录下落起始位置
            // 如果处于地图模式，强制退出
            if (_isMapMode)
            {
                exitMapMode();
            }
            changeState(KnightState::FALLING);
            return;
        }
    }
    
    // 更新地面水平移动（只在RUNNING状态和MAP_WALKING状态）
    if (_state == KnightState::RUNNING || _state == KnightState::MAP_WALKING)
    {
        Vec2 pos = this->getPosition();
        float newX = pos.x;
        bool moved = false;
        
        // 计算实际移动速度（考虑Sprintmaster护符）
        float actualMoveSpeed = _moveSpeed;
        if (_charmSprintmaster)
        {
            actualMoveSpeed *= 1.2f;  // 增加20%移动速度
        }
        
        // 地图模式下移动速度减小到1/3
        if (_state == KnightState::MAP_WALKING)
        {
            actualMoveSpeed /= 3.0f;
        }
        
        if (_isMovingLeft)
        {
            newX -= actualMoveSpeed * dt;
            moved = true;
        }
        if (_isMovingRight)
        {
            newX += actualMoveSpeed * dt;
            moved = true;
        }
        
        if (moved)
        {
            // 检查墙壁碰撞
            pos.x = newX;
            this->setPosition(pos);
            
            float correctedX;
            if (_isMovingRight && checkWallCollision(correctedX, true))
            {
                pos.x = correctedX;
                this->setPosition(pos);
            }
            else if (_isMovingLeft && checkWallCollision(correctedX, false))
            {
                pos.x = correctedX;
                this->setPosition(pos);
            }
        }
    }
}
