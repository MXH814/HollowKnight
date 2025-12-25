#include "NextScene.h"
#include "TheKnight.h"
#include "GameScene.h"

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
    
    // 初始化震动相关变量
    _isShaking = false;
    _shakeDuration = 0.0f;
    _shakeElapsed = 0.0f;
    _shakeIntensity = 0.0f;
    _shakeOffset = Vec2::ZERO;

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
    }

    // 创建出口提示标签
    _exitLabel = Label::createWithSystemFont(u8"按 Q 离开", "Arial", 24);
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
        if (keyCode == EventKeyboard::KeyCode::KEY_Q && _isNearExit && !_isTransitioning) {
            _isTransitioning = true;
            // 创建全黑过渡场景
            auto blackScene = Scene::create();
            auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 255));
            blackScene->addChild(blackLayer);

            // 先切换到黑屏，延迟后再进入目标场景
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, blackScene));

            // 延迟后进入真正的目标场景
            blackLayer->runAction(Sequence::create(
                DelayTime::create(1.0f),
                CallFunc::create([]() {
                    auto scene = GameScene::createScene();
                    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
                }),
                nullptr
            ));
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

    // 检测尖刺
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