#ifndef __MAINMENU_SCENE_H__
#define __MAINMENU_SCENE_H__

#include "cocos2d.h"

class MainMenu : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    // �ص�����
    void menuStartCallback(cocos2d::Ref* pSender);
    void menuOptionsCallback(cocos2d::Ref* pSender);
    void menuCloseCallback(cocos2d::Ref* pSender);

    // �ֶ�ʵ�� "static create()" �����ĺ�
    CREATE_FUNC(MainMenu);
};

#endif // __MAINMENU_SCENE_H__