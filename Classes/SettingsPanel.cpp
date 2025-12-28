#include "SettingsPanel.h"
#include "AudioSettings.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;
using namespace CocosDenshion;

bool SettingsPanel::init()
{
    if (!Node::init())
        return false;
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    
    // 初始化临时音量为当前全局值
    _tempBgmVolume = AudioSettings::g_bgmVolume;
    _tempSfxVolume = AudioSettings::g_sfxVolume;
    
    // 创建全黑背景层
    _backgroundLayer = LayerColor::create(Color4B(0, 0, 0, 255));
    _backgroundLayer->setContentSize(visibleSize);
    this->addChild(_backgroundLayer, 0);
    
    float centerX = visibleSize.width / 2;
    float centerY = visibleSize.height / 2;
    
    // ========== 创建主选择菜单面板 ==========
    _mainMenuPanel = Node::create();
    this->addChild(_mainMenuPanel, 1);
    
    // 标题
    auto titleLabel = Label::createWithTTF(u8"设置", "fonts/NotoSerifCJKsc-Regular.otf", 72);
    titleLabel->setTextColor(Color4B::WHITE);
    titleLabel->setPosition(Vec2(centerX, visibleSize.height - 150));
    _mainMenuPanel->addChild(titleLabel);

    auto titlebg = Sprite::create("Menu/credits fleur.png");
    titlebg->setPosition(Vec2(centerX, visibleSize.height - 240));
    _mainMenuPanel->addChild(titlebg);
    
    // 音量调节按钮
    auto volumeLabel = Label::createWithTTF(u8"音量调节", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    volumeLabel->setTextColor(Color4B::WHITE);
    auto volumeItem = MenuItemLabel::create(volumeLabel, [this](Ref*) {
        SimpleAudioEngine::getInstance()->playEffect("Music/click.wav", false, 1.0f, 0.0f, AudioSettings::getSFXVolume());
        showVolumePanel();
    });
    volumeItem->setPosition(Vec2(centerX, centerY + 50));
    
    // 键位说明按钮
    auto keyBindLabel = Label::createWithTTF(u8"键位说明", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    keyBindLabel->setTextColor(Color4B::WHITE);
    auto keyBindItem = MenuItemLabel::create(keyBindLabel, [this](Ref*) {
        SimpleAudioEngine::getInstance()->playEffect("Music/click.wav", false, 1.0f, 0.0f, AudioSettings::getSFXVolume());
        showKeyBindPanel();
    });
    keyBindItem->setPosition(Vec2(centerX, centerY - 50));
    
    // 返回按钮
    auto closeLabel = Label::createWithTTF(u8"返回", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    closeLabel->setTextColor(Color4B::WHITE);
    auto closeItem = MenuItemLabel::create(closeLabel, CC_CALLBACK_1(SettingsPanel::onCloseClicked, this));
    closeItem->setPosition(Vec2(centerX, centerY - 200));
    
    auto mainMenu = Menu::create(volumeItem, keyBindItem, closeItem, nullptr);
    mainMenu->setPosition(Vec2::ZERO);
    _mainMenuPanel->addChild(mainMenu);
    
    // ========== 创建音量调节面板 ==========
    _volumePanel = Node::create();
    _volumePanel->setVisible(false);
    this->addChild(_volumePanel, 1);
    
    // 音量标题
    auto volumeTitleLabel = Label::createWithTTF(u8"音量调节", "fonts/NotoSerifCJKsc-Regular.otf", 72);
    volumeTitleLabel->setTextColor(Color4B::WHITE);
    volumeTitleLabel->setPosition(Vec2(centerX, visibleSize.height - 150));
    _volumePanel->addChild(volumeTitleLabel);

    auto volumebg = Sprite::create("Menu/credits fleur.png");
    volumebg->setPosition(Vec2(centerX, visibleSize.height - 240));
    _volumePanel->addChild(volumebg);
    
    float startY = centerY + 50;
    
    // 背景音乐音量标签
    auto bgmLabel = Label::createWithTTF(u8"背景音乐", "fonts/NotoSerifCJKsc-Regular.otf", 36);
    bgmLabel->setTextColor(Color4B::WHITE);
    bgmLabel->setPosition(Vec2(centerX - 200, startY));
    _volumePanel->addChild(bgmLabel);
    
    // 背景音乐 - 按钮
    auto bgmMinusLabel = Label::createWithTTF("-", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    bgmMinusLabel->setTextColor(Color4B::WHITE);
    auto bgmMinusItem = MenuItemLabel::create(bgmMinusLabel, [this](Ref*) {
        SimpleAudioEngine::getInstance()->playEffect("Music/click.wav", false, 1.0f, 0.0f, AudioSettings::getSFXVolume());
        if (_tempBgmVolume > 0) {
            _tempBgmVolume--;
            updateVolumeLabels();
        }
    });
    bgmMinusItem->setPosition(Vec2(centerX, startY));
    
    // 背景音乐音量数值显示
    _bgmVolumeLabel = Label::createWithTTF(std::to_string(_tempBgmVolume), "fonts/NotoSerifCJKsc-Regular.otf", 36);
    _bgmVolumeLabel->setTextColor(Color4B::WHITE);
    _bgmVolumeLabel->setPosition(Vec2(centerX + 80, startY));
    _volumePanel->addChild(_bgmVolumeLabel);
    
    // 背景音乐 + 按钮
    auto bgmPlusLabel = Label::createWithTTF("+", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    bgmPlusLabel->setTextColor(Color4B::WHITE);
    auto bgmPlusItem = MenuItemLabel::create(bgmPlusLabel, [this](Ref*) {
        SimpleAudioEngine::getInstance()->playEffect("Music/click.wav", false, 1.0f, 0.0f, AudioSettings::getSFXVolume());
        if (_tempBgmVolume < 10) {
            _tempBgmVolume++;
            updateVolumeLabels();
        }
    });
    bgmPlusItem->setPosition(Vec2(centerX + 160, startY));
    
    // 音效音量标签
    auto sfxLabel = Label::createWithTTF(u8"音效", "fonts/NotoSerifCJKsc-Regular.otf", 36);
    sfxLabel->setTextColor(Color4B::WHITE);
    sfxLabel->setPosition(Vec2(centerX - 200, startY - 80));
    _volumePanel->addChild(sfxLabel);
    
    // 音效 - 按钮
    auto sfxMinusLabel = Label::createWithTTF("-", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    sfxMinusLabel->setTextColor(Color4B::WHITE);
    auto sfxMinusItem = MenuItemLabel::create(sfxMinusLabel, [this](Ref*) {
        if (_tempSfxVolume > 0) {
            _tempSfxVolume--;
            updateVolumeLabels();
        }
        SimpleAudioEngine::getInstance()->playEffect("Music/click.wav", false, 1.0f, 0.0f, _tempSfxVolume / 10.0f);
    });
    sfxMinusItem->setPosition(Vec2(centerX, startY - 80));
    
    // 音效音量数值显示
    _sfxVolumeLabel = Label::createWithTTF(std::to_string(_tempSfxVolume), "fonts/NotoSerifCJKsc-Regular.otf", 36);
    _sfxVolumeLabel->setTextColor(Color4B::WHITE);
    _sfxVolumeLabel->setPosition(Vec2(centerX + 80, startY - 80));
    _volumePanel->addChild(_sfxVolumeLabel);
    
    // 音效 + 按钮
    auto sfxPlusLabel = Label::createWithTTF("+", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    sfxPlusLabel->setTextColor(Color4B::WHITE);
    auto sfxPlusItem = MenuItemLabel::create(sfxPlusLabel, [this](Ref*) {
        if (_tempSfxVolume < 10) {
            _tempSfxVolume++;
            updateVolumeLabels();
        }
        SimpleAudioEngine::getInstance()->playEffect("Music/click.wav", false, 1.0f, 0.0f, _tempSfxVolume / 10.0f);
    });
    sfxPlusItem->setPosition(Vec2(centerX + 160, startY - 80));
    
    // 保存按钮
    auto saveLabel = Label::createWithTTF(u8"保存", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    saveLabel->setTextColor(Color4B::WHITE);  
    auto saveItem = MenuItemLabel::create(saveLabel, CC_CALLBACK_1(SettingsPanel::onSaveClicked, this));
    saveItem->setPosition(Vec2(centerX - 100, 100));
    
    // 返回上级菜单按钮（不保存）
    auto volumeBackLabel = Label::createWithTTF(u8"取消", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    volumeBackLabel->setTextColor(Color4B::WHITE);
    auto volumeBackItem = MenuItemLabel::create(volumeBackLabel, CC_CALLBACK_1(SettingsPanel::onBackToMainMenu, this));
    volumeBackItem->setPosition(Vec2(centerX + 100, 100));
    
    auto volumeMenu = Menu::create(bgmMinusItem, bgmPlusItem, sfxMinusItem, sfxPlusItem, saveItem, volumeBackItem, nullptr);
    volumeMenu->setPosition(Vec2::ZERO);
    _volumePanel->addChild(volumeMenu);
    
    // ========== 创建键位说明面板 ==========
    _keyBindPanel = Node::create();
    _keyBindPanel->setVisible(false);
    this->addChild(_keyBindPanel, 1);
    
    // 键位标题
    auto keyBindTitleLabel = Label::createWithTTF(u8"键盘", "fonts/NotoSerifCJKsc-Regular.otf", 72);
    keyBindTitleLabel->setTextColor(Color4B::WHITE);
    keyBindTitleLabel->setPosition(Vec2(centerX, visibleSize.height - 150));
    _keyBindPanel->addChild(keyBindTitleLabel);

    auto keyBindbg = Sprite::create("Menu/credits fleur.png");
    keyBindbg->setPosition(Vec2(centerX, visibleSize.height - 240));
    _keyBindPanel->addChild(keyBindbg);

    // 键位列表
    std::vector<std::pair<std::string, std::string>> keyBindings = {
        {u8"上", u8"W"},
        {u8"下", u8"S"},
        {u8"左", u8"A"},
        {u8"右", u8"D"},
        {u8"攻击", u8"J"},
        {u8"跳跃", u8"K"},
        {u8"冲刺", u8"L"},
        {u8"聚集/技能", u8"Space"},
        {u8"暂停", u8"ESC"}
    };
    
    float keyStartY = centerY + 225;
    for (size_t i = 0; i < keyBindings.size(); ++i)
    {
        auto actionLabel = Label::createWithTTF(keyBindings[i].first, "fonts/NotoSerifCJKsc-Regular.otf", 36);
        actionLabel->setTextColor(Color4B(200, 200, 200, 255));
        actionLabel->setPosition(Vec2(centerX - 150, keyStartY - i * 60));
        _keyBindPanel->addChild(actionLabel);
        
        auto keyLabel = Label::createWithTTF(keyBindings[i].second, "fonts/NotoSerifCJKsc-Regular.otf", 36);
        keyLabel->setTextColor(Color4B::WHITE);
        keyLabel->setPosition(Vec2(centerX + 150, keyStartY - i * 60));
        _keyBindPanel->addChild(keyLabel);
    }
    
    // 返回上级菜单按钮
    auto keyBindBackLabel = Label::createWithTTF(u8"返回", "fonts/NotoSerifCJKsc-Regular.otf", 48);
    keyBindBackLabel->setTextColor(Color4B::WHITE);
    auto keyBindBackItem = MenuItemLabel::create(keyBindBackLabel, CC_CALLBACK_1(SettingsPanel::onBackToMainMenu, this));
    keyBindBackItem->setPosition(Vec2(centerX, 100));
    
    auto keyBindMenu = Menu::create(keyBindBackItem, nullptr);
    keyBindMenu->setPosition(Vec2::ZERO);
    _keyBindPanel->addChild(keyBindMenu);
    
    this->setVisible(false);
    
    return true;
}

void SettingsPanel::hideAllPanels()
{
    _mainMenuPanel->setVisible(false);
    _volumePanel->setVisible(false);
    _keyBindPanel->setVisible(false);
}

void SettingsPanel::showMainMenu()
{
    hideAllPanels();
    _mainMenuPanel->setVisible(true);
}

void SettingsPanel::showVolumePanel()
{
    // 进入音量面板时，加载当前全局设置
    _tempBgmVolume = AudioSettings::g_bgmVolume;
    _tempSfxVolume = AudioSettings::g_sfxVolume;
    updateVolumeLabels();
    hideAllPanels();
    _volumePanel->setVisible(true);
}

void SettingsPanel::showKeyBindPanel()
{
    hideAllPanels();
    _keyBindPanel->setVisible(true);
}

void SettingsPanel::show()
{
    showMainMenu();
    this->setVisible(true);
}

void SettingsPanel::hide()
{
    this->setVisible(false);
}

void SettingsPanel::onCloseClicked(Ref* sender)
{
    SimpleAudioEngine::getInstance()->playEffect("Music/click.wav", false, 1.0f, 0.0f, AudioSettings::getSFXVolume());
    hide();
    
    if (_closeCallback)
    {
        _closeCallback();
    }
}

void SettingsPanel::onBackToMainMenu(Ref* sender)
{
    SimpleAudioEngine::getInstance()->playEffect("Music/click.wav", false, 1.0f, 0.0f, AudioSettings::getSFXVolume());
    // 取消时恢复临时值为全局值
    _tempBgmVolume = AudioSettings::g_bgmVolume;
    _tempSfxVolume = AudioSettings::g_sfxVolume;
    showMainMenu();
}

void SettingsPanel::onSaveClicked(Ref* sender)
{
    SimpleAudioEngine::getInstance()->playEffect("Music/click.wav", false, 1.0f, 0.0f, _tempSfxVolume / 10.0f);
    
    // 保存临时值到全局变量
    AudioSettings::setBGMVolume(_tempBgmVolume);
    AudioSettings::setSFXVolume(_tempSfxVolume);
    
    // 立即应用BGM音量
    AudioSettings::applyBGMVolume();
    
    showMainMenu();
}

void SettingsPanel::updateVolumeLabels()
{
    if (_bgmVolumeLabel)
    {
        _bgmVolumeLabel->setString(std::to_string(_tempBgmVolume));
    }
    if (_sfxVolumeLabel)
    {
        _sfxVolumeLabel->setString(std::to_string(_tempSfxVolume));
    }
}