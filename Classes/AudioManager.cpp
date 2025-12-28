/**
 * @file AudioManager.cpp
 * @brief 音频管理器实现
 */

#include "AudioManager.h"

USING_NS_CC;
using namespace cocos2d::experimental;

// 单例实例
AudioManager* AudioManager::_instance = nullptr;

// 音频文件路径定义
const std::string AudioManager::BGM_MAIN_MENU = "Audio/MainMenuScene.mp3";
const std::string AudioManager::BGM_GAME_SCENE = "Audio/GameScene.mp3";
const std::string AudioManager::BGM_NEXT_SCENE = "Audio/NextScene.mp3";
const std::string AudioManager::BGM_BOSS_SCENE = "Audio/BossScene.mp3";

const std::string AudioManager::SFX_CLICK = "Audio/Click.mp3";
const std::string AudioManager::SFX_JUMP = "Audio/Jump.mp3";
const std::string AudioManager::SFX_LAND = "Audio/Land.mp3";
const std::string AudioManager::SFX_HARD_LAND = "Audio/HardLand.mp3";
const std::string AudioManager::SFX_RUN = "Audio/Run.mp3";
const std::string AudioManager::SFX_DASH = "Audio/Dash.mp3";
const std::string AudioManager::SFX_SLASH = "Audio/Slash.mp3";
const std::string AudioManager::SFX_GET_ATTACKED = "Audio/GetAttacked.mp3";
const std::string AudioManager::SFX_DEATH = "Audio/Death.mp3";
const std::string AudioManager::SFX_VENGEFUL_SPIRIT = "Audio/VengefulSpirit.mp3";
const std::string AudioManager::SFX_VENGEFUL_SPIRIT_DISAPPEAR = "Audio/VengefulSpiritDisapear.mp3";

const std::string AudioManager::SFX_ENEMY_TAKE_DAMAGE = "Audio/EnemyTakeDamage.mp3";
const std::string AudioManager::SFX_ENEMY_DEATH = "Audio/EnemyDeath.mp3";

const std::string AudioManager::SFX_HORNET_ATTACK1 = "Audio/HornetAttack1.mp3";
const std::string AudioManager::SFX_HORNET_ATTACK2 = "Audio/HornetAttack2.mp3";
const std::string AudioManager::SFX_HORNET_ATTACK3 = "Audio/HornetAttack3.mp3";
const std::string AudioManager::SFX_HORNET_ATTACK4 = "Audio/HornetAttack4.mp3";

AudioManager::AudioManager()
    : _currentBGMId(AudioEngine::INVALID_AUDIO_ID)
    , _bgmVolume(1.0f)
    , _effectsVolume(1.0f)
    , _isPreloaded(false)
{
}

AudioManager::~AudioManager()
{
}

AudioManager* AudioManager::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new AudioManager();
    }
    return _instance;
}

void AudioManager::preloadAllAudio()
{
    if (_isPreloaded) return;
    
    CCLOG("AudioManager: 开始预加载所有音频资源...");
    
    // 预加载背景音乐
    AudioEngine::preload(BGM_MAIN_MENU);
    AudioEngine::preload(BGM_GAME_SCENE);
    AudioEngine::preload(BGM_NEXT_SCENE);
    AudioEngine::preload(BGM_BOSS_SCENE);
    
    // 预加载UI音效
    AudioEngine::preload(SFX_CLICK);
    
    // 预加载角色音效
    AudioEngine::preload(SFX_JUMP);
    AudioEngine::preload(SFX_LAND);
    AudioEngine::preload(SFX_HARD_LAND);
    AudioEngine::preload(SFX_RUN);
    AudioEngine::preload(SFX_DASH);
    AudioEngine::preload(SFX_SLASH);
    AudioEngine::preload(SFX_GET_ATTACKED);
    AudioEngine::preload(SFX_DEATH);
    AudioEngine::preload(SFX_VENGEFUL_SPIRIT);
    AudioEngine::preload(SFX_VENGEFUL_SPIRIT_DISAPPEAR);
    
    // 预加载小怪音效
    AudioEngine::preload(SFX_ENEMY_TAKE_DAMAGE);
    AudioEngine::preload(SFX_ENEMY_DEATH);
    
    // 预加载Hornet Boss音效
    AudioEngine::preload(SFX_HORNET_ATTACK1);
    AudioEngine::preload(SFX_HORNET_ATTACK2);
    AudioEngine::preload(SFX_HORNET_ATTACK3);
    AudioEngine::preload(SFX_HORNET_ATTACK4);
    
    _isPreloaded = true;
    
    CCLOG("AudioManager: 音频资源预加载完成!");
}

// === 背景音乐控制 ===

void AudioManager::playMainMenuBGM()
{
    stopBGM();
    _currentBGMId = AudioEngine::play2d(BGM_MAIN_MENU, true, _bgmVolume);
    CCLOG("AudioManager: 播放主菜单背景音乐");
}

void AudioManager::playGameSceneBGM()
{
    stopBGM();
    _currentBGMId = AudioEngine::play2d(BGM_GAME_SCENE, true, _bgmVolume);
    CCLOG("AudioManager: 播放 GameScene 背景音乐");
}

void AudioManager::playNextSceneBGM()
{
    stopBGM();
    _currentBGMId = AudioEngine::play2d(BGM_NEXT_SCENE, true, _bgmVolume);
    CCLOG("AudioManager: 播放 NextScene 背景音乐");
}

void AudioManager::playBossSceneBGM()
{
    stopBGM();
    _currentBGMId = AudioEngine::play2d(BGM_BOSS_SCENE, true, _bgmVolume);
    CCLOG("AudioManager: 播放 BossScene 背景音乐");
}

void AudioManager::stopBGM()
{
    if (_currentBGMId != AudioEngine::INVALID_AUDIO_ID)
    {
        AudioEngine::stop(_currentBGMId);
        _currentBGMId = AudioEngine::INVALID_AUDIO_ID;
    }
}

void AudioManager::pauseBGM()
{
    if (_currentBGMId != AudioEngine::INVALID_AUDIO_ID)
    {
        AudioEngine::pause(_currentBGMId);
    }
}

void AudioManager::resumeBGM()
{
    if (_currentBGMId != AudioEngine::INVALID_AUDIO_ID)
    {
        AudioEngine::resume(_currentBGMId);
    }
}

// === 角色动作音效 ===

void AudioManager::playJumpSound()
{
    AudioEngine::play2d(SFX_JUMP, false, _effectsVolume);
}

void AudioManager::playLandSound()
{
    AudioEngine::play2d(SFX_LAND, false, _effectsVolume);
}

void AudioManager::playHardLandSound()
{
    AudioEngine::play2d(SFX_HARD_LAND, false, _effectsVolume);
}

void AudioManager::playRunSound()
{
    AudioEngine::play2d(SFX_RUN, false, _effectsVolume);
}

void AudioManager::playDashSound()
{
    AudioEngine::play2d(SFX_DASH, false, _effectsVolume);
}

void AudioManager::playSlashSound()
{
    AudioEngine::play2d(SFX_SLASH, false, _effectsVolume);
}

void AudioManager::playGetAttackedSound()
{
    AudioEngine::play2d(SFX_GET_ATTACKED, false, _effectsVolume);
}

void AudioManager::playDeathSound()
{
    AudioEngine::play2d(SFX_DEATH, false, _effectsVolume);
}

// === 法术音效 ===

void AudioManager::playVengefulSpiritSound()
{
    AudioEngine::play2d(SFX_VENGEFUL_SPIRIT, false, _effectsVolume);
}

void AudioManager::playVengefulSpiritDisappearSound()
{
    AudioEngine::play2d(SFX_VENGEFUL_SPIRIT_DISAPPEAR, false, _effectsVolume);
}

// === 小怪音效 ===

void AudioManager::playEnemyTakeDamageSound()
{
    AudioEngine::play2d(SFX_ENEMY_TAKE_DAMAGE, false, _effectsVolume);
}

void AudioManager::playEnemyDeathSound()
{
    AudioEngine::play2d(SFX_ENEMY_DEATH, false, _effectsVolume);
}

// === Hornet Boss 音效 ===

void AudioManager::playHornetAttack1Sound()
{
    AudioEngine::play2d(SFX_HORNET_ATTACK1, false, _effectsVolume);
}

void AudioManager::playHornetAttack2Sound()
{
    AudioEngine::play2d(SFX_HORNET_ATTACK2, false, _effectsVolume);
}

void AudioManager::playHornetAttack3Sound()
{
    AudioEngine::play2d(SFX_HORNET_ATTACK3, false, _effectsVolume);
}

void AudioManager::playHornetAttack4Sound()
{
    AudioEngine::play2d(SFX_HORNET_ATTACK4, false, _effectsVolume);
}

// === UI 音效 ===

void AudioManager::playClickSound()
{
    AudioEngine::play2d(SFX_CLICK, false, _effectsVolume);
}

// === 音量控制 ===

void AudioManager::setBGMVolume(float volume)
{
    _bgmVolume = volume;
    if (_currentBGMId != AudioEngine::INVALID_AUDIO_ID)
    {
        AudioEngine::setVolume(_currentBGMId, volume);
    }
}

void AudioManager::setEffectsVolume(float volume)
{
    _effectsVolume = volume;
}
