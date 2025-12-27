/**
 * @file CharmManager.h
 * @brief 护符管理器类头文件
 * @details 实现护符系统的单例管理器，负责护符的装备、卸载和UI显示
 * 
 * C++ 特性使用说明：
 * - 单例模式：使用静态成员实现
 * - STL 容器：std::vector 存储护符和凹槽
 * - const 成员函数：所有 getter 方法
 * - 结构体：CharmInfo 存储护符数据
 * - 删除的函数：禁止拷贝构造和赋值（C++11）
 */

#ifndef __CHARM_MANAGER_H__
#define __CHARM_MANAGER_H__

#include "cocos2d.h"
#include <vector>
#include <string>

USING_NS_CC;

// 前向声明
class TheKnight;

/**
 * @struct CharmInfo
 * @brief 护符信息数据结构
 */
struct CharmInfo {
    std::string imagePath;       ///< 护符图片路径
    Vec2 originalPos;            ///< 原始位置
    Sprite* sprite;              ///< 护符精灵
    Sprite* darkSprite;          ///< 灰色占位精灵
    Sprite* selectFrame;         ///< 选中框
    int cost;                    ///< 凹槽消耗
    int equippedSlot;            ///< 装备槽位
    bool isEquipped;             ///< 是否已装备
    
    CharmInfo() 
        : sprite(nullptr)
        , darkSprite(nullptr)
        , selectFrame(nullptr)
        , cost(0)
        , equippedSlot(-1)
        , isEquipped(false) {}
};

/**
 * @class CharmManager
 * @brief 护符管理器（单例模式）
 * @details 管理所有护符的装备状态和UI显示
 * 
 * 设计说明：
 * - 使用单例模式确保全局唯一实例
 * - 禁止拷贝和赋值（C++11 delete）
 * - 所有 getter 使用 const 修饰
 */
class CharmManager {
public:
    // ========================================================================
    // 单例接口
    // ========================================================================
    
    /**
     * @brief 获取单例实例
     * @return 管理器指针
     */
    static CharmManager* getInstance();
    
    // ========================================================================
    // 护符状态接口（const getter）
    // ========================================================================
    
    int getCharmWaywardCompass() const { return _charmWaywardCompass; }
    int getCharmShamanStone() const { return _charmShamanStone; }
    int getCharmStalwartShell() const { return _charmStalwartShell; }
    int getCharmSteadyBody() const { return _charmSteadyBody; }
    int getCharmSoulCatcher() const { return _charmSoulCatcher; }
    int getCharmSprintMaster() const { return _charmSprintMaster; }
    
    // Setters
    void setCharmWaywardCompass(int value) { _charmWaywardCompass = value; }
    void setCharmShamanStone(int value) { _charmShamanStone = value; }
    void setCharmStalwartShell(int value) { _charmStalwartShell = value; }
    void setCharmSteadyBody(int value) { _charmSteadyBody = value; }
    void setCharmSoulCatcher(int value) { _charmSoulCatcher = value; }
    void setCharmSprintMaster(int value) { _charmSprintMaster = value; }
    
    // ========================================================================
    // 凹槽系统接口
    // ========================================================================
    
    /** @brief 获取最大凹槽数 */
    int getMaxNotches() const { return MAX_NOTCHES; }
    
    /** @brief 获取已使用凹槽数 */
    int getUsedNotches() const { return _usedNotches; }
    
    /** @brief 重新计算已使用凹槽数 */
    void recalculateUsedNotches();
    
    // ========================================================================
    // UI 接口
    // ========================================================================
    
    /**
     * @brief 显示护符面板
     * @param parentNode 父节点
     * @param canEquip 是否可以装卸护符
     */
    void showCharmPanel(Node* parentNode, bool canEquip = false);
    
    /** @brief 关闭护符面板 */
    void hideCharmPanel();
    
    /** @brief 护符面板是否打开 */
    bool isPanelOpen() const { return _isPanelOpen; }
    
    /** @brief 设置是否可以装卸护符 */
    void setCanEquip(bool canEquip) { _canEquip = canEquip; }
    
    /** @brief 是否可以装卸护符 */
    bool canEquip() const { return _canEquip; }
    
    /**
     * @brief 将护符状态同步到骑士
     * @param knight 骑士指针
     */
    void syncToKnight(TheKnight* knight);

private:
    // ========================================================================
    // 私有构造（单例模式）
    // ========================================================================
    
    CharmManager();
    ~CharmManager();
    
    // 禁止拷贝和赋值（C++11）
    CharmManager(const CharmManager&) = delete;
    CharmManager& operator=(const CharmManager&) = delete;
    
    // ========================================================================
    // UI 创建与更新
    // ========================================================================
    
    void createCharmUI(Node* layer, const Size& visibleSize);
    void updateNotchDisplay();
    void updateEquippedPositions();
    void updateInfoPanel(int charmIndex, bool show);
    void selectCharm(int charmIndex);
    void toggleEquipCharm();
    void updateSelectFrame();
    
    // ========================================================================
    // 输入处理
    // ========================================================================
    
    void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);

private:
    // ========================================================================
    // 静态成员
    // ========================================================================
    
    static CharmManager* _instance;
    static const int MAX_NOTCHES = 6;  ///< 最大凹槽数（编译时常量）
    
    // ========================================================================
    // 护符状态（0 = 未装备，1 = 已装备）
    // ========================================================================
    
    int _charmWaywardCompass;
    int _charmShamanStone;
    int _charmStalwartShell;
    int _charmSteadyBody;
    int _charmSoulCatcher;
    int _charmSprintMaster;
    
    int _usedNotches;  ///< 已使用凹槽数
    
    // ========================================================================
    // UI 相关
    // ========================================================================
    
    bool _isPanelOpen;
    Node* _panelLayer;
    Node* _parentNode;
    std::vector<Sprite*> _notchSprites;  ///< STL 容器
    std::vector<CharmInfo> _charms;      ///< STL 容器
    
    // 选择相关
    int _selectedCharmIndex;
    Sprite* _selectFrameSprite;
    
    bool _canEquip;
    Label* _hintLabel;
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

#endif // __CHARM_MANAGER_H__
