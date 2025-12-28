#include "CorniferNPC.h"

USING_NS_CC;

CorniferNPC* CorniferNPC::create() {
    CorniferNPC* ret = new (std::nothrow) CorniferNPC();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

CorniferNPC::~CorniferNPC() {
    CC_SAFE_RELEASE(_writingAnim);
    CC_SAFE_RELEASE(_turnLeftAnim);
    CC_SAFE_RELEASE(_talkLeftAnim);
    CC_SAFE_RELEASE(_turnRightAnim);
    CC_SAFE_RELEASE(_talkRightAnim);
}

bool CorniferNPC::init() {
    if (!Node::init()) {
        return false;
    }

    // 初始化状态
    _currentState = State::Writing;
    _isPlayerNearby = false;
    _isFacingRight = false;
    _playerPos = Vec2::ZERO;
    _mousePos = Vec2::ZERO;

    _hasPurchasedMap = false;
    _showPurchaseSuccess = false;
    _dialogueIndex = 0;
    _isChoiceActive = false;
    _choiceSelection = 0;

    _isDialogueActive = false;
    _isPromptShowing = false;
    _hasShownMapPrompt = false;

    _writingAnim = nullptr;
    _turnLeftAnim = nullptr;
    _talkLeftAnim = nullptr;
    _turnRightAnim = nullptr;
    _talkRightAnim = nullptr;

    // 初始化精灵
    _corniferSprite = Sprite::create("Cornifer/Cornifer_0.png");
    if (_corniferSprite) {
        this->addChild(_corniferSprite);
    }

    initAnimations();

    // 默认播放写字动画
    if (_writingAnim && _corniferSprite) {
        _corniferSprite->runAction(RepeatForever::create(Animate::create(_writingAnim)));
    }

    // --- 创建交互提示节点 ---
    _promptNode = Node::create();
    _promptNode->setPosition(Vec2(0, 230));
    _promptNode->setCascadeOpacityEnabled(true);
    _promptNode->setOpacity(0);
    _promptNode->setVisible(false);
    this->addChild(_promptNode, 10);

    auto promptLabel = Label::createWithTTF(u8"聆听", "fonts/NotoSerifCJKsc-Regular.otf", 36);
    if (promptLabel) {
        promptLabel->setPosition(Vec2::ZERO);
        _promptNode->addChild(promptLabel);

        float pHalfH = promptLabel->getContentSize().height / 2;
        float pPadding = 10.0f;

        auto pUp = Sprite::create("Cornifer/dialogbox_up.png");
        if (pUp) {
            pUp->setScale(0.4f);
            float h = pUp->getContentSize().height * 0.4f;
            pUp->setPosition(Vec2(0, pHalfH + pPadding - 10 + h / 2.0f));
            _promptNode->addChild(pUp);
        }
        auto pLow = Sprite::create("Cornifer/dialogbox_low.png");
        if (pLow) {
            pLow->setScale(0.4f);
            float h = pLow->getContentSize().height * 0.4f;
            pLow->setPosition(Vec2(0, -pHalfH - pPadding + 10 - h / 2.0f));
            _promptNode->addChild(pLow);
        }
    }

    // --- 创建对话窗口 UI ---
    _dialogueWindow = Node::create();
    _dialogueWindow->setPosition(Vec2(0, 300));
    _dialogueWindow->setCascadeOpacityEnabled(true);
    _dialogueWindow->setOpacity(0);
    _dialogueWindow->setVisible(false);
    this->addChild(_dialogueWindow, 10);

    // 创建文字标签
    _dialogueLabel = Label::createWithTTF("", "fonts/NotoSerifCJKsc-Regular.otf", 24);
    if (_dialogueLabel) {
        _dialogueLabel->setDimensions(500, 0);
        _dialogueLabel->setAlignment(TextHAlignment::CENTER);
        _dialogueLabel->setPosition(Vec2::ZERO);
        _dialogueLabel->setTextColor(Color4B::WHITE);
        _dialogueWindow->addChild(_dialogueLabel, 1);
    }

    _borderTop = Sprite::create("Cornifer/dialogbox_up.png");
    if (_borderTop) {
        _borderTop->setScale(0.5f);
        _dialogueWindow->addChild(_borderTop, 0);
    }

    _borderBottom = Sprite::create("Cornifer/dialogbox_low.png");
    if (_borderBottom) {
        _borderBottom->setScale(0.5f);
        _dialogueWindow->addChild(_borderBottom, 0);
    }

    _choiceNode = Node::create();
    _choiceNode->setPosition(Vec2(0, -50));
    _choiceNode->setVisible(false);
    _choiceNode->setCascadeOpacityEnabled(true);
    _dialogueWindow->addChild(_choiceNode, 2);

    _yesLabel = Label::createWithTTF(u8"是", "fonts/NotoSerifCJKsc-Regular.otf", 22);
    if (_yesLabel) {
        _yesLabel->setPosition(Vec2(-40, 0));
        _yesLabel->setTextColor(Color4B::WHITE);
        _choiceNode->addChild(_yesLabel);
    }

    _noLabel = Label::createWithTTF(u8"否", "fonts/NotoSerifCJKsc-Regular.otf", 22);
    if (_noLabel) {
        _noLabel->setPosition(Vec2(40, 0));
        _noLabel->setTextColor(Color4B::WHITE);
        _choiceNode->addChild(_noLabel);
    }

    // --- 输入监听 ---
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseMove = CC_CALLBACK_1(CorniferNPC::onMouseMove, this);
    mouseListener->onMouseDown = CC_CALLBACK_1(CorniferNPC::onMouseDown, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

    auto keyListener = EventListenerKeyboard::create();
    keyListener->onKeyPressed = CC_CALLBACK_2(CorniferNPC::onKeyPressed, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyListener, this);

    this->scheduleUpdate();

    return true;
}

void CorniferNPC::initAnimations() {
    Vector<SpriteFrame*> writingFrames;
    for (int i = 0; i <= 7; i++) {
        auto tex = Director::getInstance()->getTextureCache()->addImage("Cornifer/Cornifer_" + std::to_string(i) + ".png");
        if (tex) writingFrames.pushBack(SpriteFrame::createWithTexture(tex, Rect(0, 0, tex->getContentSize().width, tex->getContentSize().height)));
    }
    if (!writingFrames.empty()) { _writingAnim = Animation::createWithSpriteFrames(writingFrames, 0.1f); _writingAnim->retain(); }

    Vector<SpriteFrame*> turnLeftFrames;
    for (int i = 0; i <= 1; i++) {
        auto tex = Director::getInstance()->getTextureCache()->addImage("Cornifer/Cornifer_turnleft_" + std::to_string(i) + ".png");
        if (tex) turnLeftFrames.pushBack(SpriteFrame::createWithTexture(tex, Rect(0, 0, tex->getContentSize().width, tex->getContentSize().height)));
    }
    if (!turnLeftFrames.empty()) { _turnLeftAnim = Animation::createWithSpriteFrames(turnLeftFrames, 0.1f); _turnLeftAnim->retain(); }

    Vector<SpriteFrame*> talkLeftFrames;
    for (int i = 0; i <= 6; i++) {
        auto tex = Director::getInstance()->getTextureCache()->addImage("Cornifer/Cornifer_left_" + std::to_string(i) + ".png");
        if (tex) talkLeftFrames.pushBack(SpriteFrame::createWithTexture(tex, Rect(0, 0, tex->getContentSize().width, tex->getContentSize().height)));
    }
    if (!talkLeftFrames.empty()) { _talkLeftAnim = Animation::createWithSpriteFrames(talkLeftFrames, 0.1f); _talkLeftAnim->retain(); }

    Vector<SpriteFrame*> turnRightFrames;
    auto turnRightTex = Director::getInstance()->getTextureCache()->addImage("Cornifer/Cornifer_turnright.png");
    if (turnRightTex) {
        Rect r(0, 0, turnRightTex->getContentSize().width, turnRightTex->getContentSize().height);
        turnRightFrames.pushBack(SpriteFrame::createWithTexture(turnRightTex, r));
        turnRightFrames.pushBack(SpriteFrame::createWithTexture(turnRightTex, r));
    }
    if (!turnRightFrames.empty()) { _turnRightAnim = Animation::createWithSpriteFrames(turnRightFrames, 0.1f); _turnRightAnim->retain(); }

    Vector<SpriteFrame*> talkRightFrames;
    for (int i = 0; i <= 6; i++) {
        auto tex = Director::getInstance()->getTextureCache()->addImage("Cornifer/Cornifer_right_" + std::to_string(i) + ".png");
        if (tex) talkRightFrames.pushBack(SpriteFrame::createWithTexture(tex, Rect(0, 0, tex->getContentSize().width, tex->getContentSize().height)));
    }
    if (!talkRightFrames.empty()) { _talkRightAnim = Animation::createWithSpriteFrames(talkRightFrames, 0.1f); _talkRightAnim->retain(); }
}

void CorniferNPC::update(float dt) {
    checkPlayerDistance();

    bool shouldShowPrompt = _isPlayerNearby && !_isDialogueActive;

    if (shouldShowPrompt != _isPromptShowing) {
        _isPromptShowing = shouldShowPrompt;
        _promptNode->stopAllActions();

        if (_isPromptShowing) {
            CCLOG("[Cornifer] 显示交互提示：聆听");
            _promptNode->setVisible(true);
            _promptNode->runAction(FadeIn::create(0.2f));
        }
        else {
            CCLOG("[Cornifer] 隐藏交互提示");
            _promptNode->runAction(Sequence::create(FadeOut::create(0.2f), Hide::create(), nullptr));
        }
    }

    if (_isPlayerNearby) {
        Size size = _corniferSprite->getContentSize();
        Vec2 npcWorldCenter = _corniferSprite->convertToWorldSpace(Vec2(size.width / 2, size.height / 2));

        bool shouldFaceRight = (_playerPos.x > npcWorldCenter.x);

        if (_currentState == State::Writing) {
            _currentState = State::Turning;
            _corniferSprite->stopAllActions();
            _corniferSprite->setFlippedX(false);
            _isFacingRight = shouldFaceRight;

            Animation* turnAnim = _isFacingRight ? _turnRightAnim : _turnLeftAnim;
            Animation* talkAnim = _isFacingRight ? _talkRightAnim : _talkLeftAnim;

            if (turnAnim && talkAnim) {
                auto turnAction = Animate::create(turnAnim);
                auto seq = Sequence::create(turnAction, CallFunc::create([this, talkAnim]() {
                    _currentState = State::Talking;
                    auto talkAction = RepeatForever::create(Animate::create(talkAnim));
                    _corniferSprite->runAction(talkAction);
                    CCLOG("[Cornifer] 切换到 Talking 动画");
                    }), nullptr);
                _corniferSprite->runAction(seq);
            }
        }
        else if (_currentState == State::Talking) {
            if (_isFacingRight != shouldFaceRight) {
                _isFacingRight = shouldFaceRight;
                _corniferSprite->stopAllActions();
                Animation* talkAnim = _isFacingRight ? _talkRightAnim : _talkLeftAnim;
                if (talkAnim) _corniferSprite->runAction(RepeatForever::create(Animate::create(talkAnim)));
            }
        }
    }
    else {
        if (_currentState != State::Writing) {
            _currentState = State::Writing;
            _corniferSprite->stopAllActions();
            _corniferSprite->setFlippedX(false);
            if (_writingAnim) _corniferSprite->runAction(RepeatForever::create(Animate::create(_writingAnim)));
            closeDialogue();
        }
    }
}

void CorniferNPC::checkPlayerDistance() {
    if (!_corniferSprite) return;

    Size size = _corniferSprite->getContentSize();
    Vec2 npcWorldCenter = _corniferSprite->convertToWorldSpace(Vec2(size.width / 2, size.height / 2));

    float rectWidth = 400.0f;
    float rectHeight = 300.0f;

    Rect detectionRect(
        npcWorldCenter.x - rectWidth / 2,
        npcWorldCenter.y - rectHeight / 2,
        rectWidth,
        rectHeight
    );

    bool wasNearby = _isPlayerNearby;
    _isPlayerNearby = detectionRect.containsPoint(_playerPos);

    if (_isPlayerNearby != wasNearby) {
        if (_isPlayerNearby) {
            CCLOG("[Cornifer] 玩家进入检测范围");
            CCLOG("    检测矩形: 左下(%.1f, %.1f) 右上(%.1f, %.1f)",
                detectionRect.getMinX(), detectionRect.getMinY(),
                detectionRect.getMaxX(), detectionRect.getMaxY());
            CCLOG("    玩家位置: (%.1f, %.1f)", _playerPos.x, _playerPos.y);
        }
        else {
            CCLOG("[Cornifer] 玩家离开检测范围");
        }
    }
}

void CorniferNPC::updateFacing() {}

void CorniferNPC::onMouseMove(Event* event) {
    EventMouse* e = (EventMouse*)event;
    Vec2 screenPos = e->getLocation();

    auto scene = Director::getInstance()->getRunningScene();
    if (scene && scene->getDefaultCamera()) {
        auto camera = scene->getDefaultCamera();
        Size viewport = Director::getInstance()->getWinSize();

        Vec3 nearPoint, farPoint;
        Vec3 srcNear(screenPos.x, screenPos.y, 0.0f);
        Vec3 srcFar(screenPos.x, screenPos.y, 1.0f);

        camera->unproject(viewport, &srcNear, &nearPoint);
        camera->unproject(viewport, &srcFar, &farPoint);

        Vec3 direction = farPoint - nearPoint;
        if (std::abs(direction.z) > 1e-6) {
            float t = -nearPoint.z / direction.z;
            Vec3 intersection = nearPoint + direction * t;

            Vec2 mousePos = Vec2(intersection.x, intersection.y);

            if (_isChoiceActive && _dialogueWindow->isVisible() && _yesLabel && _noLabel) {
                Vec2 localMousePos = _choiceNode->convertToNodeSpace(mousePos);
                int lastSelection = _choiceSelection;

                auto getBox = [](Label* label) {
                    Rect box = label->getBoundingBox();
                    return Rect(box.origin.x - 20, box.origin.y - 20,
                        box.size.width + 40, box.size.height + 40);
                    };

                if (getBox(_yesLabel).containsPoint(localMousePos)) _choiceSelection = 0;
                else if (getBox(_noLabel).containsPoint(localMousePos)) _choiceSelection = 1;

                if (lastSelection != _choiceSelection) updateChoiceUI();
            }
        }
    }
}

void CorniferNPC::onMouseDown(Event* event) {
    if (_isChoiceActive && _dialogueWindow->isVisible()) {
        EventMouse* e = (EventMouse*)event;

        Vec2 clickWorldPos;
        auto scene = Director::getInstance()->getRunningScene();
        if (scene && scene->getDefaultCamera()) {
            auto camera = scene->getDefaultCamera();
            Size viewport = Director::getInstance()->getWinSize();
            Vec3 nearPoint, farPoint;
            Vec3 srcNear(e->getLocation().x, e->getLocation().y, 0.0f);
            Vec3 srcFar(e->getLocation().x, e->getLocation().y, 1.0f);
            camera->unproject(viewport, &srcNear, &nearPoint);
            camera->unproject(viewport, &srcFar, &farPoint);

            Vec3 direction = farPoint - nearPoint;
            if (std::abs(direction.z) > 1e-6) {
                float t = -nearPoint.z / direction.z;
                Vec3 intersection = nearPoint + direction * t;
                clickWorldPos = Vec2(intersection.x, intersection.y);
            }
            else {
                clickWorldPos = Vec2(nearPoint.x, nearPoint.y);
            }
        }
        else {
            clickWorldPos = e->getLocation();
        }

        Vec2 localClickPos = _choiceNode->convertToNodeSpace(clickWorldPos);

        if (_yesLabel->getBoundingBox().containsPoint(localClickPos)) {
            selectChoice(0);
        }
        else if (_noLabel->getBoundingBox().containsPoint(localClickPos)) {
            selectChoice(1);
        }
    }
}

void CorniferNPC::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {
    if (_isDialogueActive) {
        if (_dialogueWindow->getNumberOfRunningActions() > 0) return;
        handleDialogueInput(keyCode);
    }
    
    else if ((keyCode == EventKeyboard::KeyCode::KEY_W || keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_W) && _isPlayerNearby) {
        CCLOG("[Cornifer] 玩家按下 W 键，打开对话");
        showDialogue();
    }
}

void CorniferNPC::showDialogue() {
    _isDialogueActive = true;

    _dialogueWindow->stopAllActions();
    _dialogueWindow->setVisible(true);
    _dialogueWindow->setOpacity(0);
    _dialogueWindow->runAction(FadeIn::create(0.5f));

    _dialogueIndex = 0;
    _isChoiceActive = false;
    _choiceSelection = 0;
    _showPurchaseSuccess = false;
    updateDialogueContent();

    CCLOG("[Cornifer] 对话窗口已打开");
}

void CorniferNPC::closeDialogue() {
    if (!_isDialogueActive) return;

    if (_hasPurchasedMap && !_hasShownMapPrompt) {

        auto scene = Director::getInstance()->getRunningScene();
        if (!scene) return;

        auto visibleSize = Director::getInstance()->getVisibleSize();

        // 获取相机位置，计算屏幕中心的世界坐标
        Vec2 screenCenter;
        auto camera = scene->getDefaultCamera();
        if (camera) {
            Vec3 camPos = camera->getPosition3D();
            screenCenter = Vec2(camPos.x, camPos.y);
        }
        else {
            screenCenter = Vec2(visibleSize.width / 2, visibleSize.height / 2);
        }

        // 创建一个容器节点
        auto container = Node::create();
        container->setPosition(Vec2::ZERO);
        container->setCascadeOpacityEnabled(true);
        scene->addChild(container, 100, "MapPromptLayer");

        // 使用 Sprite 创建半透明黑色背景（比 LayerColor 更可靠）
        auto bg = Sprite::create();
        bg->setTextureRect(Rect(0, 0, visibleSize.width * 2, visibleSize.height * 2));
        bg->setColor(Color3B::BLACK);
        bg->setOpacity(180);
        bg->setPosition(screenCenter);
        container->addChild(bg, 0);

        // 地图图片
        auto mapSprite = Sprite::create("Cornifer/Map_prompt.png");
        if (mapSprite) {
            mapSprite->setPosition(screenCenter);
            container->addChild(mapSprite, 1);
        }

        // "地图已获取"文字
        auto label = Label::createWithTTF(u8"地图已获取", "fonts/NotoSerifCJKsc-Regular.otf", 48);
        if (label) {
            label->setPosition(Vec2(screenCenter.x, screenCenter.y + 200));
            label->setTextColor(Color4B::WHITE);
            container->addChild(label, 1);
        }

        // 淡入淡出效果
        container->setOpacity(0);
        container->runAction(Sequence::create(
            FadeIn::create(0.3f),
            DelayTime::create(1.5f),
            FadeOut::create(0.3f),
            RemoveSelf::create(),
            nullptr
        ));
    }

    _hasShownMapPrompt = true;

    _isDialogueActive = false;
    _isChoiceActive = false;
    _showPurchaseSuccess = false;

    _dialogueWindow->stopAllActions();
    _dialogueWindow->runAction(Sequence::create(FadeOut::create(0.5f), Hide::create(), nullptr));

    CCLOG("[Cornifer] 对话窗口已关闭");
}

void CorniferNPC::handleDialogueInput(EventKeyboard::KeyCode keyCode) {
    if (_isChoiceActive) {
        // 使用 Y 键选择"是"
        if (keyCode == EventKeyboard::KeyCode::KEY_Y || keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_Y) {
            selectChoice(0);
            _hasPurchasedMap = true;
        }
        // 使用 N 键选择"否"
        else if (keyCode == EventKeyboard::KeyCode::KEY_N || keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_N) {
            selectChoice(1);
        }
    }
    else {
        // 【修改】将 E 键改为 W 键推进对话
        if (keyCode == EventKeyboard::KeyCode::KEY_W || keyCode == EventKeyboard::KeyCode::KEY_CAPITAL_W) {
            advanceDialogue();
        }
    }
}

void CorniferNPC::selectChoice(int choiceIndex) {
    if (choiceIndex == 0) {
        _showPurchaseSuccess = true;
        _dialogueIndex = 0;
        _isChoiceActive = false;
        fadeToUpdateContent();
    }
    else {
        closeDialogue();
    }
}

void CorniferNPC::advanceDialogue() {
    _dialogueIndex++;
    fadeToUpdateContent();
}

void CorniferNPC::fadeToUpdateContent() {
    _dialogueWindow->stopAllActions();
    auto seq = Sequence::create(
        FadeOut::create(0.5f),
        CallFunc::create([this]() {
            this->updateDialogueContent();
            }),
        FadeIn::create(0.5f),
        nullptr
    );
    _dialogueWindow->runAction(seq);
}

void CorniferNPC::updateDialogueContent() {
    std::vector<std::string> prePurchaseDialogues = {
        u8"嗯？啊，你好。你是来探索这些美丽的遗迹的吗？我是制图师柯尼法。",
        u8"我本性热衷于探索，迷路后再次找到正确的路，这种快乐是无可比拟的；你和我都非常幸运。",
        u8"我是一位制图师，正在绘制这个区域的地图。你要来一份我努力到现在的工作成果吗？"
    };

    std::vector<std::string> purchaseSuccessDialogues = {
        u8"明智的选择！这份地图应该能帮你指明方向。",
        u8"祝你旅途平安，小家伙！我还有很多测绘工作要做。"
    };

    std::vector<std::string> postPurchaseDialogues = {
        u8"又见面了！希望那份地图对你有所帮助。",
        u8"我很快就要启程前往下一个区域了。也许我们还会再见面的。"
    };

    std::vector<std::string>* currentDialogues;
    if (_showPurchaseSuccess) {
        currentDialogues = &purchaseSuccessDialogues;
    }
    else if (_hasPurchasedMap) {
        currentDialogues = &postPurchaseDialogues;
    }
    else {
        currentDialogues = &prePurchaseDialogues;
    }

    if (_dialogueIndex >= static_cast<int>(currentDialogues->size())) {
        closeDialogue();
        return;
    }

    if (_dialogueLabel) {
        std::string text = (*currentDialogues)[_dialogueIndex];
        _dialogueLabel->setString(text);

        _dialogueLabel->updateContent();

        float textHeight = _dialogueLabel->getContentSize().height;
        float halfHeight = textHeight / 2.0f;

        float padding = 40.0f;

        float borderHeight = 0.0f;
        if (_borderTop) {
            borderHeight = _borderTop->getContentSize().height * 0.5f;
        }

        CCLOG("[Cornifer] 对话文字高度: %.1f, 边框高度: %.1f", textHeight, borderHeight);

        if (_borderTop) {
            float topY = halfHeight + padding + borderHeight / 2.0f;
            _borderTop->setPosition(Vec2(0, topY));
            CCLOG("  上边框位置: (0, %.1f)", topY);
        }

        if (!_hasPurchasedMap && !_showPurchaseSuccess && _dialogueIndex == 2) {
            _isChoiceActive = true;
            _choiceNode->setVisible(true);

            float choiceY = -halfHeight - padding;
            _choiceNode->setPosition(Vec2(0, choiceY));

            if (_borderBottom) {
                float bottomY = choiceY - 40.0f - borderHeight / 2.0f;
                _borderBottom->setPosition(Vec2(0, bottomY));
                CCLOG("  下边框位置(有选项): (0, %.1f)", bottomY);
            }
        }
        else {
            _isChoiceActive = false;
            _choiceNode->setVisible(false);

            if (_borderBottom) {
                float bottomY = -halfHeight - padding - borderHeight / 2.0f;
                _borderBottom->setPosition(Vec2(0, bottomY));
                CCLOG("  下边框位置(无选项): (0, %.1f)", bottomY);
            }
        }
    }

    updateChoiceUI();
}

void CorniferNPC::updateChoiceUI() {
    if (!_yesLabel || !_noLabel) return;

    _yesLabel->setColor(Color3B::WHITE);
    _noLabel->setColor(Color3B::WHITE);

    if (_choiceSelection == 0) {
        _yesLabel->setScale(1.3f);
        _noLabel->setScale(1.0f);
    }
    else {
        _yesLabel->setScale(1.0f);
        _noLabel->setScale(1.3f);
    }
}

void CorniferNPC::setSpawnPoint(Vec2 point) {
    this->setPosition(point + Vec2(0, 40));
}

void CorniferNPC::setPlayerPosition(const cocos2d::Vec2& playerPos)
{
    _playerPos = playerPos;
}