#include "BossScene.h"
#include "CharmManager.h"
#include "SimpleAudioEngine.h"
#include "SettingsPanel.h"

USING_NS_CC;
using namespace CocosDenshion;

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
    scale = 1.0f;
    _map = TMXTiledMap::create("Maps/Bossroom.tmx");
    CCASSERT(_map != nullptr, "TMX 地图加载失败");

    _map->setAnchorPoint(Vec2::ZERO);
    _map->setPosition(origin);
    _map->setScale(scale);
    this->addChild(_map, 0);
    
    // 保存地图尺寸
    auto mapContentSize = _map->getContentSize();
    _mapSize = Size(mapContentSize.width * scale, mapContentSize.height * scale);

    // 解析碰撞层
    parseCollisionLayer();

    // 获取玩家起始位置
    auto objectGroup = _map->getObjectGroup("Objects");
    CCASSERT(objectGroup != nullptr, "地图缺少对象层 Objects");

    auto startPoint = objectGroup->getObject("PlayerStart");
    float startX = startPoint["x"].asFloat() * scale;
    float startY = startPoint["y"].asFloat() * scale;

    // 创建小骑士
    _knight = TheKnight::create();
    if (_knight)
    {
        _knight->setPosition(Vec2(startX, startY));
        _knight->setScale(1.0f);
        _knight->setPlatforms(_platforms);
        
        // 进入Boss场景时重置：HP满，Soul为0
        _knight->setHP(_knight->getMaxHP());
        _knight->setSoul(0);
        
        // 同步护符状态到玩家
        CharmManager::getInstance()->syncToKnight(_knight);
        
        this->addChild(_knight, 10, "PlayerInstance");
    }

    // 创建 Hornet Boss
    _hornet = HornetBoss::createWithFolder("Hornet");
    if (_hornet)
    {
        float groundY = 180.0f;
        float minX = 500.0f;
        float maxX = 1900.0f;
        
        _hornet->setBoundaries(groundY, minX, maxX);
        _hornet->setName("HornetBoss");
        this->addChild(_hornet, 9);
        
        if (_knight)
        {
            _hornet->startAI(_knight);
        }
        else
        {
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

    // 创建HP和Soul UI
    createHPAndSoulUI();
    
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

    // 添加键盘事件监听 (Q键打开护符面板)
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        if (keyCode == EventKeyboard::KeyCode::KEY_Q)
        {
            auto charmManager = CharmManager::getInstance();
            if (charmManager->isPanelOpen())
            {
                charmManager->hideCharmPanel();
                // 关闭面板后同步护符状态到玩家
                if (_knight)
                {
                    charmManager->syncToKnight(_knight);
                }
            }
            else
            {
                charmManager->showCharmPanel(this);
            }
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

    // 直接移除黑层
    if (blackLayer && blackLayer->getParent()) {
        blackLayer->removeFromParent();
    }
    
    CCLOG("Map Size: %f x %f", _mapSize.width, _mapSize.height);
    CCLOG("Screen Size: %f x %f", visibleSize.width, visibleSize.height);

    // 启用update
    this->scheduleUpdate();

    SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    SimpleAudioEngine::getInstance()->playBackgroundMusic("Music/Greenpath.wav", true);

    return true;
}

void BossScene::createHPAndSoulUI()
{
    if (!_knight) return;
    
    // 创建UI层
    _uiLayer = Node::create();
    if (!_uiLayer) return;
    this->addChild(_uiLayer, 1000);
    
    // 血条背景
    _hpBg = Sprite::create("Hp/hpbg.png");
    if (_hpBg)
    {
        _hpBg->setPosition(Vec2(200, 950));
        _uiLayer->addChild(_hpBg);
    }
    
    // 初始化血量显示
    _lastDisplayedHP = _knight->getHP();
    _lastDisplayedSoul = -1;  // 初始化为-1，确保第一次更新时会触发
    
    // 灵魂背景 - Boss场景初始Soul为0，使用soul_1作为默认图像但隐藏
    int currentSoul = _knight->getSoul();
    
    _soulBg = Sprite::create("Hp/soul_1_0.png");  // 使用soul_1作为默认
    if (_soulBg)
    {
        _soulBg->setScale(0.9f);
        _soulBg->setPosition(Vec2(152, 935));
        _uiLayer->addChild(_soulBg);
        
        // Soul为0时隐藏
        if (currentSoul <= 0)
        {
            _soulBg->setVisible(false);
        }
        else
        {
            int soulLevel = currentSoul;
            if (soulLevel > 6) soulLevel = 6;
            
            Vector<SpriteFrame*> soulFrames;
            for (int i = 0; i <= 2; i++) {
                std::string frameName = "Hp/soul_" + std::to_string(soulLevel) + "_" + std::to_string(i) + ".png";
                auto texture = Director::getInstance()->getTextureCache()->addImage(frameName);
                if (texture) {
                    auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height));
                    if (frame) {
                        soulFrames.pushBack(frame);
                    }
                }
            }
            
            if (!soulFrames.empty())
            {
                auto soulAnimation = Animation::createWithSpriteFrames(soulFrames, 0.25f);
                auto soulAnimate = Animate::create(soulAnimation);
                _soulBg->runAction(RepeatForever::create(soulAnimate));
            }
        }
        
        _lastDisplayedSoul = currentSoul;
    }
    
    int maxHp = _knight->getMaxHP();
    float gap = 50;
    
    // 创建血量图标
    for (int i = 0; i < maxHp; i++)
    {
        auto hpBar = Sprite::create("Hp/hp1.png");
        if (hpBar)
        {
            hpBar->setPosition(Vec2(260 + i * gap, 980));
            hpBar->setScale(0.5f);
            hpBar->setVisible(i < _lastDisplayedHP);
            _uiLayer->addChild(hpBar);
            _hpBars.push_back(hpBar);
        }
    }
    
    // 失去血量图标
    _hpLose = Sprite::create("Hp/hp8.png");
    if (_hpLose)
    {
        _hpLose->setPosition(Vec2(260 + _lastDisplayedHP * gap, 978));
        _hpLose->setScale(0.5f);
        _hpLose->setVisible(_lastDisplayedHP < maxHp);
        _uiLayer->addChild(_hpLose);
    }
}

void BossScene::updateHPAndSoulUI(float dt)
{
    if (!_knight || !_uiLayer) return;
    
    // 更新UI层位置跟随摄像机
    auto camera = this->getDefaultCamera();
    if (camera)
    {
        Vec2 camPos = camera->getPosition();
        Size visibleSize = Director::getInstance()->getVisibleSize();
        _uiLayer->setPosition(Vec2(camPos.x - visibleSize.width / 2, camPos.y - visibleSize.height / 2));
        
        // 更新Boss HP标签位置
        if (_bossHPLabel)
        {
            _bossHPLabel->setPosition(Vec2(camPos.x, camPos.y - visibleSize.height / 2 + 50));
        }
    }
    
    int currentHP = _knight->getHP();
    int currentSoul = _knight->getSoul();
    int maxHp = _knight->getMaxHP();
    float gap = 50;
    
    // 更新血量显示
    if (currentHP != _lastDisplayedHP)
    {
        for (int i = 0; i < (int)_hpBars.size(); i++)
        {
            _hpBars[i]->setVisible(i < currentHP);
        }
        
        if (_hpLose)
        {
            _hpLose->setPosition(Vec2(260 + currentHP * gap, 978));
            _hpLose->setVisible(currentHP < maxHp);
        }
        
        _lastDisplayedHP = currentHP;
    }
    
    // 更新灵魂显示
    if (_soulBg && currentSoul != _lastDisplayedSoul)
    {
        _lastDisplayedSoul = currentSoul;
        
        _soulBg->stopAllActions();
        
        // Soul为0时隐藏，否则显示对应等级的动画
        if (currentSoul <= 0)
        {
            _soulBg->setVisible(false);
        }
        else
        {
            _soulBg->setVisible(true);
            
            // Soul值1-6对应资源文件soul_1到soul_6
            int soulLevel = currentSoul;
            if (soulLevel > 6) soulLevel = 6;
            
            Vector<SpriteFrame*> soulFrames;
            for (int i = 0; i <= 2; i++) {
                std::string frameName = "Hp/soul_" + std::to_string(soulLevel) + "_" + std::to_string(i) + ".png";
                auto texture = Director::getInstance()->getTextureCache()->addImage(frameName);
                if (texture) {
                    auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height));
                    if (frame) {
                        soulFrames.pushBack(frame);
                    }
                }
            }
            
            if (!soulFrames.empty())
            {
                auto soulAnimation = Animation::createWithSpriteFrames(soulFrames, 0.25f);
                auto soulAnimate = Animate::create(soulAnimation);
                _soulBg->runAction(RepeatForever::create(soulAnimate));
            }
        }
    }
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
        
        float x = dict["x"].asFloat() * scale;
        float y = dict["y"].asFloat() * scale;
        float width = dict["width"].asFloat() * scale;
        float height = dict["height"].asFloat() * scale;
        
        Platform platform;
        platform.rect = Rect(x, y, width, height);
        platform.node = nullptr;
        _platforms.push_back(platform);
        
        std::string name = dict["name"].asString();
        CCLOG("创建碰撞平台: %s - x:%.1f y:%.1f w:%.1f h:%.1f", 
              name.c_str(), x, y, width, height);
    }
    
    CCLOG("创建了 %zu 个碰撞平台", _platforms.size());
}

void BossScene::updateCamera()
{
    if (!_knight) return;
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 knightPos = _knight->getPosition();
    
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
    
    float offsetLerpFactor = 0.05f;
    _cameraOffsetY += (_targetCameraOffsetY - _cameraOffsetY) * offsetLerpFactor;
    
    float cameraX = knightPos.x;
    float cameraY = knightPos.y + _cameraOffsetY;
    
    cameraX = std::max(cameraX, visibleSize.width / 2);
    cameraX = std::min(cameraX, _mapSize.width - visibleSize.width / 2);
    
    cameraY = std::max(cameraY, visibleSize.height / 2);
    cameraY = std::min(cameraY, _mapSize.height - visibleSize.height / 2);
    
    auto camera = this->getDefaultCamera();
    Vec2 currentCamPos = camera->getPosition();
    float lerpFactor = 0.1f;
    
    float newX = currentCamPos.x + (cameraX - currentCamPos.x) * lerpFactor;
    float newY = currentCamPos.y + (cameraY - currentCamPos.y) * lerpFactor;
    
    camera->setPosition(Vec2(newX, newY));
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
        
        Rect bossRect = _hornet->getBossHitRect();
        if (knightRect.intersectsRect(bossRect))
        {
            knightHit = true;
        }
        
        if (!knightHit)
        {
            Rect weaponRect = _hornet->getWeaponRect();
            if (weaponRect.size.width > 0 && knightRect.intersectsRect(weaponRect))
            {
                knightHit = true;
            }
        }
        
        if (!knightHit)
        {
            Rect attack4Rect = _hornet->getAttack4Rect();
            if (attack4Rect.size.width > 0 && knightRect.intersectsRect(attack4Rect))
            {
                knightHit = true;
            }
        }
        
        if (knightHit)
        {
            bool fromRight = (_hornet->getPositionX() > _knight->getPositionX());
            _knight->setKnockbackDirection(fromRight);
            _knight->takeDamage(1);
        }
    }
    
    // ========== 2. 检测TheKnight对Hornet的伤害 ==========
    Rect bossHurtRect = _hornet->getBossHitRect();
    
    float dt = Director::getInstance()->getDeltaTime();
    if (_knightAttackCooldown > 0)
    {
        _knightAttackCooldown -= dt;
    }
    if (_spellAttackCooldown > 0)
    {
        _spellAttackCooldown -= dt;
    }
    
    if (_knightAttackCooldown <= 0)
    {
        Rect slashRect;
        if (_knight->getSlashEffectBoundingBox(slashRect))
        {
            if (slashRect.intersectsRect(bossHurtRect))
            {
                _hornet->onDamaged();
                
                int soulGain = 1;
                if (_knight->getCharmSoulCatcher())
                {
                    soulGain += 1;
                }
                _knight->addSoul(soulGain);
                
                _knight->bounceFromDownSlash();
                
                _knightAttackCooldown = 0.3f;
            }
        }
    }
    
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
                _hornet->onDamaged();
                if (_knight->getCharmShamanStone())
                {
                    _hornet->onDamaged();
                }
                
                _spellAttackCooldown = 0.2f;
            }
        }
    }
}

void BossScene::update(float dt)
{
    updateCamera();
    updateHPAndSoulUI(dt);
    
    // 碰撞检测
    checkCombatCollisions();
}

void BossScene::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}