#ifndef __FIGHTING_SCENE_H__
#define __FIGHTING_SCENE_H__

#include "cocos2d.h"
#include "Card.h"
#include "Hero.h"
#include "Monster.h"

class FightingScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    // implement the "static create()" method manually
    CREATE_FUNC(FightingScene);

private:
    void createBackground();
    void createCharacters();
    void updateHealthAndBlockLabels();
    void startPlayerTurn();
    void startMonsterTurn();
    void endTurn();
    void checkBattleEnd();
    void drawCard(); // ��һ����
    void discardCard(int index); // ��һ����
    void shuffleDrawPile(); // ϴ��
    void initializeDrawPile(); // ��ʼ���ƶ�
    void createHealthLabels();
    void createBlockLabels();
    void updateHandDisplay();
    void addCardEffectLabel(cocos2d::Sprite* cardSprite, const std::string& effect);
    void playCard(int index);
    void highlightSelectedCard();

    int _selectedCardIndex = -1; // -1 ��ʾû��ѡ�еĿ���
    std::vector<Card> _drawPile; // ���ƶ�
    std::vector<Card> _discardPile; // ���ƶ�
    bool _isPlayerTurn; // �Ƿ�����һغ�
    int _turnCount; // �غ���
    std::vector<Card> _cards; // ����
    cocos2d::Sprite* _selectedCard; // ѡ�еĿ���
    Hero* _hero; // Ӣ�۶���
    Monster* _monster; // �������
    cocos2d::Label* _heroHealthLabel; // ��ʾӢ��Ѫ���ı�ǩ
    cocos2d::Label* _monsterHealthLabel; // ��ʾ����Ѫ���ı�ǩ
    cocos2d::Label* _heroBlockLabel; // ��ʾӢ�۸񵲵ı�ǩ
    cocos2d::Label* _monsterBlockLabel; // ��ʾ����񵲵ı�ǩ
    std::vector<cocos2d::Sprite*> _cardSprites; // ���ƾ���
    std::vector<std::chrono::steady_clock::time_point> _lastClickTimes; // ��¼ÿ�ſ��Ƶ��ϴε��ʱ��
};

#endif // __FIGHTING_SCENE_H__