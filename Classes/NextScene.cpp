#include "NextScene.h"
#include "TheKnight.h"

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

    // 初始化落地标志
    _hasLandedOnce = false;

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
        {"Maps/Forgotten Crossroads3.tmx", Vec2(0, 120 * 16)},
        {"Maps/Forgotten Crossroads4.tmx", Vec2(120 * 16, 120 * 16)}
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
        
        // 为地图1创建trap1背景精灵
        if (chunk.file == "Maps/Forgotten Crossroads1.tmx") {
            createTrapSprites(map, "Collision", "trap1", "Maps/solidtrap.png", scale, mapPos);
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

    // 启用 update 实现摄像机跟随
    this->scheduleUpdate();

    // 移除黑层
    if (blackLayer && blackLayer->getParent()) {
        blackLayer->removeFromParent();
    }

    return true;
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

        // 获取类型（兼容 type 和 class 属性）
        std::string type = "";
        if (dict.find("type") != dict.end()) {
            type = dict["type"].asString();
        }
        if (dict.find("class") != dict.end()) {
            type = dict["class"].asString();
        }

        // 只处理指定类型的陷阱
        if (type != trapType) {
            continue;
        }

        float x = dict["x"].asFloat() * scale + mapOffset.x;
        float y = (dict["y"].asFloat() + 166)* scale + mapOffset.y;
        float width = dict["width"].asFloat() * scale;
        float height = dict["height"].asFloat() * scale;

        if (width > 0 && height > 0)
        {
            auto sprite = Sprite::create(spritePath);
            if (sprite) {
                // 设置精灵大小匹配对象区域
                Size originalSize = sprite->getContentSize();
                sprite->setScaleX(width / originalSize.width);
                sprite->setScaleY(height / originalSize.height);
                
                // 设置位置（对象左下角 + 尺寸一半 = 中心点）
                sprite->setPosition(Vec2(x + width / 2, y + height / 2));
                
                // 添加到场景，zOrder为1使其显示在地图之上但在玩家之下
                this->addChild(sprite, 1);
                
                CCLOG("创建陷阱精灵: x=%.1f, y=%.1f, w=%.1f, h=%.1f", x, y, width, height);
            }
            else {
                CCLOG("警告：无法加载精灵 %s", spritePath.c_str());
            }
        }
    }
}

void NextScene::update(float dt)
{
    auto knight = this->getChildByName("Player");
    if (!knight) return;

    // Layer 没有摄像机，获取父场景的摄像机
    auto scene = this->getScene();
    if (!scene) return;

    auto camera = scene->getDefaultCamera();
    if (!camera) return;

    // 摄像机平滑跟随角色
    Vec2 knightPos = knight->getPosition();
    Vec2 cameraPos = camera->getPosition();
    
    // 添加 Y 轴偏移
    float cameraOffsetY = 200.0f;
    Vec2 targetPos = Vec2(knightPos.x, knightPos.y + cameraOffsetY);
    
    // 平滑插值
    float lerpFactor = 0.1f;
    Vec2 newPos = cameraPos + (targetPos - cameraPos) * lerpFactor;
    
    camera->setPosition(newPos);
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
            // 创建 Platform 结构体，与GameScene相同的碰撞方式
            Platform platform;
            platform.rect = Rect(x, y, width, height);
            platform.node = nullptr;
            _platforms.push_back(platform);

            CCLOG("创建碰撞平台: x=%.1f, y=%.1f, w=%.1f, h=%.1f", x, y, width, height);
        }
    }
}