#pragma once

#ifndef __SETTINGS_PANEL_H__
#define __SETTINGS_PANEL_H__

#include "cocos2d.h"
#include "AudioSettings.h"

class SettingsPanel : public cocos2d::Node
{
public:
    virtual bool init();
    CREATE_FUNC(SettingsPanel);
    
    void show();
    void hide();
    
    using CloseCallback = std::function<void()>;
    void setCloseCallback(const CloseCallback& callback) { _closeCallback = callback; }
    
private:
    void showMainMenu();
    void showVolumePanel();
    void showKeyBindPanel();
    void hideAllPanels();
    
    void onCloseClicked(cocos2d::Ref* sender);
    void onBackToMainMenu(cocos2d::Ref* sender);
    void onSaveClicked(cocos2d::Ref* sender);  // 新增：保存按钮回调
    void updateVolumeLabels();
    
    cocos2d::LayerColor* _backgroundLayer = nullptr;
    
    // 主菜单
    cocos2d::Node* _mainMenuPanel = nullptr;
    
    // 音量调节面板
    cocos2d::Node* _volumePanel = nullptr;
    cocos2d::Label* _bgmVolumeLabel = nullptr;
    cocos2d::Label* _sfxVolumeLabel = nullptr;
    
    // 键位说明面板
    cocos2d::Node* _keyBindPanel = nullptr;
    
    CloseCallback _closeCallback;
    
    // 临时音量值（编辑中，未保存）
    int _tempBgmVolume;
    int _tempSfxVolume;
};

#endif // __SETTINGS_PANEL_H__
