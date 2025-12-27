/**
 * @file CppFeaturesSummary.md
 * @brief 项目 C++ 特性使用总结
 * @details 本项目使用的 C++ 特性清单及示例
 */

# 空洞骑士项目 C++ 特性使用总结

## 项目符合标准

### 代码格式统一
- 所有文件使用统一的缩进（2或4空格）
- 花括号风格一致
- 空行和空格使用规范

### 命名符合 Google C++ Style
- 类名：`PascalCase`（如 `TheKnight`, `CharmManager`）
- 函数名：`camelCase` 或 `PascalCase`（如 `getBoundingBox`, `UpdateSlash`）
- 成员变量：`_camelCase`（下划线前缀，如 `_hp`, `_moveSpeed`）
- 常量：`kConstantName`（如 `kMaxNotches`, `kDefaultGravity`）
- 枚举值：`UPPER_CASE` 或 `kPascalCase`

### C++ 风格类型转换
- `static_cast<>`: 用于安全的类型转换
- `dynamic_cast<>`: 用于多态类型转换

### 合理使用 const
- 所有 getter 方法使用 `const` 修饰
- 常量参数使用 `const` 引用传递
- 不修改对象的方法声明为 `const`

### 注释规范
- 使用 Doxygen 风格注释
- 每个文件、类、函数都有说明
- 重要变量有行内注释

---

## C++ 特性使用清单（≥3条）

### 1. STL 容器 ?

**文件**: `TheKnight.h`, `CharmManager.cpp`, `LoadingScene.cpp`

```cpp
// 使用 std::vector 存储平台列表
std::vector<Platform> _platforms;

// 使用 std::vector 存储护符信息
std::vector<CharmInfo> _charms;
std::vector<Sprite*> _notchSprites;

// 使用 std::vector 存储音效路径
const std::vector<std::string> kEffectList = {
    "Music/click.wav",
    "Music/hero_running.wav",
    // ...
};
```

### 2. 迭代器 ?

**文件**: `CharmManager.cpp`, `LoadingScene.cpp`

```cpp
// 使用迭代器遍历护符配置
for (auto it = charmConfigs.begin(); it != charmConfigs.end(); ++it, ++index) {
    CharmInfo info;
    info.imagePath = it->imagePath;
    // ...
}

// 使用 cbegin/cend 遍历纹理列表
for (auto it = kTextureList.cbegin(); it != kTextureList.cend(); ++it) {
    texture_cache->addImage(*it);
}
```

### 3. 类与多态 ?

**文件**: `TheKnight.h`, `Enemy.h`

```cpp
// 继承自 cocos2d::Sprite，重写虚函数
class TheKnight : public Sprite {
public:
    virtual bool init() override;  // 重写虚函数
    void update(float dt) override;  // 重写更新函数
    // ...
};

// 敌人基类，可被派生类继承
class Enemy : public Node {
public:
    virtual bool init() override;
    void update(float dt) override;
    // ...
};
```

### 4. 模板 ?

**文件**: `GameCommon.h`

```cpp
// 数值限制模板函数
template <typename T>
inline T Clamp(T value, T min_val, T max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

// 线性插值模板函数
template <typename T>
inline T Lerp(T start, T end, float t) {
    return static_cast<T>(start + (end - start) * Clamp(t, 0.0f, 1.0f));
}

// 安全类型转换模板
template <typename To, typename From>
inline To* SafeCast(From* ptr) {
    return dynamic_cast<To*>(ptr);
}
```

### 5. 异常处理 ?

**文件**: `GameCommon.h`

```cpp
// 游戏异常基类
class GameException : public std::runtime_error {
public:
    explicit GameException(const std::string& message)
        : std::runtime_error(message) {}
    
    GameException(const std::string& message, const std::string& file, int line)
        : std::runtime_error(message + " [" + file + ":" + std::to_string(line) + "]") {}
};

// 资源加载异常
class ResourceLoadException : public GameException {
public:
    explicit ResourceLoadException(const std::string& resource_path)
        : GameException("Failed to load resource: " + resource_path) {}
};

// 异常宏定义
#define THROW_GAME_EXCEPTION(msg) \
    throw hollow_knight::GameException(msg, __FILE__, __LINE__)
```

### 6. 函数 & 操作符重载 ?

**文件**: `GameCommon.h`, `CharmManager.h`

```cpp
// 构造函数重载
GameException(const std::string& message);
GameException(const std::string& message, const std::string& file, int line);

// Platform 结构体构造函数重载
struct Platform {
    Platform() : node(nullptr) {}
    Platform(const Rect& r, Node* n) : rect(r), node(n) {}
};

// 删除的操作符（禁止赋值）
CharmManager& operator=(const CharmManager&) = delete;
```

### 7. C++11 或以上功能 ?

**文件**: 多个文件

```cpp
// enum class（强类型枚举）
enum class KnightState {
    IDLE,
    RUNNING,
    // ...
};

// override 关键字
bool init() override;
void update(float dt) override;

// nullptr（空指针字面量）
if (_instance == nullptr) {
    _instance = new CharmManager();
}

// auto 关键字（类型推导）
auto visible_size = Director::getInstance()->getVisibleSize();
auto* texture_cache = Director::getInstance()->getTextureCache();

// 范围 for 循环
for (const auto& music_path : kBackgroundMusicList) {
    audio->preloadBackgroundMusic(music_path.c_str());
}

// Lambda 表达式
charmBg->runAction(
    Sequence::create(
        DelayTime::create(2.0f),
        CallFunc::create([this, layer, visibleSize]() {
            this->createCharmUI(layer, visibleSize);
        }),
        nullptr
    )
);

// = delete（删除的函数）
CharmManager(const CharmManager&) = delete;
CharmManager& operator=(const CharmManager&) = delete;

// constexpr（编译时常量）
constexpr int kCharmCount = 6;
constexpr float kDefaultGravity = 4000.0f;

// 初始化列表
CharmManager::CharmManager()
    : _charmWaywardCompass(0)
    , _charmShamanStone(0)
    // ...

// std::nothrow（异常安全内存分配）
Enemy* enemy = new (std::nothrow) Enemy();
```

---

## 文件变更清单

| 文件 | 变更内容 |
|------|----------|
| `GameCommon.h` | 新增：通用定义、模板、异常类 |
| `TheKnight.h` | 重构：添加注释、代码组织 |
| `Enemy.h` | 重构：添加注释、代码组织 |
| `Enemy.cpp` | 重构：代码风格统一 |
| `CharmManager.h` | 重构：添加注释、= delete |
| `CharmManager.cpp` | 重构：static_cast、迭代器、范围for |
| `LoadingScene.h` | 重构：添加注释 |
| `LoadingScene.cpp` | 重构：STL容器、迭代器、范围for |

---

## 编译验证

所有修改均已通过编译验证，项目可正常构建运行。
