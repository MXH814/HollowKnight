#include "PauseMenu.h"
#include "MainMenuScene.h"
#include "SimpleAudioEngine.h"
#include "SettingsPanel.h"

USING_NS_CC;
using namespace CocosDenshion;

bool PauseMenu::init()
{
    if (!Node::init())
        return false;
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    
    // 创建半透明黑色背景层
    _backgroundLayer = LayerColor::create(Color4B(0, 0, 0, 180));
    _backgroundLayer->setContentSize(visibleSize);
    this->addChild(_backgroundLayer, 0);
    
    auto top = Sprite::create("Menu/pausemenu_top.png");
    top->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 200));
    this->addChild(top);
    
    auto bottom = Sprite::create("Menu/pausemenu_bottom.png");
    bottom->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 200));
    this->addChild(bottom);


    // 创建"回到游戏"按钮
    auto resumeLabel = Label::createWithTTF(u8"继续", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    resumeLabel->setTextColor(Color4B::WHITE);
    auto resumeItem = MenuItemLabel::create(resumeLabel, CC_CALLBACK_1(PauseMenu::onResumeClicked, this));
    resumeItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 80));
    
    // 创建"设置"按钮
    auto settingsLabel = Label::createWithTTF(u8"选项", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    settingsLabel->setTextColor(Color4B::WHITE);
    auto settingsItem = MenuItemLabel::create(settingsLabel, CC_CALLBACK_1(PauseMenu::onSettingsClicked, this));
    settingsItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 20));
    
    // 创建"退出到菜单"按钮
    auto exitLabel = Label::createWithTTF(u8"退出到菜单", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    exitLabel->setTextColor(Color4B::WHITE);
    auto exitItem = MenuItemLabel::create(exitLabel, CC_CALLBACK_1(PauseMenu::onExitClicked, this));
    exitItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 120));
    
    // 创建菜单
    _menu = Menu::create(resumeItem, settingsItem, exitItem, nullptr);
    _menu->setPosition(Vec2::ZERO);
    this->addChild(_menu, 1);
    
    // 创建设置面板
    _settingsPanel = SettingsPanel::create();
    _settingsPanel->setCloseCallback([this]() {
        // 关闭设置面板后恢复暂停菜单的交互
    });
    this->addChild(_settingsPanel, 10);
    
    // 初始状态为隐藏
    this->setVisible(false);
    _isVisible = false;
    
    return true;
}

void PauseMenu::show()
{
    this->setVisible(true);
    _isVisible = true;
    
    // 暂停游戏
    Director::getInstance()->pause();
    
    SimpleAudioEngine::getInstance()->playEffect("Music/pause.wav");
}

void PauseMenu::hide()
{
    this->setVisible(false);
    _isVisible = false;
    
    // 恢复游戏
    Director::getInstance()->resume();
}

void PauseMenu::onResumeClicked(Ref* sender)
{
    SimpleAudioEngine::getInstance()->playEffect("Music/click.wav");
    hide();
    
    if (_resumeCallback)
    {
        _resumeCallback();
    }
}

void PauseMenu::onSettingsClicked(Ref* sender)
{
    SimpleAudioEngine::getInstance()->playEffect("Music/click.wav");
    
    if (_settingsPanel)
    {
        _settingsPanel->show();
    }
    
    if (_settingsCallback)
    {
        _settingsCallback();
    }
    else
    {
        CCLOG("设置界面");
    }
}

void PauseMenu::onExitClicked(Ref* sender)
{
    SimpleAudioEngine::getInstance()->playEffect("Music/click.wav");
    
    // 先恢复游戏（避免场景切换时仍处于暂停状态）
    Director::getInstance()->resume();
    
    // 停止背景音乐
    SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    
    if (_exitCallback)
    {
        _exitCallback();
    }
    else
    {
        // 默认行为：返回主菜单
        Director::getInstance()->replaceScene(
            TransitionFade::create(0.5f, MainMenuScene::createScene(), Color3B::BLACK)
        );
    }
}