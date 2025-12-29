#include "GameScene.h"
#include "TheKnight.h"
#include "NextScene.h"
#include <BossScene.h>
#include "CharmManager.h"
#include "Monster/CrawlidMonster.h"
#include "SimpleAudioEngine.h"
#include "PauseMenu.h"
#include "GeoManager.h"
#include "KnightStateManager.h"

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

Scene* GameScene::createSceneForRespawn()
{
    s_hasCustomSpawn = true;
    s_customSpawnPos = Vec2::ZERO;  // 不使用自定义位置，使用椅子位置
    s_spawnFacingRight = true;
    s_spawnDoJump = false;  // 不跳跃，直接坐下

    CCLOG("GameScene::createSceneForRespawn - Knight will respawn on chair");

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

    // 先加载地图和交互对象
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
        loadInteractiveObjects(map, scale, mapPos);  // 确保这个在 Knight 创建前调用
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
        // 【修改】根据不同的场景切换方式设置不同的初始位置
        if (s_hasCustomSpawn && s_spawnDoJump)
        {
            // 从 NextScene 的 Exit 出口返回，使用自定义出生点
            _knight->setPosition(s_customSpawnPos);
            CCLOG("Knight spawning from Exit at custom position: (%.1f, %.1f)", 
                  s_customSpawnPos.x, s_customSpawnPos.y);
        }
        else
        {
            // 默认起始位置
            _knight->setPosition(Vec2(startX, startY));
        }
        _knight->setScale(1.0f);

        _knight->setPlatforms(_platforms);
        this->addChild(_knight, 5, "Player");

        CharmManager::getInstance()->syncToKnight(_knight);

        // 【新增】从 NextScene 返回时恢复保存的状态
        if (s_hasCustomSpawn && s_spawnDoJump)
        {
            // 从 NextScene 正常返回（非死亡），恢复保存的状态
            auto stateManager = KnightStateManager::getInstance();
            if (stateManager->hasState())
            {
                _knight->setHP(stateManager->getHP());
                _knight->setSoul(stateManager->getSoul());
                CCLOG("从 NextScene 返回，恢复状态: HP=%d, Soul=%d", 
                      stateManager->getHP(), stateManager->getSoul());
            }
        }

        // 检查是否从 NextScene 死亡返回，需要坐在椅子上
        if (s_hasCustomSpawn && !s_spawnDoJump)
        {
            // 从 NextScene 死亡返回，让 Knight 坐在椅子上
            CCLOG("Knight died in NextScene, respawning on chair in GameScene");

            // 查找椅子位置
            Vec2 chairPos = Vec2::ZERO;
            for (const auto& obj : _interactiveObjects)
            {
                if (obj.name == "Chair")
                {
                    chairPos = obj.position;
                    break;
                }
            }

            if (chairPos != Vec2::ZERO)
            {
                _knight->setPosition(chairPos);
                _knight->setNearChair(true);
                _knight->setHP(_knight->getMaxHP());

                this->scheduleOnce([this, chairPos](float dt) {
                    if (_knight)
                    {
                        _knight->setPosition(chairPos);
                        _knight->setNearChair(true);
                        _knight->startSitting();
                        CCLOG("Knight automatically sitting on chair after respawn at (%.1f, %.1f)",
                            chairPos.x, chairPos.y);
                    }
                    }, 0.3f, "auto_sit");
            }
            else
            {
                CCLOG("Warning: Chair not found, Knight will spawn at default position");
            }

            // 重置标志
            s_hasCustomSpawn = false;
            s_customSpawnPos = Vec2::ZERO;
        }
        else if (s_hasCustomSpawn && s_spawnDoJump)
        {
            // 【已修改】位置已在上面设置，这里只触发跳跃动作
            float horizontalSpeed = s_spawnFacingRight ? 1.0f : -1.0f;
            _knight->triggerJumpFromExternal(horizontalSpeed);

            CCLOG("玩家从NextScene返回：位置(%.1f, %.1f)，朝向%s，触发跳跃动作",
                s_customSpawnPos.x, s_customSpawnPos.y,
                s_spawnFacingRight ? "右" : "左");

            s_hasCustomSpawn = false;
            s_customSpawnPos = Vec2::ZERO;
            s_spawnFacingRight = true;
            s_spawnDoJump = false;
        }
    }

    _interactionLabel = Label::createWithSystemFont(u8"休息", "fonts/ZCOOLXiaoWei-Regular.ttf", 24);
    _interactionLabel->setTextColor(Color4B::WHITE);
    _interactionLabel->setVisible(false);
    this->addChild(_interactionLabel, 100, "InteractionLabel");

    createHPAndSoulUI();

    // 【新增】创建暂停菜单
    _pauseMenu = PauseMenu::create();
    if (_pauseMenu)
    {
        _uiLayer->addChild(_pauseMenu, 2000);
    }

    {
        Size vs = Director::getInstance()->getVisibleSize();
        Vec2 org = Director::getInstance()->getVisibleOrigin();
        Vec2 centerLocal = Vec2(vs.width * 0.5f, vs.height * 0.5f);

        // 主标题
        auto sceneTitle = Label::createWithSystemFont(u8"德特茅斯", "fonts/NotoSerifCJKsc-Regular.otf", 95);
        sceneTitle->setTextColor(Color4B::WHITE);
        sceneTitle->setAnchorPoint(Vec2(0.5f, 0.5f));

        // 副标题（小字）
        auto sceneSubtitle = Label::createWithSystemFont(u8"衰败的小镇", "fonts/NotoSerifCJKsc-Regular.otf", 45);
        sceneSubtitle->setTextColor(Color4B::WHITE);
        sceneSubtitle->setAnchorPoint(Vec2(0.5f, 0.5f));

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

        // 主标题位置（相对于 UI 层的视口坐标）
        sceneTitle->setPosition(Vec2(centerLocal.x + parentOffset.x, centerLocal.y + parentOffset.y + 80));
        parentNode->addChild(sceneTitle, 2000, "SceneTitleLabel");

        // 副标题靠主标题下方一点
        sceneSubtitle->setPosition(Vec2(centerLocal.x + parentOffset.x, centerLocal.y + parentOffset.y + 160));
        parentNode->addChild(sceneSubtitle, 2000, "SceneSubtitleLabel");

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

        // 动作序列：淡入 -> 保持 -> 淡出 -> 移除
        auto in = FadeTo::create(0.15f, 255);
        auto hold = DelayTime::create(1.8f);
        auto out = FadeTo::create(0.4f, 0);
        auto remove = RemoveSelf::create();

        sceneTitle->setOpacity(0);
        sceneSubtitle->setOpacity(0);
        if (topImg) topImg->setOpacity(0);
        if (bottomImg) bottomImg->setOpacity(0);

        sceneTitle->runAction(Sequence::create(in, hold, out, remove->clone(), nullptr));
        sceneSubtitle->runAction(Sequence::create(DelayTime::create(0.05f), in->clone(), hold->clone(), out->clone(), remove->clone(), nullptr));

        if (topImg) {
            auto moveInL = MoveBy::create(0.15f, Vec2(20.0f, 0));
            topImg->runAction(Sequence::create(Spawn::create(in->clone(), moveInL, nullptr), hold->clone(), out->clone(), remove->clone(), nullptr));
        }
        if (bottomImg) {
            auto moveInR = MoveBy::create(0.15f, Vec2(-20.0f, 0));
            bottomImg->runAction(Sequence::create(Spawn::create(in->clone(), moveInR, nullptr), hold->clone(), out->clone(), remove->clone(), nullptr));
        }
    }

    _cameraOffsetY = 0.0f;
    _targetCameraOffsetY = 0.0f;
    _wasSitting = false;

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
    
    // 【新增】按键释放事件
    keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        // TAB 键释放时隐藏地图
        if (keyCode == EventKeyboard::KeyCode::KEY_TAB)
        {
            hideMap();
        }
    };
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

    this->scheduleUpdate();

    if (blackLayer1 && blackLayer1->getParent()) {
        blackLayer1->removeFromParent();
    }
    
    CCLOG("共创建 %zu 个碰撞平台", _platforms.size());
    CCLOG("共加载 %zu 个交互对象", _interactiveObjects.size());

    // 播放 Dirtmouth 背景音乐（循环）
    CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("Music/Dirtmouth.wav", true);

    return true;
}

void GameScene::createHPAndSoulUI()
{
    if (!_knight) return;
    
    // 【修复】创建UI层（跟随摄像机移动）
    _uiLayer = Node::create();
    if (!_uiLayer) return;
    this->addChild(_uiLayer, 1000);  // 最高层级
    
    // 创建血条背景
    _hpBg = Sprite::create("Hp/hpbg.png");
    if (_hpBg)
    {
        _hpBg->setPosition(Vec2(200, 950));
        _uiLayer->addChild(_hpBg);
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
        _uiLayer->addChild(_soulBg);
        
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
            _hpEmptyBars.push_back(hpEmpty);
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
            hpBar->setVisible(i < _lastDisplayedHP);  // 只显示当前血量
            _uiLayer->addChild(hpBar);
            _hpBars.push_back(hpBar);
        }
    }
    
    // 【删除】不再需要单独的 _hpLose
    // _hpLose = Sprite::create("Hp/hp8.png");
    // ...
    
    // 【新增】创建 Geo UI（在血条下方）
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

void GameScene::updateHPAndSoulUI(float dt)
{
    if (!_knight || !_uiLayer) return;
    
    // 【修复】更新UI层位置跟随摄像机
    auto camera = this->getDefaultCamera();
    if (camera)
    {
        Vec2 camPos = camera->getPosition();
        Size visibleSize = Director::getInstance()->getVisibleSize();
        // UI层左下角对齐摄像机视口左下角
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
        
        // 【修改】血量变化时，增加血量图标缩放动画
        if (currentHP > _lastDisplayedHP)
        {
            for (int i = _lastDisplayedHP; i < currentHP; i++)
            {
                if (i < (int)_hpBars.size())
                {
                    auto hpBar = _hpBars[i];
                    hpBar->setVisible(true);
                    
                    // 添加一个小的缩放动画效果
                    hpBar->setScale(0.0f);
                    hpBar->runAction(Sequence::create(
                        ScaleTo::create(0.15f, 0.6f),
                        ScaleTo::create(0.1f, 0.5f),
                        nullptr
                    ));
                }
            }
        }
        
        _lastDisplayedHP = currentHP;
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
    
    // 【新增】更新 Geo 显示
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
        
        // 【修改】隐藏对应的空血槽
        if (_hpRecoverCurrent - 1 < (int)_hpEmptyBars.size())
        {
            _hpEmptyBars[_hpRecoverCurrent - 1]->setVisible(false);
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

                // 【新增】保存骑士状态
                KnightStateManager::getInstance()->saveState(_knight->getHP(), _knight->getSoul());

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

        // 使用对象名称 + .png 作为图片路径
        std::string imagePath = "Maps/" + name + ".png";
        auto fgSprite = Sprite::create(imagePath);

        if (!fgSprite) {
            imagePath = name + ".png";
            fgSprite = Sprite::create(imagePath);
        }
        
        if (fgSprite)
        {
            float spriteHeight = fgSprite->getContentSize().height;
            // 计算世界坐标位置
            float worldX = objX * scale + mapOffset.x;
            float worldY = (objY + spriteHeight) * scale + mapOffset.y;

            // TMX 对象的 y 坐标是对象底部，锚点设为左下角
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

void GameScene::showMap()
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
    _mapSprite = Sprite::create("Maps/Dirtmouth_Map_Clean.png");
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
    
    CCLOG("显示地图");
}

void GameScene::hideMap()
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