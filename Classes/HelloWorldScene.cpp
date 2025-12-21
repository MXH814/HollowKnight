#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include <random>

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 设置地图大小（比屏幕大很多）
    _mapSize = Size(visibleSize.width * 4, visibleSize.height * 3);

    // 创建游戏层
    _gameLayer = Node::create();
    _gameLayer->setContentSize(_mapSize);
    this->addChild(_gameLayer, 0);

    // 添加绿色背景（覆盖整个地图）
    auto background = LayerColor::create(Color4B(100, 180, 100, 255), _mapSize.width, _mapSize.height);
    _gameLayer->addChild(background, 0);

    // 添加关闭按钮（固定在屏幕上，不随镜头移动）
    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
        float y = origin.y + closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));
    }

    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100);  // UI层在最上面

    // 创建HP显示标签
    _hpLabel = Label::createWithTTF("HP: 5", "fonts/Marker Felt.ttf", 32);
    if (_hpLabel)
    {
        _hpLabel->setAnchorPoint(Vec2(0, 1));
        _hpLabel->setPosition(Vec2(origin.x + 20, origin.y + visibleSize.height - 20));
        _hpLabel->setTextColor(Color4B(255, 255, 255, 255));
        this->addChild(_hpLabel, 100);  // UI层
    }
    
    // 创建Soul显示标签
    _soulLabel = Label::createWithTTF("Soul: 0/6", "fonts/Marker Felt.ttf", 32);
    if (_soulLabel)
    {
        _soulLabel->setAnchorPoint(Vec2(1, 1));
        _soulLabel->setPosition(Vec2(origin.x + visibleSize.width - 20, origin.y + visibleSize.height - 20));
        _soulLabel->setTextColor(Color4B(200, 200, 255, 255));
        this->addChild(_soulLabel, 100);  // UI层
    }

    // 创建平台
    createPlatforms();
    createRandomPlatforms();

    // 创建小骑士并添加到游戏层
    _knight = TheKnight::create();
    if (_knight)
    {
        // 将小骑士放置在地面平台上
        _knight->setPosition(Vec2(200, 60));
        _knight->setScale(1.0f);
        _knight->setPlatforms(_platforms);
        _gameLayer->addChild(_knight, 10);
    }
    else
    {
        problemLoading("TheKnight sprites");
    }

    // 创建敌人
    createEnemies();

    // 初始化镜头偏移
    _cameraOffsetY = 0.0f;
    _targetCameraOffsetY = 0.0f;

    // 启用update
    this->scheduleUpdate();

    return true;
}

void HelloWorld::createEnemies()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // 在地面上生成一些敌人
    int numEnemies = 8;
    float groundY = 50;  // 地面高度
    
    std::uniform_int_distribution<> xDist(200, (int)(_mapSize.width - 200));
    
    for (int i = 0; i < numEnemies; i++)
    {
        auto enemy = Enemy::create();
        if (enemy)
        {
            float x = 300 + i * 400;  // 间隔分布
            if (x > _mapSize.width - 100) x = xDist(gen);
            
            enemy->setPosition(Vec2(x, groundY));
            enemy->setMoveBounds(x - 150, x + 150);  // 设置移动范围
            _gameLayer->addChild(enemy, 5);
            _enemies.push_back(enemy);
        }
    }
}

void HelloWorld::updateHPLabel()
{
    if (_hpLabel && _knight)
    {
        char hpText[32];
        sprintf(hpText, "HP: %d", _knight->getHP());
        _hpLabel->setString(hpText);
    }
}

void HelloWorld::updateSoulLabel()
{
    if (_soulLabel && _knight)
    {
        char soulText[32];
        sprintf(soulText, "Soul: %d/%d", _knight->getSoul(), _knight->getMaxSoul());
        _soulLabel->setString(soulText);
    }
}

void HelloWorld::updateCombat(float dt)
{
    if (!_knight) return;
    
    Rect knightRect = _knight->getBoundingBox();
    
    // 获取攻击特效碰撞盒
    Rect slashRect;
    bool hasSlashEffect = _knight->getSlashEffectBoundingBox(slashRect);
    
    // 获取法术特效碰撞盒
    Sprite* vengefulSpirit = _knight->getVengefulSpiritEffect();
    Rect spellRect;
    bool hasSpellEffect = false;
    if (vengefulSpirit)
    {
        auto effectSize = vengefulSpirit->getContentSize();
        auto effectPos = vengefulSpirit->getPosition();
        spellRect = Rect(effectPos.x - effectSize.width / 2,
                        effectPos.y - effectSize.height / 2,
                        effectSize.width,
                        effectSize.height);
        hasSpellEffect = true;
    }
    
    // 遍历所有敌人
    for (auto it = _enemies.begin(); it != _enemies.end(); )
    {
        Enemy* enemy = *it;
        
        if (enemy->isDead())
        {
            enemy->removeFromParent();
            it = _enemies.erase(it);
            continue;
        }
        
        Rect enemyRect = enemy->getBoundingBox();
        
        // 检测骑士攻击是否命中敌人（使用受击冷却判断）
        if (hasSlashEffect && enemy->canTakeDamage() && slashRect.intersectsRect(enemyRect))
        {
            enemy->takeDamage(1);
            // 攻击命中，获得Soul（考虑SoulCatcher护符）
            int soulGain = 1;
            if (_knight->getCharmSoulCatcher())
            {
                soulGain += 1;  // SoulCatcher: +1 Soul
            }
            _knight->addSoul(soulGain);
        }
        
        // 检测法术是否命中敌人
        if (hasSpellEffect && enemy->canTakeDamage() && spellRect.intersectsRect(enemyRect))
        {
            // 法术伤害（考虑ShamanStone护符）
            int spellDamage = 2;
            if (_knight->getCharmShamanStone())
            {
                spellDamage += 1;  // ShamanStone: +1 伤害
            }
            enemy->takeDamage(spellDamage);
            // 法术命中不获得灵魂
        }
        
        // 检测敌人是否碰到骑士
        if (!_knight->isInvincible() && !_knight->isStunned() && 
            enemyRect.intersectsRect(knightRect))
        {
            // 计算敌人相对于骑士的位置，决定后退方向
            Vec2 knightPos = _knight->getPosition();
            Vec2 enemyPos = enemy->getPosition();
            bool fromRight = enemyPos.x > knightPos.x;
            
            _knight->setKnockbackDirection(fromRight);
            _knight->takeDamage(1);
            updateHPLabel();
        }
        
        ++it;
    }
}

void HelloWorld::createPlatforms()
{
    // 地面平台（覆盖整个地图底部）
    {
        auto ground = LayerColor::create(Color4B(80, 60, 50, 255));
        ground->setContentSize(Size(_mapSize.width, 50));
        ground->setPosition(Vec2(0, 0));
        _gameLayer->addChild(ground, 1);
        
        Platform platform;
        platform.rect = Rect(0, 0, _mapSize.width, 50);
        platform.node = ground;
        _platforms.push_back(platform);
    }
    
    // 左边界墙
    {
        auto wall = LayerColor::create(Color4B(60, 50, 45, 255));
        wall->setContentSize(Size(30, _mapSize.height));
        wall->setPosition(Vec2(0, 0));
        _gameLayer->addChild(wall, 1);
        
        Platform platform;
        platform.rect = Rect(0, 0, 30, _mapSize.height);
        platform.node = wall;
        _platforms.push_back(platform);
    }
    
    // 右边界墙
    {
        auto wall = LayerColor::create(Color4B(60, 50, 45, 255));
        wall->setContentSize(Size(30, _mapSize.height));
        wall->setPosition(Vec2(_mapSize.width - 30, 0));
        _gameLayer->addChild(wall, 1);
        
        Platform platform;
        platform.rect = Rect(_mapSize.width - 30, 0, 30, _mapSize.height);
        platform.node = wall;
        _platforms.push_back(platform);
    }
}

void HelloWorld::createRandomPlatforms()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // 平台宽度范围
    std::uniform_int_distribution<> widthDist(150, 350);
    // 平台高度（厚度）
    int platformHeight = 20;
    
    // 在地图中生成多层平台
    int numLayers = 6;  // 平台层数
    float layerHeight = (_mapSize.height - 100) / numLayers;
    
    for (int layer = 0; layer < numLayers; layer++)
    {
        float baseY = 100 + layer * layerHeight;
        
        // 每层生成多个平台
        int platformsPerLayer = 4 + layer % 2;  // 每层4-5个平台
        float sectionWidth = (_mapSize.width - 100) / platformsPerLayer;
        
        for (int i = 0; i < platformsPerLayer; i++)
        {
            // 随机决定是否生成这个平台（增加随机性）
            std::uniform_int_distribution<> skipDist(0, 10);
            if (skipDist(gen) < 2) continue;  // 20%概率跳过
            
            int platWidth = widthDist(gen);
            
            // X位置在该区段内随机
            float minX = 50 + i * sectionWidth;
            float maxX = minX + sectionWidth - platWidth;
            if (maxX < minX) maxX = minX;
            
            std::uniform_int_distribution<> xDist((int)minX, (int)maxX);
            float platX = xDist(gen);
            
            // Y位置在层高范围内随机偏移
            std::uniform_int_distribution<> yOffsetDist(-30, 30);
            float platY = baseY + yOffsetDist(gen);
            
            // 随机颜色（深色系）
            std::uniform_int_distribution<> colorDist(60, 100);
            Color4B color(colorDist(gen), colorDist(gen) - 10, colorDist(gen) - 20, 255);
            
            auto plat = LayerColor::create(color);
            plat->setContentSize(Size(platWidth, platformHeight));
            plat->setPosition(Vec2(platX, platY));
            _gameLayer->addChild(plat, 1);
            
            Platform platform;
            platform.rect = Rect(platX, platY, platWidth, platformHeight);
            platform.node = plat;
            _platforms.push_back(platform);
        }
    }
    
    // 添加一些障碍物（竖直的柱子）
    std::uniform_int_distribution<> obstacleDist(0, 100);
    int numObstacles = 8;
    
    for (int i = 0; i < numObstacles; i++)
    {
        float obstacleX = 150 + ((_mapSize.width - 300) / numObstacles) * i + obstacleDist(gen);
        
        std::uniform_int_distribution<> heightDist(80, 200);
        int obstacleHeight = heightDist(gen);
        int obstacleWidth = 30;
        
        // 随机放在某个平台上或地面上
        std::uniform_int_distribution<> yPosDist(50, 300);
        float obstacleY = yPosDist(gen);
        
        Color4B color(50, 45, 40, 255);
        
        auto obstacle = LayerColor::create(color);
        obstacle->setContentSize(Size(obstacleWidth, obstacleHeight));
        obstacle->setPosition(Vec2(obstacleX, obstacleY));
        _gameLayer->addChild(obstacle, 1);
        
        Platform platform;
        platform.rect = Rect(obstacleX, obstacleY, obstacleWidth, obstacleHeight);
        platform.node = obstacle;
        _platforms.push_back(platform);
    }
}

void HelloWorld::updateCamera()
{
    if (!_knight) return;
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 knightPos = _knight->getPosition();
    
    // 根据看向状态更新目标偏移
    float lookOffset = 150.0f;  // 向上/下看时镜头偏移量
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
    
    // 平滑过渡镜头偏移
    float offsetLerpFactor = 0.05f;
    _cameraOffsetY += (_targetCameraOffsetY - _cameraOffsetY) * offsetLerpFactor;
    
    // 计算相机应该移动到的位置（让骑士保持在屏幕中心）
    float targetX = -knightPos.x + visibleSize.width / 2;
    float targetY = -knightPos.y + visibleSize.height / 2 - _cameraOffsetY;
    
    // 限制相机范围，不要超出地图边界
    targetX = std::min(targetX, 0.0f);
    targetX = std::max(targetX, -(_mapSize.width - visibleSize.width));
    
    targetY = std::min(targetY, 0.0f);
    targetY = std::max(targetY, -(_mapSize.height - visibleSize.height));
    
    // 平滑移动相机（插值）
    Vec2 currentPos = _gameLayer->getPosition();
    float lerpFactor = 0.1f;  // 平滑因子
    
    float newX = currentPos.x + (targetX - currentPos.x) * lerpFactor;
    float newY = currentPos.y + (targetY - currentPos.y) * lerpFactor;
    
    _gameLayer->setPosition(Vec2(newX, newY));
}

void HelloWorld::update(float dt)
{
    updateCamera();
    updateCombat(dt);
    updateHPLabel();
    updateSoulLabel();
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}