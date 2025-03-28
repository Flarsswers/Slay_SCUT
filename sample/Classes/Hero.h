#ifndef __HERO_H__
#define __HERO_H__

#include "cocos2d.h"
#include <vector>
#include "Card.h" // ���� Card ���Ѿ�����

class Hero : public cocos2d::Sprite
{
public:
    // ��������ʵ��
    static Hero* create(const std::string& filename);

    // ��ʼ������
    bool init(const std::string& filename);

    // ���úͻ�ȡ���ǵ�����ֵ
    void setHealth(int health);
    int getHealth() const;

	// ���úͻ�ȡ���ǵķ���ֵ
    void setBlock(int block);
    int getBlock() const;

    // ��������ӿ�
    void addCardToDeck(const Card& card);
    const std::vector<Card>& getDeck() const;
    void clearDeck();

private:
    int _health; // ����ֵ
	int _block; // ����ֵ
	std::vector<Card> _deck; // ����

    // ��ʼ��Ĭ�Ͽ���
    void createDefaultDeck();
};

#endif // __HERO_H__
