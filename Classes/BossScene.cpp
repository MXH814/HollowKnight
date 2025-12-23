#include "BossScene.h"

USING_NS_CC;

Scene* BossScene::createScene()
{
    return BossScene::create();
}

bool BossScene::init()
{
    if (!Scene::init())
        return false;

    // 获取屏幕尺寸
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 在加载地图之前添加全黑遮罩层
    auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 255));
    this->addChild(blackLayer, 10, "LoadingBlack");

    // 加载 TMX 地图
    _mapScale = 1.0f;  // 不缩放，与原项目保持一致
    _map = TMXTiledMap::create("Maps/Bossroom.tmx");
    CCASSERT(_map != nullptr, "TMX 地图加载失败");

    // 【关键点】锚点和位置必须是 (0,0)
    _map->setAnchorPoint(Vec2::ZERO);
    _map->setPosition(origin);
    _map->setScale(_mapScale);
    this->addChild(_map, 0);
    
    // 计算地图尺寸
    auto mapContentSize = _map->getContentSize();
    _mapSize = Size(mapContentSize.width * _mapScale, mapContentSize.height * _mapScale);

    // 解析碰撞层
    parseCollisionLayer();

    // 获取玩家起始位置
    auto objectGroup = _map->getObjectGroup("Objects");
    CCASSERT(objectGroup != nullptr, "地图缺少对象层 Objects");

    auto startPoint = objectGroup->getObject("PlayerStart");
    float startX = startPoint["x"].asFloat() * _mapScale;
    float startY = startPoint["y"].asFloat() * _mapScale;

    // 创建小骑士
    _knight = TheKnight::create();
    if (_knight)
    {
        _knight->setPosition(Vec2(startX, startY));
        _knight->setScale(1.0f);
        _knight->setPlatforms(_platforms);
        this->addChild(_knight, 10, "PlayerInstance");
    }

    // 创建 Hornet Boss
    _hornet = HornetBoss::createWithFolder("Hornet");
    if (_hornet)
    {
        // 【关键】使用原项目中固定的边界值
        // groundY = 180.0f, minX = 500.0f, maxX = 1900.0f
        float groundY = 180.0f;
        float minX = 500.0f;
        float maxX = 1900.0f;
        
        _hornet->setBoundaries(groundY, minX, maxX);
        _hornet->setName("HornetBoss");
        this->addChild(_hornet, 9);
        
        // 启动Boss AI，传入玩家节点
        if (_knight)
        {
            _hornet->startAI(_knight);
        }
        else
        {
            // 保险起见，如果没有玩家也让他先入场
            _hornet->playEntryAnimation(600, 800);
        }
        
        CCLOG("Boss边界设置: 地面Y=%.1f, 最小X=%.1f, 最大X=%.1f", groundY, minX, maxX);
    }

    // 添加关闭按钮
    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(BossScene::menuCloseCallback, this));

    if (closeItem != nullptr &&
        closeItem->getContentSize().width > 0 &&
        closeItem->getContentSize().height > 0)
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
        float y = origin.y + closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));
    }

    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100);

    // 创建HP显示标签
    _hpLabel = Label::createWithTTF("HP: 5", "fonts/Marker Felt.ttf", 32);
    if (_hpLabel)
    {
        _hpLabel->setAnchorPoint(Vec2(0, 1));
        _hpLabel->setPosition(Vec2(origin.x + 20, origin.y + visibleSize.height - 20));
        _hpLabel->setTextColor(Color4B(255, 255, 255, 255));
        this->addChild(_hpLabel, 100);
    }
    
    // 创建Soul显示标签
    _soulLabel = Label::createWithTTF("Soul: 0/6", "fonts/Marker Felt.ttf", 32);
    if (_soulLabel)
    {
        _soulLabel->setAnchorPoint(Vec2(1, 1));
        _soulLabel->setPosition(Vec2(origin.x + visibleSize.width - 20, origin.y + visibleSize.height - 20));
        _soulLabel->setTextColor(Color4B(200, 200, 255, 255));
        this->addChild(_soulLabel, 100);
    }
    
    // 创建Boss HP显示标签
    _bossHPLabel = Label::createWithTTF("HORNET", "fonts/Marker Felt.ttf", 28);
    if (_bossHPLabel)
    {
        _bossHPLabel->setAnchorPoint(Vec2(0.5f, 0));
        _bossHPLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 20));
        _bossHPLabel->setTextColor(Color4B(255, 100, 100, 255));
        this->addChild(_bossHPLabel, 100);
    }

    // 初始化摄像机偏移
    _cameraOffsetY = 0.0f;
    _targetCameraOffsetY = 0.0f;
    
    // 初始化攻击冷却
    _knightAttackCooldown = 0.0f;
    _spellAttackCooldown = 0.0f;

    // 直接移除黑层
    if (blackLayer && blackLayer->getParent()) {
        blackLayer->removeFromParent();
    }
    
    // 输出地图信息（调试用）
    CCLOG("Map Size: %f x %f", _mapSize.width, _mapSize.height);
    CCLOG("Screen Size: %f x %f", visibleSize.width, visibleSize.height);

    // 启用update
    this->scheduleUpdate();

    return true;
}

void BossScene::parseCollisionLayer()
{
    auto collisionGroup = _map->getObjectGroup("Collision");
    if (!collisionGroup)
    {
        CCLOG("警告：地图缺少 Collision 对象层");
        return;
    }
    
    auto& objects = collisionGroup->getObjects();
    
    for (auto& obj : objects)
    {
        ValueMap& dict = obj.asValueMap();
        
        float x = dict["x"].asFloat() * _mapScale;
        float y = dict["y"].asFloat() * _mapScale;
        float width = dict["width"].asFloat() * _mapScale;
        float height = dict["height"].asFloat() * _mapScale;
        
        Platform platform;
        platform.rect = Rect(x, y, width, height);
        platform.node = nullptr;
        _platforms.push_back(platform);
        
        std::string name = dict["name"].asString();
        CCLOG("创建碰撞平台: %s - x:%.1f y:%.1f w:%.1f h:%.1f", 
              name.c_str(), x, y, width, height);
    }
    
    CCLOG("共创建 %zu 个碰撞平台", _platforms.size());
}

void BossScene::updateCamera()
{
    if (!_knight) return;
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 knightPos = _knight->getPosition();
    
    // 根据看向状态调整目标偏移
    float lookOffset = 150.0f;
    if (_knight->isLookingUp())
    {
        _targetCameraOffsetY = lookOffset;
    }
    else if (_knight->isLookingDown())
    {
        _targetCameraOffsetY = -lookOffset;
    }
    else
    {
        _targetCameraOffsetY = 0.0f;
    }
    
    // 平滑插值看向偏移
    float offsetLerpFactor = 0.05f;
    _cameraOffsetY += (_targetCameraOffsetY - _cameraOffsetY) * offsetLerpFactor;
    
    // 计算摄像机目标位置（骑士在屏幕中心）
    float cameraX = knightPos.x;
    float cameraY = knightPos.y + _cameraOffsetY;
    
    // 限制摄像机范围，不要超出地图边界
    cameraX = std::max(cameraX, visibleSize.width / 2);
    cameraX = std::min(cameraX, _mapSize.width - visibleSize.width / 2);
    
    cameraY = std::max(cameraY, visibleSize.height / 2);
    cameraY = std::min(cameraY, _mapSize.height - visibleSize.height / 2);
    
    // 平滑移动摄像机
    auto camera = this->getDefaultCamera();
    Vec2 currentCamPos = camera->getPosition();
    float lerpFactor = 0.1f;
    
    float newX = currentCamPos.x + (cameraX - currentCamPos.x) * lerpFactor;
    float newY = currentCamPos.y + (cameraY - currentCamPos.y) * lerpFactor;
    
    camera->setPosition(Vec2(newX, newY));
    
    // 更新UI位置（UI需要跟随摄像机）
    if (_hpLabel)
    {
        _hpLabel->setPosition(Vec2(newX - visibleSize.width / 2 + 20, 
                                   newY + visibleSize.height / 2 - 20));
    }
    if (_soulLabel)
    {
        _soulLabel->setPosition(Vec2(newX + visibleSize.width / 2 - 20, 
                                     newY + visibleSize.height / 2 - 20));
    }
    if (_bossHPLabel)
    {
        _bossHPLabel->setPosition(Vec2(newX, newY - visibleSize.height / 2 + 50));
    }
}

void BossScene::updateHPLabel()
{
    if (_hpLabel && _knight)
    {
        char hpText[32];
        sprintf(hpText, "HP: %d", _knight->getHP());
        _hpLabel->setString(hpText);
    }
}

void BossScene::updateSoulLabel()
{
    if (_soulLabel && _knight)
    {
        char soulText[32];
        sprintf(soulText, "Soul: %d/%d", _knight->getSoul(), _knight->getMaxSoul());
        _soulLabel->setString(soulText);
    }
}

void BossScene::checkCombatCollisions()
{
    if (!_knight || !_hornet) return;
    if (_knight->isDead()) return;
    
    // ========== 1. 检测Hornet对TheKnight的伤害 ==========
    if (!_knight->isInvincible() && !_knight->isStunned())
    {
        Rect knightRect = _knight->getBoundingBox();
        bool knightHit = false;
        
        // 检测Boss本体碰撞
        Rect bossRect = _hornet->getBossHitRect();
        if (knightRect.intersectsRect(bossRect))
        {
            knightHit = true;
        }
        
        // 检测投掷武器碰撞（Attack2）
        if (!knightHit)
        {
            Rect weaponRect = _hornet->getWeaponRect();
            if (weaponRect.size.width > 0 && knightRect.intersectsRect(weaponRect))
            {
                knightHit = true;
            }
        }
        
        // 检测乱舞攻击碰撞（Attack4）
        if (!knightHit)
        {
            Rect attack4Rect = _hornet->getAttack4Rect();
            if (attack4Rect.size.width > 0 && knightRect.intersectsRect(attack4Rect))
            {
                knightHit = true;
            }
        }
        
        // 如果被击中
        if (knightHit)
        {
            // 设置后退方向（根据Boss位置）
            bool fromRight = (_hornet->getPositionX() > _knight->getPositionX());
            _knight->setKnockbackDirection(fromRight);
            _knight->takeDamage(1);
        }
    }
    
    // ========== 2. 检测TheKnight对Hornet的伤害 ==========
    Rect bossHurtRect = _hornet->getBossHitRect();
    
    // 更新冷却计时器
    float dt = Director::getInstance()->getDeltaTime();
    if (_knightAttackCooldown > 0)
    {
        _knightAttackCooldown -= dt;
    }
    if (_spellAttackCooldown > 0)
    {
        _spellAttackCooldown -= dt;
    }
    
    // 检测骑士斩击（Slash）碰撞
    if (_knightAttackCooldown <= 0)
    {
        Rect slashRect;
        if (_knight->getSlashEffectBoundingBox(slashRect))
        {
            if (slashRect.intersectsRect(bossHurtRect))
            {
                _hornet->onDamaged();
                
                // 增加灵魂值（考虑灵魂捕手护符）
                int soulGain = 1;
                if (_knight->getCharmSoulCatcher())
                {
                    soulGain += 1;
                }
                _knight->addSoul(soulGain);
                
                // 下劈命中敌人后弹反
                _knight->bounceFromDownSlash();
                
                // 设置攻击冷却，防止一次攻击多次伤害
                _knightAttackCooldown = 0.3f;
            }
        }
    }
    
    // 检测法术（Vengeful Spirit）碰撞
    if (_spellAttackCooldown <= 0)
    {
        Sprite* spellEffect = _knight->getVengefulSpiritEffect();
        if (spellEffect)
        {
            auto effectSize = spellEffect->getContentSize();
            auto effectPos = spellEffect->getPosition();
            Rect spellRect(effectPos.x - effectSize.width / 2,
                           effectPos.y - effectSize.height / 2,
                           effectSize.width,
                           effectSize.height);
            
            if (spellRect.intersectsRect(bossHurtRect))
            {
                // 法术伤害（考虑萨满之石护符）
                _hornet->onDamaged();
                if (_knight->getCharmShamanStone())
                {
                    // 萨满之石额外伤害
                    _hornet->onDamaged();
                }
                
                // 设置法术冷却
                _spellAttackCooldown = 0.2f;
            }
        }
    }
}

void BossScene::update(float dt)
{
    updateCamera();
    updateHPLabel();
    updateSoulLabel();
    
    // 碰撞检测
    checkCombatCollisions();
}

void BossScene::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}