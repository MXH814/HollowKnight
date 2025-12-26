#include "GameScene.h"
#include "TheKnight.h"
#include "NextScene.h"
#include <BossScene.h>
#include "CharmManager.h"
#include "Monster/CrawlidMonster.h"

USING_NS_CC;

// 静态变量初始化
bool GameScene::s_hasCustomSpawn = false;
Vec2 GameScene::s_customSpawnPos = Vec2::ZERO;
bool GameScene::s_spawnFacingRight = true;
bool GameScene::s_spawnDoJump = false;

Scene* GameScene::createScene()
{
    s_hasCustomSpawn = false;
    s_customSpawnPos = Vec2::ZERO;
    s_spawnFacingRight = true;
    s_spawnDoJump = false;
    
    return GameScene::create();
}

Scene* GameScene::createSceneWithSpawn(const Vec2& spawnPos, bool facingRight)
{
    s_hasCustomSpawn = true;
    s_customSpawnPos = spawnPos;
    s_spawnFacingRight = facingRight;
    s_spawnDoJump = true;
    
    CCLOG("GameScene::createSceneWithSpawn - 设置自定义出生点: pos(%.1f, %.1f), facingRight=%d", 
          spawnPos.x, spawnPos.y, facingRight);
    
    return GameScene::create();
}

bool GameScene::init()
{
    if (!Scene::init())
        return false;

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto blackLayer1 = LayerColor::create(Color4B(0, 0, 0, 255));
    this->addChild(blackLayer1, 10, "LoadingBlack");

    struct MapChunk {
        std::string file;
        Vec2 position;
    };

    scale = 2.61f;

    std::vector<MapChunk> chunks = {
        {"Maps/Dirtmouth1.tmx", Vec2(0, 0)},
        {"Maps/Dirtmouth2.tmx", Vec2(150 * 16, 0)},
        {"Maps/Dirtmouth3.tmx", Vec2(290 * 16, 0)}
    };

    float totalMapWidth = 0.0f;
    float maxMapHeight = 0.0f;

    for (const auto& chunk : chunks) {
        auto map = TMXTiledMap::create(chunk.file);
        CCASSERT(map != nullptr, ("地图加载失败: " + chunk.file).c_str());

        map->setScale(scale);
        map->setAnchorPoint(Vec2::ZERO);
        Vec2 mapPos = Vec2(origin.x + chunk.position.x * scale, 
                           origin.y + chunk.position.y * scale);
        map->setPosition(mapPos);
        this->addChild(map, 0);

        auto mapContentSize = map->getContentSize();
        float mapRight = mapPos.x + mapContentSize.width * scale;
        float mapTop = mapPos.y + mapContentSize.height * scale;
        totalMapWidth = std::max(totalMapWidth, mapRight);
        maxMapHeight = std::max(maxMapHeight, mapTop);

        createCollisionFromTMX(map, "Collision", scale, mapPos);
        loadInteractiveObjects(map, scale, mapPos);
        loadForegroundObjects(map, scale, mapPos);
    }
    
    _mapSize = Size(totalMapWidth, maxMapHeight);

    auto firstMap = TMXTiledMap::create("Maps/Dirtmouth2.tmx");
    auto objectGroup = firstMap->getObjectGroup("Objects");
    CCASSERT(objectGroup != nullptr, "地图缺少对象层 Objects");

    auto startPoint = objectGroup->getObject("PlayerStart");
    float mapOffsetX = 150 * 16 * scale;
    float startX = startPoint["x"].asFloat() * scale + mapOffsetX;
    float startY = startPoint["y"].asFloat() * scale;

    _knight = TheKnight::create();
    if (_knight)
    {
        if (s_hasCustomSpawn)
        {
            CCLOG("使用自定义出生点位置: (%.1f, %.1f), 朝向: %s", 
                  s_customSpawnPos.x, s_customSpawnPos.y, 
                  s_spawnFacingRight ? "右" : "左");
            
            _knight->setPosition(s_customSpawnPos);
            float scaleValue = s_spawnFacingRight ? 1.0f : -1.0f;
            _knight->setScaleX(scaleValue);
            _knight->setScaleY(1.0f);
        }
        else
        {
            _knight->setPosition(Vec2(startX, startY));
            _knight->setScale(1.0f);
        }
        
        _knight->setPlatforms(_platforms);
        this->addChild(_knight, 5, "Player");
        
        CharmManager::getInstance()->syncToKnight(_knight);
        
        if (s_hasCustomSpawn && s_spawnDoJump)
        {
            float horizontalSpeed = s_spawnFacingRight ? 1.0f : -1.0f;
            _knight->triggerJumpFromExternal(horizontalSpeed);
            
            CCLOG("玩家从NextScene返回：位置(%.1f, %.1f)，朝向%s，触发跳跃动作", 
                  s_customSpawnPos.x, s_customSpawnPos.y, 
                  s_spawnFacingRight ? "右" : "左");
        }
        
        s_hasCustomSpawn = false;
        s_customSpawnPos = Vec2::ZERO;
        s_spawnFacingRight = true;
        s_spawnDoJump = false;
    }

    _interactionLabel = Label::createWithSystemFont(u8"休息", "fonts/ZCOOLXiaoWei-Regular.ttf", 24);
    _interactionLabel->setTextColor(Color4B::WHITE);
    _interactionLabel->setVisible(false);
    this->addChild(_interactionLabel, 100, "InteractionLabel");

    createHPAndSoulUI();

    _cameraOffsetY = 0.0f;
    _targetCameraOffsetY = 0.0f;
    _wasSitting = false;

    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        if (keyCode == EventKeyboard::KeyCode::KEY_Q)
        {
            auto charmManager = CharmManager::getInstance();
            if (charmManager->isPanelOpen())
            {
                charmManager->hideCharmPanel();
                if (_knight)
                {
                    charmManager->syncToKnight(_knight);
                }
            }
            else
            {
                bool canEquip = _knight && _knight->isSitting();
                charmManager->showCharmPanel(this, canEquip);
            }
            return;
        }
        
        if (_knight && _knight->isNearChair() && !_knight->isSitting())
        {
            if (keyCode == EventKeyboard::KeyCode::KEY_W || 
                keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_W)
            {
                _knight->startSitting();
            }
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

    this->scheduleUpdate();

    if (blackLayer1 && blackLayer1->getParent()) {
        blackLayer1->removeFromParent();
    }
    
    CCLOG("共创建 %zu 个碰撞平台", _platforms.size());

    return true;
}

void GameScene::createHPAndSoulUI()
{
    if (!_knight) return;
    
    // 创建UI层（固定在屏幕上，不随场景移动）
//     _uiLayer = Node::create();
//     if (!_uiLayer) return;
//     this->addChild(_uiLayer, 1000);  // 最高层级
    
    // 创建血条背景
    _hpBg = Sprite::create("Hp/hpbg.png");
    if (_hpBg)
    {
        _hpBg->setPosition(Vec2(200, 950));
        this->addChild(_hpBg, 1001);
    }
    
    // 初始化血量和灵魂显示
    _lastDisplayedHP = _knight->getHP();
    _lastDisplayedSoul = -1;  // 初始化为-1，确保第一次更新时会触发动画
    
    // 灵魂背景 - 根据当前灵魂值选择对应资源
    int currentSoul = _knight->getSoul();
    int soulLevel = currentSoul;
    if (soulLevel > 6) soulLevel = 6;
    if (soulLevel < 1) soulLevel = 1;  // 最小为1，因为没有soul_0资源
    
    std::string soulImage = "Hp/soul_" + std::to_string(soulLevel) + "_0.png";
    _soulBg = Sprite::create(soulImage);
    if (_soulBg)
    {
        _soulBg->setScale(0.9f);
        _soulBg->setPosition(Vec2(152, 935));
        this->addChild(_soulBg, 1001);
        
        // 如果Soul为0，隐藏灵魂显示
        if (currentSoul <= 0)
        {
            _soulBg->setVisible(false);
        }
        else
        {
            // 初始化灵魂动画
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
            hpBar->setVisible(i < _lastDisplayedHP);  // 只显示当前血量
            this->addChild(hpBar, 1001);
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
        this->addChild(_hpLose, 1001);
    }
}

void GameScene::updateHPAndSoulUI(float dt)
{
    if (!_knight || !_uiLayer) return;
    
    // 更新UI层位置跟随摄像机
    auto camera = this->getDefaultCamera();
    if (camera)
    {
        Vec2 camPos = camera->getPosition();
        Size visibleSize = Director::getInstance()->getVisibleSize();
        // UI层左上角对齐
        _uiLayer->setPosition(Vec2(camPos.x - visibleSize.width / 2, camPos.y - visibleSize.height / 2));
    }
    
    // 更新血量恢复动画
    if (_isHPRecovering)
    {
        updateHPRecoveryAnimation(dt);
    }
    
    int currentHP = _knight->getHP();
    int currentSoul = _knight->getSoul();
    int maxHp = _knight->getMaxHP();
    float gap = 50;
    
    // 如果不在恢复动画中，正常更新血量显示
    if (!_isHPRecovering && currentHP != _lastDisplayedHP)
    {
        // 更新血量图标显示
        for (int i = 0; i < (int)_hpBars.size(); i++)
        {
            _hpBars[i]->setVisible(i < currentHP);
        }
        
        // 更新失去血量图标位置
        if (_hpLose)
        {
            _hpLose->setPosition(Vec2(260 + currentHP * gap, 978));
            _hpLose->setVisible(currentHP < maxHp);
        }
        
        // 更新灵魂显示
        if (_soulBg && currentSoul != _lastDisplayedSoul)
        {
            _lastDisplayedSoul = currentSoul;
            
            // 停止之前的动画
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
                
                // 创建灵魂动画帧
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
        
        _lastDisplayedHP = currentHP;
    }
}

void GameScene::startHPRecoveryAnimation()
{
    if (!_knight) return;
    
    int maxHP = _knight->getMaxHP();
    int currentHP = _knight->getHP();
    
    // 如果已经满血，不需要恢复动画
    if (currentHP >= maxHP) return;
    
    // 设置骑士血量为满血
    _knight->setHP(maxHP);
    
    // 开始恢复动画
    _isHPRecovering = true;
    _hpRecoverCurrent = _lastDisplayedHP;  // 从当前显示的血量开始
    _hpRecoverTarget = maxHP;
    _hpRecoverTimer = 0.0f;
    
    CCLOG("开始血量恢复动画: %d -> %d", _hpRecoverCurrent, _hpRecoverTarget);
}

void GameScene::updateHPRecoveryAnimation(float dt)
{
    if (!_isHPRecovering) return;
    
    _hpRecoverTimer += dt;
    
    // 每隔一段时间恢复一点血量显示
    if (_hpRecoverTimer >= _hpRecoverInterval)
    {
        _hpRecoverTimer = 0.0f;
        _hpRecoverCurrent++;
        
        // 更新血量图标显示
        if (_hpRecoverCurrent - 1 < (int)_hpBars.size())
        {
            auto hpBar = _hpBars[_hpRecoverCurrent - 1];
            hpBar->setVisible(true);
            
            // 添加一个小的缩放动画效果
            hpBar->setScale(0.0f);
            hpBar->runAction(Sequence::create(
                ScaleTo::create(0.15f, 0.6f),
                ScaleTo::create(0.1f, 0.5f),
                nullptr
            ));
        }
        
        // 更新失去血量图标位置
        float gap = 50;
        if (_hpLose)
        {
            _hpLose->setPosition(Vec2(260 + _hpRecoverCurrent * gap, 978));
            _hpLose->setVisible(_hpRecoverCurrent < _hpRecoverTarget);
        }
        
        _lastDisplayedHP = _hpRecoverCurrent;
        
        CCLOG("血量恢复: %d / %d", _hpRecoverCurrent, _hpRecoverTarget);
        
        // 检查是否恢复完成
        if (_hpRecoverCurrent >= _hpRecoverTarget)
        {
            _isHPRecovering = false;
            CCLOG("血量恢复完成");
        }
    }
}

void GameScene::loadInteractiveObjects(TMXTiledMap* map, float scale, const Vec2& mapOffset)
{
    auto objectGroup = map->getObjectGroup("Objects");
    if (!objectGroup) return;

    auto& objects = objectGroup->getObjects();
    
    for (auto& obj : objects)
    {
        auto& dict = obj.asValueMap();
        std::string name = dict["name"].asString();
        
        float x = dict["x"].asFloat() * scale + mapOffset.x;
        float y = dict["y"].asFloat() * scale + mapOffset.y;
        float width = dict["width"].asFloat() * scale;
        float height = dict["height"].asFloat() * scale;
        
        // 检测椅子对象
        if (name == "Chair")
        {
            InteractiveObject chairObj;
            chairObj.position = Vec2(x + width / 2, y + height / 2);
            chairObj.name = "Chair";
            chairObj.radius = 72.5f;  // 50.0f * 1.45 = 72.5f
            
            _interactiveObjects.push_back(chairObj);
            
            CCLOG("加载交互对象: %s at (%.1f, %.1f)", name.c_str(), chairObj.position.x, chairObj.position.y);
        }
        // 检测出口对象
        else if (name == "Exit")
        {
            InteractiveObject exitObj;
            exitObj.position = Vec2(x + width / 2, y + height / 2);
            exitObj.name = "Exit";
            exitObj.radius = 217.5f;  // 150.0f * 1.45 = 217.5f
            
            _interactiveObjects.push_back(exitObj);
            
            CCLOG("加载出口对象: %s at (%.1f, %.1f)", name.c_str(), exitObj.position.x, exitObj.position.y);
        }
    }
}

void GameScene::checkInteractions()
{
    if (_isTransitioning) return;
    
    if (!_knight || !_interactionLabel) return;

    Vec2 playerPos = _knight->getPosition();
    bool showPrompt = false;
    bool nearChair = false;

    for (auto& obj : _interactiveObjects)
    {
        float distance = playerPos.distance(obj.position);
        
        if (distance < obj.radius)
        {
            if (obj.name == "Chair")
            {
                nearChair = true;
                if (!_knight->isSitting())
                {
                    showPrompt = true;
                    _interactionLabel->setString(u8"休息");
                    _interactionLabel->setPosition(Vec2(playerPos.x, playerPos.y + 80));
                }
            }
            else if (obj.name == "Exit")
            {
                // 进入出口，切换到 NextScene
                _isTransitioning = true;

                auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 0));
                this->addChild(blackLayer, 1000);

                blackLayer->runAction(Sequence::create(
                    FadeIn::create(0.5f),
                    CallFunc::create([]() {
                        Director::getInstance()->replaceScene(
                            TransitionFade::create(0.5f, NextScene::createScene(), Color3B::BLACK)
                        );
                        }),
                    nullptr
                ));
                return;
            }
        }
    }

    _knight->setNearChair(nearChair);
    _interactionLabel->setVisible(showPrompt);
}

void GameScene::updateCamera()
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
    
    // 计算摄像机目标位置（骑士在屏幕下1/3处，水平居中）
//     float verticalOffset = visibleSize.height / 6.0f;
    float cameraX = knightPos.x;
    float cameraY = knightPos.y + visibleSize.height / 3.0f + _cameraOffsetY;
    
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
}

void GameScene::update(float dt)
{
    // 更新摄像机
    updateCamera();
    
    // 更新HP和Soul UI
    updateHPAndSoulUI(dt);
    
    // 检测坐下状态变化（坐下时自动回血）
    if (_knight)
    {
        bool isSitting = _knight->isSitting();
        
        // 刚坐下时触发回血
        if (isSitting && !_wasSitting)
        {
            startHPRecoveryAnimation();
        }
        
        _wasSitting = isSitting;
    }
    
    // 检测交互
    checkInteractions();
}

void GameScene::createCollisionFromTMX(TMXTiledMap* map, const std::string& layerName, float scale, const Vec2& mapOffset)
{
    auto collisionGroup = map->getObjectGroup(layerName);
    if (!collisionGroup) {
        CCLOG("警告：地图中没有找到 %s 对象层", layerName.c_str());
        return;
    }

    auto& objects = collisionGroup->getObjects();

    for (auto& obj : objects)
    {
        auto& dict = obj.asValueMap();

        // 检查类型（支持 type 或 class 属性）
//         std::string type = dict["type"].asString();
        std::string type = "";
        if (dict.find("type") != dict.end()) {
            type = dict["type"].asString();
        }
        if (dict.find("class") != dict.end()) {
            type = dict["class"].asString();
        }

        // 只处理 crash 类型的碰撞体
        if (type != "crash") {
            continue;
        }

        float x = dict["x"].asFloat() * scale + mapOffset.x;
        float y = dict["y"].asFloat() * scale + mapOffset.y;
        float width = dict["width"].asFloat() * scale;
        float height = dict["height"].asFloat() * scale;

        if (width > 0 && height > 0)
        {
            // 创建 Platform 结构（与 BossScene 相同的碰撞方式）
            Platform platform;
            platform.rect = Rect(x, y, width, height);
            platform.node = nullptr;
            _platforms.push_back(platform);

            CCLOG("创建碰撞平台: x=%.1f, y=%.1f, w=%.1f, h=%.1f", x, y, width, height);
        }
    }
}

void GameScene::loadForegroundObjects(TMXTiledMap* map, float scale, const Vec2& mapOffset)
{
    auto objectGroup = map->getObjectGroup("Objects");
    if (!objectGroup) {
        CCLOG("警告：未找到 Objects 对象层");
        return;
    }

    auto& objects = objectGroup->getObjects();

    for (auto& obj : objects)
    {
        auto& dict = obj.asValueMap();

        // 获取对象的class或type属性
        std::string objClass = "";
        if (dict.find("class") != dict.end()) {
            objClass = dict["class"].asString();
        }
        else if (dict.find("type") != dict.end()) {
            objClass = dict["type"].asString();
        }

        // 只处理bg类的对象
        if (objClass != "bg") {
            continue;
        }

        // 获取对象名称作为图片文件名
        std::string name = dict["name"].asString();
        if (name.empty()) {
            CCLOG("警告：bg类对象没有名称，跳过");
            continue;
        }

        // 获取对象在地图中的原始位置（未缩放）
        float objX = dict["x"].asFloat();
        float objY = dict["y"].asFloat();
        float objWidth = dict["width"].asFloat();
        float objHeight = dict["height"].asFloat();

        // 使用对象名称 + .png 作为图片路径
        std::string imagePath = "Maps/" + name + ".png";
        auto fgSprite = Sprite::create(imagePath);

        if (!fgSprite) {
            imagePath = name + ".png";
            fgSprite = Sprite::create(imagePath);
        }
        float spriteHeight = fgSprite->getContentSize().height;
        if (fgSprite)
        {
            // 计算世界坐标位置
            float worldX = objX * scale + mapOffset.x;
            float worldY = (objY + spriteHeight) * scale + mapOffset.y;

            // TMX 对象的 y 坐标是对象底部，锚点设为左下角
            fgSprite->setAnchorPoint(Vec2(0, 0));
            fgSprite->setPosition(Vec2(worldX, worldY));
            fgSprite->setScale(scale);

            // 添加到比角色更高的z-order层（角色是5，前景用10）
            this->addChild(fgSprite, 10);

            CCLOG("加载前景对象: %s at (%.1f, %.1f)", imagePath.c_str(), worldX, worldY);
        }
        else
        {
            CCLOG("警告：无法加载前景图片: %s", imagePath.c_str());
        }
    }
}