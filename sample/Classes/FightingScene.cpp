#include "FightingScene.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

// ��������
Scene* FightingScene::createScene()
{
    return FightingScene::create();
}

// �Ҵ�����
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in FightingScene.cpp\n");
}

// ��ʼ������
bool FightingScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    _visibleSize = Director::getInstance()->getVisibleSize();
    _origin = Director::getInstance()->getVisibleOrigin();

    // ��������
    createBackground();

    // ������ɫ�͹���
    createCharacters();

    // ��ʼ�����ƶѺ����ƶ�
    initializeDrawPile();

    // ����Ѫ����ǩ
    createHealthLabels();

    // �����񵲱�ǩ
    createBlockLabels();

	// ����Ѫ����ǩ
    updateHealthAndBlockLabels();

    // ��ʼ��һغ�
    startPlayerTurn();

    return true;
}

// ����Ѫ����ǩ
void FightingScene::createHealthLabels()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ����Ӣ��Ѫ����ǩ
    _heroHealthLabel = Label::createWithTTF("Hero Health: 100", "fonts/Marker Felt.ttf", 60);
    _heroHealthLabel->setTextColor(Color4B::RED); // ���ñ�ǩ��ɫΪ��ɫ
    _heroHealthLabel->setPosition(Vec2(origin.x + visibleSize.width / 4, origin.y + visibleSize.height - _heroHealthLabel->getContentSize().height));
    this->addChild(_heroHealthLabel, 1);

    // ��������Ѫ����ǩ
    _monsterHealthLabel = Label::createWithTTF("Monster Health: 100", "fonts/Marker Felt.ttf", 60);
    _monsterHealthLabel->setTextColor(Color4B::RED); // ���ñ�ǩ��ɫΪ��ɫ
    _monsterHealthLabel->setPosition(Vec2(origin.x + 3 * visibleSize.width / 4, origin.y + visibleSize.height - _monsterHealthLabel->getContentSize().height));
    this->addChild(_monsterHealthLabel, 1);
}

// �����񵲱�ǩ
void FightingScene::createBlockLabels()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ����Ӣ�۸񵲱�ǩ
    _heroBlockLabel = Label::createWithTTF("Hero Block: 0", "fonts/Marker Felt.ttf", 60);
    _heroBlockLabel->setTextColor(Color4B::BLUE); // ���ñ�ǩ��ɫΪ��ɫ
    _heroBlockLabel->setPosition(Vec2(origin.x + visibleSize.width / 4, origin.y + visibleSize.height - _heroHealthLabel->getContentSize().height - 70));
    this->addChild(_heroBlockLabel, 1);

    // ��������񵲱�ǩ
    _monsterBlockLabel = Label::createWithTTF("Monster Block: 0", "fonts/Marker Felt.ttf", 60);
    _monsterBlockLabel->setTextColor(Color4B::BLUE); // ���ñ�ǩ��ɫΪ��ɫ
    _monsterBlockLabel->setPosition(Vec2(origin.x + 3 * visibleSize.width / 4, origin.y + visibleSize.height - _monsterHealthLabel->getContentSize().height - 70));
    this->addChild(_monsterBlockLabel, 1);
}

// ����Ѫ���͸񵲱�ǩ
void FightingScene::updateHealthAndBlockLabels()
{
    _heroHealthLabel->setString("Hero Health: " + std::to_string(_hero->getHealth()));
    _monsterHealthLabel->setString("Monster Health: " + std::to_string(_monster->getHealth()));
    _heroBlockLabel->setString("Hero Block: " + std::to_string(_hero->getBlock()));
    _monsterBlockLabel->setString("Monster Block: " + std::to_string(_monster->getBlock()));
}

// ��������
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

// ������ɫ�͹���
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

// ��ʼ��һغ�(���ơ���ӻغϽ�����ť�����ð�ťλ��)
void FightingScene::startPlayerTurn()
{
    _isPlayerTurn = true;

    for (int i = 0; i < 5; i++) {
        drawCard();
    }

    // ���һ����ť���ֶ������غ�
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto endTurnButton = ui::Button::create("button.png", "button_selected.png");

    if (endTurnButton == nullptr)
    {
        problemLoading("'buttonNormal.png' or 'buttonSelected.png'");
        return;
    }

    endTurnButton->setScale(0.25f); // ����ť��С��ԭ����50%

	// ���ð�ťλ��
    endTurnButton->setPosition(Vec2(origin.x + visibleSize.width - endTurnButton->getContentSize().width / 2,
        origin.y + visibleSize.width / 2 - endTurnButton->getContentSize().height / 2));

    endTurnButton->addClickEventListener([this](Ref* sender) {
        this->endTurn();
        });

    this->addChild(endTurnButton, 1);
}

// ��ʼ����غ�
void FightingScene::startMonsterTurn()
{
    _isPlayerTurn = false;
	// Ŀǰ����غ϶��������10�˺������5����
    int damage = 10;
    int block = 5;

	_monster->setBlock(block);

    // �����
    int heroBlock = _hero->getBlock();
    if (heroBlock > 0)
    {
        if (heroBlock >= damage)
        {
            _hero->setBlock(heroBlock - damage);
            damage = 0;
        }
        else
        {
            damage -= heroBlock;
            _hero->setBlock(0);
        }
    }

    // �������������˺�
    int newHealth = _hero->getHealth() - damage;
    _hero->setHealth(newHealth);
    CCLOG("Hero Health: %d", _hero->getHealth());
    updateHealthAndBlockLabels();
    endTurn();
}

// �жϻغ��Ƿ����
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

// ���ս���Ƿ����
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
        updateHandDisplay(); // ����������ʾ
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
        updateHandDisplay(); // ����������ʾ
    }
}

// ϴ��
void FightingScene::shuffleDrawPile()
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(_drawPile.begin(), _drawPile.end(), g);
}

// ��ʼ���ƶ�
void FightingScene::initializeDrawPile()
{
    // ��ȡ Hero �Ŀ��鲢����Ϊ���ƶ�
    _drawPile = _hero->getDeck();
    shuffleDrawPile();
}

// ���������Ч����ǩ
void FightingScene::addCardEffectLabel(cocos2d::Sprite* cardSprite, const std::string& effect)
{
    auto effectLabel = Label::createWithTTF(effect, "fonts/Marker Felt.ttf", 48);
    effectLabel->setColor(cocos2d::Color3B::BLACK);
    effectLabel->setPosition(Vec2(cardSprite->getContentSize().width / 2, cardSprite->getContentSize().height / 2));
    cardSprite->addChild(effectLabel, 1);
}

// ����������ʾ
void FightingScene::updateHandDisplay()
{
    // �����ǰ������ʾ
    for (auto cardSprite : _cardSprites)
    {
        this->removeChild(cardSprite);
    }
    _cardSprites.clear();
    _lastClickTimes.clear(); // ����ϴε��ʱ��

    // ��ȡ�ɼ������С��ԭ��
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ����ÿ�ſ��Ƶı���ͼƬΪ "cardBackground.jpg"
    auto tempCardSprite = Sprite::create("cardBackground.jpg");
    float cardWidth = tempCardSprite->getContentSize().width;

    // ���㿨��֮��ļ��
    float totalWidth = _cards.size() * cardWidth;
    float spacing = 0.0f;
    if (totalWidth > visibleSize.width)
    {
        cardWidth = (visibleSize.width - 20.0f) / _cards.size(); // 20.0f �����ұ߾�
        spacing = 0.0f;
    }
    else
    {
        spacing = (visibleSize.width - totalWidth) / (_cards.size() + 1);
    }

    float startX = origin.x + spacing + cardWidth / 2;

    for (size_t i = 0; i < _cards.size(); ++i)
    {
        auto cardSprite = Sprite::create("cardBackground.jpg");
        cardSprite->setPosition(Vec2(startX + i * (cardWidth + spacing), origin.y + cardSprite->getContentSize().height / 3));
        cardSprite->setScale(cardWidth / cardSprite->getContentSize().width);
        this->addChild(cardSprite, 1);
        _cardSprites.push_back(cardSprite);
        _lastClickTimes.push_back(std::chrono::steady_clock::now()); // ��ʼ���ϴε��ʱ��

        addCardEffectLabel(cardSprite, _cards[i].getEffect());

        // ��ӵ���¼��������������Ӧ��
        auto listener = EventListenerTouchOneByOne::create();
        listener->setSwallowTouches(true);
        listener->onTouchBegan = [this, i](Touch* touch, Event* event) -> bool {
            auto target = static_cast<Sprite*>(event->getCurrentTarget());
            if (target->getBoundingBox().containsPoint(touch->getLocation()))
            {
                handleCardTap(i, touch);
                return true;
            }
            return false;
            };
        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, cardSprite);
    }
}

// Ӧ��BUFF��DEBUFF
void FightingScene::applyEffects(int& damage, int& block, const std::vector<std::shared_ptr<Effect>>& effects, bool isTargetMonster)
{
    for (const auto& effect : effects)
    {
        if (auto buff = dynamic_cast<Buff*>(effect.get()))
        {
            switch (buff->getType())
            {
            case Effect::Type::Strength:
                damage += buff->getLevel();
                break;
            default:
                break;
            }
        }
        else if (auto debuff = dynamic_cast<Debuff*>(effect.get()))
        {
            switch (debuff->getType())
            {
            case Effect::Type::Vulnerable:
                if (isTargetMonster)
                {
                    damage = static_cast<int>(damage * 1.5);
                }
                break;
            default:
                break;
            }
        }
    }
}

// Ӧ�ÿ���Ч��
void FightingScene::applyCardEffects(const Card& card)
{
    int damage = card.getAttack();
    int block = card.getBlock();

    // Ӧ��Ӣ�۵�Ч��
    applyEffects(damage, block, _hero->getEffects(), false);

    // Ӧ�ù����Ч��
    applyEffects(damage, block, _monster->getEffects(), true);

    // �������ĸ�
    int monsterBlock = _monster->getBlock();
    if (monsterBlock > 0)
    {
        if (monsterBlock >= damage)
        {
            _monster->setBlock(monsterBlock - damage);
            damage = 0;
        }
        else
        {
            damage -= monsterBlock;
            _monster->setBlock(0);
        }
    }

    // ������������ֵ
    int newHealth = _monster->getHealth() - damage;
    _monster->setHealth(newHealth);
    CCLOG("Monster Health: %d", _monster->getHealth());

    // ����Ӣ�۵ĸ�
    if (block > 0)
    {
        int newBlock = _hero->getBlock() + block;
        _hero->setBlock(newBlock);
        CCLOG("Hero Block: %d", _hero->getBlock());
    }

    updateHealthAndBlockLabels();
}

// �������
void FightingScene::playCard(int index)
{
    if (_isCooldown) {
        // ���������ȴ״̬��ֱ�ӷ���
        return;
    }

    if (index >= 0 && index < _cards.size())
    {
        // ִ�д�����Ƶ��߼�
        Card playedCard = _cards[index];
        applyCardEffects(playedCard);

        // ������ȴ״̬
        _isCooldown = true;
        this->runAction(Sequence::create(
            Spawn::create(
                Sequence::create(
                    DelayTime::create(COOLDOWN_TIME),
                    CallFunc::create([this]() { _isCooldown = false; }),
                    nullptr
                ),
                Sequence::create(
                    DelayTime::create(DISCARD_DELAY),
                    CallFunc::create([this, index]() {
                        discardCard(index);
                        _selectedCardIndex = -1;
                        highlightSelectedCard();
                        }),
                    nullptr
                ),
                nullptr
            ),
            nullptr
        ));
    }
}

// ����ѡ�еĿ���
void FightingScene::highlightSelectedCard()
{
    for (size_t i = 0; i < _cardSprites.size(); ++i)
    {
        if (i == _selectedCardIndex)
        {
            _cardSprites[i]->setColor(Color3B::YELLOW); // ����ѡ�еĿ���
        }
        else
        {
            _cardSprites[i]->setColor(Color3B::WHITE); // ȡ��������������
        }
    }
}

// �����Ƶ���¼�
void FightingScene::handleCardTap(size_t cardIndex, cocos2d::Touch* touch)
{
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastClickTimes[cardIndex]).count();

    if (duration < DOUBLE_CLICK_THRESHOLD)
    {
        playCard(cardIndex);
    }
    else
    {
        _selectedCardIndex = cardIndex;
        highlightSelectedCard();
    }
    _lastClickTimes[cardIndex] = now;
}
