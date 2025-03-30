#ifndef __CARD_H__
#define __CARD_H__

#include "cocos2d.h"
#include "Effect.h"

class Card
{
public:
    enum class Type
    {
        Attack,
        Skill,
        Power
    };

    Card(const std::string& name, Type type, int cost, const std::string& effect, const std::string& backgroundFile, int attack = 0, int defense = 0);
    ~Card();

    // ��ȡ��������
    const std::string& getName() const;

    // ��ȡ��������
    Type getType() const;

    // ��ȡ������������
    int getCost() const;

    // ��ȡ����Ч��
    const std::string& getEffect() const;

    // ��ȡ���ƹ�����
    int getAttack() const;

    // ��ȡ���Ʒ�����
    int getBlock() const;

    // ���ɴ��п���Ч���ı���ͼ��
    cocos2d::Sprite* createCardSprite();

    // ��ȡ���ƾ���
    cocos2d::Sprite* getSprite() const;

    // ���Ч��
    void addEffect(std::shared_ptr<Effect> effect);

    // ��ȡЧ���б�
    const std::vector<std::shared_ptr<Effect>>& getEffects() const;

private:
    std::string _name;
    Type _type;
    int _cost;
    std::string _effect;
    std::string _backgroundFile;
    int _attack;
    int _block;
    cocos2d::Sprite* _sprite; // ��ӿ��ƾ����Ա����
    std::vector<std::shared_ptr<Effect>> _effects; // ���Ч���б�
};

#endif // __CARD_H__
