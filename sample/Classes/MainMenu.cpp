#include "MainMenu.h"
#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

Scene* MainMenu::createScene()
{
    return MainMenu::create();
}

bool MainMenu::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ��������
    auto background = Sprite::create("mainmenu_background.png");
    if (background == nullptr)
    {
        printf("Error while loading: %s\n", "mainmenu_background.png");
        return false;
    }
    background->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(background, 0);

    // �����˵���
    auto startItem = MenuItemImage::create(
        "StartNormal.png",
        "StartSelected.png",
        CC_CALLBACK_1(MainMenu::menuStartCallback, this));
    if (startItem == nullptr || startItem->getContentSize().width <= 0 || startItem->getContentSize().height <= 0)
    {
        printf("Error while loading: %s and %s\n", "StartNormal.png", "StartSelected.png");
        return false;
    }
    startItem->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y + 100));

    auto optionsItem = MenuItemImage::create(
        "OptionsNormal.png",
        "OptionsSelected.png",
        CC_CALLBACK_1(MainMenu::menuOptionsCallback, this));
    if (optionsItem == nullptr || optionsItem->getContentSize().width <= 0 || optionsItem->getContentSize().height <= 0)
    {
        printf("Error while loading: %s and %s\n", "OptionsNormal.png", "OptionsSelected.png");
        return false;
    }
    optionsItem->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));

    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(MainMenu::menuCloseCallback, this));
    if (closeItem == nullptr || closeItem->getContentSize().width <= 0 || closeItem->getContentSize().height <= 0)
    {
        printf("Error while loading: %s and %s\n", "CloseNormal.png", "CloseSelected.png");
        return false;
    }
    closeItem->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y - 100));

    // �����˵�
    auto menu = Menu::create(startItem, optionsItem, closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    return true;
}

void MainMenu::menuStartCallback(Ref* pSender)
{
    // �л�����Ϸ����
    auto scene = HelloWorld::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(1.0, scene));
}

void MainMenu::menuOptionsCallback(Ref* pSender)
{
    // ����ѡ��˵�
    // ����������ѡ��˵����߼�
    printf("Options menu clicked\n");
}

void MainMenu::menuCloseCallback(Ref* pSender)
{
    // �ر���Ϸ
    Director::getInstance()->end();
}