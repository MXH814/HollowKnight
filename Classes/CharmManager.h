#pragma once

#include "cocos2d.h"

USING_NS_CC;

class TheKnight;

// 护符信息结构
struct CharmInfo {
    std::string imagePath;
    Vec2 originalPos;
    Sprite* sprite;
    Sprite* darkSprite;
    Sprite* selectFrame;  // 选中框
    int cost;
    int equippedSlot;
    bool isEquipped;
};

class CharmManager
{
public:
    // 单例模式
    static CharmManager* getInstance();
    
    // 护符状态获取
    int getCharmWaywardCompass() const { return _charmWaywardCompass; }
    int getCharmShamanStone() const { return _charmShamanStone; }
    int getCharmStalwartShell() const { return _charmStalwartShell; }
    int getCharmSteadyBody() const { return _charmSteadyBody; }
    int getCharmSoulCatcher() const { return _charmSoulCatcher; }
    int getCharmSprintMaster() const { return _charmSprintMaster; }
    
    // 护符状态设置
    void setCharmWaywardCompass(int value) { _charmWaywardCompass = value; }
    void setCharmShamanStone(int value) { _charmShamanStone = value; }
    void setCharmStalwartShell(int value) { _charmStalwartShell = value; }
    void setCharmSteadyBody(int value) { _charmSteadyBody = value; }
    void setCharmSoulCatcher(int value) { _charmSoulCatcher = value; }
    void setCharmSprintMaster(int value) { _charmSprintMaster = value; }
    
    // 凹槽系统
    int getMaxNotches() const { return MAX_NOTCHES; }
    int getUsedNotches() const { return _usedNotches; }
    
    // 重新计算已使用凹槽数
    void recalculateUsedNotches();
    
    // 显示护符面板 (在指定场景上)
    void showCharmPanel(Node* parentNode, bool canEquip = false);
    
    // 关闭护符面板
    void hideCharmPanel();
    
    // 护符面板是否打开
    bool isPanelOpen() const { return _isPanelOpen; }
    
    // 设置是否可以装卸护符
    void setCanEquip(bool canEquip) { _canEquip = canEquip; }
    bool canEquip() const { return _canEquip; }
    
    // 将护符状态同步到 TheKnight
    void syncToKnight(TheKnight* knight);
    
private:
    CharmManager();
    ~CharmManager();
    
    // 创建护符UI界面
    void createCharmUI(Node* layer, const Size& visibleSize);
    
    // 更新凹槽显示
    void updateNotchDisplay();
    
    // 更新装备位置
    void updateEquippedPositions();
    
    // 更新信息面板
    void updateInfoPanel(int charmIndex, bool show);
    
    // 护符选择处理
    void selectCharm(int charmIndex);
    
    // 护符装备/卸载处理
    void toggleEquipCharm();
    
    // 更新选中框显示
    void updateSelectFrame();
    
    // 键盘事件处理
    void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
    
private:
    static CharmManager* _instance;
    
    // 护符状态 (0 = 未装备, 1 = 已装备)
    int _charmWaywardCompass;
    int _charmShamanStone;
    int _charmStalwartShell;
    int _charmSteadyBody;
    int _charmSoulCatcher;
    int _charmSprintMaster;
    
    // 凹槽系统
    static const int MAX_NOTCHES = 6;
    int _usedNotches;
    
    // UI 相关
    bool _isPanelOpen;
    Node* _panelLayer;
    Node* _parentNode;
    std::vector<Sprite*> _notchSprites;
    std::vector<CharmInfo> _charms;
    
    // 选择相关
    int _selectedCharmIndex;  // 当前选中的护符索引
    Sprite* _selectFrameSprite;  // 选中框精灵
    
    // 是否可以装卸护符（只有坐在椅子上才能装卸）
    bool _canEquip;
    
    // 提示文字
    Label* _hintLabel;
    
    // 键盘监听器
    EventListenerKeyboard* _keyboardListener;
    
    // 信息面板元素
    Sprite* _infoCharmIcon;
    Label* _infoCharmName;
    Label* _infoCharmDesc;
    Label* _infoCharmCost;
    Node* _costContainer;
    
    // 装备位置参数
    float _equippedStartX;
    float _equippedY;
    float _equippedGap;
};
