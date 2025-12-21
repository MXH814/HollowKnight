#ifndef  _APP_DELEGATE_H_
#define  _APP_DELEGATE_H_

//AppDelegate 软件代理
#include "cocos2d.h"

/**
@brief    The cocos2d Application.  cocos2d的应用程序。

Private inheritance here hides part of interface from Director.  私有继承在这里对Director隐藏了部分接口。
*/
class  AppDelegate : private cocos2d::Application
{
public:
    AppDelegate();
    virtual ~AppDelegate();

    virtual void initGLContextAttrs();  //初始化initGLContextAttrs，不用管

    /**
    @brief    Implement Director and Scene init code here.  执行导演和场景初始化代码在这里。
    @return true    Initialize success, app continue.  初始化成功，应用程序继续执行。
    @return false   Initialize failed, app terminate.  初始化失败，应用程序终止运行。
    */
    virtual bool applicationDidFinishLaunching();  //初始化函数

    /**
    @brief  Called when the application moves to the background  当应用程序移动到后台时调用。
    @param  the pointer of the application  应用程序的指针
    */
    virtual void applicationDidEnterBackground();

    /**
    @brief  Called when the application reenters the foreground  当应用程序即将进入到后台时调用。
    @param  the pointer of the application  应用程序的指针
    */
    virtual void applicationWillEnterForeground();
};

#endif // _APP_DELEGATE_H_

