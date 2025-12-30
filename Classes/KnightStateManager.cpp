#include "KnightStateManager.h"

KnightStateManager* KnightStateManager::_instance = nullptr;

KnightStateManager* KnightStateManager::getInstance()
{
    if (!_instance)
    {
        _instance = new KnightStateManager();
    }
    return _instance;
}

KnightStateManager::KnightStateManager()
    : _hp(5)
    , _soul(0)
    , _hasState(false)
{
}

void KnightStateManager::setHP(int value)
{
    _hp = value;
    _hasState = true;
}

void KnightStateManager::setSoul(int value)
{
    _soul = value;
    _hasState = true;
}

void KnightStateManager::saveState(int hp, int soul)
{
    _hp = hp;
    _soul = soul;
    _hasState = true;
    CCLOG("KnightStateManager: 保存状态 HP=%d, Soul=%d", _hp, _soul);
}

void KnightStateManager::clearState()
{
    _hasState = false;
    CCLOG("KnightStateManager: 清除保存的状态");
}

void KnightStateManager::reset()
{
    _hp = 5;
    _soul = 0;
    _hasState = false;
    CCLOG("KnightStateManager: 重置为初始状态");
}