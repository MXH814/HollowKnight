#include "NextScene.h"
#include "TheKnight.h"
#include "GameScene.h"
#include "CharmManager.h"
#include "Monster/MonsterSpawner.h"
#include "Monster/CrawlidMonster.h"
#include "Monster/TiktikMonster.h"
#include "Monster/GruzzerMonster.h" // 【新增】添加 GruzzerMonster 头文件
#include "Monster/VengeflyMonster.h" // 【新增】添加 VengeflyMonster 头文件
#include "SimpleAudioEngine.h"
#include "GeoManager.h"

USING_NS_CC;
using namespace CocosDenshion;

// 静态变量初始化（在文件顶部，USING_NS_CC 之后）
bool NextScene::s_isRespawning = false;
Vec2 NextScene::s_shadePosition = Vec2::ZERO;

Scene* NextScene::createScene()
{
    // 【新增】自动检测是否有保存的 Shade 位置
    if (s_shadePosition != Vec2::ZERO)
    {
        CCLOG("NextScene::createScene - Auto-detecting shade spawn needed");
        return createSceneWithRespawn();
    }
    
    s_isRespawning = false;  // 重置重生标志
    
    auto scene = Scene::create();
    auto layer = NextScene::create();
    scene->addChild(layer);

    auto knight = layer->getChildByName("Player");
    if (knight) {
        Size visibleSize = Director::getInstance()->getVisibleSize();
        float verticalOffset = visibleSize.height / 6.0f;
        scene->getDefaultCamera()->setPosition(
            Vec2(knight->getPositionX(), knight->getPositionY() + verticalOffset)
        );
    }

    return scene;
}

// 新增：带重生标志的场景创建方法
Scene* NextScene::createSceneWithRespawn()
{
    s_isRespawning = true;  // 设置重生标志
    
    CCLOG("NextScene::createSceneWithRespawn - Knight will respawn");
    
    auto scene = Scene::create();
    auto layer = NextScene::create();
    scene->addChild(layer);

    auto knight = layer->getChildByName("Player");
    if (knight) {
        Size visibleSize = Director::getInstance()->getVisibleSize();
        float verticalOffset = visibleSize.height / 6.0f;
        scene->getDefaultCamera()->setPosition(
            Vec2(knight->getPositionX(), knight->getPositionY() + verticalOffset)
        );
    }

    return scene;
}

bool NextScene::init()
{
    if (!Layer::init())
        return false;

    // 【新增】调试日志
    CCLOG("====== NextScene::init ======");
    CCLOG("  s_isRespawning: %s", s_isRespawning ? "TRUE" : "FALSE");
    CCLOG("  s_shadePosition: (%.1f, %.1f)", s_shadePosition.x, s_shadePosition.y);
    CCLOG("=============================");

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _hasLandedOnce = false;
    _isTransitioning = false;
    _isNearExit = false;
    _isNearThorn = false;
    
    _isInSpikeDeath = false;
    _lastSafePosition = Vec2::ZERO;
    _spikeDeathTimer = 0.0f;
    _spikeDeathPhase = 0;
    _blackScreen = nullptr;
    
    _isShaking = false;
    _shakeDuration = 0.0f;
    _shakeElapsed = 0.0f;
    _shakeIntensity = 0.0f;
    _shakeOffset = Vec2::ZERO;
    
    _lastDisplayedHP = 0;
    _lastDisplayedSoul = 0;
    
    // 初始化攻击冷却时间
    _knightAttackCooldown = 0.0f;
    _spellAttackCooldown = 0.0f;
    
    // 【新增】初始化 _shade 为 nullptr
    _shade = nullptr;

    auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 255));
    this->addChild(blackLayer, 10, "LoadingBlack");

    struct MapChunk {
        std::string file;
        Vec2 position;
    };

    float scale = 2.61f;

    std::vector<MapChunk> chunks = {
        {"Maps/Forgotten Crossroads1.tmx", Vec2(0, 0)},
        {"Maps/Forgotten Crossroads2.tmx", Vec2(120 * 16, 0)},
        {"Maps/Forgotten Crossroads3.tmx", Vec2(0, 105 * 16)},
        {"Maps/Forgotten Crossroads4.tmx", Vec2(120 * 16, 105 * 16)}
    };

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
        loadForegroundObjects(map, scale, mapPos);
        
        if (chunk.file == "Maps/Forgotten Crossroads1.tmx") {
            createTrapSprites(map, "Collision", "trap1", "Maps/solidtrap.png", scale, mapPos);
            loadThornObjects(map, scale, mapPos);
        }
        
        if (chunk.file == "Maps/Forgotten Crossroads4.tmx") {
            loadExitObjects(map, scale, mapPos);
            loadGeoObjects(map, scale, mapPos);  // 【新增】加载 Geo 对象
        }
    }

    auto fourthMap = TMXTiledMap::create("Maps/Forgotten Crossroads4.tmx");
    auto objectGroup = fourthMap->getObjectGroup("Objects");
    CCASSERT(objectGroup != nullptr, "地图缺少对象层 Objects");

    auto startPoint = objectGroup->getObject("PlayerStart");
    
    Vec2 map4Offset = Vec2(120 * 16 * scale, 120 * 16 * scale);
    float startX = startPoint["x"].asFloat() * scale + origin.x + map4Offset.x;
    float startY = startPoint["y"].asFloat() * scale + origin.y + map4Offset.y;

    CCLOG("玩家起始位置: x=%.1f, y=%.1f", startX, startY);

    auto knight = TheKnight::create();
    if (knight)
    {
        // 处理重生逻辑
        if (s_isRespawning)
        {
            CCLOG("==== Processing respawn logic in NextScene ====");
            
            // 重生位置使用最后的安全位置，如果没有则使用默认位置
            Vec2 respawnPos = _lastSafePosition;
            if (respawnPos == Vec2::ZERO)
            {
                // 使用默认起始位置
                respawnPos = Vec2(startX, startY);
            }
            
            CCLOG("Knight respawning at position: (%.1f, %.1f)", respawnPos.x, respawnPos.y);
            knight->setPosition(respawnPos);
            knight->setScale(1.0f);
            
            // 重置血量为满血
            knight->setHP(knight->getMaxHP());
        }
        else
        {
            knight->setPosition(Vec2(startX, startY));
            knight->setScale(1.0f);
        }
        
        knight->setPlatforms(_platforms);
        this->addChild(knight, 5, "Player");
        
        // 【修改】立即设置 _player 引用
        _player = knight;
        
        CharmManager::getInstance()->syncToKnight(knight);
        
        // 【修改】重生逻辑：延迟生成 Shade，确保 _player 已经设置
        if (s_isRespawning)
        {
            CCLOG("Knight added to scene, setting up respawn...");
            CCLOG("_player pointer: %p", _player);  // 调试日志
            CCLOG("s_shadePosition: (%.1f, %.1f)", s_shadePosition.x, s_shadePosition.y);  // 【新增】调试日志
            
            // 延迟一帧后生成 Shade
            this->scheduleOnce([this](float dt) {
                // 如果有 Shade 位置，生成 Shade
                if (s_shadePosition != Vec2::ZERO)
                {
                    CCLOG("Spawning Shade at death position: (%.1f, %.1f)", s_shadePosition.x, s_shadePosition.y);
                    CCLOG("_player pointer before spawn: %p", _player);  // 调试日志
                    
                    this->spawnShade(s_shadePosition);
                    
                    // 【修改】只重置重生标志，不重置 Shade 位置
                    // Shade 位置会在 Shade 被击败时才重置
                }
                else
                {
                    CCLOG("Warning: s_shadePosition is ZERO, not spawning Shade");
                }
                
                // 重置重生标志
                s_isRespawning = false;
                // 【删除】不在这里重置 s_shadePosition
                // s_shadePosition = Vec2::ZERO;  // ❌ 删除这行
            }, 0.1f, "respawn_delay");
        }
    }

    // 生成所有怪物和 NPC
    MonsterSpawner::spawnAllCrawlidsInNextScene(this);

    // 【修改】获取 Cornifer NPC 的引用 - 添加更详细的调试信息
    _cornifer = this->getChildByName<CorniferNPC*>("NPC_Cornifer");
    if (_cornifer)
    {
        CCLOG("[NextScene] ✓ 成功获取 Cornifer NPC 引用");
        CCLOG("    Cornifer 位置: (%.1f, %.1f)", _cornifer->getPositionX(), _cornifer->getPositionY());
    }
    else
    {
        CCLOG("[NextScene] ✗ 警告: 未找到 Cornifer NPC！");
        CCLOG("    检查 MonsterSpawner 是否成功生成了 Cornifer");
    }
    
    createHPAndSoulUI();

    // 【新增】创建暂停菜单
    _pauseMenu = PauseMenu::create();
    if (_pauseMenu)
    {
        _uiLayer->addChild(_pauseMenu, 2000);
    }

    // 【修改】创建出口提示容器（包含文字和装饰图片）
    _exitContainer = Node::create();
    this->addChild(_exitContainer, 100, "ExitContainer");
    _exitContainer->setVisible(false);
    
    _exitLabel = Label::createWithSystemFont(u8"按 W 上升", "fonts/NotoSerifCJKsc-Regular.otf", 36);
    _exitLabel->setTextColor(Color4B::WHITE);
    _exitLabel->setPosition(Vec2::ZERO);
    _exitContainer->addChild(_exitLabel, 1);
    
    // 添加顶部装饰图片
    _exitTopImg = Sprite::create("Menu/pausemenu_top.png");
    if (_exitTopImg)
    {
        _exitTopImg->setPosition(Vec2(0, 60));
        _exitTopImg->setScale(0.6f);
        _exitContainer->addChild(_exitTopImg, 0);
    }
    
    // 添加底部装饰图片
    _exitBottomImg = Sprite::create("Menu/pausemenu_bottom.png");
    if (_exitBottomImg)
    {
        _exitBottomImg->setPosition(Vec2(0, -50));
        _exitBottomImg->setScale(0.6f);
        _exitContainer->addChild(_exitBottomImg, 0);
    }

    // 修改键盘监听器，添加 ESC 键处理
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        // 【新增】ESC 键打开/关闭暂停菜单
        if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
        {
            if (_pauseMenu)
            {
                if (_pauseMenu->isVisible())
                {
                    _pauseMenu->hide();
                }
                else
                {
                    _pauseMenu->show();
                }
            }
            return;
        }
        
        // 【新增】暂停时不处理其他按键
        if (_pauseMenu && _pauseMenu->isVisible())
        {
            return;
        }
        
        // 【新增】TAB 键显示地图
        if (keyCode == EventKeyboard::KeyCode::KEY_TAB)
        {
            showMap();
            return;
        }
        
        if (keyCode == EventKeyboard::KeyCode::KEY_W || keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_W)
        {
            if (_isNearExit && !_isTransitioning)
            {
                _isTransitioning = true;
                
                auto knight = dynamic_cast<TheKnight*>(this->getChildByName("Player"));
                bool facingRight = true;
                if (knight)
                {
                    facingRight = knight->getScaleX() > 0;
                }
                
                Vec2 spawnPos(12479.7f, 435.0f);
                
                auto blackScene = Scene::create();
                auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 255));
                blackScene->addChild(blackLayer);

                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, blackScene));

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
        
        if (keyCode == EventKeyboard::KeyCode::KEY_Q)
        {
            auto charmManager = CharmManager::getInstance();
            auto scene = this->getScene();
            if (!scene) return;
            
            if (charmManager->isPanelOpen())
            {
                charmManager->hideCharmPanel();
                auto knight = dynamic_cast<TheKnight*>(this->getChildByName("Player"));
                if (knight)
                {
                    charmManager->syncToKnight(knight);
                }
                return;
            }
            
            charmManager->showCharmPanel(scene);
        }
    };
    
    // 【新增】按键释放事件
    keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        // TAB 键释放时隐藏地图
        if (keyCode == EventKeyboard::KeyCode::KEY_TAB)
        {
            hideMap();
        }
    };
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

    // 启用 update
    this->scheduleUpdate();

    // 播放 Crossroads 背景音乐（循环）
    CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("Music/Crossroads.wav", true);

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
        exitObj.radius = std::max(width, height) / 2 + 72.5f;  // 50.0f * 1.45 = 72.5f
        
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
        thornObj.size = Size(width + 72.5f, height + 72.5f);  // 50.0f * 1.45 = 72.5f
        
        _thornObjects.push_back(thornObj);
        
        CCLOG("加载尖刺对象: at (%.1f, %.1f), size=(%.1f, %.1f)", 
              thornObj.position.x, thornObj.position.y, 
              thornObj.size.width, thornObj.size.height);
    }
}

// 新增：加载 Geo 对象
void NextScene::loadGeoObjects(TMXTiledMap* map, float scale, const Vec2& mapOffset)
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
        
        if (objClass != "Geo") {
            continue;
        }
        
        float x = dict["x"].asFloat() * scale + mapOffset.x;
        float y = dict["y"].asFloat() * scale + mapOffset.y;
        float width = dict["width"].asFloat() * scale;
        float height = dict["height"].asFloat() * scale;
        
        GeoObject geoObj;
        geoObj.position = Vec2(x + width / 2, y + height / 2);
        geoObj.size = Size(width, height);
        
        _geoObjects.push_back(geoObj);
        
        CCLOG("加载 Geo 对象: at (%.1f, %.1f), size=(%.1f, %.1f)", 
              geoObj.position.x, geoObj.position.y, 
              geoObj.size.width, geoObj.size.height);
    }
    
    CCLOG("共加载 %zu 个 Geo 对象", _geoObjects.size());
}

void NextScene::checkInteractions()
{
    if (_isTransitioning) return;
    
    auto knight = this->getChildByName("Player");
    if (!knight) return;

    Vec2 knightPos = knight->getPosition();
    
    // 检测出口
    _isNearExit = false;
    if (_exitContainer) {
        for (auto& exitObj : _exitObjects)
        {
            float distance = knightPos.distance(exitObj.position);
            
            if (distance < exitObj.radius)
            {
                _isNearExit = true;
                _exitContainer->setPosition(Vec2(knightPos.x, knightPos.y + 200));
                break;
            }
        }
        _exitContainer->setVisible(_isNearExit);
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
    
    // 【新增】检查是否是最后一条生命
    int currentHP = knight->getHP();
    
    CCLOG("=== 尖刺碰撞检测 ===");
    CCLOG("  当前血量: %d", currentHP);
    
    // 【修改】如果是最后一条命,触发特殊的尖刺死亡流程
    if (currentHP <= 1)
    {
        CCLOG("  -> 最后一条命!触发完整尖刺死亡流程");
        
        // 保存死亡位置
        Vec2 currentPos = knight->getPosition();
        s_shadePosition = currentPos;
        CCLOG("  -> 保存 Shade 位置: (%.1f, %.1f)", s_shadePosition.x, s_shadePosition.y);
        
        // 【关键修改】进入尖刺死亡流程，但使用特殊的阶段5（最后一条命）
        _isInSpikeDeath = true;
        _spikeDeathPhase = 5;  // 新阶段：最后一条命的死亡
        _spikeDeathTimer = 0.0f;
        
        // 调用骑士的尖刺死亡函数（播放动画）
        knight->startSpikeDeath();
        
        CCLOG("  -> 进入阶段5：播放完整SpikeDeath动画后触发真正死亡");
        return;
    }
    
    // 【保留】原有的尖刺重生逻辑(血量 > 1 时)
    _isInSpikeDeath = true;
    _spikeDeathPhase = 1;
    _spikeDeathTimer = 0.0f;
    
    // 临时保存位置(重生后会清除)
    Vec2 currentPos = knight->getPosition();
    s_shadePosition = currentPos;
    CCLOG("  -> 尖刺重生模式(血量 > 1)");
    CCLOG("  -> 临时保存位置: (%.1f, %.1f)", s_shadePosition.x, s_shadePosition.y);
    
    // 调用骑士的尖刺死亡函数
    knight->startSpikeDeath();
    
    CCLOG("尖刺死亡流程开始,阶段1:播放SpikeDeath动画");
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
                // 【新增】重生完成后，清除临时保存的 Shade 位置
                // 因为这不是真正的死亡，只是原地重生
                s_shadePosition = Vec2::ZERO;
                CCLOG("尖刺重生完成，清除临时 Shade 位置");
                
                // 重生完成
                _isInSpikeDeath = false;
                _spikeDeathPhase = 0;
                _spikeDeathTimer = 0.0f;
                CCLOG("尖刺死亡流程完成");
            }
            break;
        }
        
        case 5:  // 【修改】阶段5：最后一条命的尖刺死亡 - 等待SpikeDeath动画完成后退出尖刺死亡模式
        {
            // 【关键修改】等待SpikeDeath动画播放完成（0.64秒）
            if (_spikeDeathTimer >= 0.64f)
            {
                CCLOG("  -> SpikeDeath动画播放完成");
                CCLOG("  -> 退出尖刺死亡模式，让普通死亡流程接管");
                
                // 【关键】设置HP=0，触发普通死亡流程
                knight->setHP(0);
                
                // 【关键】退出尖刺死亡模式，让update()中的普通死亡检测接管
                _isInSpikeDeath = false;
                _spikeDeathPhase = 0;
                _spikeDeathTimer = 0.0f;
                
                // s_shadePosition 已经在 startSpikeDeath 中保存，这里不需要再保存
                CCLOG("  -> 尖刺死亡模式已退出，普通死亡流程将在下一帧开始");
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

// === 修正：参考BossScene的战斗碰撞检测方法 ===
void NextScene::checkCombatCollisions()
{
    auto knight = dynamic_cast<TheKnight*>(this->getChildByName("Player"));
    if (!knight || knight->isDead()) return;
    
    Vec2 knightPos = knight->getPosition();
    
    // 更新攻击冷却时间
    float dt = Director::getInstance()->getDeltaTime();
    if (_knightAttackCooldown > 0)
    {
        _knightAttackCooldown -= dt;
    }
    if (_spellAttackCooldown > 0)
    {
        _spellAttackCooldown -= dt;
    }
    
    // 【修复】使用安全的遍历方式 - 先收集需要处理的怪物
    std::vector<CrawlidMonster*> crawlids;
    std::vector<TiktikMonster*> tiktiks;
    std::vector<GruzzerMonster*> gruzzers;  // 【新增】
    std::vector<VengeflyMonster*> vengeflies; // 【新增】
    
    auto& children = this->getChildren();
    for (auto child : children)
    {
        auto crawlid = dynamic_cast<CrawlidMonster*>(child);
        if (crawlid && crawlid->_health > 0) {
            crawlids.push_back(crawlid);
        }
        
        auto tiktik = dynamic_cast<TiktikMonster*>(child);
        if (tiktik && tiktik->_health > 0) {
            tiktiks.push_back(tiktik);
        }
        
        // 【新增】收集 Gruzzer
        auto gruzzer = dynamic_cast<GruzzerMonster*>(child);
        if (gruzzer && gruzzer->_health > 0) {
            gruzzers.push_back(gruzzer);
        }
        
        // 【新增】收集 Vengefly
        auto vengefly = dynamic_cast<VengeflyMonster*>(child);
        if (vengefly && vengefly->_health > 0) {
            vengeflies.push_back(vengefly);
        }
    }
    
    // ========== 处理 Crawlid ==========
    for (auto crawlid : crawlids)
    {
        Vec2 crawlidPos = crawlid->getPosition();
        Rect crawlidBox = crawlid->getBoundingBox();
        
        // Knight 攻击 Crawlid (普通攻击)
        if (_knightAttackCooldown <= 0)
        {
            Rect slashRect;
            if (knight->getSlashEffectBoundingBox(slashRect))
            {
                if (slashRect.intersectsRect(crawlidBox) && !crawlid->_isStunned)
                {
                    CCLOG("Knight Slash 命中 %s!", crawlid->getName().c_str());
                    
                    int knockbackDir = (knightPos.x < crawlidPos.x) ? 1 : -1;
                    crawlid->takeDamage(1, 100.0f, knockbackDir);
                    
                    int soulGain = 1;
                    if (knight->getCharmSoulCatcher())
                    {
                        soulGain += 1;
                    }
                    knight->addSoul(soulGain);
                    knight->bounceFromDownSlash();
                    
                    _knightAttackCooldown = 0.3f;
                }
            }
        }
        
        // Knight 法术攻击 Crawlid
        if (_spellAttackCooldown <= 0)
        {
            Sprite* spellEffect = knight->getVengefulSpiritEffect();
            if (spellEffect)
            {
                auto effectSize = spellEffect->getContentSize();
                auto effectPos = spellEffect->getPosition();
                Rect spellRect(effectPos.x - effectSize.width / 2,
                               effectPos.y - effectSize.height / 2,
                               effectSize.width,
                               effectSize.height);
                
                if (spellRect.intersectsRect(crawlidBox))
                {
                    CCLOG("Knight Vengeful Spirit 命中 %s!", crawlid->getName().c_str());
                    
                    int knockbackDir = (knightPos.x < crawlidPos.x) ? 1 : -1;
                    
                    crawlid->takeDamage(1, 100.0f, knockbackDir);
                    
                    this->runAction(Sequence::create(
                        DelayTime::create(0.05f),
                        CallFunc::create([crawlid, knockbackDir]() {
                            if (crawlid && crawlid->_health > 0) {
                                bool wasStunned = crawlid->_isStunned;
                                crawlid->_isStunned = false;
                                crawlid->takeDamage(1, 100.0f, knockbackDir);
                                if (wasStunned) {
                                    crawlid->_isStunned = true;
                                }
                            }
                        }),
                        nullptr
                    ));
                    
                    if (knight->getCharmShamanStone())
                    {
                        this->runAction(Sequence::create(
                            DelayTime::create(0.10f),
                            CallFunc::create([crawlid, knockbackDir]() {
                                if (crawlid && crawlid->_health > 0) {
                                    bool wasStunned = crawlid->_isStunned;
                                    crawlid->_isStunned = false;
                                    crawlid->takeDamage(1, 100.0f, knockbackDir);
                                    if (wasStunned) {
                                        crawlid->_isStunned = true;
                                    }
                                }
                            }),
                            nullptr
                        ));
                        
                        this->runAction(Sequence::create(
                            DelayTime::create(0.15f),
                            CallFunc::create([crawlid, knockbackDir]() {
                                if (crawlid && crawlid->_health > 0) {
                                    bool wasStunned = crawlid->_isStunned;
                                    crawlid->_isStunned = false;
                                    crawlid->takeDamage(1, 100.0f, knockbackDir);
                                    if (wasStunned) {
                                        crawlid->_isStunned = true;
                                    }
                                }
                            }),
                            nullptr
                        ));
                        
                        CCLOG("  -> Shaman Stone 加成! 总伤害: 4点");
                    }
                    else
                    {
                        CCLOG("  -> 基础法术伤害: 2点");
                    }
                    
                    _spellAttackCooldown = 0.2f;
                }
            }
        }
        
        // Crawlid 攻击 Knight
        if (!knight->isInvincible() && !knight->isSpikeDeathState())
        {
            if (!crawlid->_isStunned)
            {
                Rect knightBox = knight->getBoundingBox();
                
                if (knightBox.intersectsRect(crawlidBox))
                {
                    CCLOG("%s 接触伤害命中 Knight!", crawlid->getName().c_str());
                    
                    bool knockbackFromRight = (crawlidPos.x > knightPos.x);
                    
                    knight->setKnockbackDirection(knockbackFromRight);
                    knight->takeDamage(1);
                }
            }
        }
    }
    
    // ========== 处理 Tiktik ==========
    for (auto tiktik : tiktiks)
    {
        Vec2 tiktikPos = tiktik->getPosition();
        Rect tiktikBox = tiktik->getBoundingBox();
        
        // Knight 攻击 Tiktik (普通攻击)
        if (_knightAttackCooldown <= 0)
        {
            Rect slashRect;
            if (knight->getSlashEffectBoundingBox(slashRect))
            {
                if (slashRect.intersectsRect(tiktikBox) && !tiktik->_isStunned)
                {
                    CCLOG("Knight Slash 命中 %s!", tiktik->getName().c_str());
                    
                    int knockbackDir = (knightPos.x < tiktikPos.x) ? 1 : -1;
                    tiktik->takeDamage(1, 100.0f, knockbackDir);
                    
                    int soulGain = 1;
                    if (knight->getCharmSoulCatcher())
                    {
                        soulGain += 1;
                    }
                    knight->addSoul(soulGain);
                    knight->bounceFromDownSlash();
                    
                    _knightAttackCooldown = 0.3f;
                }
            }
        }
        
        // Knight 法术攻击 Tiktik
        if (_spellAttackCooldown <= 0)
        {
            Sprite* spellEffect = knight->getVengefulSpiritEffect();
            if (spellEffect)
            {
                auto effectSize = spellEffect->getContentSize();
                auto effectPos = spellEffect->getPosition();
                Rect spellRect(effectPos.x - effectSize.width / 2,
                               effectPos.y - effectSize.height / 2,
                               effectSize.width,
                               effectSize.height);
                
                if (spellRect.intersectsRect(tiktikBox))
                {
                    CCLOG("Knight Vengeful Spirit 命中 %s!", tiktik->getName().c_str());
                    
                    int knockbackDir = (knightPos.x < tiktikPos.x) ? 1 : -1;
                    
                    tiktik->takeDamage(1, 100.0f, knockbackDir);
                    
                    this->runAction(Sequence::create(
                        DelayTime::create(0.05f),
                        CallFunc::create([tiktik, knockbackDir]() {
                            if (tiktik && tiktik->_health > 0) {
                                bool wasStunned = tiktik->_isStunned;
                                tiktik->_isStunned = false;
                                tiktik->takeDamage(1, 100.0f, knockbackDir);
                                if (wasStunned) {
                                    tiktik->_isStunned = true;
                                }
                            }
                        }),
                        nullptr
                    ));
                    
                    if (knight->getCharmShamanStone())
                    {
                        this->runAction(Sequence::create(
                            DelayTime::create(0.10f),
                            CallFunc::create([tiktik, knockbackDir]() {
                                if (tiktik && tiktik->_health > 0) {
                                    bool wasStunned = tiktik->_isStunned;
                                    tiktik->_isStunned = false;
                                    tiktik->takeDamage(1, 100.0f, knockbackDir);
                                    if (wasStunned) {
                                        tiktik->_isStunned = true;
                                    }
                                }
                            }),
                            nullptr
                        ));
                        
                        this->runAction(Sequence::create(
                            DelayTime::create(0.15f),
                            CallFunc::create([tiktik, knockbackDir]() {
                                if (tiktik && tiktik->_health > 0) {
                                    bool wasStunned = tiktik->_isStunned;
                                    tiktik->_isStunned = false;
                                    tiktik->takeDamage(1, 100.0f, knockbackDir);
                                    if (wasStunned) {
                                        tiktik->_isStunned = true;
                                    }
                                }
                            }),
                            nullptr
                        ));
                        
                        CCLOG("  -> Shaman Stone 加成! 总伤害: 4点");
                    }
                    else
                    {
                        CCLOG("  -> 基础法术伤害: 2点");
                    }
                    
                    _spellAttackCooldown = 0.2f;
                }
            }
        }
        
        // Tiktik 攻击 Knight
        if (!knight->isInvincible() && !knight->isSpikeDeathState())
        {
            if (!tiktik->_isStunned)
            {
                Rect knightBox = knight->getBoundingBox();
                
                if (knightBox.intersectsRect(tiktikBox))
                {
                    CCLOG("%s 接触伤害命中 Knight!", tiktik->getName().c_str());
                    
                    bool knockbackFromRight = (tiktikPos.x > knightPos.x);
                    
                    knight->setKnockbackDirection(knockbackFromRight);
                    knight->takeDamage(1);
                }
            }
        }
    }
    
    // ========== 【新增】处理 Gruzzer ==========
    for (auto gruzzer : gruzzers)
    {
        Vec2 gruzzerPos = gruzzer->getPosition();
        Rect gruzzerBox = gruzzer->getBoundingBox();
        
        // Knight 攻击 Gruzzer (普通攻击)
        if (_knightAttackCooldown <= 0)
        {
            Rect slashRect;
            if (knight->getSlashEffectBoundingBox(slashRect))
            {
                if (slashRect.intersectsRect(gruzzerBox) && !gruzzer->_isStunned)
                {
                    CCLOG("Knight Slash 命中 %s!", gruzzer->getName().c_str());
                    
                    int knockbackDir = (knightPos.x < gruzzerPos.x) ? 1 : -1;
                    gruzzer->takeDamage(1, 100.0f, knockbackDir);
                    
                    int soulGain = 1;
                    if (knight->getCharmSoulCatcher())
                    {
                        soulGain += 1;
                    }
                    knight->addSoul(soulGain);
                    knight->bounceFromDownSlash();
                    
                    _knightAttackCooldown = 0.3f;
                }
            }
        }
        
        // Knight 法术攻击 Gruzzer
        if (_spellAttackCooldown <= 0)
        {
            Sprite* spellEffect = knight->getVengefulSpiritEffect();
            if (spellEffect)
            {
                auto effectSize = spellEffect->getContentSize();
                auto effectPos = spellEffect->getPosition();
                Rect spellRect(effectPos.x - effectSize.width / 2,
                               effectPos.y - effectSize.height / 2,
                               effectSize.width,
                               effectSize.height);
                
                if (spellRect.intersectsRect(gruzzerBox))
                {
                    CCLOG("Knight Vengeful Spirit 命中 %s!", gruzzer->getName().c_str());
                    
                    int knockbackDir = (knightPos.x < gruzzerPos.x) ? 1 : -1;
                    
                    gruzzer->takeDamage(1, 100.0f, knockbackDir);
                    
                    this->runAction(Sequence::create(
                        DelayTime::create(0.05f),
                        CallFunc::create([gruzzer, knockbackDir]() {
                            if (gruzzer && gruzzer->_health > 0) {
                                bool wasStunned = gruzzer->_isStunned;
                                gruzzer->_isStunned = false;
                                gruzzer->takeDamage(1, 100.0f, knockbackDir);
                                if (wasStunned) {
                                    gruzzer->_isStunned = true;
                                }
                            }
                        }),
                        nullptr
                    ));
                    
                    if (knight->getCharmShamanStone())
                    {
                        this->runAction(Sequence::create(
                            DelayTime::create(0.10f),
                            CallFunc::create([gruzzer, knockbackDir]() {
                                if (gruzzer && gruzzer->_health > 0) {
                                    bool wasStunned = gruzzer->_isStunned;
                                    gruzzer->_isStunned = false;
                                    gruzzer->takeDamage(1, 100.0f, knockbackDir);
                                    if (wasStunned) {
                                        gruzzer->_isStunned = true;
                                    }
                                }
                            }),
                            nullptr
                        ));
                        
                        this->runAction(Sequence::create(
                            DelayTime::create(0.15f),
                            CallFunc::create([gruzzer, knockbackDir]() {
                                if (gruzzer && gruzzer->_health > 0) {
                                    bool wasStunned = gruzzer->_isStunned;
                                    gruzzer->_isStunned = false;
                                    gruzzer->takeDamage(1, 100.0f, knockbackDir);
                                    if (wasStunned) {
                                        gruzzer->_isStunned = true;
                                    }
                                }
                            }),
                            nullptr
                        ));
                        
                        CCLOG("  -> Shaman Stone 加成! 总伤害: 4点");
                    }
                    else
                    {
                        CCLOG("  -> 基础法术伤害: 2点");
                    }
                    
                    _spellAttackCooldown = 0.2f;
                }
            }
        }
        
        // Gruzzer 攻击 Knight
        if (!knight->isInvincible() && !knight->isSpikeDeathState())
        {
            if (!gruzzer->_isStunned)
            {
                Rect knightBox = knight->getBoundingBox();
                
                if (knightBox.intersectsRect(gruzzerBox))
                {
                    CCLOG("%s 接触伤害命中 Knight!", gruzzer->getName().c_str());
                    
                    bool knockbackFromRight = (gruzzerPos.x > knightPos.x);
                    
                    knight->setKnockbackDirection(knockbackFromRight);
                    knight->takeDamage(1);
                }
            }
        }
    }
    
    // ========== 【新增】处理 Vengefly ==========
    for (auto vengefly : vengeflies)
    {
        Vec2 vengeflyPos = vengefly->getPosition();
        Rect vengeflyBox = vengefly->getBoundingBox();
        
        // 【重要】更新 Vengefly 的玩家位置（用于追击逻辑）
        vengefly->setPlayerPosition(knightPos);
        
        // Knight 攻击 Vengefly (普通攻击)
        if (_knightAttackCooldown <= 0)
        {
            Rect slashRect;
            if (knight->getSlashEffectBoundingBox(slashRect))
            {
                if (slashRect.intersectsRect(vengeflyBox) && !vengefly->_isStunned)
                {
                    CCLOG("Knight Slash 命中 %s!", vengefly->getName().c_str());
                    
                    int knockbackDir = (knightPos.x < vengeflyPos.x) ? 1 : -1;
                    vengefly->takeDamage(1, 100.0f, knockbackDir);
                    
                    int soulGain = 1;
                    if (knight->getCharmSoulCatcher())
                    {
                        soulGain += 1;
                    }
                    knight->addSoul(soulGain);
                    knight->bounceFromDownSlash();
                    
                    _knightAttackCooldown = 0.3f;
                }
            }
        }
        
        // Knight 法术攻击 Vengefly
        if (_spellAttackCooldown <= 0)
        {
            Sprite* spellEffect = knight->getVengefulSpiritEffect();
            if (spellEffect)
            {
                auto effectSize = spellEffect->getContentSize();
                auto effectPos = spellEffect->getPosition();
                Rect spellRect(effectPos.x - effectSize.width / 2,
                           effectPos.y - effectSize.height / 2,
                           effectSize.width,
                           effectSize.height);
                
                if (spellRect.intersectsRect(vengeflyBox))
                {
                    CCLOG("Knight Vengeful Spirit 命中 %s!", vengefly->getName().c_str());
                    
                    int knockbackDir = (knightPos.x < vengeflyPos.x) ? 1 : -1;
                    
                    vengefly->takeDamage(1, 100.0f, knockbackDir);
                    
                    this->runAction(Sequence::create(
                        DelayTime::create(0.05f),
                        CallFunc::create([vengefly, knockbackDir]() {
                            if (vengefly && vengefly->_health > 0) {
                                bool wasStunned = vengefly->_isStunned;
                                vengefly->_isStunned = false;
                                vengefly->takeDamage(1, 100.0f, knockbackDir);
                                if (wasStunned) {
                                    vengefly->_isStunned = true;
                                }
                            }
                        }),
                        nullptr
                    ));
                    
                    if (knight->getCharmShamanStone())
                    {
                        this->runAction(Sequence::create(
                            DelayTime::create(0.10f),
                            CallFunc::create([vengefly, knockbackDir]() {
                                if (vengefly && vengefly->_health > 0) {
                                    bool wasStunned = vengefly->_isStunned;
                                    vengefly->_isStunned = false;
                                    vengefly->takeDamage(1, 100.0f, knockbackDir);
                                    if (wasStunned) {
                                        vengefly->_isStunned = true;
                                    }
                                }
                            }),
                            nullptr
                        ));
                        
                        this->runAction(Sequence::create(
                            DelayTime::create(0.15f),
                            CallFunc::create([vengefly, knockbackDir]() {
                                if (vengefly && vengefly->_health > 0) {
                                    bool wasStunned = vengefly->_isStunned;
                                    vengefly->_isStunned = false;
                                    vengefly->takeDamage(1, 100.0f, knockbackDir);
                                    if (wasStunned) {
                                        vengefly->_isStunned = true;
                                    }
                                }
                            }),
                            nullptr
                        ));
                        
                        CCLOG("  -> Shaman Stone 加成! 总伤害: 4点");
                    }
                    else
                    {
                        CCLOG("  -> 基础法术伤害: 2点");
                    }
                    
                    _spellAttackCooldown = 0.2f;
                }
            }
        }
        
        // Vengefly 攻击 Knight
        if (!knight->isInvincible() && !knight->isSpikeDeathState())
        {
            if (!vengefly->_isStunned)
            {
                Rect knightBox = knight->getBoundingBox();
                
                if (knightBox.intersectsRect(vengeflyBox))
                {
                    CCLOG("%s 接触伤害命中 Knight!", vengefly->getName().c_str());
                    
                    bool knockbackFromRight = (vengeflyPos.x > knightPos.x);
                    
                    knight->setKnockbackDirection(knockbackFromRight);
                    knight->takeDamage(1);
                }
            }
        }
    }

    if (_knightAttackCooldown <= 0)
    {
        Rect slashRect;
        if (knight->getSlashEffectBoundingBox(slashRect))
        {
            for (auto& geoObj : _geoObjects)
            {
                Rect geoRect(
                    geoObj.position.x - geoObj.size.width / 2,
                    geoObj.position.y - geoObj.size.height / 2,
                    geoObj.size.width,
                    geoObj.size.height
                );

                if (slashRect.intersectsRect(geoRect))
                {
                    // 轻微屏幕震动
                    shakeScreen(0.1f, 5.0f);

                    // 增加 2 Geo
                    GeoManager::getInstance()->addGeo(2);
                    CCLOG("攻击 Geo 对象! 获得 2 Geo, 总计: %d", GeoManager::getInstance()->getGeo());

                    // 【关键】设置攻击冷却，防止同一次攻击重复触发
                    _knightAttackCooldown = 0.3f;

                    break;  // 每次攻击只触发一个 Geo 对象
                }
            }
        }
    }
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
    Size visibleSize = Director::getInstance()->getVisibleSize();
    
    // ==================== 【修改】检查 Knight 死亡状态,等待死亡动画播放完成 ====================
    static bool isPlayingDeathAnim = false; // 标记是否正在播放死亡动画
    static float deathAnimTimer = 0.0f;     // 死亡动画计时器
    static Vec2 savedDeathPos = Vec2::ZERO; // 【新增】保存死亡位置
    
    if (knight->isDead() && !_isInSpikeDeath)
    {
        if (!isPlayingDeathAnim)
        {
            CCLOG("Knight died in NextScene, starting death animation timer");
            isPlayingDeathAnim = true;
            deathAnimTimer = 0.0f;
            savedDeathPos = knightPos;  // 【新增】在开始播放死亡动画时保存位置
            
            // 【新增】保存 Shade 位置
            s_shadePosition = knightPos;
            CCLOG("Saved death position for Shade: (%.1f, %.1f)", s_shadePosition.x, s_shadePosition.y);
        }
        
        deathAnimTimer += dt;
        // 死亡动画13帧，每帧0.1秒，共1.3秒
        if (deathAnimTimer >= 1.3f)
        {
            CCLOG("Death animation completed, triggering death callback");
            isPlayingDeathAnim = false;
            deathAnimTimer = 0.0f;
            onKnightDeath(savedDeathPos);  // 【修改】使用保存的位置
            savedDeathPos = Vec2::ZERO;
            return;
        }
        
        // 死亡动画播放期间，只更新摄像机和UI，跳过其他游戏逻辑
    }
    else if (!knight->isDead() && isPlayingDeathAnim)
    {
        // Knight 复活了，重置标志
        isPlayingDeathAnim = false;
        deathAnimTimer = 0.0f;
        savedDeathPos = Vec2::ZERO;
    }
    // ==================== 检查结束 ====================
    
    float verticalOffset = visibleSize.height / 6.0f;
    
    if (_isInSpikeDeath)
    {
        updateSpikeDeath(dt, knight);
        
        Vec2 cameraPos = camera->getPosition();
        Vec2 targetPos = Vec2(knightPos.x, knightPos.y + verticalOffset);
        float lerpFactor = 0.1f;
        Vec2 newPos = cameraPos + (targetPos - cameraPos) * lerpFactor;
        newPos = newPos + _shakeOffset;
        camera->setPosition(newPos);
        
        updateHPAndSoulUI(dt);
        return;
    }
    
    // 【新增】只在非死亡动画播放期间执行游戏逻辑
    if (!isPlayingDeathAnim)
    {
        if (!knight->isDead() && !knight->isSpikeDeathState() && !knight->isHazardRespawnState())
        {
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
        
        // === 使用新的战斗碰撞检测方法(参考BossScene) ===
        checkCombatCollisions();
        
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
                    CCLOG("玩家碰到尖刺！开始尖刺死亡流程");
                    startSpikeDeath(knight);
                    break;
                }
            }
        }

        static bool wasHardLanding = false;
        bool isHardLanding = knight->isHardLanding();
        
        if (isHardLanding && !wasHardLanding)
        {
            shakeScreen(0.6f, 30.0f);
            CCLOG("玩家重落地，触发屏幕震动");
        }
        wasHardLanding = isHardLanding;

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
                float factor = 1.0f - (_shakeElapsed / _shakeDuration);
                float offsetX = ((rand() % 200) / 100.0f - 1.0f) * _shakeIntensity * factor;
                float offsetY = ((rand() % 200) / 100.0f - 1.0f) * _shakeIntensity * factor;
                
                _shakeOffset = Vec2(offsetX, offsetY);
            }
        }

        checkInteractions();

        // 更新 Cornifer 的 Knight 位置信息
        if (_cornifer && knight)
        {
            Vec2 knightWorldPos = knight->getPosition();
            _cornifer->setPlayerPosition(knightWorldPos);
        }
    }

    // 【关键】摄像机和UI更新始终执行，确保死亡动画期间画面正常
    Vec2 cameraPos = camera->getPosition();
    Vec2 targetPos = Vec2(knightPos.x, knightPos.y + verticalOffset);
    
    float lerpFactor = 0.1f;
    Vec2 newPos = cameraPos + (targetPos - cameraPos) * lerpFactor;
    newPos = newPos + _shakeOffset;
    
    camera->setPosition(newPos);
    
    updateHPAndSoulUI(dt);
}

// ShadowEnemy 相关方法
void NextScene::updateShade(float dt)
{
    // Shade 的 update 会在其自身的 scheduleUpdate 中调用
    // 这里可以添加额外的逻辑，比如检查 Shade 是否被击败等
    if (_shade && _shade->getParent() == nullptr)
    {
        // Shade 已经被移除（可能是被击败了）
        _shade = nullptr;
        
        // 【新增】Shade 被击败后，重置静态位置变量
        CCLOG("Shade has been defeated/collected, resetting s_shadePosition");
        s_shadePosition = Vec2::ZERO;
    }
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
            
            // 修复：确保前景对象在 Knight 上面
            this->addChild(fgSprite, 10);  // 移除注释，z-order = 10
            
            CCLOG("加载前景对象: %s at (%.1f, %.1f), z-order=10", imagePath.c_str(), worldX, worldY);
        }
        else
        {
            CCLOG("警告：无法加载前景图片: %s", imagePath.c_str());
        }
    }
}

// 新增：Knight 死亡回调
void NextScene::onKnightDeath(const Vec2& deathPos)
{
    CCLOG("NextScene::onKnightDeath - Knight died at position (%.1f, %.1f)", deathPos.x, deathPos.y);
    
    // 保存死亡位置用于下次重生时生成 Shade
    s_shadePosition = deathPos;
    
    // 移除现有的 Shade（如果有）
    removeShade();
    
    // 【修改】死亡后切换到 GameScene 并坐在椅子上
    auto blackLayer = LayerColor::create(Color4B(0, 0, 0, 0));
    this->addChild(blackLayer, 2000);
    
    blackLayer->runAction(Sequence::create(
        FadeIn::create(1.0f),
        CallFunc::create([]() {
            // 使用新的重生方法切换到 GameScene
            auto gameScene = GameScene::createSceneForRespawn();
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, gameScene, Color3B::BLACK));
        }),
        nullptr
    ));
}

// 修改：生成 Shade - 增加更详细的调试信息
void NextScene::spawnShade(const Vec2& position)
{
    CCLOG("=== NextScene::spawnShade START ===");
    CCLOG("  Position: (%.1f, %.1f)", position.x, position.y);
    CCLOG("  _player pointer: %p", _player);
    
    // 移除旧的 Shade
    removeShade();
    
    // 创建新的 Shade
    _shade = ShadowEnemy::create();
    if (_shade)
    {
        CCLOG("  Shade created successfully");
        _shade->setPosition(position);
        
        // 【修改】确保 _player 不为空
        if (_player)
        {
            _shade->setTarget(_player);
            CCLOG("  Shade target set to _player");
        }
        else
        {
            CCLOG("  WARNING: _player is nullptr! Cannot set target");
        }
        
        this->addChild(_shade, 4);  // 添加到场景中，层级略低于玩家
        
        CCLOG("  Shade added to scene at z-order 4");
        CCLOG("  Shade position: (%.1f, %.1f)", _shade->getPositionX(), _shade->getPositionY());
        CCLOG("  Shade parent: %p", _shade->getParent());
        CCLOG("=== Shade spawned successfully ===");
    }
    else
    {
        CCLOG("  ERROR: Failed to create Shade!");
        CCLOG("=== FAILED to spawn Shade ===");
    }
}

// 新增：移除 Shade
void NextScene::removeShade()
{
    if (_shade)
    {
        _shade->removeFromParent();
        _shade = nullptr;
        CCLOG("Shade removed from NextScene");
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
    _lastDisplayedSoul = -1;
    
    int maxHp = knight->getMaxHP();
    float gap = 50;
    
    // 【修改】先创建所有空血槽图标（底层）
    for (int i = 0; i < maxHp; i++)
    {
        auto hpEmpty = Sprite::create("Hp/hp8.png");
        if (hpEmpty)
        {
            hpEmpty->setPosition(Vec2(260 + i * gap, 978));
            hpEmpty->setScale(0.5f);
            hpEmpty->setVisible(i >= _lastDisplayedHP);  // 失去的血量位置显示
            _uiLayer->addChild(hpEmpty);
            _hpEmptyBars.push_back(hpEmpty);  // 需要在头文件中添加此成员变量
        }
    }
    
    // 创建满血图标（上层，会覆盖空血槽）
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
    
    // 【删除】不再需要单独的 _hpLose
    // _hpLose = Sprite::create("Hp/hp8.png");
    
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

    {
        Size vs = Director::getInstance()->getVisibleSize();
        Vec2 org = Director::getInstance()->getVisibleOrigin();
        Vec2 centerLocal = Vec2(vs.width * 0.5f, vs.height * 0.5f);

        // 主标题
        auto sceneTitle = Label::createWithSystemFont(u8"遗忘十字路", "fonts/NotoSerifCJKsc-Regular.otf", 100);
        sceneTitle->setTextColor(Color4B::WHITE);
        sceneTitle->setAnchorPoint(Vec2(0.5f, 0.5f));

        // 两张图片：Maps/toptitle.png 和 Maps/bottomtitle.png
        Sprite* topImg = Sprite::create("Maps/toptitle.png");
        Sprite* bottomImg = Sprite::create("Maps/bottomtitle.png");

        // 如果图片未找到，设为 nullptr（安全检查）
        if (topImg && topImg->getContentSize().width == 0) { topImg = nullptr; }
        if (bottomImg && bottomImg->getContentSize().width == 0) { bottomImg = nullptr; }

        // 添加到 _uiLayer（若存在），否则添加到场景直接居中
        Node* parentNode = _uiLayer ? _uiLayer : this;
        Vec2 parentOffset = Vec2::ZERO;
        if (!_uiLayer) {
            parentOffset = org;
        }

        sceneTitle->setPosition(Vec2(centerLocal.x + parentOffset.x, centerLocal.y + parentOffset.y + 80));
        parentNode->addChild(sceneTitle, 2000, "SceneTitleLabel");

        if (topImg) {
            topImg->setAnchorPoint(Vec2(0.5f, 0.5f));
            topImg->setPosition(Vec2(centerLocal.x - 30, centerLocal.y + parentOffset.y + 270));
            parentNode->addChild(topImg, 1999, "SceneTopImage");
        }
        if (bottomImg) {
            bottomImg->setAnchorPoint(Vec2(0.5f, 0.5f));
            bottomImg->setPosition(Vec2(centerLocal.x + 14, centerLocal.y + parentOffset.y - 40));
            parentNode->addChild(bottomImg, 1999, "SceneBottomImage");
        }

        // 动作序列：延迟 -> 淡入 -> 保持 -> 淡出 -> 移除
        auto delay = DelayTime::create(0.1f);
        auto in = FadeTo::create(1.5f, 255);
        auto hold = DelayTime::create(1.0f);
        auto out = FadeTo::create(0.4f, 0);
        auto remove = RemoveSelf::create();

        sceneTitle->setOpacity(0);
        if (topImg) topImg->setOpacity(0);
        if (bottomImg) bottomImg->setOpacity(0);

        // 【修改】所有动画前添加延迟
        sceneTitle->runAction(Sequence::create(delay, in, hold, out, remove->clone(), nullptr));

        if (topImg) {
            auto moveInL = MoveBy::create(0.15f, Vec2(20.0f, 0));
            topImg->runAction(Sequence::create(delay->clone(), Spawn::create(in->clone(), moveInL, nullptr), hold->clone(), out->clone(), remove->clone(), nullptr));
        }
        if (bottomImg) {
            auto moveInR = MoveBy::create(0.15f, Vec2(-20.0f, 0));
            bottomImg->runAction(Sequence::create(delay->clone(), Spawn::create(in->clone(), moveInR, nullptr), hold->clone(), out->clone(), remove->clone(), nullptr));
        }
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

void NextScene::updateHPAndSoulUI(float dt)
{
    auto knight = dynamic_cast<TheKnight*>(this->getChildByName("Player"));
    if (!knight || !_uiLayer) return;
    
    // 更新UI层位置跟随摄像头
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
            _hpBars.at(i)->setVisible(i < currentHP);
        }
        
        // 【修改】更新所有空血槽的显示
        for (int i = 0; i < (int)_hpEmptyBars.size(); i++)
        {
            _hpEmptyBars.at(i)->setVisible(i >= currentHP);
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

void NextScene::showMap()
{
    if (_isMapVisible) return;
    _isMapVisible = true;
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    
    // 创建半透明黑色遮罩层
    _mapOverlay = LayerColor::create(Color4B(0, 0, 0, 180));
    if (_mapOverlay)
    {
        _uiLayer->addChild(_mapOverlay, 3000);
    }
    
    // 创建地图图片
    _mapSprite = Sprite::create("Maps/Forgotten_Crossroads_Map_Clean.png");
    if (_mapSprite)
    {
        _mapSprite->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
        
        // 根据需要调整地图大小，使其适应屏幕
        float mapWidth = _mapSprite->getContentSize().width;
        float mapHeight = _mapSprite->getContentSize().height;
        float scaleX = (visibleSize.width * 0.75f) / mapWidth;
        float scaleY = (visibleSize.height * 0.75f) / mapHeight;
        float mapScale = std::min(scaleX, scaleY);
        _mapSprite->setScale(mapScale);
        
        _uiLayer->addChild(_mapSprite, 3001);
    }
    
    CCLOG("显示地图: Forgotten_Crossroads_Map_Clean.png");
}

void NextScene::hideMap()
{
    if (!_isMapVisible) return;
    _isMapVisible = false;
    
    // 移除地图图片
    if (_mapSprite)
    {
        _mapSprite->removeFromParent();
        _mapSprite = nullptr;
    }
    
    // 移除遮罩层
    if (_mapOverlay)
    {
        _mapOverlay->removeFromParent();
        _mapOverlay = nullptr;
    }
    
    CCLOG("隐藏地图");
}