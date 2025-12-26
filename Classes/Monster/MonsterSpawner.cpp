#include "MonsterSpawner.h"
#include "CrawlidMonster.h"
#include "TiktikMonster.h"
#include "GruzzerMonster.h"
#include "VengeflyMonster.h"
#include "CorniferNPC.h"  // 【新增】包含 Cornifer NPC 头文件

USING_NS_CC;

int MonsterSpawner::spawnAllCrawlidsInNextScene(cocos2d::Node* parentNode)
{
    if (!parentNode) {
        CCLOGERROR("MonsterSpawner::spawnAllCrawlidsInNextScene - 父节点为空!");
        return 0;
    }
    
    CCLOG("========================================");
    CCLOG("=== MonsterSpawner: 开始生成小怪 ===");
    
    // ==================== Crawlid 生成配置 ====================
    struct CrawlidSpawnData {
        Vec2 position;
        float patrolRange;
        std::string name;
    };
    
    std::vector<CrawlidSpawnData> crawlidSpawns = {
        {Vec2(5024.3f, 5796.7f), 250.0f, "Crawlid_1"},
        {Vec2(2678.0f, 5744.5f), 200.0f, "Crawlid_2"},
        {Vec2(5290.0f, 3074.4f), 220.0f, "Crawlid_3"},
        {Vec2(8086.1f, 3131.9f), 120.0f, "Crawlid_4"},
        {Vec2(6957.9f, 5681.8f), 200.0f, "Crawlid_5"}
    };
    
    int successCount = 0;
    
    // 生成所有 Crawlid
    for (size_t i = 0; i < crawlidSpawns.size(); i++)
    {
        auto& spawnData = crawlidSpawns[i];
        auto crawlid = CrawlidMonster::createAndSpawn(
            parentNode, 
            spawnData.position, 
            spawnData.patrolRange
        );
        
        if (crawlid) {
            crawlid->setName(spawnData.name);
            successCount++;
            
            CCLOG("[成功] %s 生成成功 at (%.1f, %.1f), 巡逻范围: %.1f", 
                  spawnData.name.c_str(), 
                  spawnData.position.x, 
                  spawnData.position.y, 
                  spawnData.patrolRange);
        } else {
            CCLOG("[失败] %s 生成失败!", spawnData.name.c_str());
        }
    }
    
    // ==================== Tiktik 生成配置 ====================
    CCLOG(">>> 开始生成 Tiktik 怪物 <<<");
    
    struct TiktikSpawnData {
        Vec2 rockCenter;
        float rockHalfWidth;
        float rockHalfHeight;
        std::string name;
    };
    
    std::vector<TiktikSpawnData> tiktikSpawns = {
        {Vec2(1876.6f, 1022.6f), 297.5f / 2.0f, 67.9f / 2.0f, "Tiktik_1"},
        {Vec2(2979.3f, 1510.6f), 430.6f / 2.0f, 67.9f / 2.0f, "Tiktik_2"},
        {Vec2(2979.3f, 2118.8f), 435.9f / 2.0f, 78.3f / 2.0f, "Tiktik_3"},
        {Vec2(2439.0f, 2755.6f), 435.9f / 2.0f, 83.5f / 2.0f, "Tiktik_4"},
        {Vec2(1874.0f, 3618.2f), 292.3f / 2.0f, 75.7f / 2.0f, "Tiktik_5"},
        {Vec2(2441.7f, 4771.8f), 441.1f / 2.0f, 70.5f / 2.0f, "Tiktik_6"},
        {Vec2(5025.8f, 6087.3f), 230.2f / 2.0f, 154.0f / 2.0f, "Tiktik_7"},
        
        // 【新增】两个新的 Tiktik
        // Tiktik_8: Knight位置(6502.2, 3658.7)附近的岩石
        {Vec2(6534.1f, 3589.5f), 133.1f / 2.0f, 138.3f / 2.0f, "Tiktik_8"},
        
        // Tiktik_9: Knight位置(7136.0, 3590.8)附近的岩石
        {Vec2(7137.0f, 3521.6f), 133.1f / 2.0f, 138.3f / 2.0f, "Tiktik_9"}
    };
    
    // 生成所有 Tiktik
    for (size_t i = 0; i < tiktikSpawns.size(); i++)
    {
        auto& spawnData = tiktikSpawns[i];
        
        CCLOG("  生成 %s:", spawnData.name.c_str());
        CCLOG("    岩石中心: (%.1f, %.1f)", spawnData.rockCenter.x, spawnData.rockCenter.y);
        CCLOG("    半宽: %.1f, 半高: %.1f", spawnData.rockHalfWidth, spawnData.rockHalfHeight);
        
        auto tiktik = TiktikMonster::createAndSpawn(
            parentNode,
            spawnData.rockCenter,
            spawnData.rockHalfWidth,
            spawnData.rockHalfHeight
        );
        
        if (tiktik) {
            tiktik->setName(spawnData.name);
            successCount++;
            
            CCLOG("  [成功] %s 生成成功 at (%.1f, %.1f)", 
                  spawnData.name.c_str(), 
                  tiktik->getPosition().x, 
                  tiktik->getPosition().y);
        } else {
            CCLOG("  [失败] %s 生成失败!", spawnData.name.c_str());
        }
    }
    
    // ==================== Gruzzer 生成配置 ====================
    CCLOG(">>> 开始生成 Gruzzer 怪物 <<<");
    
    // 活动范围：左下(1476.4, 665.0) 右上(3749.8, 6509.1)
    Rect gruzzerRange(1476.4f, 665.0f, 3749.8f - 1476.4f, 6509.1f - 665.0f);
    
    struct GruzzerSpawnData {
        Vec2 startPos;
        float speed;
        std::string name;
    };
    
    // 在范围内均匀分散8个Gruzzer的初始位置
    std::vector<GruzzerSpawnData> gruzzerSpawns = {
        // 左上区域
        {Vec2(1800.0f, 5500.0f), 150.0f, "Gruzzer_1"},
        {Vec2(2200.0f, 6200.0f), 140.0f, "Gruzzer_2"},
        
        // 左中区域
        {Vec2(1700.0f, 3500.0f), 160.0f, "Gruzzer_3"},
        {Vec2(2400.0f, 4200.0f), 145.0f, "Gruzzer_4"},
        
        // 左下区域
        {Vec2(1600.0f, 1500.0f), 155.0f, "Gruzzer_5"},
        {Vec2(2800.0f, 2000.0f), 150.0f, "Gruzzer_6"},
        
        // 右侧区域
        {Vec2(3200.0f, 4800.0f), 165.0f, "Gruzzer_7"},
        {Vec2(3500.0f, 2500.0f), 140.0f, "Gruzzer_8"}
    };
    
    // 生成所有 Gruzzer
    for (size_t i = 0; i < gruzzerSpawns.size(); i++)
    {
        auto& spawnData = gruzzerSpawns[i];
        
        CCLOG("  生成 %s:", spawnData.name.c_str());
        CCLOG("    初始位置: (%.1f, %.1f)", spawnData.startPos.x, spawnData.startPos.y);
        CCLOG("    飞行速度: %.1f", spawnData.speed);
        CCLOG("    活动范围: 左下(%.1f, %.1f) 右上(%.1f, %.1f)", 
              gruzzerRange.getMinX(), gruzzerRange.getMinY(),
              gruzzerRange.getMaxX(), gruzzerRange.getMaxY());
        
        auto gruzzer = GruzzerMonster::createAndSpawn(
            parentNode,
            spawnData.startPos,
            gruzzerRange,
            spawnData.speed
        );
        
        if (gruzzer) {
            gruzzer->setName(spawnData.name);
            successCount++;
            
            CCLOG("  [成功] %s 生成成功 at (%.1f, %.1f)", 
                  spawnData.name.c_str(), 
                  gruzzer->getPosition().x, 
                  gruzzer->getPosition().y);
        } else {
            CCLOG("  [失败] %s 生成失败!", spawnData.name.c_str());
        }
    }
    
    // ==================== Vengefly 生成配置 ====================
    CCLOG(">>> 开始生成 Vengefly 怪物 <<<");
    
    struct VengeflySpawnData {
        Vec2 position;
        float patrolRadius;
        std::string name;
    };
    
    // Vengefly 初始位置，按照顺时针方向均匀分布
    std::vector<VengeflySpawnData> vengeflySpawns = {
        {Vec2(4268.0f, 6018.0f), 300.0f, "Vengefly_1"},
        {Vec2(6392.0f, 6746.3f), 320.0f, "Vengefly_2"},
        {Vec2(7334.0f, 4953.5f), 310.0f, "Vengefly_3"},
        {Vec2(5542.0f, 4298.1f), 330.0f, "Vengefly_4"},
        {Vec2(3800.0f, 4924.5f), 340.0f, "Vengefly_5"},
        
        // 【新增】两个新的 Vengefly
        // Vengefly_6: Knight位置(6502.2, 3658.7)附近
        {Vec2(6698.7f, 3652.4f), 310.0f, "Vengefly_6"},
        
        // Vengefly_7: Knight位置(7136.0, 3590.8)附近
        {Vec2(6890.3f, 3974.6f), 325.0f, "Vengefly_7"}
    };
    
    // 生成所有 Vengefly
    for (size_t i = 0; i < vengeflySpawns.size(); i++)
    {
        auto& spawnData = vengeflySpawns[i];
        
        CCLOG("  生成 %s:", spawnData.name.c_str());
        CCLOG("    位置: (%.1f, %.1f)", spawnData.position.x, spawnData.position.y);
        CCLOG("    巡逻半径: %.1f", spawnData.patrolRadius);
        
        auto vengefly = VengeflyMonster::createAndSpawn(
            parentNode,
            spawnData.position,
            spawnData.patrolRadius
        );
        
        if (vengefly) {
            vengefly->setName(spawnData.name);
            successCount++;
            
            CCLOG("  [成功] %s 生成成功 at (%.1f, %.1f)", 
                  spawnData.name.c_str(), 
                  vengefly->getPosition().x, 
                  vengefly->getPosition().y);
        } else {
            CCLOG("  [失败] %s 生成失败!", spawnData.name.c_str());
        }
    }
    
    // ==================== Cornifer NPC 生成配置 ====================
    CCLOG(">>> 开始生成 Cornifer NPC <<<");

    Vec2 corniferDisplayPos(8000.0f, 6161.0f);
    Vec2 corniferSpawnPoint = corniferDisplayPos + Vec2(0, 40);

    auto cornifer = CorniferNPC::create();
    if (cornifer)
    {
        cornifer->setName("NPC_Cornifer");
        cornifer->setSpawnPoint(corniferSpawnPoint);
        
        // 【修改】降低 Cornifer 的 zOrder，确保 Knight (zOrder=5) 显示在上层
        parentNode->addChild(cornifer, 3, "NPC_Cornifer");  // 从 10 改为 3
        successCount++;
        
        CCLOG("  [成功] NPC_Cornifer 生成成功");
        CCLOG("    地面高度: %.1f", corniferDisplayPos.y);
        CCLOG("    传入出生点: (%.1f, %.1f)", corniferSpawnPoint.x, corniferSpawnPoint.y);
        CCLOG("    实际位置: (%.1f, %.1f)", cornifer->getPositionX(), cornifer->getPositionY());
        CCLOG("    zOrder: 3 (Knight zOrder: 5，确保 Knight 在上层)");
    }
    else
    {
        CCLOG("  [失败] NPC_Cornifer 生成失败!");
    }
    
    CCLOG("=== MonsterSpawner: 完成生成小怪, 总计成功: %d 个 ===", successCount);
    
    return successCount;
}