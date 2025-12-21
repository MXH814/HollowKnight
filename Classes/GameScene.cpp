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

    // 获取屏幕尺寸
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 加载 TMX 地图
    auto map = TMXTiledMap::create("Maps/d.tmx");
    CCASSERT(map != nullptr, "TMX 地图加载失败");
    // 【关键点 1】锚点必须是 (0,0)
    map->setScale(3.3f);
    map->setAnchorPoint(Vec2::ZERO);

    // 【关键点 2】位置必须是 (0,0)
    map->setPosition(origin);
    this->addChild(map, 0);

    auto objectGroup = map->getObjectGroup("Objects");
    CCASSERT(objectGroup != nullptr, "地图缺少对象层 Objects");

    auto startPoint = objectGroup->getObject("PlayerStart");
    float startX = startPoint["x"].asFloat();
    float startY = startPoint["y"].asFloat();

    auto camera = this->getDefaultCamera();
    // 获取摄像机可移动范围
    float minX = visibleSize.width / 2;
    float minY = visibleSize.height / 2;
    float maxX = map->getContentSize().width * map->getScale() - visibleSize.width / 2;
    float maxY = map->getContentSize().height * map->getScale() - visibleSize.height / 2;

    // 限制摄像机位置
    float camX = std::max(minX, std::min(startX, maxX));
    float camY = std::max(minY, std::min(startY, maxY));
    camera->setPosition(Vec2(4000, 600));

    // 输出地图信息（调试用）
    Size mapSize = map->getContentSize();
    CCLOG("Map Size: %f x %f", mapSize.width, mapSize.height);
    CCLOG("Screen Size: %f x %f", visibleSize.width, visibleSize.height);

    return true;
}