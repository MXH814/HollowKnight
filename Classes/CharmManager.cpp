/**
 * @file CharmManager.cpp
 * @brief 护符管理器类实现
 * @details 实现护符系统的单例管理器
 * 
 * C++ 特性使用说明：
 * - 单例模式：使用静态成员实现
 * - STL 容器：std::vector 存储护符和凹槽
 * - 范围 for 循环：遍历护符列表
 * - auto 关键字：类型推导
 * - 初始化列表：构造函数使用
 */

#include "CharmManager.h"
#include "TheKnight.h"

USING_NS_CC;

// ============================================================================
// 静态成员初始化
// ============================================================================

CharmManager* CharmManager::_instance = nullptr;

// ============================================================================
// 单例获取
// ============================================================================

CharmManager* CharmManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new CharmManager();
    }
    return _instance;
}

// ============================================================================
// 构造与析构
// ============================================================================

CharmManager::CharmManager()
    : _charmWaywardCompass(0)
    , _charmShamanStone(0)
    , _charmStalwartShell(0)
    , _charmSteadyBody(0)
    , _charmSoulCatcher(0)
    , _charmSprintMaster(0)
    , _usedNotches(0)
    , _isPanelOpen(false)
    , _panelLayer(nullptr)
    , _parentNode(nullptr)
    , _infoCharmIcon(nullptr)
    , _infoCharmName(nullptr)
    , _infoCharmDesc(nullptr)
    , _infoCharmCost(nullptr)
    , _costContainer(nullptr)
    , _equippedStartX(0.0f)
    , _equippedY(0.0f)
    , _equippedGap(100.0f)
    , _selectedCharmIndex(0)
    , _selectFrameSprite(nullptr)
    , _keyboardListener(nullptr)
    , _canEquip(false)
    , _hintLabel(nullptr) {
}

CharmManager::~CharmManager() {
}

// ============================================================================
// 凹槽计算
// ============================================================================

void CharmManager::recalculateUsedNotches() {
    _usedNotches = 0;
    if (_charmWaywardCompass == 1) _usedNotches += 1;
    if (_charmShamanStone == 1) _usedNotches += 3;
    if (_charmStalwartShell == 1) _usedNotches += 2;
    if (_charmSteadyBody == 1) _usedNotches += 1;
    if (_charmSoulCatcher == 1) _usedNotches += 2;
    if (_charmSprintMaster == 1) _usedNotches += 1;
}

// ============================================================================
// 与骑士同步
// ============================================================================

void CharmManager::syncToKnight(TheKnight* knight) {
    if (knight == nullptr) {
        return;
    }
    
    knight->setCharmStalwartShell(_charmStalwartShell);
    knight->setCharmSoulCatcher(_charmSoulCatcher);
    knight->setCharmShamanStone(_charmShamanStone);
    knight->setCharmSprintmaster(_charmSprintMaster);
    knight->setCharmSteadyBody(_charmSteadyBody);
}

// ============================================================================
// 面板显示
// ============================================================================

void CharmManager::showCharmPanel(Node* parentNode, bool canEquip) {
    if (_isPanelOpen) {
        hideCharmPanel();
        return;
    }
    
    _parentNode = parentNode;
    _isPanelOpen = true;
    _selectedCharmIndex = 0;
    _canEquip = canEquip;
    
    const auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // 创建半透明遮罩层
    auto layer = LayerColor::create(Color4B(0, 0, 0, 200));
    layer->setName("CharmPanel");
    
    // 获取摄像机位置以正确定位 UI
    auto* scene = dynamic_cast<Scene*>(parentNode);
    Vec2 cameraOffset = Vec2::ZERO;
    if (scene != nullptr) {
        auto* camera = scene->getDefaultCamera();
        if (camera != nullptr) {
            Vec2 camPos = camera->getPosition();
            cameraOffset = Vec2(camPos.x - visibleSize.width / 2.0f,
                               camPos.y - visibleSize.height / 2.0f);
        }
    }
    layer->setPosition(cameraOffset);
    
    parentNode->addChild(layer, 2000);
    _panelLayer = layer;
    
    // 显示 Charm_Prompt 图片
    auto charmBg = Sprite::create("Charm/Charm_Prompt.png");
    charmBg->setPosition(Vec2(visibleSize.width / 2.0f, visibleSize.height / 2.0f));
    charmBg->setScale(1.5f);
    layer->addChild(charmBg);
    
    auto saveIcon = Sprite::create("Charm/Save_Icon_1.png");
    saveIcon->setPosition(Vec2(visibleSize.width - 100.0f, 100.0f));
    layer->addChild(saveIcon);
    
    // 保存动画帧
    Vector<SpriteFrame*> saveFrames;
    for (int i = 1; i <= 20; ++i) {
        std::string frameName = "Charm/Save_Icon_" + std::to_string(i) + ".png";
        auto frame = SpriteFrame::create(frameName, Rect(0, 0,
            saveIcon->getContentSize().width, saveIcon->getContentSize().height));
        if (frame != nullptr) {
            saveFrames.pushBack(frame);
        }
    }
    
    // 播放动画
    auto animation = Animation::createWithSpriteFrames(saveFrames, 0.1f);
    auto animate = Animate::create(animation);
    auto repeatAnimate = Repeat::create(animate, 1);
    saveIcon->runAction(Sequence::create(
        repeatAnimate,
        RemoveSelf::create(),
        nullptr
    ));
    
    // 延迟后显示护符界面
    charmBg->runAction(
        Sequence::create(
            DelayTime::create(2.0f),
            RemoveSelf::create(),
            CallFunc::create([this, layer, visibleSize]() {
                this->createCharmUI(layer, visibleSize);
            }),
            nullptr
        )
    );
}

void CharmManager::createCharmUI(Node* layer, const Size& visibleSize) {
    // 显示 Charm 标题
    auto label = Label::createWithTTF(u8"护符", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    label->setColor(Color3B::WHITE);
    label->setPosition(Vec2(visibleSize.width / 2.0f, visibleSize.height - 40.0f));
    layer->addChild(label);
    
    // 根据是否可以装卸显示不同提示
    std::string hintText = _canEquip 
        ? u8"AD选择 K装备/卸下 Q关闭"
        : u8"AD选择 Q关闭 (坐下后可装卸)";
    _hintLabel = Label::createWithTTF(hintText, "fonts/NotoSerifCJKsc-Regular.otf", 32);
    _hintLabel->setColor(Color3B::WHITE);
    _hintLabel->setPosition(Vec2(visibleSize.width - 300.0f, 180.0f));
    layer->addChild(_hintLabel);
    
    auto aocao = Label::createWithTTF(u8"凹槽", "fonts/NotoSerifCJKsc-Regular.otf", 28);
    aocao->setColor(Color3B::WHITE);
    aocao->setPosition(Vec2(365.0f, visibleSize.height / 2.0f + 60.0f));
    layer->addChild(aocao);
    
    auto zhuangbei = Label::createWithTTF(u8"已装备", "fonts/NotoSerifCJKsc-Regular.otf", 44);
    zhuangbei->setColor(Color3B::WHITE);
    zhuangbei->setPosition(Vec2(400.0f, visibleSize.height / 2.0f + 220.0f));
    layer->addChild(zhuangbei);
    
    // 边框装饰
    auto top = Sprite::create("Charm/top.png");
    top->setPosition(Vec2(visibleSize.width / 2.0f, visibleSize.height - 70.0f));
    layer->addChild(top);
    
    auto edging1 = Sprite::create("Charm/edging1.png");
    edging1->setPosition(Vec2(200.0f, visibleSize.height - 160.0f));
    layer->addChild(edging1);
    
    auto edging2 = Sprite::create("Charm/edging2.png");
    edging2->setPosition(Vec2(visibleSize.width - 200.0f, visibleSize.height - 160.0f));
    layer->addChild(edging2);
    
    auto edging3 = Sprite::create("Charm/edging3.png");
    edging3->setPosition(Vec2(200.0f, 160.0f));
    layer->addChild(edging3);
    
    auto edging4 = Sprite::create("Charm/edging4.png");
    edging4->setPosition(Vec2(visibleSize.width - 200.0f, 160.0f));
    layer->addChild(edging4);
    
    auto smalledge1 = Sprite::create("Charm/smalledge1.png");
    smalledge1->setPosition(Vec2(100.0f, visibleSize.height / 2.0f));
    layer->addChild(smalledge1);
    
    auto smalledge2 = Sprite::create("Charm/smalledge2.png");
    smalledge2->setPosition(Vec2(visibleSize.width - 100.0f, visibleSize.height / 2.0f));
    layer->addChild(smalledge2);
    
    auto bottom = Sprite::create("Charm/bottom.png");
    bottom->setPosition(Vec2(visibleSize.width / 2.0f, 80.0f));
    layer->addChild(bottom);
    
    auto line = Sprite::create("Charm/line.png");
    line->setPosition(Vec2(735.0f, visibleSize.height / 2.0f - 58.0f));
    line->setScaleX(1.44f);
    layer->addChild(line);
    
    // ===== 凹槽系统 =====
    _notchSprites.clear();
    
    const float notchStartX = visibleSize.width / 2.0f - 600.0f;
    const float notchY = visibleSize.height / 2.0f + 15.0f;
    
    recalculateUsedNotches();
    
    // 使用范围 for 循环创建凹槽
    for (int i = 0; i < MAX_NOTCHES; ++i) {
        std::string notchImage = (i < _usedNotches) 
            ? "Charm/charm_cost_1.png" 
            : "Charm/charm_cost_0.png";
        auto notch = Sprite::create(notchImage);
        notch->setPosition(Vec2(notchStartX + static_cast<float>(i) * 50.0f, notchY));
        layer->addChild(notch);
        _notchSprites.push_back(notch);
    }
    
    // ===== Charm 系统 =====
    _charms.clear();
    
    constexpr int kCharmCount = 6;
    constexpr float kOriginalGap = 150.0f;
    const float originalStartX = visibleSize.width / 2.0f - 8.0f * kOriginalGap / 2.0f;
    const float originalY = visibleSize.height / 2.0f - 100.0f;
    
    _equippedStartX = notchStartX;
    _equippedY = notchY + 125.0f;
    _equippedGap = 100.0f;
    
    constexpr int kRows = 3;
    constexpr int kCols = 6;
    constexpr float kRowGap = 100.0f;
    
    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            auto charmBack = Sprite::create("Charm/charm_back.png");
            charmBack->setScale(0.6f);
            charmBack->setPosition(Vec2(
                originalStartX + static_cast<float>(col) * kOriginalGap,
                originalY - static_cast<float>(row) * kRowGap
            ));
            layer->addChild(charmBack);
        }
    }
    
    // 护符配置结构体
    struct CharmConfig {
        std::string imagePath;
        int* statePtr;
        int cost;
    };
    
    // 使用 STL 容器存储配置
    std::vector<CharmConfig> charmConfigs = {
        {"Charm/Charm_WaywardCompass.png", &_charmWaywardCompass, 1},
        {"Charm/Charm_ShamanStone.png", &_charmShamanStone, 3},
        {"Charm/Charm_StalwartShell.png", &_charmStalwartShell, 2},
        {"Charm/Charm_SteadyBody.png", &_charmSteadyBody, 1},
        {"Charm/Charm_SoulCatcher.png", &_charmSoulCatcher, 2},
        {"Charm/Charm_SprintMaster.png", &_charmSprintMaster, 1}
    };
    
    // 使用迭代器遍历配置
    int index = 0;
    for (auto it = charmConfigs.begin(); it != charmConfigs.end(); ++it, ++index) {
        CharmInfo info;
        info.imagePath = it->imagePath;
        info.cost = it->cost;
        info.originalPos = Vec2(originalStartX + static_cast<float>(index) * kOriginalGap, originalY);
        info.equippedSlot = -1;
        info.isEquipped = (*(it->statePtr) == 1);
        info.selectFrame = nullptr;
        
        // 创建 Charm 图片
        info.sprite = Sprite::create(info.imagePath);
        info.sprite->setPosition(info.originalPos);
        layer->addChild(info.sprite);
        
        // 创建暗色占位图片
        info.darkSprite = Sprite::create(info.imagePath);
        info.darkSprite->setPosition(info.originalPos);
        info.darkSprite->setColor(Color3B(100, 100, 100));
        info.darkSprite->setVisible(info.isEquipped);
        layer->addChild(info.darkSprite);
        
        _charms.push_back(info);
    }
    
    // 创建选中框
    _selectFrameSprite = Sprite::create();
    auto drawNode = DrawNode::create();
    constexpr float kFrameSize = 70.0f;
    Vec2 vertices[] = {
        Vec2(-kFrameSize / 2.0f, -kFrameSize / 2.0f),
        Vec2(kFrameSize / 2.0f, -kFrameSize / 2.0f),
        Vec2(kFrameSize / 2.0f, kFrameSize / 2.0f),
        Vec2(-kFrameSize / 2.0f, kFrameSize / 2.0f)
    };
    drawNode->drawPolygon(vertices, 4, Color4F(0, 0, 0, 0), 3, Color4F(1, 1, 1, 1));
    _selectFrameSprite->addChild(drawNode);
    layer->addChild(_selectFrameSprite, 100);
    
    updateEquippedPositions();
    updateSelectFrame();
    
    // ===== 右侧信息面板 =====
    const float infoPanelX = visibleSize.width - 450.0f;
    const float infoPanelY = visibleSize.height / 2.0f + 100.0f;
    
    _infoCharmIcon = Sprite::create("Charm/Charm_WaywardCompass.png");
    _infoCharmIcon->setPosition(Vec2(infoPanelX, infoPanelY - 10.0f));
    _infoCharmIcon->setScale(1.5f);
    _infoCharmIcon->setVisible(false);
    _infoCharmIcon->setName("InfoCharmIcon");
    layer->addChild(_infoCharmIcon);
    
    _infoCharmName = Label::createWithTTF(u8"", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    _infoCharmName->setColor(Color3B::WHITE);
    _infoCharmName->setPosition(Vec2(infoPanelX, infoPanelY + 150.0f));
    _infoCharmName->setName("InfoCharmName");
    layer->addChild(_infoCharmName);
    
    _infoCharmDesc = Label::createWithTTF(u8"", "fonts/NotoSerifCJKsc-Regular.otf", 32);
    _infoCharmDesc->setColor(Color3B::WHITE);
    _infoCharmDesc->setPosition(Vec2(infoPanelX, infoPanelY - 150.0f));
    _infoCharmDesc->setMaxLineWidth(400);
    _infoCharmDesc->setName("InfoCharmDesc");
    layer->addChild(_infoCharmDesc);
    
    _costContainer = Node::create();
    _costContainer->setPosition(Vec2(infoPanelX + 45.0f, infoPanelY + 100.0f));
    _costContainer->setName("InfoCharmCostContainer");
    layer->addChild(_costContainer);
    
    _infoCharmCost = Label::createWithTTF(u8"", "fonts/NotoSerifCJKsc-Regular.otf", 32);
    _infoCharmCost->setColor(Color3B::WHITE);
    _infoCharmCost->setPosition(Vec2(infoPanelX - 50.0f, infoPanelY + 100.0f));
    _infoCharmCost->setName("InfoCharmCost");
    layer->addChild(_infoCharmCost);
    
    updateInfoPanel(_selectedCharmIndex, true);
    
    // 添加键盘监听
    _keyboardListener = EventListenerKeyboard::create();
    _keyboardListener->onKeyPressed = CC_CALLBACK_2(CharmManager::onKeyPressed, this);
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(
        _keyboardListener, layer);
}

// ============================================================================
// 输入处理
// ============================================================================

void CharmManager::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {
    if (!_isPanelOpen || _charms.empty()) {
        return;
    }
    
    const int charmCount = static_cast<int>(_charms.size());
    
    switch (keyCode) {
        case EventKeyboard::KeyCode::KEY_A:
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
            _selectedCharmIndex--;
            if (_selectedCharmIndex < 0) {
                _selectedCharmIndex = charmCount - 1;
            }
            updateSelectFrame();
            updateInfoPanel(_selectedCharmIndex, true);
            break;
            
        case EventKeyboard::KeyCode::KEY_D:
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
            _selectedCharmIndex++;
            if (_selectedCharmIndex >= charmCount) {
                _selectedCharmIndex = 0;
            }
            updateSelectFrame();
            updateInfoPanel(_selectedCharmIndex, true);
            break;
            
        case EventKeyboard::KeyCode::KEY_K:
            if (_canEquip) {
                toggleEquipCharm();
            } else {
                CCLOG("只有坐在椅子上才能装卸护符");
            }
            break;
            
        default:
            break;
    }
}

// ============================================================================
// 选择框更新
// ============================================================================

void CharmManager::updateSelectFrame() {
    if (_selectFrameSprite == nullptr || _charms.empty()) {
        return;
    }
    
    if (_selectedCharmIndex >= 0 && _selectedCharmIndex < static_cast<int>(_charms.size())) {
        const auto& charm = _charms[_selectedCharmIndex];
        _selectFrameSprite->setPosition(charm.originalPos);
        _selectFrameSprite->setVisible(true);
    }
}

void CharmManager::selectCharm(int charmIndex) {
    if (charmIndex < 0 || charmIndex >= static_cast<int>(_charms.size())) {
        return;
    }
    
    _selectedCharmIndex = charmIndex;
    updateSelectFrame();
    updateInfoPanel(_selectedCharmIndex, true);
}

// ============================================================================
// 装备/卸载护符
// ============================================================================

void CharmManager::toggleEquipCharm() {
    if (_selectedCharmIndex < 0 || _selectedCharmIndex >= static_cast<int>(_charms.size())) {
        return;
    }
    
    if (!_canEquip) {
        CCLOG("只有坐在椅子上才能装卸护符");
        return;
    }
    
    auto& charm = _charms[_selectedCharmIndex];
    
    // 获取对应的状态指针
    int* statePtr = nullptr;
    switch (_selectedCharmIndex) {
        case 0: statePtr = &_charmWaywardCompass; break;
        case 1: statePtr = &_charmShamanStone; break;
        case 2: statePtr = &_charmStalwartShell; break;
        case 3: statePtr = &_charmSteadyBody; break;
        case 4: statePtr = &_charmSoulCatcher; break;
        case 5: statePtr = &_charmSprintMaster; break;
        default: return;
    }
    
    if (!charm.isEquipped) {
        // 检查凹槽是否足够
        if (_usedNotches + charm.cost > MAX_NOTCHES) {
            CCLOG("凹槽不足，无法装备 Charm");
            return;
        }
        
        // 装备 Charm
        *statePtr = 1;
        charm.isEquipped = true;
        _usedNotches += charm.cost;
        charm.darkSprite->setVisible(true);
        updateEquippedPositions();
        updateNotchDisplay();
        updateInfoPanel(_selectedCharmIndex, true);
        CCLOG("装备护符: %d", _selectedCharmIndex);
    } else {
        // 卸载 Charm
        *statePtr = 0;
        charm.isEquipped = false;
        _usedNotches -= charm.cost;
        charm.equippedSlot = -1;
        charm.darkSprite->setVisible(false);
        charm.sprite->setPosition(charm.originalPos);
        updateEquippedPositions();
        updateNotchDisplay();
        updateInfoPanel(_selectedCharmIndex, true);
        CCLOG("卸载护符: %d", _selectedCharmIndex);
    }
}

// ============================================================================
// 隐藏面板
// ============================================================================

void CharmManager::hideCharmPanel() {
    if (!_isPanelOpen || _parentNode == nullptr) {
        return;
    }
    
    // 移除键盘监听器
    if (_keyboardListener != nullptr) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(_keyboardListener);
        _keyboardListener = nullptr;
    }
    
    auto panel = _parentNode->getChildByName("CharmPanel");
    if (panel != nullptr) {
        panel->removeFromParent();
    }
    
    _isPanelOpen = false;
    _panelLayer = nullptr;
    _notchSprites.clear();
    _charms.clear();
    _infoCharmIcon = nullptr;
    _infoCharmName = nullptr;
    _infoCharmDesc = nullptr;
    _infoCharmCost = nullptr;
    _costContainer = nullptr;
    _selectFrameSprite = nullptr;
    _hintLabel = nullptr;
}

// ============================================================================
// 更新显示
// ============================================================================

void CharmManager::updateNotchDisplay() {
    for (int i = 0; i < MAX_NOTCHES; ++i) {
        if (i < static_cast<int>(_notchSprites.size())) {
            std::string notchImage = (i < _usedNotches) 
                ? "Charm/charm_cost_1.png" 
                : "Charm/charm_cost_0.png";
            _notchSprites[i]->setTexture(notchImage);
        }
    }
}

void CharmManager::updateEquippedPositions() {
    int slot = 0;
    // 使用范围 for 循环遍历护符
    for (auto& charm : _charms) {
        if (charm.isEquipped) {
            charm.equippedSlot = slot;
            Vec2 newPos = Vec2(_equippedStartX + static_cast<float>(slot) * _equippedGap, 
                              _equippedY);
            charm.sprite->setPosition(newPos);
            ++slot;
        } else {
            charm.sprite->setPosition(charm.originalPos);
        }
    }
}

void CharmManager::updateInfoPanel(int charmIndex, bool show) {
    if (_infoCharmIcon == nullptr || _infoCharmName == nullptr || 
        _infoCharmDesc == nullptr || _costContainer == nullptr || 
        _infoCharmCost == nullptr) {
        return;
    }
    
    _costContainer->removeAllChildren();
    
    if (!show || charmIndex < 0 || charmIndex >= static_cast<int>(_charms.size())) {
        _infoCharmIcon->setVisible(false);
        _infoCharmName->setString(u8"");
        _infoCharmDesc->setString(u8"");
        _infoCharmCost->setString(u8"");
        return;
    }
    
    _infoCharmIcon->setVisible(true);
    
    const bool isEquipped = _charms[charmIndex].isEquipped;
    std::string statusText = isEquipped ? u8"[已装备]" : u8"[未装备]";
    
    int costCount = 0;
    switch (charmIndex) {
        case 0:
            _infoCharmIcon->setTexture("Charm/Charm_WaywardCompass.png");
            _infoCharmName->setString(u8"流浪者罗盘 " + statusText);
            _infoCharmCost->setString(u8"消耗");
            _infoCharmDesc->setString(u8"打开地图时，将在位置更上方的那个虫子，可以让旅行者进行精确定位。");
            costCount = 1;
            break;
        case 1:
            _infoCharmIcon->setTexture("Charm/Charm_ShamanStone.png");
            _infoCharmName->setString(u8"萨满之石 " + statusText);
            _infoCharmCost->setString(u8"消耗");
            _infoCharmDesc->setString(u8"提高法术力量。灵魂艺术对敌人造成更多伤害。");
            costCount = 3;
            break;
        case 2:
            _infoCharmIcon->setTexture("Charm/Charm_StalwartShell.png");
            _infoCharmName->setString(u8"坚硬外壳 " + statusText);
            _infoCharmCost->setString(u8"消耗");
            _infoCharmDesc->setString(u8"增强韧性。受到伤害后恢复时间变长，让旅行者保持不受伤害的时间。");
            costCount = 2;
            break;
        case 3:
            _infoCharmIcon->setTexture("Charm/Charm_SteadyBody.png");
            _infoCharmName->setString(u8"稳定之躯 " + statusText);
            _infoCharmCost->setString(u8"消耗");
            _infoCharmDesc->setString(u8"防止佩戴者用骨钉攻击敌人时被震开。");
            costCount = 1;
            break;
        case 4:
            _infoCharmIcon->setTexture("Charm/Charm_SoulCatcher.png");
            _infoCharmName->setString(u8"灵魂捕手 " + statusText);
            _infoCharmCost->setString(u8"消耗");
            _infoCharmDesc->setString(u8"用骨钉攻击敌人时获得的灵魂更多。");
            costCount = 2;
            break;
        case 5:
            _infoCharmIcon->setTexture("Charm/Charm_SprintMaster.png");
            _infoCharmName->setString(u8"疾跑大师 " + statusText);
            _infoCharmCost->setString(u8"消耗");
            _infoCharmDesc->setString(u8"让旅行者能够跑得更快，也能更快冲刺。");
            costCount = 1;
            break;
        default:
            break;
    }
    
    // 创建消耗凹槽图片
    const float costGap = 40.0f;
    const float costStartX = -static_cast<float>(costCount - 1) * costGap / 2.0f;
    for (int j = 0; j < costCount; ++j) {
        auto costIcon = Sprite::create("Charm/charm_cost_1.png");
        costIcon->setPosition(Vec2(costStartX + static_cast<float>(j) * costGap, 0.0f));
        _costContainer->addChild(costIcon);
    }
}
