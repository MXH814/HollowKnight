#include "GeoManager.h"

USING_NS_CC;

GeoManager* GeoManager::_instance = nullptr;

GeoManager* GeoManager::getInstance()
{
    if (!_instance)
    {
        _instance = new GeoManager();
    }
    return _instance;
}

GeoManager::GeoManager()
    : _geo(0)  // ³õÊ¼ÖµÎª0
{
}

void GeoManager::setGeo(int value)
{
    if (value < 0) value = 0;
    _geo = value;
}

void GeoManager::addGeo(int amount)
{
    if (amount > 0)
    {
        _geo += amount;
        CCLOG("[GeoManager] Geo increased by %d, total: %d", amount, _geo);
    }
}

bool GeoManager::spendGeo(int amount)
{
    if (amount <= 0) return true;
    
    if (_geo >= amount)
    {
        _geo -= amount;
        CCLOG("[GeoManager] Geo spent %d, remaining: %d", amount, _geo);
        return true;
    }
    
    CCLOG("[GeoManager] Not enough Geo! Need %d, have %d", amount, _geo);
    return false;
}

void GeoManager::reset()
{
    _geo = 0;
    CCLOG("[GeoManager] Geo reset to 0");
}