#pragma once

#ifndef __PAUSE_MENU_H__
#define __PAUSE_MENU_H__

#include "cocos2d.h"

// 在文件顶部添加前向声明
class SettingsPanel;

class PauseMenu : public cocos2d::Node
{
public:
    // 创建暂停菜单
    // static PauseMenu* create(); // <-- 删除此行，避免重复声明
    virtual bool init();
    
    // 显示/隐藏菜单
    void show();
    void hide();
    bool isVisible() const { return _isVisible; }
    
    // 设置回调
    using ResumeCallback = std::function<void()>;
    using SettingsCallback = std::function<void()>;
    using ExitCallback = std::function<void()>;
    
    void setResumeCallback(const ResumeCallback& callback) { _resumeCallback = callback; }
    void setSettingsCallback(const SettingsCallback& callback) { _settingsCallback = callback; }
    void setExitCallback(const ExitCallback& callback) { _exitCallback = callback; }
    
    CREATE_FUNC(PauseMenu);
    
private:
    void onResumeClicked(cocos2d::Ref* sender);
    void onSettingsClicked(cocos2d::Ref* sender);
    void onExitClicked(cocos2d::Ref* sender);
    
    cocos2d::LayerColor* _backgroundLayer = nullptr;
    cocos2d::Menu* _menu = nullptr;
    bool _isVisible = false;
    
    ResumeCallback _resumeCallback;
    SettingsCallback _settingsCallback;
    ExitCallback _exitCallback;

    // 在 private 成员变量区域添加
    SettingsPanel* _settingsPanel = nullptr;
};

#endif // __PAUSE_MENU_H__
