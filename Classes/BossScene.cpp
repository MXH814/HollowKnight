#include "BossScene.h"
#include "CharmManager.h"
#include "SimpleAudioEngine.h"
#include "SettingsPanel.h"
#include "GeoManager.h"
#include "MainMenuScene.h"

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

    // 初始化摄像头偏移
    _cameraOffsetY = 0.0f;
    _targetCameraOffsetY = 0.0f;
    
    // 初始化攻击冷却
    _knightAttackCooldown = 0.0f;
    _spellAttackCooldown = 0.0f;
    
    // 初始化战斗结束状态
    _isBattleEnded = false;
    _isKnightDefeated = false;
    _isBossDefeated = false;
    _deathAnimTimer = 0.0f;
    _isPlayingDeathAnim = false;
    _rewardCollected = false;
    _isNearReward = false;  // 新增：是否在奖励拾取范围内

    // 添加键盘事件监听
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        // 如果战斗已结束且显示了结果界面，按任意键返回菜单
        if (_isBattleEnded && _resultLayer && _resultLayer->isVisible())
        {
            returnToMainMenu();
            return;
        }
        
        // 按S键拾取奖励
        if (keyCode == EventKeyboard::KeyCode::KEY_S)
        {
            if (_isNearReward && !_rewardCollected)
            {
                collectReward();
            }
        }
        
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

    _geoIcon = Sprite::create("Hp/Geo.png");
    if (_geoIcon)
    {
        _geoIcon->setPosition(Vec2(260, 900));
        _uiLayer->addChild(_geoIcon);
    }

    _lastDisplayedGeo = GeoManager::getInstance()->getGeo();
    _geoLabel = Label::createWithTTF(std::to_string(_lastDisplayedGeo), "fonts/NotoSerifCJKsc-Regular.otf", 50);
    if (_geoLabel)
    {
        _geoLabel->setTextColor(Color4B::WHITE);
        _geoLabel->setAnchorPoint(Vec2(0, 0.5f));
        _geoLabel->setPosition(Vec2(350, 900));
        _uiLayer->addChild(_geoLabel);
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

    int currentGeo = GeoManager::getInstance()->getGeo();
    if (_geoLabel && currentGeo != _lastDisplayedGeo)
    {
        _lastDisplayedGeo = currentGeo;
        _geoLabel->setString(std::to_string(currentGeo));

        // 添加数字跳动效果
        _geoLabel->stopAllActions();
        _geoLabel->setScale(1.3f);
        _geoLabel->runAction(ScaleTo::create(0.15f, 1.0f));
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

void BossScene::onKnightDefeated()
{
    if (_isBattleEnded) return;
    
    _isBattleEnded = true;
    _isKnightDefeated = true;
    
    CCLOG("Knight defeated! Showing defeat screen...");
    
    // 停止背景音乐
    SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    
    // 创建结果显示层
    Size visibleSize = Director::getInstance()->getVisibleSize();
    auto camera = this->getDefaultCamera();
    Vec2 camPos = camera->getPosition();
    
    _resultLayer = Node::create();
    _resultLayer->setPosition(camPos);
    this->addChild(_resultLayer, 2000);
    
    // 创建半透明黑色背景
    auto darkBg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    darkBg->setPosition(Vec2(-visibleSize.width / 2, -visibleSize.height / 2));
    _resultLayer->addChild(darkBg);
    
    // 创建"失败"大字
    auto defeatLabel = Label::createWithTTF(u8"失败", "fonts/NotoSerifCJKsc-Regular.otf", 120);
    defeatLabel->setTextColor(Color4B(200, 50, 50, 255));
    defeatLabel->setPosition(Vec2(0, 50));
    defeatLabel->setOpacity(0);
    _resultLayer->addChild(defeatLabel);
    
    // 创建"按任意键回到菜单"小字
    auto hintLabel = Label::createWithTTF(u8"按任意键回到菜单", "fonts/NotoSerifCJKsc-Regular.otf", 36);
    hintLabel->setTextColor(Color4B::WHITE);
    hintLabel->setPosition(Vec2(0, -50));
    hintLabel->setOpacity(0);
    _resultLayer->addChild(hintLabel);
    
    // 淡入动画
    defeatLabel->runAction(Sequence::create(
        DelayTime::create(0.3f),
        FadeIn::create(0.5f),
        nullptr
    ));
    
    hintLabel->runAction(Sequence::create(
        DelayTime::create(0.8f),
        FadeIn::create(0.5f),
        nullptr
    ));
}

void BossScene::onBossDefeated()
{
    if (_isBattleEnded) return;

    // 标记 Boss 已被击败，防止重复进入
    if (_isBossDefeated) return;
    _isBossDefeated = true;

    CCLOG("Boss defeated! Creating reward pickup...");

    // 立即断开对 Hornet 的悬挂引用，避免 removeFromParent 后悬空指针被再次访问导致崩溃
    _hornet = nullptr;

    // 隐藏Boss HP标签
    if (_bossHPLabel)
    {
        _bossHPLabel->setVisible(false);
    }

    // 延迟创建奖励拾取物（等Boss离场动画播放完）
    this->scheduleOnce([this](float dt) {
        createRewardPickup();
    }, 2.0f, "create_reward");
}

void BossScene::createRewardPickup()
{
    if (!_knight) return;
    
    // 在场景地面中间创建白色发光圆点
    Vec2 pickupPos = Vec2(1200.0f, 180.0f);
    
    // 创建简单的白色发光圆点
    auto drawNode = DrawNode::create();
    drawNode->drawDot(Vec2::ZERO, 30.0f, Color4F::WHITE);
    drawNode->setPosition(pickupPos);
    this->addChild(drawNode, 8, "RewardPickup");
    
    // 添加发光脉动效果
    auto glowAction = RepeatForever::create(Sequence::create(
        ScaleTo::create(0.6f, 1.4f),
        ScaleTo::create(0.6f, 1.0f),
        nullptr
    ));
    drawNode->runAction(glowAction);
    
    CCLOG("Created white glow pickup at scene center: (%.1f, %.1f)", pickupPos.x, pickupPos.y);
}

void BossScene::checkRewardPickup()
{
    if (_rewardCollected || !_knight) return;
    
    // 通过名称获取拾取物节点
    auto rewardNode = this->getChildByName("RewardPickup");
    if (!rewardNode) return;
    
    Vec2 knightPos = _knight->getPosition();
    Vec2 pickupPos = rewardNode->getPosition();
    
    float distance = knightPos.distance(pickupPos);
    float pickupRadius = 100.0f;
    
    bool wasNear = _isNearReward;
    _isNearReward = (distance < pickupRadius);
    
    // 获取或创建提示标签
    auto hintLabel = dynamic_cast<Label*>(this->getChildByName("PickupHint"));
    
    if (_isNearReward)
    {
        // 在范围内，显示"按S拾取"提示
        if (!hintLabel)
        {
            hintLabel = Label::createWithTTF(u8"按S拾取", "fonts/NotoSerifCJKsc-Regular.otf", 28);
            hintLabel->setTextColor(Color4B::WHITE);
            hintLabel->setName("PickupHint");
            this->addChild(hintLabel, 100);
        }
        // 提示显示在光点上方
        hintLabel->setPosition(Vec2(pickupPos.x, pickupPos.y + 80));
        hintLabel->setVisible(true);
    }
    else
    {
        // 不在范围内，隐藏提示
        if (hintLabel)
        {
            hintLabel->setVisible(false);
        }
    }
}

void BossScene::collectReward()
{
    if (_rewardCollected) return;
    _rewardCollected = true;
    
    // 获取光点位置
    auto rewardNode = this->getChildByName("RewardPickup");
    if (!rewardNode) return;
    
    Vec2 pickupPos = rewardNode->getPosition();
    
    // 隐藏"按S拾取"提示
    auto hintLabel = this->getChildByName("PickupHint");
    if (hintLabel)
    {
        hintLabel->removeFromParent();
    }
    
    // 停止光点动画并移除
    rewardNode->stopAllActions();
    rewardNode->runAction(Sequence::create(
        FadeOut::create(0.3f),
        RemoveSelf::create(),
        nullptr
    ));
    
    // 显示奖励界面
    showRewardAtPickup(pickupPos);
}

void BossScene::showRewardAtPickup(const Vec2& pickupPos)
{
    _isBattleEnded = true;
    
    CCLOG("Showing reward at pickup position: (%.1f, %.1f)", pickupPos.x, pickupPos.y);
    
    // 停止背景音乐
    SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    auto camera = this->getDefaultCamera();
    Vec2 camPos = camera->getPosition();
    
    // 创建结果显示层（跟随摄像机位置）
    _resultLayer = Node::create();
    _resultLayer->setPosition(camPos);
    this->addChild(_resultLayer, 2000);
    
    // 创建半透明黑色背景
    auto darkBg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    darkBg->setPosition(Vec2(-visibleSize.width / 2, -visibleSize.height / 2));
    _resultLayer->addChild(darkBg);
    
    // 计算光点相对于摄像机的位置（转换为结果层的本地坐标）
    Vec2 localPickupPos = pickupPos - camPos;
    
    // 在光点正上方显示蛾翼披风图片
    auto cloakSprite = Sprite::create("Hornet/MothwingCloak.png");
    if (cloakSprite)
    {
        // 位置在光点正上方 200 像素处
        cloakSprite->setPosition(Vec2(localPickupPos.x, localPickupPos.y + 250));
        cloakSprite->setScale(0.0f);
        cloakSprite->setOpacity(0);
        _resultLayer->addChild(cloakSprite);
        
        // 图片出现动画 - 放大到1.5倍
        cloakSprite->runAction(Sequence::create(
            DelayTime::create(0.2f),
            Spawn::create(
                FadeIn::create(0.4f),
                ScaleTo::create(0.4f, 1.5f),
                nullptr
            ),
            nullptr
        ));
    }
    
    // 创建"获得蛾翼披风"文字（在图片下方）
    auto rewardLabel = Label::createWithTTF(u8"获得了蛾翼披风", "fonts/NotoSerifCJKsc-Regular.otf", 42);
    rewardLabel->setTextColor(Color4B(255, 215, 0, 255)); // 金色
    rewardLabel->setPosition(Vec2(localPickupPos.x, localPickupPos.y + 80));
    rewardLabel->setOpacity(0);
    _resultLayer->addChild(rewardLabel);
    
    // 创建"按任意键回到菜单"文字（在上面文字下方）
    auto hintLabel = Label::createWithTTF(u8"按任意键回到菜单", "fonts/NotoSerifCJKsc-Regular.otf", 28);
    hintLabel->setTextColor(Color4B::WHITE);
    hintLabel->setPosition(Vec2(localPickupPos.x, localPickupPos.y + 30));
    hintLabel->setOpacity(0);
    _resultLayer->addChild(hintLabel);
    
    // 淡入动画
    rewardLabel->runAction(Sequence::create(
        DelayTime::create(0.5f),
        FadeIn::create(0.4f),
        nullptr
    ));
    
    hintLabel->runAction(Sequence::create(
        DelayTime::create(0.8f),
        FadeIn::create(0.4f),
        nullptr
    ));
    
    // 移除光点
    auto rewardNode = this->getChildByName("RewardPickup");
    if (rewardNode)
    {
        rewardNode->runAction(Sequence::create(
            FadeOut::create(0.3f),
            RemoveSelf::create(),
            nullptr
        ));
    }
}

void BossScene::returnToMainMenu()
{
    CCLOG("Returning to main menu...");

    // 停止背景音乐（立即）
    SimpleAudioEngine::getInstance()->stopBackgroundMusic();

    // 在当前场景上创建一个全屏黑色覆盖层（避免立即替换 scene 导致的双重 replace 问题）
    Size visibleSize = Director::getInstance()->getVisibleSize();
    auto camera = this->getDefaultCamera();
    Vec2 camPos = camera ? camera->getPosition() : Vec2(visibleSize.width/2, visibleSize.height/2);

    // 使用足够大的黑色 Layer 并把它放在摄像机位置，确保覆盖可视区
    auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 255), visibleSize.width, visibleSize.height);
    blackLayer->setIgnoreAnchorPointForPosition(false);
    blackLayer->setAnchorPoint(Vec2(0.5f, 0.5f));
    blackLayer->setPosition(camPos);
    this->addChild(blackLayer, 10000, "ReturnToMenuBlack");

    // 给黑屏一个短暂延迟然后一次性替换到主菜单（使用 TransitionFade）
    blackLayer->runAction(Sequence::create(
        DelayTime::create(0.5f),
        CallFunc::create([]() {
            auto menuScene = MainMenuScene::createScene();
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, menuScene, Color3B::BLACK));
        }),
        nullptr
    ));
}

void BossScene::update(float dt)
{
    updateCamera();
    updateHPAndSoulUI(dt);

    // 如果战斗已结束，只更新UI
    if (_isBattleEnded)
    {
        // 如果有奖励拾取物，检测拾取
        if (_isBossDefeated && !_rewardCollected && _rewardPickup)
        {
            checkRewardPickup();
        }
        return;
    }

    // 检测骑士死亡
    if (_knight && _knight->isDead() && !_isPlayingDeathAnim)
    {
        _isPlayingDeathAnim = true;
        _deathAnimTimer = 0.0f;
        CCLOG("Knight is dead, starting death animation timer...");
    }

    if (_isPlayingDeathAnim)
    {
        _deathAnimTimer += dt;
        // 死亡动画约1.3秒（13帧，每帧0.1秒）
        if (_deathAnimTimer >= 1.5f)
        {
            _isPlayingDeathAnim = false;
            onKnightDefeated();
            return;
        }
    }

    // 检测Boss被击败（离场）
    // 不再直接解引用 _hornet（可能已被 removeFromParent 导致悬空指针）
    if (!_isBossDefeated)
    {
        // 通过场景查找名为 "HornetBoss" 的子节点，若找不到说明已被移除/离场
        auto hornetNode = this->getChildByName("HornetBoss");
        if (!hornetNode)
        {
            CCLOG("Hornet node not found in scene -> treating as defeated");
            // 清理本地引用并进入 Boss 击败流程
            _hornet = nullptr;
            onBossDefeated();
        }
    }

    // 碰撞检测（只在未播放死亡动画时）
    if (!_isPlayingDeathAnim)
    {
        checkCombatCollisions();
    }

    // 如果Boss已被击败但奖励未收集，检测拾取
    if (_isBossDefeated && !_rewardCollected && _rewardPickup)
    {
        checkRewardPickup();
    }
}

void BossScene::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}