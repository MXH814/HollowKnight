/**
 * @file AudioManager.h
 * @brief 音频管理器 - 统一管理游戏中所有音频的预加载和播放
 */

#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "cocos2d.h"
#include "audio/include/AudioEngine.h"

class AudioManager
{
public:
    // 单例模式
    static AudioManager* getInstance();
    
    // 预加载所有音频资源
    void preloadAllAudio();
    
    // === 背景音乐控制 ===
    void playMainMenuBGM();       // 主菜单背景音乐
    void playGameSceneBGM();      // GameScene 背景音乐
    void playNextSceneBGM();      // NextScene 背景音乐
    void playBossSceneBGM();      // BossScene 背景音乐
    void stopBGM();               // 停止当前背景音乐
    void pauseBGM();              // 暂停背景音乐
    void resumeBGM();             // 恢复背景音乐
    
    // === 角色动作音效 ===
    void playJumpSound();         // 跳跃音效
    void playLandSound();         // 落地音效
    void playHardLandSound();     // 重落地音效
    void playRunSound();          // 跑步音效
    void playDashSound();         // 冲刺音效
    void playSlashSound();        // 普通攻击音效
    void playGetAttackedSound();  // 受击音效
    void playDeathSound();        // 死亡音效
    
    // === 法术音效 ===
    void playVengefulSpiritSound();         // 复仇之魂音效
    void playVengefulSpiritDisappearSound(); // 复仇之魂消失音效
    
    // === 小怪音效 ===
    void playEnemyTakeDamageSound();   // 小怪受击音效
    void playEnemyDeathSound();        // 小怪死亡音效
    
    // === Hornet Boss 音效 ===
    void playHornetAttack1Sound();  // Hornet 攻击1
    void playHornetAttack2Sound();  // Hornet 攻击2
    void playHornetAttack3Sound();  // Hornet 攻击3
    void playHornetAttack4Sound();  // Hornet 攻击4
    
    // === UI 音效 ===
    void playClickSound();          // 点击音效
    
    // 设置音量
    void setBGMVolume(float volume);
    void setEffectsVolume(float volume);
    
private:
    AudioManager();
    ~AudioManager();
    
    static AudioManager* _instance;
    
    int _currentBGMId;          // 当前背景音乐ID
    float _bgmVolume;           // 背景音乐音量
    float _effectsVolume;       // 音效音量
    bool _isPreloaded;          // 是否已预加载
    
    // 音频文件路径常量
    static const std::string BGM_MAIN_MENU;
    static const std::string BGM_GAME_SCENE;
    static const std::string BGM_NEXT_SCENE;
    static const std::string BGM_BOSS_SCENE;
    
    static const std::string SFX_CLICK;
    static const std::string SFX_JUMP;
    static const std::string SFX_LAND;
    static const std::string SFX_HARD_LAND;
    static const std::string SFX_RUN;
    static const std::string SFX_DASH;
    static const std::string SFX_SLASH;
    static const std::string SFX_GET_ATTACKED;
    static const std::string SFX_DEATH;
    static const std::string SFX_VENGEFUL_SPIRIT;
    static const std::string SFX_VENGEFUL_SPIRIT_DISAPPEAR;
    
    static const std::string SFX_ENEMY_TAKE_DAMAGE;
    static const std::string SFX_ENEMY_DEATH;
    
    static const std::string SFX_HORNET_ATTACK1;
    static const std::string SFX_HORNET_ATTACK2;
    static const std::string SFX_HORNET_ATTACK3;
    static const std::string SFX_HORNET_ATTACK4;
};

#endif // __AUDIO_MANAGER_H__
