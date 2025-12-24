#ifndef __NEXT_SCENE_H__
#define __NEXT_SCENE_H__

#include "cocos2d.h"
#include "TheKnight.h"

class NextScene : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    virtual void update(float dt) override;
    
    CREATE_FUNC(NextScene);

private:
    void createCollisionFromTMX(cocos2d::TMXTiledMap* map, 
                                 const std::string& layerName, 
                                 float scale, 
                                 const cocos2d::Vec2& mapOffset);
    
    // 创建陷阱背景
    void createTrapSprites(cocos2d::TMXTiledMap* map,
                           const std::string& layerName,
                           const std::string& trapType,
                           const std::string& spritePath,
                           float scale,
                           const cocos2d::Vec2& mapOffset);
    
    std::vector<Platform> _platforms;  // 碰撞平台列表
    bool _hasLandedOnce = false;       // 是否已经落地
};

#endif // __NEXT_SCENE_H__
