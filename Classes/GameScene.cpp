#include "GameScene.h"

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

    // 3个地图块的配置（根据实际拆分调整位置）
    std::vector<MapChunk> chunks = {
        {"Maps/Dirtmouth1.tmx", Vec2(0, 0)},
        {"Maps/Dirtmouth2.tmx", Vec2(150 * 16, 0)},
        {"Maps/Dirtmouth3.tmx", Vec2(300 * 16, 0)}
    };

    float scale = 1.2f;

    // 加载所有地图块
    for (const auto& chunk : chunks) {
        auto map = TMXTiledMap::create(chunk.file);
        CCASSERT(map != nullptr, ("地图加载失败: " + chunk.file).c_str());

        map->setScale(scale);
        map->setAnchorPoint(Vec2::ZERO);
        map->setPosition(Vec2(origin.x + chunk.position.x * scale, 
                              origin.y + chunk.position.y * scale));
        this->addChild(map, 0);
    }

    // 从地图块获取玩家起始点
    auto firstMap = TMXTiledMap::create("Maps/Dirtmouth2.tmx");
    auto objectGroup = firstMap->getObjectGroup("Objects");
    CCASSERT(objectGroup != nullptr, "地图缺少对象层 Objects");

    auto startPoint = objectGroup->getObject("PlayerStart");
    float startX = startPoint["x"].asFloat() * scale;
    float startY = startPoint["y"].asFloat() * scale;

    // 设置摄像机位置并移除黑层
    auto camera = this->getDefaultCamera();
    camera->setPosition(Vec2(3500 * scale, 450 * scale));

    auto removeBlack1 = CallFunc::create([blackLayer1]() {
        if (blackLayer1 && blackLayer1->getParent()) {
            blackLayer1->removeFromParent();
        }
    });
    return true;
}