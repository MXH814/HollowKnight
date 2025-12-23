#include "GameScene.h"
#include "TheKnight.h"
#include <BossScene.h>

USING_NS_CC;

Scene* GameScene::createScene()
{
    return GameScene::create();
}

bool GameScene::init()
{
    if (!Scene::init())
        return false;

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 在加载地图之前添加全黑遮罩层
    auto blackLayer1 = LayerColor::create(Color4B(0, 0, 0, 255));
    this->addChild(blackLayer1, 10, "LoadingBlack");

    // 定义地图块信息
    struct MapChunk {
        std::string file;
        Vec2 position;
    };

    _mapScale = 1.8f;

    // 3个地图块的配置
    std::vector<MapChunk> chunks = {
        {"Maps/Dirtmouth1.tmx", Vec2(0, 0)},
        {"Maps/Dirtmouth2.tmx", Vec2(150 * 16, 0)},
        {"Maps/Dirtmouth3.tmx", Vec2(290 * 16, 0)}
    };

    // 计算总地图尺寸
    float totalMapWidth = 0.0f;
    float maxMapHeight = 0.0f;

    // 加载所有地图块并创建碰撞体
    for (const auto& chunk : chunks) {
        auto map = TMXTiledMap::create(chunk.file);
        CCASSERT(map != nullptr, ("地图加载失败: " + chunk.file).c_str());

        map->setScale(_mapScale);
        map->setAnchorPoint(Vec2::ZERO);
        Vec2 mapPos = Vec2(origin.x + chunk.position.x * _mapScale, 
                           origin.y + chunk.position.y * _mapScale);
        map->setPosition(mapPos);
        this->addChild(map, 0);

        // 更新地图总尺寸
        auto mapContentSize = map->getContentSize();
        float mapRight = mapPos.x + mapContentSize.width * _mapScale;
        float mapTop = mapPos.y + mapContentSize.height * _mapScale;
        totalMapWidth = std::max(totalMapWidth, mapRight);
        maxMapHeight = std::max(maxMapHeight, mapTop);

        // 创建碰撞平台（使用 Platform 结构，与 BossScene 相同）
        createCollisionFromTMX(map, "Collision", _mapScale, mapPos);
        
        // 加载交互对象（如椅子、出口）
        loadInteractiveObjects(map, _mapScale, mapPos);
    }
    
    // 保存地图尺寸
    _mapSize = Size(totalMapWidth, maxMapHeight);

    // 从地图块获取玩家起始点
    auto firstMap = TMXTiledMap::create("Maps/Dirtmouth2.tmx");
    auto objectGroup = firstMap->getObjectGroup("Objects");
    CCASSERT(objectGroup != nullptr, "地图缺少对象层 Objects");

    auto startPoint = objectGroup->getObject("PlayerStart");
    float mapOffsetX = 150 * 16 * _mapScale;
    float startX = startPoint["x"].asFloat() * _mapScale + mapOffsetX;
    float startY = startPoint["y"].asFloat() * _mapScale;

    // 创建玩家（使用 TheKnight，与 BossScene 相同）
    _knight = TheKnight::create();
    if (_knight)
    {
        _knight->setPosition(Vec2(startX, startY));
        _knight->setScale(1.0f);
        _knight->setPlatforms(_platforms);  // 设置碰撞平台
        this->addChild(_knight, 5, "Player");
    }

    // 创建交互提示标签（跟随摄像机，初始隐藏）
    _interactionLabel = Label::createWithSystemFont(u8"休息", "fonts/ZCOOLXiaoWei-Regular.ttf", 24);
    _interactionLabel->setTextColor(Color4B::WHITE);
    _interactionLabel->setVisible(false);
    this->addChild(_interactionLabel, 100, "InteractionLabel");

    // 初始化摄像机偏移
    _cameraOffsetY = 0.0f;
    _targetCameraOffsetY = 0.0f;

    // 启用 update 实现摄像机跟随
    this->scheduleUpdate();

    // 移除黑层
    if (blackLayer1 && blackLayer1->getParent()) {
        blackLayer1->removeFromParent();
    }
    
    CCLOG("加载了 %zu 个碰撞平台", _platforms.size());

    return true;
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
            chairObj.radius = 50.0f;
            
            _interactiveObjects.push_back(chairObj);
            
            CCLOG("加载交互对象: %s at (%.1f, %.1f)", name.c_str(), chairObj.position.x, chairObj.position.y);
        }
        // 检测出口对象
        else if (name == "Exit")
        {
            InteractiveObject exitObj;
            exitObj.position = Vec2(x + width / 2, y + height / 2);
            exitObj.name = "Exit";
            exitObj.radius = 50.0f;
            
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

    for (auto& obj : _interactiveObjects)
    {
        float distance = playerPos.distance(obj.position);
        
        if (distance < obj.radius)
        {
            if (obj.name == "Chair")
            {
                showPrompt = true;
                _interactionLabel->setString(u8"休息");
                _interactionLabel->setPosition(Vec2(playerPos.x, playerPos.y + 80));
            }
            else if (obj.name == "Exit")
            {
                // 进入出口，切换到 BossScene
                _isTransitioning = true;
                
                auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 0));
                this->addChild(blackLayer, 1000);
                
                blackLayer->runAction(Sequence::create(
                    FadeIn::create(0.5f),
                    CallFunc::create([]() {
                        Director::getInstance()->replaceScene(
                            TransitionFade::create(0.5f, BossScene::createScene(), Color3B::BLACK)
                        );
                    }),
                    nullptr
                ));
                return;
            }
        }
    }

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
}

void GameScene::update(float dt)
{
    // 更新摄像机
    updateCamera();
    
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