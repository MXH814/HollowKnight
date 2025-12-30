#pragma once

#ifndef __AUDIO_SETTINGS_H__
#define __AUDIO_SETTINGS_H__

// 全局音量设置
namespace AudioSettings
{
    // 全局音量变量 (0-10)
    extern int g_bgmVolume;
    extern int g_sfxVolume;
    
    // 获取音量 (0.0f - 1.0f)
    inline float getBGMVolume() { return g_bgmVolume / 10.0f; }
    inline float getSFXVolume() { return g_sfxVolume / 10.0f; }
    
    // 设置音量 (0-10)
    void setBGMVolume(int volume);
    void setSFXVolume(int volume);
    
    // 应用当前BGM音量到正在播放的背景音乐
    void applyBGMVolume();
}

#endif // __AUDIO_SETTINGS_H__
