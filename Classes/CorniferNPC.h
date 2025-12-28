#ifndef __CORNIFER_NPC_H__
#define __CORNIFER_NPC_H__

#include "cocos2d.h"
#include <vector>
#include <string>

class CorniferNPC : public cocos2d::Node {
public:
    static CorniferNPC* create();
    virtual bool init() override;
    virtual void update(float dt) override;
    virtual ~CorniferNPC();

    // 设置出生点接口
    void setSpawnPoint(cocos2d::Vec2 point);
    
    // 设置 Knight 位置（每帧调用）
    void setPlayerPosition(const cocos2d::Vec2& playerPos);
    
    // 获取当前是否处于对话状态（用于禁用Knight输入）
    bool isDialogueActive() const { return _isDialogueActive; }

private:
    void initAnimations();
    void checkPlayerDistance();
    void updateFacing();

    // ... (其余私有成员保持不变)
    // 对话系统相关方法
    void showDialogue();
    void closeDialogue();
    void advanceDialogue();
    void updateDialogueContent();
    void fadeToUpdateContent();
    void updateChoiceUI();
    void handleDialogueInput(cocos2d::EventKeyboard::KeyCode keyCode);
    void selectChoice(int choiceIndex);

    // 输入回调
    void onMouseMove(cocos2d::Event* event);
    void onMouseDown(cocos2d::Event* event);
    void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

    cocos2d::Sprite* _corniferSprite;
    cocos2d::Vec2 _playerPos;
    cocos2d::Vec2 _mousePos;

    enum class State {
        Writing,
        Turning,
        Talking
    };
    State _currentState;
    bool _isPlayerNearby;
    bool _isFacingRight;

    cocos2d::Animation* _writingAnim;
    cocos2d::Animation* _turnLeftAnim;
    cocos2d::Animation* _talkLeftAnim;
    cocos2d::Animation* _turnRightAnim;
    cocos2d::Animation* _talkRightAnim;

    cocos2d::Node* _dialogueWindow;
    cocos2d::Label* _dialogueLabel;
    cocos2d::Sprite* _borderTop;
    cocos2d::Sprite* _borderBottom;

    cocos2d::Node* _promptNode;

    cocos2d::Node* _choiceNode;
    cocos2d::Label* _yesLabel;
    cocos2d::Label* _noLabel;

    bool _hasPurchasedMap;
    bool _showPurchaseSuccess;
    int _dialogueIndex;
    bool _isChoiceActive;
    int _choiceSelection;

    bool _isDialogueActive;
    bool _isPromptShowing;

    const float DETECT_RANGE = 200.0f;
};

#endif // __CORNIFER_NPC_H__