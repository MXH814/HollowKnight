#pragma once

#ifndef __KNIGHT_STATE_MANAGER_H__
#define __KNIGHT_STATE_MANAGER_H__

#include "cocos2d.h"

/**
 * @class KnightStateManager
 * @brief 全局骑士状态管理器，用于在场景之间保存和恢复骑士的状态（血量、灵魂值等）
 */
class KnightStateManager
{
public:
    // 获取单例实例
    static KnightStateManager* getInstance();
    
    // 获取当前血量
    int getHP() const { return _hp; }
    
    // 设置血量
    void setHP(int value);
    
    // 获取当前灵魂值
    int getSoul() const { return _soul; }
    
    // 设置灵魂值
    void setSoul(int value);
    
    // 保存骑士状态
    void saveState(int hp, int soul);
    
    // 检查是否有保存的状态
    bool hasState() const { return _hasState; }
    
    // 清除保存的状态（用于死亡重生时恢复满血）
    void clearState();
    
    // 重置为初始状态
    void reset();
    
private:
    KnightStateManager();
    ~KnightStateManager() = default;
    
    static KnightStateManager* _instance;
    
    int _hp;           // 保存的血量
    int _soul;         // 保存的灵魂值
    bool _hasState;    // 是否有保存的状态
};

#endif // __KNIGHT_STATE_MANAGER_H__
