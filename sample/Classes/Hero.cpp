#include "Hero.h"

USING_NS_CC;

Hero* Hero::create(const std::string& filename)
{
    Hero* hero = new (std::nothrow) Hero();
    if (hero && hero->init(filename))
    {
        hero->autorelease();
        return hero;
    }
    CC_SAFE_DELETE(hero);
    return nullptr;
}

bool Hero::init(const std::string& filename)
{
    if (!Sprite::initWithFile(filename))
    {
        return false;
    }

    // ��ʼ�����ǵ�����
    _health = 100; // Ĭ������ֵ

    // ����Ĭ�ϵĿ���
    createDefaultDeck();

    return true;
}

void Hero::setHealth(int health)
{
    _health = health;
}

int Hero::getHealth() const
{
    return _health;
}

void Hero::setBlock(int block) {
    _block = block;
}

int Hero::getBlock() const {
    return _block;
}

void Hero::addCardToDeck(const Card& card)
{
    _deck.push_back(card);
}

const std::vector<Card>& Hero::getDeck() const
{
    return _deck;
}

void Hero::clearDeck()
{
    _deck.clear();
}

void Hero::createDefaultDeck()
{
    // ����ʾ������һ����ʼ���飬���忨�����Ը���ʵ���������
    clearDeck();

    // ���ʾ�����ƣ����� Card ���캯����Card(name, type, cost, effect, background, attack, defense)��
    addCardToDeck(Card("Strike", Card::Type::Attack, 1, "Deal 6 damage", "cardBackground.jpg", 6, 0));
    addCardToDeck(Card("Strike", Card::Type::Attack, 1, "Deal 6 damage", "cardBackground.jpg", 6, 0));
    addCardToDeck(Card("Strike", Card::Type::Attack, 1, "Deal 6 damage", "cardBackground.jpg", 6, 0));
    addCardToDeck(Card("Defend", Card::Type::Skill, 1, "Gain 5 Block", "cardBackground.jpg", 0, 5));
    addCardToDeck(Card("Defend", Card::Type::Skill, 1, "Gain 5 Block", "cardBackground.jpg", 0, 5));
    addCardToDeck(Card("Defend", Card::Type::Skill, 1, "Gain 5 Block", "cardBackground.jpg", 0, 5));
    addCardToDeck(Card("Bash", Card::Type::Attack, 2, "Deal 8 damage and apply Vulnerable", "cardBackground.jpg", 8, 0));
}
