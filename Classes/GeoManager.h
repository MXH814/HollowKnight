#pragma once

#ifndef __GEO_MANAGER_H__
#define __GEO_MANAGER_H__

#include "cocos2d.h"

// 全局货币（Geo）管理器
class GeoManager
{
public:
    // 获取单例实例
    static GeoManager* getInstance();
    
    // 获取当前 Geo 数量
    int getGeo() const { return _geo; }
    
    // 设置 Geo 数量
    void setGeo(int value);
    
    // 增加 Geo
    void addGeo(int amount);
    
    // 减少 Geo（返回是否成功，如果不够则返回false）
    bool spendGeo(int amount);
    
    // 重置 Geo 为初始值
    void reset();
    
private:
    GeoManager();
    ~GeoManager() = default;
    
    static GeoManager* _instance;
    int _geo;
};

#endif // __GEO_MANAGER_H__