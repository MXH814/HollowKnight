/**
 * @file GameCommon.h
 * @brief 游戏通用定义头文件
 * @details 包含游戏中通用的类型定义、常量、异常类和工具模板
 * 
 * C++ 特性使用说明：
 * - STL 容器：std::vector, std::unordered_map
 * - 模板：AnimationBuilder, Clamp
 * - 异常处理：GameException
 * - C++11 特性：enum class, constexpr, nullptr, auto, override
 */

#ifndef GAME_COMMON_H_
#define GAME_COMMON_H_

#include "cocos2d.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <stdexcept>

// ============================================================================
// 命名空间定义
// ============================================================================

namespace hollow_knight {

// ============================================================================
// 游戏常量定义
// ============================================================================

namespace constants {

// 物理常量
constexpr float kDefaultGravity = 4000.0f;
constexpr float kDefaultMoveSpeed = 400.0f;
constexpr float kDefaultJumpForce = 1400.0f;
constexpr float kMinJumpForce = 600.0f;
constexpr float kDoubleJumpForce = 2000.0f;
constexpr float kDashSpeed = 1600.0f;
constexpr float kDashDuration = 0.25f;
constexpr float kDashCooldown = 0.5f;
constexpr float kWallSlideSpeed = 600.0f;
constexpr float kWallJumpForceX = 800.0f;
constexpr float kWallJumpForceY = 2000.0f;

// 战斗常量
constexpr int kDefaultMaxHP = 5;
constexpr int kDefaultMaxSoul = 6;
constexpr int kSpellCost = 2;
constexpr int kFocusCost = 2;
constexpr float kInvincibleDuration = 1.3f;
constexpr float kKnockbackSpeed = 400.0f;
constexpr float kKnockbackDuration = 0.24f;
constexpr float kHardLandThreshold = 1500.0f;

// 护符系统
constexpr int kMaxCharmNotches = 6;

// 动画帧率
constexpr float kDefaultFrameDelay = 0.1f;
constexpr float kFastFrameDelay = 0.03f;
constexpr float kSlowFrameDelay = 0.15f;

}  // namespace constants

// ============================================================================
// 异常类定义
// ============================================================================

/**
 * @class GameException
 * @brief 游戏异常基类
 * @details 用于处理游戏运行时的各种异常情况
 */
class GameException : public std::runtime_error {
 public:
  explicit GameException(const std::string& message)
      : std::runtime_error(message) {}
  
  GameException(const std::string& message, const std::string& file, int line)
      : std::runtime_error(message + " [" + file + ":" + std::to_string(line) + "]") {}
};

/**
 * @class ResourceLoadException
 * @brief 资源加载异常
 */
class ResourceLoadException : public GameException {
 public:
  explicit ResourceLoadException(const std::string& resource_path)
      : GameException("Failed to load resource: " + resource_path) {}
};

/**
 * @class InvalidStateException
 * @brief 无效状态异常
 */
class InvalidStateException : public GameException {
 public:
  explicit InvalidStateException(const std::string& state_info)
      : GameException("Invalid state: " + state_info) {}
};

// 异常宏定义
#define THROW_GAME_EXCEPTION(msg) \
    throw hollow_knight::GameException(msg, __FILE__, __LINE__)

#define THROW_IF_NULL(ptr, msg) \
    if ((ptr) == nullptr) { THROW_GAME_EXCEPTION(msg); }

// ============================================================================
// 工具模板函数
// ============================================================================

/**
 * @brief 数值限制模板函数
 * @tparam T 数值类型
 * @param value 要限制的值
 * @param min_val 最小值
 * @param max_val 最大值
 * @return 限制后的值
 */
template <typename T>
inline T Clamp(T value, T min_val, T max_val) {
  if (value < min_val) return min_val;
  if (value > max_val) return max_val;
  return value;
}

/**
 * @brief 线性插值模板函数
 * @tparam T 数值类型
 * @param start 起始值
 * @param end 结束值
 * @param t 插值参数 [0, 1]
 * @return 插值结果
 */
template <typename T>
inline T Lerp(T start, T end, float t) {
  return static_cast<T>(start + (end - start) * Clamp(t, 0.0f, 1.0f));
}

/**
 * @brief 安全类型转换包装
 * @tparam To 目标类型
 * @tparam From 源类型
 * @param ptr 源指针
 * @return 转换后的指针，失败返回 nullptr
 */
template <typename To, typename From>
inline To* SafeCast(From* ptr) {
  return dynamic_cast<To*>(ptr);
}

// ============================================================================
// 动画构建器模板类
// ============================================================================

/**
 * @class AnimationBuilder
 * @brief 动画构建器类（Builder 模式）
 * @details 使用链式调用简化动画创建过程
 * 
 * 使用示例:
 * @code
 * auto animation = AnimationBuilder()
 *     .SetPath("TheKnight/Idle/")
 *     .SetPrefix("Idle")
 *     .SetFrameRange(1, 9)
 *     .SetDelay(0.1f)
 *     .Build();
 * @endcode
 */
class AnimationBuilder {
 public:
  AnimationBuilder() 
      : start_frame_(1), end_frame_(1), delay_(constants::kDefaultFrameDelay) {}
  
  AnimationBuilder& SetPath(const std::string& path) {
    path_ = path;
    return *this;
  }
  
  AnimationBuilder& SetPrefix(const std::string& prefix) {
    prefix_ = prefix;
    return *this;
  }
  
  AnimationBuilder& SetFrameRange(int start, int end) {
    start_frame_ = start;
    end_frame_ = end;
    return *this;
  }
  
  AnimationBuilder& SetDelay(float delay) {
    delay_ = delay;
    return *this;
  }
  
  /**
   * @brief 构建动画对象
   * @return 动画指针，失败返回 nullptr
   */
  cocos2d::Animation* Build() const {
    cocos2d::Vector<cocos2d::SpriteFrame*> frames;
    auto* director = cocos2d::Director::getInstance();
    auto* texture_cache = director->getTextureCache();
    
    for (int i = start_frame_; i <= end_frame_; ++i) {
      std::string filename = path_ + prefix_ + std::to_string(i) + ".png";
      auto* texture = texture_cache->addImage(filename);
      
      if (texture != nullptr) {
        auto size = texture->getContentSize();
        auto* frame = cocos2d::SpriteFrame::createWithTexture(
            texture, cocos2d::Rect(0, 0, size.width, size.height));
        if (frame != nullptr) {
          frames.pushBack(frame);
        }
      }
    }
    
    if (frames.empty()) {
      return nullptr;
    }
    
    return cocos2d::Animation::createWithSpriteFrames(frames, delay_);
  }
  
 private:
  std::string path_;
  std::string prefix_;
  int start_frame_;
  int end_frame_;
  float delay_;
};

// ============================================================================
// 平台结构体定义
// ============================================================================

/**
 * @struct Platform
 * @brief 平台碰撞数据结构
 */
struct Platform {
  cocos2d::Rect rect;   ///< 平台碰撞矩形
  cocos2d::Node* node;  ///< 平台节点指针
  
  Platform() : node(nullptr) {}
  Platform(const cocos2d::Rect& r, cocos2d::Node* n) : rect(r), node(n) {}
};

// ============================================================================
// 类型别名定义
// ============================================================================

using PlatformList = std::vector<Platform>;
using AnimationMap = std::unordered_map<std::string, cocos2d::Animation*>;
using UpdateCallback = std::function<void(float)>;

}  // namespace hollow_knight

#endif  // GAME_COMMON_H_
