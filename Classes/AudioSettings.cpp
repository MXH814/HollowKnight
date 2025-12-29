#include "AudioSettings.h"
#include "SimpleAudioEngine.h"

using namespace CocosDenshion;

namespace AudioSettings
{
    // Ä¬ÈÏÒôÁ¿Îª10£¨100%£©
    int g_bgmVolume = 10;
    int g_sfxVolume = 10;
    
    void setBGMVolume(int volume)
    {
        if (volume < 0) volume = 0;
        if (volume > 10) volume = 10;
        g_bgmVolume = volume;
    }
    
    void setSFXVolume(int volume)
    {
        if (volume < 0) volume = 0;
        if (volume > 10) volume = 10;
        g_sfxVolume = volume;
    }
    
    void applyBGMVolume()
    {
        SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(getBGMVolume());
    }
}