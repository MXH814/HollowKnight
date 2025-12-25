#include "NextScene.h"
#include "TheKnight.h"
#include "GameScene.h"
#include "CharmManager.h"

USING_NS_CC;

Scene* NextScene::createScene()
{
    auto scene = Scene::create();
    
    auto layer = NextScene::create();
    scene->addChild(layer);

    // 设置摄像机初始位置（添加 Y 偏移）
    auto knight = layer->getChildByName("Player");
    if (knight) {
        float cameraOffsetY = 200.0f;
        scene->getDefaultCamera()->setPosition(
            Vec2(knight->getPositionX(), knight->getPositionY() + cameraOffsetY)
        );
    }

    return scene;
}

bool NextScene::init()
{
    if (!Layer::init())
        return false;

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 初始化标志
    _hasLandedOnce = false;
    _isTransitioning = false;
    _isNearExit = false;
    _isNearThorn = false;
    
    // 初始化尖刺伤害相关变量
    _isInSpikeDeath = false;
    _lastSafePosition = Vec2::ZERO;
    _spikeDeathTimer = 0.0f;
    _spikeDeathPhase = 0;
    _blackScreen = nullptr;
    
    // 初始化震动相关变量
    _isShaking = false;
    _shakeDuration = 0.0f;
    _shakeElapsed = 0.0f;
    _shakeIntensity = 0.0f;
    _shakeOffset = Vec2::ZERO;
    
    // 初始化UI相关变量
    _lastDisplayedHP = 0;
    _lastDisplayedSoul = 0;

    // 在加载地图之前添加全黑遮罩层
    auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 255));
    this->addChild(blackLayer, 10, "LoadingBlack");

    // 定义地图块信息
    struct MapChunk {
        std::string file;
        Vec2 position;
    };

    float scale = 1.8f;

    // 4个地图块的配置（根据实际地图尺寸调整偏移量）
    std::vector<MapChunk> chunks = {
        {"Maps/Forgotten Crossroads1.tmx", Vec2(0, 0)},
        {"Maps/Forgotten Crossroads2.tmx", Vec2(120 * 16, 0)},
        {"Maps/Forgotten Crossroads3.tmx", Vec2(0, 105 * 16)},
        {"Maps/Forgotten Crossroads4.tmx", Vec2(120 * 16, 105 * 16)}
    };

    // 加载所有地图块并创建碰撞体
    for (const auto& chunk : chunks) {
        auto map = TMXTiledMap::create(chunk.file);
        CCASSERT(map != nullptr, ("地图加载失败: " + chunk.file).c_str());

        map->setScale(scale);
        map->setAnchorPoint(Vec2::ZERO);
        Vec2 mapPos = Vec2(origin.x + chunk.position.x * scale, 
                           origin.y + chunk.position.y * scale);
        map->setPosition(mapPos);
        this->addChild(map, 0);

        createCollisionFromTMX(map, "Collision", scale, mapPos);
        
        // 加载前景对象（bg类，显示在角色上层）
        loadForegroundObjects(map, scale, mapPos);
        
        // 为地图1创建trap1背景精灵和加载尖刺对象
        if (chunk.file == "Maps/Forgotten Crossroads1.tmx") {
            createTrapSprites(map, "Collision", "trap1", "Maps/solidtrap.png", scale, mapPos);
            // 从地图1加载尖刺对象
            loadThornObjects(map, scale, mapPos);
        }
        
        // 从地图4加载出口对象
        if (chunk.file == "Maps/Forgotten Crossroads4.tmx") {
            loadExitObjects(map, scale, mapPos);
        }
    }

    // 从第四个地图块获取玩家起始点（需要加上地图4的偏移量）
    auto fourthMap = TMXTiledMap::create("Maps/Forgotten Crossroads4.tmx");
    auto objectGroup = fourthMap->getObjectGroup("Objects");
    CCASSERT(objectGroup != nullptr, "地图缺少对象层 Objects");

    auto startPoint = objectGroup->getObject("PlayerStart");
    
    // 修复：加上地图4的偏移量
    Vec2 map4Offset = Vec2(120 * 16 * scale, 120 * 16 * scale);
    float startX = startPoint["x"].asFloat() * scale + origin.x + map4Offset.x;
    float startY = startPoint["y"].asFloat() * scale + origin.y + map4Offset.y;

    CCLOG("玩家起始位置: x=%.1f, y=%.1f", startX, startY);

    // 创建TheKnight角色，与GameScene实现一致
    auto knight = TheKnight::create();
    if (knight)
    {
        knight->setPosition(Vec2(startX, startY));
        knight->setScale(1.0f);
        knight->setPlatforms(_platforms);  // 传递碰撞平台
        this->addChild(knight, 5, "Player");
        
        // 同步护符状态到玩家
        CharmManager::getInstance()->syncToKnight(knight);
    }

    // 创建HP和Soul UI
    createHPAndSoulUI();

    // 创建出口提示标签
    _exitLabel = Label::createWithSystemFont(u8"按 W 进入", "Arial", 24);
    _exitLabel->setTextColor(Color4B::WHITE);
    _exitLabel->setVisible(false);
    this->addChild(_exitLabel, 100, "ExitLabel");

    // 创建尖刺警告标签
    _thornLabel = Label::createWithSystemFont(u8"危险！前方有尖刺", "Arial", 24);
    _thornLabel->setTextColor(Color4B::RED);
    _thornLabel->setVisible(false);
    this->addChild(_thornLabel, 100, "ThornLabel");

    // 添加键盘监听
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        // 按W键处理出口交互
        if (keyCode == EventKeyboard::KeyCode::KEY_W || keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_W)
        {
            // 如果在出口附近且没有正在切换场景，则切换场景
            if (_isNearExit && !_isTransitioning)
            {
                _isTransitioning = true;
                
                // 获取骑士的朝向
                auto knight = dynamic_cast<TheKnight*>(this->getChildByName("Player"));
                bool facingRight = true;
                if (knight)
                {
                    facingRight = knight->getScaleX() > 0;
                }
                
                // 目标位置
                Vec2 spawnPos(8606.7f, 300.0f);
                
                // 创建全黑过渡场景
                auto blackScene = Scene::create();
                auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 255));
                blackScene->addChild(blackLayer);

                // 先切换到黑屏，延迟后再进入目标场景
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, blackScene));

                // 延迟后进入真正的目标场景
                blackLayer->runAction(Sequence::create(
                    DelayTime::create(1.0f),
                    CallFunc::create([spawnPos, facingRight]() {
                        auto gameScene = GameScene::createSceneWithSpawn(spawnPos, facingRight);
                        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, gameScene));
                    }),
                    nullptr
                ));
                return;
            }
        }
        
        // 按Q键处理护符面板
        if (keyCode == EventKeyboard::KeyCode::KEY_Q)
        {
            auto charmManager = CharmManager::getInstance();
            auto scene = this->getScene();
            if (!scene) return;
            
            // 如果护符面板已打开，按Q关闭
            if (charmManager->isPanelOpen())
            {
                charmManager->hideCharmPanel();
                // 关闭面板后同步护符状态到玩家
                auto knight = dynamic_cast<TheKnight*>(this->getChildByName("Player"));
                if (knight)
                {
                    charmManager->syncToKnight(knight);
                }
                return;
            }
            
            // 否则打开护符面板
            charmManager->showCharmPanel(scene);
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

    // 启用 update 实现摄像机跟随
    this->scheduleUpdate();

    // 移除黑层
    if (blackLayer && blackLayer->getParent()) {
        blackLayer->removeFromParent();
    }

    return true;
}

void NextScene::loadExitObjects(TMXTiledMap* map, float scale, const Vec2& mapOffset)
{
    auto objectGroup = map->getObjectGroup("Objects");
    if (!objectGroup) {
        CCLOG("警告：地图中没有找到 Objects 对象层");
        return;
    }

    auto& objects = objectGroup->getObjects();
    
    for (auto& obj : objects)
    {
        auto& dict = obj.asValueMap();
        
        // 获取对象的 class 或 type 属性
        std::string objClass = "";
        if (dict.find("class") != dict.end()) {
            objClass = dict["class"].asString();
        }
        else if (dict.find("type") != dict.end()) {
            objClass = dict["type"].asString();
        }
        
        // 只处理 Exit 类的对象
        if (objClass != "Exit") {
            continue;
        }
        
        float x = dict["x"].asFloat() * scale + mapOffset.x;
        float y = dict["y"].asFloat() * scale + mapOffset.y;
        float width = dict["width"].asFloat() * scale;
        float height = dict["height"].asFloat() * scale;
        
        ExitObject exitObj;
        exitObj.position = Vec2(x + width / 2, y + height / 2);
        exitObj.radius = std::max(width, height) / 2 + 50.0f;
        
        _exitObjects.push_back(exitObj);
        
        CCLOG("加载出口对象: at (%.1f, %.1f), radius=%.1f", exitObj.position.x, exitObj.position.y, exitObj.radius);
    }
}

void NextScene::loadThornObjects(TMXTiledMap* map, float scale, const Vec2& mapOffset)
{
    auto objectGroup = map->getObjectGroup("Objects");
    if (!objectGroup) {
        CCLOG("警告：地图中没有找到 Objects 对象层");
        return;
    }

    auto& objects = objectGroup->getObjects();
    
    for (auto& obj : objects)
    {
        auto& dict = obj.asValueMap();
        
        std::string objClass = "";
        if (dict.find("class") != dict.end()) {
            objClass = dict["class"].asString();
        }
        else if (dict.find("type") != dict.end()) {
            objClass = dict["type"].asString();
        }
        
        if (objClass != "thorn") {
            continue;
        }
        
        float x = dict["x"].asFloat() * scale + mapOffset.x;
        float y = dict["y"].asFloat() * scale + mapOffset.y;
        float width = dict["width"].asFloat() * scale;
        float height = dict["height"].asFloat() * scale;
        
        ThornObject thornObj;
        thornObj.position = Vec2(x + width / 2, y + height / 2);
        thornObj.size = Size(width + 50.0f, height + 50.0f);
        
        _thornObjects.push_back(thornObj);
        
        CCLOG("加载尖刺对象: at (%.1f, %.1f), size=(%.1f, %.1f)", 
              thornObj.position.x, thornObj.position.y, 
              thornObj.size.width, thornObj.size.height);
    }
}

void NextScene::checkInteractions()
{
    if (_isTransitioning) return;
    
    auto knight = this->getChildByName("Player");
    if (!knight) return;

    Vec2 knightPos = knight->getPosition();
    
    // 检测出口
    _isNearExit = false;
    if (_exitLabel) {
        for (auto& exitObj : _exitObjects)
        {
            float distance = knightPos.distance(exitObj.position);
            
            if (distance < exitObj.radius)
            {
                _isNearExit = true;
                _exitLabel->setPosition(Vec2(knightPos.x, knightPos.y + 80));
                break;
            }
        }
        _exitLabel->setVisible(_isNearExit);
    }

    // 检测尖刺（仅用于显示警告标签，实际碰撞检测在update中）
    _isNearThorn = false;
    if (_thornLabel) {
        for (auto& thornObj : _thornObjects)
        {
            Rect thornRect(
                thornObj.position.x - thornObj.size.width / 2,
                thornObj.position.y - thornObj.size.height / 2,
                thornObj.size.width,
                thornObj.size.height
            );
            
            if (thornRect.containsPoint(knightPos))
            {
                _isNearThorn = true;
                _thornLabel->setPosition(Vec2(knightPos.x, knightPos.y + 100));
                break;
            }
        }
        _thornLabel->setVisible(_isNearThorn);
    }
}

void NextScene::startSpikeDeath(TheKnight* knight)
{
    if (_isInSpikeDeath) return;
    
    _isInSpikeDeath = true;
    _spikeDeathPhase = 1;
    _spikeDeathTimer = 0.0f;
    
    // 调用骑士的尖刺死亡函数
    knight->startSpikeDeath();
    
    CCLOG("尖刺死亡流程开始，阶段1：播放SpikeDeath动画");
}

void NextScene::updateSpikeDeath(float dt, TheKnight* knight)
{
    _spikeDeathTimer += dt;
    
    switch (_spikeDeathPhase)
    {
        case 1:  // 阶段1：等待SpikeDeath动画完成
        {
            // SpikeDeath动画8帧，每帧0.08秒，共0.64秒
            if (_spikeDeathTimer >= 0.64f)
            {
                // 创建全屏黑屏 - 使用足够大的尺寸确保覆盖整个可视区域
                Size visibleSize = Director::getInstance()->getVisibleSize();
                _blackScreen = LayerColor::create(Color4B(0, 0, 0, 0), visibleSize.width * 3, visibleSize.height * 3);
                _blackScreen->setIgnoreAnchorPointForPosition(false);
                _blackScreen->setAnchorPoint(Vec2(0.5f, 0.5f));
                
                // 将黑屏放置在摄像机位置
                auto scene = this->getScene();
                if (scene)
                {
                    auto camera = scene->getDefaultCamera();
                    if (camera)
                    {
                        _blackScreen->setPosition(camera->getPosition());
                    }
                }
                
                this->addChild(_blackScreen, 1000, "SpikeDeathBlack");
                
                // 黑屏淡入
                _blackScreen->runAction(FadeTo::create(0.3f, 255));
                
                _spikeDeathPhase = 2;
                _spikeDeathTimer = 0.0f;
                CCLOG("尖刺死亡流程阶段2：黑屏淡入");
            }
            break;
        }
        
        case 2:  // 阶段2：黑屏显示
        {
            // 更新黑屏位置跟随摄像机
            if (_blackScreen)
            {
                auto scene = this->getScene();
                if (scene)
                {
                    auto camera = scene->getDefaultCamera();
                    if (camera)
                    {
                        _blackScreen->setPosition(camera->getPosition());
                    }
                }
            }
            
            if (_spikeDeathTimer >= 0.5f)  // 黑屏0.5秒
            {
                // 将骑士移动到安全位置
                Vec2 respawnPos = _lastSafePosition;
                if (respawnPos == Vec2::ZERO)
                {
                    // 如果没有记录安全位置，使用初始位置
                    respawnPos = knight->getPosition();
                }
                
                // 隐藏骑士，准备重生
                knight->setVisible(false);
                
                // 开始重生动画
                knight->startHazardRespawn(respawnPos);
                
                _spikeDeathPhase = 3;
                _spikeDeathTimer = 0.0f;
                CCLOG("尖刺死亡流程阶段3：开始重生，位置(%.1f, %.1f)", respawnPos.x, respawnPos.y);
            }
            break;
        }
        
        case 3:  // 阶段3：等待一小段时间然后淡出黑屏
        {
            // 更新黑屏位置跟随摄像机
            if (_blackScreen)
            {
                auto scene = this->getScene();
                if (scene)
                {
                    auto camera = scene->getDefaultCamera();
                    if (camera)
                    {
                        _blackScreen->setPosition(camera->getPosition());
                    }
                }
            }
            
            if (_spikeDeathTimer >= 0.1f)
            {
                // 显示骑士
                knight->setVisible(true);
                
                // 黑屏淡出
                if (_blackScreen)
                {
                    _blackScreen->runAction(Sequence::create(
                        FadeTo::create(0.3f, 0),
                        RemoveSelf::create(),
                        nullptr
                    ));
                    _blackScreen = nullptr;
                }
                
                _spikeDeathPhase = 4;
                _spikeDeathTimer = 0.0f;
                CCLOG("尖刺死亡流程阶段4：黑屏淡出");
            }
            break;
        }
        
        case 4:  // 阶段4：等待HazardRespawn动画完成
        {
            // HazardRespawn动画20帧，每帧0.05秒，共1.0秒
            if (_spikeDeathTimer >= 1.0f || !knight->isHazardRespawnState())
            {
                // 重生完成
                _isInSpikeDeath = false;
                _spikeDeathPhase = 0;
                _spikeDeathTimer = 0.0f;
                CCLOG("尖刺死亡流程完成");
            }
            break;
        }
    }
}

bool NextScene::onContactBegin(PhysicsContact& contact)
{
    // 此方法不再使用，保留以兼容头文件声明
    return true;
}

void NextScene::shakeScreen(float duration, float intensity)
{
    _isShaking = true;
    _shakeDuration = duration;
    _shakeElapsed = 0.0f;
    _shakeIntensity = intensity;
    _shakeOffset = Vec2::ZERO;
    
    CCLOG("开始屏幕震动: duration=%.2f, intensity=%.1f", duration, intensity);
}

void NextScene::update(float dt)
{
    auto knight = dynamic_cast<TheKnight*>(this->getChildByName("Player"));
    if (!knight) return;

    auto scene = this->getScene();
    if (!scene) return;

    auto camera = scene->getDefaultCamera();
    if (!camera) return;

    Vec2 knightPos = knight->getPosition();
    
    // 处理尖刺死亡流程
    if (_isInSpikeDeath)
    {
        updateSpikeDeath(dt, knight);
        
        // 尖刺死亡期间仍然更新摄像机位置
        Vec2 cameraPos = camera->getPosition();
        float cameraOffsetY = 200.0f;
        Vec2 targetPos = Vec2(knightPos.x, knightPos.y + cameraOffsetY);
        float lerpFactor = 0.1f;
        Vec2 newPos = cameraPos + (targetPos - cameraPos) * lerpFactor;
        newPos = newPos + _shakeOffset;
        camera->setPosition(newPos);
        
        updateHPAndSoulUI(dt);
        return;
    }
    
    // 记录安全位置（当骑士在地面上且不处于特殊状态时）
    if (!knight->isDead() && !knight->isSpikeDeathState() && !knight->isHazardRespawnState())
    {
        // 简单判断：如果骑士Y坐标接近某个平台顶部，就记录为安全位置
        for (const auto& platform : _platforms)
        {
            float platformTop = platform.rect.getMaxY();
            if (knightPos.x > platform.rect.getMinX() &&
                knightPos.x < platform.rect.getMaxX() &&
                std::abs(knightPos.y - platformTop) < 20.0f)
            {
                _lastSafePosition = knightPos;
                knight->setLastSafePosition(knightPos);
                break;
            }
        }
    }
    
    // 检测尖刺碰撞
    if (!knight->isDead() && !knight->isSpikeDeathState() && !knight->isHazardRespawnState() && !knight->isInvincible())
    {
        for (auto& thornObj : _thornObjects)
        {
            Rect thornRect(
                thornObj.position.x - thornObj.size.width / 2,
                thornObj.position.y - thornObj.size.height / 2,
                thornObj.size.width,
                thornObj.size.height
            );
            
            if (thornRect.containsPoint(knightPos))
            {
                // 触发尖刺死亡
                CCLOG("玩家碰到尖刺！开始尖刺死亡流程");
                startSpikeDeath(knight);
                break;
            }
        }
    }

    // 检测骑士重落地状态触发震动
    static bool wasHardLanding = false;
    bool isHardLanding = knight->isHardLanding();
    
    // 当骑士刚进入重落地状态时触发震动
    if (isHardLanding && !wasHardLanding)
    {
        shakeScreen(0.6f, 30.0f);
        CCLOG("玩家重落地，触发屏幕震动");
    }
    wasHardLanding = isHardLanding;

    // 更新震动效果
    if (_isShaking)
    {
        _shakeElapsed += dt;
        
        if (_shakeElapsed >= _shakeDuration)
        {
            _isShaking = false;
            _shakeOffset = Vec2::ZERO;
            CCLOG("屏幕震动结束");
        }
        else
        {
            // 计算震动衰减因子（逐渐减弱）
            float factor = 1.0f - (_shakeElapsed / _shakeDuration);
            
            // 随机偏移
            float offsetX = ((rand() % 200) / 100.0f - 1.0f) * _shakeIntensity * factor;
            float offsetY = ((rand() % 200) / 100.0f - 1.0f) * _shakeIntensity * factor;
            
            _shakeOffset = Vec2(offsetX, offsetY);
        }
    }

    // 摄像机平滑跟随角色
    Vec2 cameraPos = camera->getPosition();
    
    float cameraOffsetY = 200.0f;
    Vec2 targetPos = Vec2(knightPos.x, knightPos.y + cameraOffsetY);
    
    float lerpFactor = 0.1f;
    Vec2 newPos = cameraPos + (targetPos - cameraPos) * lerpFactor;
    
    // 应用震动偏移
    newPos = newPos + _shakeOffset;
    
    camera->setPosition(newPos);
    
    // 更新HP和Soul UI
    updateHPAndSoulUI(dt);
    
    checkInteractions();
}

void NextScene::createCollisionFromTMX(TMXTiledMap* map, const std::string& layerName, float scale, const Vec2& mapOffset)
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

        std::string type = "";
        if (dict.find("type") != dict.end()) {
            type = dict["type"].asString();
        }
        if (dict.find("class") != dict.end()) {
            type = dict["class"].asString();
        }

        if (type != "crash") {
            continue;
        }

        float x = dict["x"].asFloat() * scale + mapOffset.x;
        float y = dict["y"].asFloat() * scale + mapOffset.y;
        float width = dict["width"].asFloat() * scale;
        float height = dict["height"].asFloat() * scale;

        if (width > 0 && height > 0)
        {
            Platform platform;
            platform.rect = Rect(x, y, width, height);
            platform.node = nullptr;
            _platforms.push_back(platform);

            CCLOG("创建碰撞平台: x=%.1f, y=%.1f, w=%.1f, h=%.1f", x, y, width, height);
        }
    }
}

void NextScene::loadForegroundObjects(TMXTiledMap* map, float scale, const Vec2& mapOffset)
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
        
        std::string objClass = "";
        if (dict.find("class") != dict.end()) {
            objClass = dict["class"].asString();
        }
        else if (dict.find("type") != dict.end()) {
            objClass = dict["type"].asString();
        }
        
        if (objClass != "bg") {
            continue;
        }
        
        std::string name = dict["name"].asString();
        if (name.empty()) {
            CCLOG("警告：bg类对象没有名称，跳过");
            continue;
        }
        
        float objX = dict["x"].asFloat();
        float objY = dict["y"].asFloat();
        
        std::string imagePath = "Maps/" + name + ".png";
        auto fgSprite = Sprite::create(imagePath);
        
        if (!fgSprite) {
            imagePath = name + ".png";
            fgSprite = Sprite::create(imagePath);
        }
        
        if (fgSprite)
        {
            float spriteHeight = fgSprite->getContentSize().height;
            float worldX = objX * scale + mapOffset.x;
            float worldY = (objY + spriteHeight) * scale + mapOffset.y;
            
            fgSprite->setAnchorPoint(Vec2(0, 0));
            fgSprite->setPosition(Vec2(worldX, worldY));
            fgSprite->setScale(scale);
            
            this->addChild(fgSprite, 10);
            
            CCLOG("加载前景对象: %s at (%.1f, %.1f)", imagePath.c_str(), worldX, worldY);
        }
        else
        {
            CCLOG("警告：无法加载前景图片: %s", imagePath.c_str());
        }
    }
}

void NextScene::createTrapSprites(TMXTiledMap* map, const std::string& layerName, 
                                   const std::string& trapType, const std::string& spritePath,
                                   float scale, const Vec2& mapOffset)
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

        std::string type = "";
        if (dict.find("type") != dict.end()) {
            type = dict["type"].asString();
        }
        if (dict.find("class") != dict.end()) {
            type = dict["class"].asString();
        }

        if (type != trapType) {
            continue;
        }

        float x = dict["x"].asFloat() * scale + mapOffset.x;
        float y = (dict["y"].asFloat() + 166) * scale + mapOffset.y;
        float width = dict["width"].asFloat() * scale;
        float height = dict["height"].asFloat() * scale;

        if (width > 0 && height > 0)
        {
            auto sprite = Sprite::create(spritePath);
            if (sprite) {
                Size originalSize = sprite->getContentSize();
                sprite->setScaleX(width / originalSize.width);
                sprite->setScaleY(height / originalSize.height);
                sprite->setPosition(Vec2(x + width / 2, y + height / 2));
                this->addChild(sprite, 1);
                
                CCLOG("创建陷阱精灵: x=%.1f, y=%.1f, w=%.1f, h=%.1f", x, y, width, height);
            }
            else {
                CCLOG("警告：无法加载精灵 %s", spritePath.c_str());
            }
        }
    }
}

void NextScene::createHPAndSoulUI()
{
    auto knight = dynamic_cast<TheKnight*>(this->getChildByName("Player"));
    if (!knight) return;
    
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
    
    // 初始化血量和灵魂显示
    _lastDisplayedHP = knight->getHP();
    _lastDisplayedSoul = -1;  // 初始化为-1，确保第一次更新时会触发
    
    // 灵魂背景 - 使用soul_1作为默认图像
    int currentSoul = knight->getSoul();
    
    _soulBg = Sprite::create("Hp/soul_1_0.png");
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
    
    int maxHp = knight->getMaxHP();
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

void NextScene::updateHPAndSoulUI(float dt)
{
    auto knight = dynamic_cast<TheKnight*>(this->getChildByName("Player"));
    if (!knight || !_uiLayer) return;
    
    // 更新UI层位置跟随摄像机
    auto scene = this->getScene();
    if (scene)
    {
        auto camera = scene->getDefaultCamera();
        if (camera)
        {
            Vec2 camPos = camera->getPosition();
            Size visibleSize = Director::getInstance()->getVisibleSize();
            _uiLayer->setPosition(Vec2(camPos.x - visibleSize.width / 2, camPos.y - visibleSize.height / 2));
        }
    }
    
    int currentHP = knight->getHP();
    int currentSoul = knight->getSoul();
    int maxHp = knight->getMaxHP();
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