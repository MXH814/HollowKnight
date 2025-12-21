#include "main.h"
#include "AppDelegate.h" 
#include "cocos2d.h"  //cocos2d标准头文件

USING_NS_CC; //宏定义，using namespace cocos2d;

int WINAPI _tWinMain(HINSTANCE hInstance,        //Win32入口函数不是main，而是winmain，图形窗口使用
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);  //宏定义，不用管，不用动

    // create the application instance  创建一个应用程序实例
    AppDelegate app;  //类变量 AppDelegate私有继承自Application
    return Application::getInstance()->run();  //程序启动 等价于return ((Application*)&app)->run();
}
