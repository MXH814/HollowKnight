#ifndef __MONSTER_SPAWNER_H__
#define __MONSTER_SPAWNER_H__

#include "cocos2d.h"

// 前向声明（如果需要）
class CorniferNPC;

class MonsterSpawner
{
public:
    /**
     * 在 NextScene 中生成所有小怪和 NPC
     * @param parentNode 父节点（通常是 NextScene）
     * @return 成功生成的实体数量
     */
    static int spawnAllCrawlidsInNextScene(cocos2d::Node* parentNode);
    
private:
    MonsterSpawner() = delete;  // 禁止实例化
};

#endif // __MONSTER_SPAWNER_H__