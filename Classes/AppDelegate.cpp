#include "AppDelegate.h"
#include "LoadingScene.h"

#define USE_AUDIO_ENGINE 1  //音频引擎，使用时解开
// #define USE_SIMPLE_AUDIO_ENGINE 1  //简单音频引擎，使用时解开

#if USE_AUDIO_ENGINE && USE_SIMPLE_AUDIO_ENGINE
#error "Don't use AudioEngine and SimpleAudioEngine at the same time. Please just select one in your game!"  //两个音频引擎不能同时开启，否则报错
#endif

#if USE_AUDIO_ENGINE
#include "audio/include/AudioEngine.h"
using namespace cocos2d::experimental;
#elif USE_SIMPLE_AUDIO_ENGINE
#include "audio/include/SimpleAudioEngine.h"
using namespace CocosDenshion;
#endif

USING_NS_CC;

//窗口尺寸设置
static cocos2d::Size designResolutionSize = cocos2d::Size(1920, 1080);
static cocos2d::Size smallResolutionSize = cocos2d::Size(480, 320);
static cocos2d::Size mediumResolutionSize = cocos2d::Size(1024, 768);
static cocos2d::Size largeResolutionSize = cocos2d::Size(2048, 1536);

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate()   //析构函数，应用程序结束时销毁操作
{
#if USE_AUDIO_ENGINE
    AudioEngine::end();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::end();
#endif
}

// if you want a different context, modify the value of glContextAttrs  如果你想要一个不同的上下文，修改glContextAttrs的值
// it will affect all platforms  它将影响所有平台
void AppDelegate::initGLContextAttrs()
{
    // set OpenGL context attributes: red,green,blue,alpha,depth,stencil,multisamplesCount  //设置颜色等属性，不要动
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};

    GLView::setGLContextAttrs(glContextAttrs);
}

// if you want to use the package manager to install more packages,
// don't modify or remove this function  如果你想使用包管理器修改更多的包，请不要修改或删除此功能（不要动）（开发安卓平台功能）
static int register_all_packages()
{
    return 0; //flag for packages manager
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director  初始化导演
    auto director = Director::getInstance();  //单例模式，全局只有一各对象
    auto glview = director->getOpenGLView();  //获取视图
    if(!glview) {                             //如果没有，创建一个
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        glview = GLViewImpl::createWithRect("HollowKnight", cocos2d::Rect(0, 0, designResolutionSize.width, designResolutionSize.height));  //如果是上述平台，创建一个矩形窗口(x,y,width,height)
#else
        glview = GLViewImpl::create("HollowKnight");  //如果是手机端，直接创建
#endif
        director->setOpenGLView(glview);  //设置视图
    }

    // turn on display FPS  开启显示帧率
    director->setDisplayStats(false);   //true显示，false不显示

    // set FPS. the default value is 1.0/60 if you don't call this  设置帧率，默认60帧，不要低于60
    director->setAnimationInterval(1.0f / 60);

    // Set the design resolution  设置分辨率
    glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::NO_BORDER);
    
#if 0    
    //对窗口分辨率的大小调配（自动适配，建议不用）
    auto frameSize = glview->getFrameSize();
    // if the frame's height is larger than the height of medium size.
    if (frameSize.height > mediumResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(largeResolutionSize.height/designResolutionSize.height, largeResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is larger than the height of small size.
    else if (frameSize.height > smallResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(mediumResolutionSize.height/designResolutionSize.height, mediumResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is smaller than the height of medium size.
    else
    {        
        director->setContentScaleFactor(MIN(smallResolutionSize.height/designResolutionSize.height, smallResolutionSize.width/designResolutionSize.width));
    }
#endif

    register_all_packages();  //注册所有的包，安卓用，不要动

    // create a scene. it's an autorelease object   创建一个场景。这是一个自动释放对象。
    auto scene = LoadingScene::createScene();

    // run  调用导演来运行场景，控制游戏中不同的场景切换
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::pauseAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
    SimpleAudioEngine::getInstance()->pauseAllEffects();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::resumeAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
    SimpleAudioEngine::getInstance()->resumeAllEffects();
#endif
}
