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
    _mapScale = 0.8f;
    _map = TMXTiledMap::create("Maps/Bossroom.tmx");
    CCASSERT(_map != nullptr, "TMX 地图加载失败");

    _map->setAnchorPoint(Vec2::ZERO);
    _map->setPosition(Vec2::ZERO);
    _map->setScale(_mapScale);
    this->addChild(_map, 0);
    
    // 计算缩放后的地图尺寸
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
        this->addChild(_knight, 10);
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

    // 初始化摄像机偏移
    _cameraOffsetY = 0.0f;
    _targetCameraOffsetY = 0.0f;

    // 直接移除黑层
    if (blackLayer && blackLayer->getParent()) {
        blackLayer->removeFromParent();
    }

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
        platform.node = nullptr;  // TMX地图碰撞不需要节点
        _platforms.push_back(platform);
        
        // 调试输出
        std::string name = dict["name"].asString();
        CCLOG("解析碰撞区域: %s - x:%.1f y:%.1f w:%.1f h:%.1f", 
              name.c_str(), x, y, width, height);
    }
    
    CCLOG("共解析 %zu 个碰撞区域", _platforms.size());
}

void BossScene::updateCamera()
{
    if (!_knight) return;
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 knightPos = _knight->getPosition();
    
    // 根据看向状态设置目标偏移
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
    
    // 平滑插值镜头偏移
    float offsetLerpFactor = 0.05f;
    _cameraOffsetY += (_targetCameraOffsetY - _cameraOffsetY) * offsetLerpFactor;
    
    // 计算摄像机目标位置（让骑士在屏幕中央）
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

void BossScene::update(float dt)
{
    updateCamera();
    updateHPLabel();
    updateSoulLabel();
}

void BossScene::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}