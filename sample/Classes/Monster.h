#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "cocos2d.h"

class Monster : public cocos2d::Sprite
{
public:
    // ��������ʵ��
    static Monster* create(const std::string& filename);

    // ��ʼ������
    bool init(const std::string& filename);

    // ���úͻ�ȡ���������ֵ
    void setHealth(int health);
    int getHealth() const;

	// ���úͻ�ȡ����ķ���ֵ
    void setBlock(int block);
    int getBlock() const;

private:
    int _health;
    int _block;
};

#endif // __MONSTER_H__
