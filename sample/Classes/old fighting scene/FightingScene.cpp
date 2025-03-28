#include "FightingScene.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

Scene* FightingScene::createScene()
{
    return FightingScene::create();
}

static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in FightingScene.cpp\n");
}

bool FightingScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ��ʼ��ѡ�п���
    _selectedCard = nullptr;
    _selectedCardIndex = -1;

    // ��������
    createBackground();

    // ������ɫ�͹���
    createCharacters();

	// ��ʼ�����ƶѺ����ƶ�
    initializeDrawPile();

    // ����Ѫ����ǩ
    _heroHealthLabel = Label::createWithTTF("Hero Health: 100", "fonts/Marker Felt.ttf", 60);
    _heroHealthLabel->setTextColor(Color4B::RED); // ���ñ�ǩ��ɫΪ��ɫ
    _heroHealthLabel->setPosition(Vec2(origin.x + visibleSize.width / 4, origin.y + visibleSize.height - _heroHealthLabel->getContentSize().height));
    this->addChild(_heroHealthLabel, 1);

    _monsterHealthLabel = Label::createWithTTF("Monster Health: 100", "fonts/Marker Felt.ttf", 60);
    _monsterHealthLabel->setTextColor(Color4B::RED); // ���ñ�ǩ��ɫΪ��ɫ
    _monsterHealthLabel->setPosition(Vec2(origin.x + 3 * visibleSize.width / 4, origin.y + visibleSize.height - _monsterHealthLabel->getContentSize().height));
    this->addChild(_monsterHealthLabel, 1);

    startPlayerTurn();

    return true;
}

void FightingScene::createBackground()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto background = Sprite::create("background.png");
    if (background == nullptr)
    {
        problemLoading("'background.png'");
        return;
    }

    background->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(background, 0);
}

void FightingScene::createCharacters()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ����Ӣ��
    _hero = Hero::create("hero.png");
    if (_hero == nullptr)
    {
        problemLoading("'hero.png'");
        return;
    }

    _hero->setPosition(Vec2(origin.x + _hero->getContentSize().width / 2, origin.y + visibleSize.height / 2));
    this->addChild(_hero, 1);

    // ��������
    _monster = Monster::create("monster.png");
    if (_monster == nullptr)
    {
        problemLoading("'monster.png'");
        return;
    }

    _monster->setPosition(Vec2(origin.x + visibleSize.width - _monster->getContentSize().width / 2, origin.y + visibleSize.height / 2));
    this->addChild(_monster, 1);
}

bool FightingScene::onCardTouchBegan(cocos2d::Sprite* cardSprite, int index, cocos2d::Touch* touch, cocos2d::Event* event)
{
    Vec2 touchLocation = touch->getLocation();
    if (cardSprite->getBoundingBox().containsPoint(touchLocation))
    {
        if (_selectedCard == cardSprite)
        {
            // ����ٴε��ѡ�еĿ��ƣ���������
            playCard(index);
        }
        else
        {
            // ѡ�п���
            selectCard(cardSprite, index);
        }
        return true;
    }
    return false;
}

void FightingScene::selectCard(cocos2d::Sprite* card, int index)
{
    if (_selectedCard != nullptr)
    {
        // ȡ��֮ǰѡ�еĿ��Ƶ�ѡ��״̬
        _selectedCard->setColor(cocos2d::Color3B::WHITE);
    }

    // ���õ�ǰѡ�еĿ���
    _selectedCard = card;
    _selectedCardIndex = index;

    // ����ѡ�п��Ƶ���ɫΪ��ɫ
    _selectedCard->setColor(cocos2d::Color3B::GRAY);
}

void FightingScene::playCard(int index)
{
    // ������Ƶ��߼�
    CCLOG("Play Card: %s", _cards[index].getName().c_str());
    CCLOG("Card Effect: %s", _cards[index].getEffect().c_str());

    // ���ݿ���Ч���Թ�������˺�
    if (_cards[index].getType() == Card::Type::Attack)
    {
        int damage = _cards[index].getAttack();
        int newHealth = _monster->getHealth() - damage;
        _monster->setHealth(newHealth);
        CCLOG("Monster Health: %d", _monster->getHealth());
    }

    // ����Ѫ����ǩ
    updateHealthLabels();

    discardCard(index);
}

void FightingScene::updateHealthLabels()
{
    _heroHealthLabel->setString("Hero Health: " + std::to_string(_hero->getHealth()));
    _monsterHealthLabel->setString("Monster Health: " + std::to_string(_monster->getHealth()));
}

void FightingScene::startPlayerTurn()
{
    _isPlayerTurn = true;
    createCards();

    // ���һ����ť���ֶ������غ�
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto endTurnButton = ui::Button::create("button.png", "button.png");

    if (endTurnButton == nullptr)
    {
        problemLoading("'buttonNormal.png' or 'buttonSelected.png'");
        return;
    }

    endTurnButton->setScale(0.25f); // ����ť��С��ԭ����50%


    endTurnButton->setPosition(Vec2(origin.x + visibleSize.width - endTurnButton->getContentSize().width / 2,
        origin.y + endTurnButton->getContentSize().height / 2));

    endTurnButton->addClickEventListener([this](Ref* sender) {
        this->endPlayerTurn();
        });

    this->addChild(endTurnButton, 1);
}

void FightingScene::startMonsterTurn()
{
    _isPlayerTurn = false;
    // ������������10���˺�
    int newHealth = _hero->getHealth() - 10;
    _hero->setHealth(newHealth);
    CCLOG("Hero Health: %d", _hero->getHealth());
    updateHealthLabels();
    endTurn();
}

void FightingScene::endTurn()
{
    checkBattleEnd();
    if (!_isPlayerTurn)
    {
        startPlayerTurn();
    }
    else
    {
        startMonsterTurn();
    }
}

void FightingScene::checkBattleEnd()
{
    if (_hero->getHealth() <= 0)
    {
        CCLOG("Hero is dead. Game Over.");
        // ������Ϸ�����߼�
    }
    else if (_monster->getHealth() <= 0)
    {
        CCLOG("Monster is dead. You Win!");
        // ������Ϸʤ���߼�
    }
}

// ��һ����
void FightingScene::drawCard()
{
    if (_drawPile.empty())
    {
        // ������ƶ�Ϊ�գ������ƶѵ���ϴ����ƶ�
        _drawPile = _discardPile;
        _discardPile.clear();
        shuffleDrawPile();
    }

    if (!_drawPile.empty())
    {
        // �ӳ��ƶѶ���һ����
        Card drawnCard = _drawPile.back();
        _drawPile.pop_back();
        _cards.push_back(drawnCard);
    }
}

// ��һ����
void FightingScene::discardCard(int index)
{
    if (index >= 0 && index < _cards.size())
    {
        // �����Ʒ������ƶ�
        _discardPile.push_back(_cards[index]);
        _cards.erase(_cards.begin() + index);
    }
}

// ϴ��
void FightingScene::shuffleDrawPile()
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(_drawPile.begin(), _drawPile.end(), g);
}

void FightingScene::updateHandDisplay()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // �Ƴ��������еĿ���
    removeExistingCards();

    // ��ʾ��ǰ�����еĿ���
    displayDrawnCards(_cards.size(), visibleSize, origin);
}

void FightingScene::removeExistingCards()
{
    auto children = this->getChildren();
    std::vector<Node*> toRemove;
    for (const auto& child : children)
    {
        if (child->getTag() >= 1000 && child->getTag() < 1010)
        {
            toRemove.push_back(child);
        }
    }
    for (const auto& node : toRemove)
    {
        node->removeFromParent();
    }
}

void FightingScene::displayDrawnCards(int numCards, const Size& visibleSize, const Vec2& origin)
{
    float cardWidth = visibleSize.width / (numCards + 1);
    float cardHeight = visibleSize.height / 5;

    for (int i = 0; i < _cards.size(); ++i)
    {
        auto cardSprite = _cards[i].createCardSprite();
        if (cardSprite == nullptr)
        {
            continue;
        }

        setupCardSprite(cardSprite, i, cardWidth, cardHeight, origin);
        addCardEffectLabel(cardSprite, _cards[i].getEffect());
        addCardTouchListener(cardSprite, i);
    }
}

void FightingScene::setupCardSprite(cocos2d::Sprite* cardSprite, int index, float cardWidth, float cardHeight, const Vec2& origin)
{
    cardSprite->setTag(1000 + index);

    float scaleX = cardWidth / cardSprite->getContentSize().width;
    float scaleY = cardHeight / cardSprite->getContentSize().height;
    float scale = std::max(scaleX, scaleY);
    cardSprite->setScale(scale);

    float xPos = origin.x + (index + 1) * cardWidth;
    float yPos = origin.y + cardHeight / 2;
    cardSprite->setPosition(Vec2(xPos, yPos));
    this->addChild(cardSprite, 1);
}

void FightingScene::addCardEffectLabel(cocos2d::Sprite* cardSprite, const std::string& effect)
{
    auto effectLabel = Label::createWithTTF(effect, "fonts/Marker Felt.ttf", 24);
    effectLabel->setColor(cocos2d::Color3B::BLACK);
    effectLabel->setPosition(Vec2(cardSprite->getContentSize().width / 2, cardSprite->getContentSize().height / 2));
    cardSprite->addChild(effectLabel, 1);
}

void FightingScene::addCardTouchListener(cocos2d::Sprite* cardSprite, int index)
{
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = [this, cardSprite, index](Touch* touch, Event* event) {
        return this->onCardTouchBegan(cardSprite, index, touch, event);
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, cardSprite);
}

void FightingScene::initializeDrawPile()
{
    // ��ȡ Hero �Ŀ��鲢����Ϊ���ƶ�
    _drawPile = _hero->getDeck();
    shuffleDrawPile();
}

void FightingScene::endPlayerTurn()
{
    endTurn();
}
