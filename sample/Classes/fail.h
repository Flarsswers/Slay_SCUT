#ifndef __FAIL_SCENE_H__
#define __FAIL_SCENE_H__

#include "cocos2d.h"

class FailScene : public cocos2d::Scene
{
public:
    // ���������ľ�̬����
    static cocos2d::Scene* createScene();

    // ��ʼ������
    virtual bool init();

    // ʵ��"static create()"����
    CREATE_FUNC(FailScene);

private:
    // ��������
    void createBackground();

    // ����ʧ����ʾ�ͷ��ذ�ť
    void createUI();
};

#endif // __FAIL_SCENE_H__
