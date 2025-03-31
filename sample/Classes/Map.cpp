// Map.cpp
#include "Map.h"
#include "FightingScene.h"
#include "question.h"
#include "Rest.h"
#include <random>

USING_NS_CC;

namespace MyGame {

    // �洢��ͼ���ݣ��������ͺ�λ����Ϣ�������洢ָ�룩
    struct RoomInfo {
        RoomType type;
        cocos2d::Vec2 position;
    };
    static std::vector<std::vector<RoomInfo>> staticMapInfo;

    // �洢��ǰ�ɷ��ʵ���߲㼶����̬�������ڳ����л�ʱ���֣�
    static int maxAccessibleLayer = 0;

    // �Ƿ�ո������һ�����䣨���ڴӷ��䳡�����ص�ͼʱ�жϣ�
    static bool roomCompleted = false;

    // ���������Ϸ״̬�ĺ���ʵ��
    void resetGameState() {
        // ��յ�ͼ��Ϣ
        staticMapInfo.clear();

        // ���ÿɷ��ʲ㼶
        maxAccessibleLayer = 0;

        // ���÷������״̬
        roomCompleted = false;
    }

    Scene* Map::createScene()
    {
        return Map::create();
    }

    bool Map::init()
    {
        if (!Scene::init())
        {
            return false;
        }

        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        // ��ʼ����ǰ��͵�ǰ����
        currentLayer = 0;
        currentRoom = -1;

        // ���� ScrollView
        scrollView = ui::ScrollView::create();
        scrollView->setDirection(ui::ScrollView::Direction::VERTICAL);
        scrollView->setContentSize(visibleSize);
        scrollView->setInnerContainerSize(Size(visibleSize.width, visibleSize.height * 3));  // ���ù�����Χ
        scrollView->setPosition(origin);
        this->addChild(scrollView);

        // ��ӵ�ͼ����ͼƬ
        auto background = Sprite::create("map_background.png");
        if (background)
        {
            background->setPosition(Vec2(visibleSize.width / 2, scrollView->getInnerContainerSize().height / 2));
            scrollView->addChild(background, 0);  // ������ͼƬ��ӵ���ײ�
        }

        // ����Ƿ��Ѿ������˵�ͼ����
        if (staticMapInfo.empty())
        {
            // ���ɶ���ͼ
            int layers = 10;  // ������10��
            int roomsPerLayer = 3;  // ÿ����3������
            generateRandomMap(layers, roomsPerLayer);
        }
        else
        {
            // ʹ���Ѵ洢�ĵ�ͼ�����������ɵ�ͼ
            generateMapFromSavedInfo();
        }

        // ����ո������һ�����䣬���ӿɷ��ʵĲ㼶
        if (roomCompleted)
        {
            maxAccessibleLayer++;
            roomCompleted = false;
        }

        // ���·���Ŀɷ�����
        updateRoomAccessibility();

        // ��Ӳ㼶��ʾ��ǩ
        createLayerLabels();

        return true;
    }

    // �����㼶��ǩ
    void Map::createLayerLabels()
    {
        auto visibleSize = Director::getInstance()->getVisibleSize();

        // ��ӵ�ǰ�ɷ��ʲ㼶��ʾ
        auto accessLabel = Label::createWithTTF(
            "�ɷ��ʲ㼶: " + std::to_string(maxAccessibleLayer + 1),
            "fonts/Marker Felt.ttf",
            32
        );
        accessLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 50));
        this->addChild(accessLabel, 3);
    }

    // ���·���Ŀɷ�����
    void Map::updateRoomAccessibility()
    {
        for (int i = 0; i < mapLayers.size(); i++)
        {
            for (auto& room : mapLayers[i])
            {
                // �����ǰ�㼶�������ɷ��ʲ㼶������õ��
                if (i > maxAccessibleLayer)
                {
                    room.item->setEnabled(false);
                    // ���û�ɫ�ɰ��ʾ�޷�����
                    room.item->setColor(Color3B(100, 100, 100));
                }
                else
                {
                    room.item->setEnabled(true);
                    room.item->setColor(Color3B(255, 255, 255));
                }
            }
        }
    }

    RoomType Map::getRandomRoomType()
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());

        // ʹ����ɢ�ֲ������ò�ͬ�������͵ĸ���
        // BATTLE: 0.5, QUESTION: 0.3, REST: 0.2
        static std::discrete_distribution<> dis({ 0, 50, 40, 10 });  // ��Ӧ START(0), BATTLE(1), QUESTION(2), REST(3)

        // ��ȡ���ֵ��ת��ΪRoomType
        int randomType = dis(gen);
        return static_cast<RoomType>(randomType);
    }


    void Map::generateRandomMap(int layers, int roomsPerLayer)
    {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        // ��ӿ�ʼ��ͼ��
        auto startItem = MenuItemImage::create(
            "start.png",  // ʹ������ͼ��
            "start_selected.png",
            [](Ref*) {}  // ��ʼ�㲻��Ҫ�ص�����
        );
        if (startItem)
        {
            startItem->setPosition(Vec2(
                visibleSize.width / 2,
                scrollView->getInnerContainerSize().height - 100  // ���ÿ�ʼ��λ��
            ));
            startItem->setScale(0.5);  // ����ͼ���С
            auto menu = Menu::create(startItem, nullptr);
            menu->setPosition(Vec2::ZERO);
            scrollView->addChild(menu, 1);  // ȷ��ͼ���ڱ���ͼƬ֮��
        }

        // �������
        mapLayers.clear();
        staticMapInfo.clear();

        for (int i = 0; i < layers; ++i)
        {
            std::vector<Room> layer;
            std::vector<RoomInfo> layerInfo;  // �洢��ǰ��ķ�����Ϣ
            int roomCount;  // ÿ��ķ�������

            // ���ݲ�ͬ�㼶���ò�ͬ�ķ�������������
            if (i == layers - 1) {
                // ���һ�㣨Boss�㣩ֻ��1������
                roomCount = 1;
            }
            else if (i == layers - 2) {
                // Boss��ǰһ��������3�����䣨������Ϣ���䣩
                roomCount = roomsPerLayer >= 3 ? roomsPerLayer : 3;
            }
            else {
                // ������������3������
                roomCount = roomsPerLayer >= 3 ? roomsPerLayer : 3;
            }

            for (int j = 0; j < roomCount; ++j)
            {
                RoomType roomType;

                // ���ݲ�ͬ�㼶���ò�ͬ�ķ�������
                if (i == layers - 1) {
                    roomType = RoomType::BOSS;  // ���һ���� BOSS ����
                }
                else if (i == layers - 2) {
                    roomType = RoomType::REST;  // Boss��ǰһ��ȫ������Ϣ����
                }
                else {
                    roomType = getRandomRoomType();  // ������������ɷ�������
                }

                MenuItemImage* roomItem = nullptr;
                            switch (roomType)
                {
                case RoomType::BATTLE:
                    roomItem = MenuItemImage::create(
                        "battle_normal.png",
                        "battle_selected.png",
                        CC_CALLBACK_1(Map::menuBattleCallback, this)
                    );
                    break;
                case RoomType::QUESTION:
                    roomItem = MenuItemImage::create(
                        "question_normal.jpg",
                        "question_selected.png",
                        CC_CALLBACK_1(Map::menuQuestionCallback, this)
                    );
                    break;
                case RoomType::REST:
                    roomItem = MenuItemImage::create(
                        "rest.jpg",
                        "rest_selected.png",
                        CC_CALLBACK_1(Map::menuRestCallback, this)
                    );
                    break;
                case RoomType::BOSS:
                    roomItem = MenuItemImage::create(
                        "boss.png",  // ʹ������ͼ��
                        "boss_selected.png",
                        CC_CALLBACK_1(Map::menuBossCallback, this)
                    );
                    break;
                }

                if (roomItem)
                {
                    // ���ݷ�����������λ��
                    Vec2 position;
                    if (roomCount == 1) {

                     // ���ֻ��һ�����䣬�����м�
                        position = Vec2(
                            visibleSize.width / 2,
                            scrollView->getInnerContainerSize().height - (i + 1) * 200  // ���ݲ�������λ��
                        );
                    }
                    else {
                        // ���ݷ����������ȷֲ�
                        float spacing = visibleSize.width / (roomCount + 1);
                        position = Vec2(
                            origin.x + spacing * (j + 1),
                            scrollView->getInnerContainerSize().height - (i + 1) * 200  // ���ݲ�������λ��
                        );
                    }

                    roomItem->setPosition(position);
                    roomItem->setScale(0.5);  // ������ť��С
                    // �洢�㼶��Ϣ
                    roomItem->setTag(i);

                    auto menu = Menu::create(roomItem, nullptr);
                    menu->setPosition(Vec2::ZERO);
                    scrollView->addChild(menu, 1);  // ȷ���˵��ڱ���ͼƬ֮��

                    layer.push_back({ roomType, roomItem });

                    // ����λ����Ϣ�Ա�����ؽ�
                    layerInfo.push_back({ roomType, position });
                }
            }
            mapLayers.push_back(layer);
            staticMapInfo.push_back(layerInfo);  // �洢�ò���Ϣ
        }

        connectRooms(mapLayers);
    }

    void Map::generateMapFromSavedInfo()
    {
        // ������е�ͼ����
        mapLayers.clear();

        // ���ݱ���ĵ�ͼ��Ϣ���´�������
        for (int i = 0; i < staticMapInfo.size(); i++)
        {
            const auto& layerInfo = staticMapInfo[i];
            std::vector<Room> layer;

            for (const auto& roomInfo : layerInfo)
            {
                MenuItemImage* roomItem = nullptr;

                switch (roomInfo.type)
                {
                case RoomType::BATTLE:
                    roomItem = MenuItemImage::create(
                        "battle_normal.png",
                        "battle_selected.png",
                        CC_CALLBACK_1(Map::menuBattleCallback, this)
                    );
                    break;
                case RoomType::QUESTION:
                    roomItem = MenuItemImage::create(
                        "question_normal.jpg",
                        "question_selected.png",
                        CC_CALLBACK_1(Map::menuQuestionCallback, this)
                    );
                    break;
                case RoomType::REST:
                    roomItem = MenuItemImage::create(
                        "rest.jpg",
                        "rest_selected.png",
                        CC_CALLBACK_1(Map::menuRestCallback, this)
                    );
                    break;
                case RoomType::BOSS:
                    roomItem = MenuItemImage::create(
                        "boss.png",  // ʹ������ͼ��
                        "boss_selected.png",
                        CC_CALLBACK_1(Map::menuBossCallback, this)
                    );
                    break;
                }

                if (roomItem)
                {
                    roomItem->setPosition(roomInfo.position);
                    roomItem->setScale(0.5);  // ������ť��С
                    // �洢�㼶��Ϣ
                    roomItem->setTag(i);

                    auto menu = Menu::create(roomItem, nullptr);
                    menu->setPosition(Vec2::ZERO);
                    scrollView->addChild(menu, 1);  // ȷ���˵��ڱ���ͼƬ֮��

                    layer.push_back({ roomInfo.type, roomItem });
                }
            }
            mapLayers.push_back(layer);
        }

        connectRooms(mapLayers);
    }

    void Map::connectRooms(const std::vector<std::vector<Room>>& map)
    {
        auto drawNode = DrawNode::create();
        scrollView->addChild(drawNode, 2);  // ȷ���������ڰ�ť֮��

        for (size_t i = 0; i < map.size() - 1; ++i)
        {
            const auto& currentLayer = map[i];
            const auto& nextLayer = map[i + 1];

            if (nextLayer.empty() || currentLayer.empty()) {
                continue;  // ȷ���㲻Ϊ��
            }

            // ����BOSS������������ֻ��һ�����䣩
            if (nextLayer.size() == 1) {
                // ���е�ǰ�㷿�䶼���ӵ���һ���BOSS����
                auto bossPos = nextLayer[0].item->getPosition();
                for (const auto& currentRoom : currentLayer) {
                    auto startPos = currentRoom.item->getPosition();
                    drawNode->drawLine(startPos, bossPos, Color4F::WHITE);
                }
                continue;
            }

            // ȷ��ÿ������������һ�����ӵ���һ��ķ���
            for (const auto& currentRoom : currentLayer)
            {
                // ���ѡ��һ����һ��ķ����������
                int nextRoomIndex = std::rand() % nextLayer.size();
                auto startPos = currentRoom.item->getPosition();
                auto endPos = nextLayer[nextRoomIndex].item->getPosition();
                drawNode->drawLine(startPos, endPos, Color4F::WHITE);
            }

            // ȷ��ÿ������������һ�����ӵ���һ��ķ���
            for (const auto& nextRoom : nextLayer)
            {
                // ���ѡ��һ����ǰ��ķ����������
                int currentRoomIndex = std::rand() % currentLayer.size();
                auto startPos = currentLayer[currentRoomIndex].item->getPosition();
                auto endPos = nextRoom.item->getPosition();
                drawNode->drawLine(startPos, endPos, Color4F::WHITE);
            }

            // �����Ӷ��������
            for (const auto& currentRoom : currentLayer)
            {
                for (const auto& nextRoom : nextLayer)
                {
                    if (std::rand() % 2 == 0)
                    {
                        auto startPos = currentRoom.item->getPosition();
                        auto endPos = nextRoom.item->getPosition();
                        drawNode->drawLine(startPos, endPos, Color4F::WHITE);
                    }
                }
            }
        }
    }

    void Map::menuBattleCallback(Ref* pSender)
    {
        auto item = static_cast<MenuItemImage*>(pSender);
        int layerIndex = item->getTag();

        // ����Ƿ���Է�������㼶
        if (layerIndex > maxAccessibleLayer)
        {
            // ������ʾ��ǩ
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto label = Label::createWithTTF("unable to enter now", "fonts/Marker Felt.ttf", 32);
            label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
            this->addChild(label, 5);

            // 2����Ƴ���ʾ
            label->runAction(Sequence::create(
                DelayTime::create(2.0f),
                RemoveSelf::create(),
                nullptr
            ));

            return;
        }

        // ���õ�ǰѡ�еĲ�ͷ���
        currentLayer = layerIndex;
        currentRoom = 0;

        // ���Ϊ����ɷ���
        roomCompleted = true;

        // �л���ս������
        auto scene = FightingScene::createScene();
        Director::getInstance()->replaceScene(scene);
    }

    void Map::menuQuestionCallback(Ref* pSender)
    {
        auto item = static_cast<MenuItemImage*>(pSender);
        int layerIndex = item->getTag();

        // ����Ƿ���Է�������㼶
        if (layerIndex > maxAccessibleLayer)
        {
            // ������ʾ��ǩ
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto label = Label::createWithTTF("unable to enter now", "fonts/Marker Felt.ttf", 32);
            label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
            this->addChild(label, 5);

            // 2����Ƴ���ʾ
            label->runAction(Sequence::create(
                DelayTime::create(2.0f),
                RemoveSelf::create(),
                nullptr
            ));

            return;
        }

        // ���õ�ǰѡ�еĲ�ͷ���
        currentLayer = layerIndex;
        currentRoom = 0;

        // ���Ϊ����ɷ���
        roomCompleted = true;

        // �л����ʺų���
        auto scene = Question::createScene();
        Director::getInstance()->replaceScene(scene);
    }

    void Map::menuRestCallback(Ref* pSender)
    {
        auto item = static_cast<MenuItemImage*>(pSender);
        int layerIndex = item->getTag();

        // ����Ƿ���Է�������㼶
        if (layerIndex > maxAccessibleLayer)
        {
            // ������ʾ��ǩ
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto label = Label::createWithTTF("unable to enter now", "fonts/Marker Felt.ttf", 32);
            label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
            this->addChild(label, 5);

            // 2����Ƴ���ʾ
            label->runAction(Sequence::create(
                DelayTime::create(2.0f),
                RemoveSelf::create(),
                nullptr
            ));

            return;
        }

        // ���õ�ǰѡ�еĲ�ͷ���
        currentLayer = layerIndex;
        currentRoom = 0;

        // ���Ϊ����ɷ���
        roomCompleted = true;

        // �л�����Ϣ����
        auto scene = Rest::createScene();
        Director::getInstance()->replaceScene(scene);
    }

    void Map::menuBossCallback(Ref* pSender)
    {
        auto item = static_cast<MenuItemImage*>(pSender);
        int layerIndex = item->getTag();

        // ����Ƿ���Է�������㼶
        if (layerIndex > maxAccessibleLayer)
        {
            // ������ʾ��ǩ
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto label = Label::createWithTTF("Unable to enter now", "fonts/Marker Felt.ttf", 32);
            label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
            this->addChild(label, 5);

            // 2����Ƴ���ʾ
            label->runAction(Sequence::create(
                DelayTime::create(2.0f),
                RemoveSelf::create(),
                nullptr
            ));

            return;
        }

        // ���õ�ǰѡ�еĲ�ͷ���
        currentLayer = layerIndex;
        currentRoom = 0;

        // ���Ϊ����ɷ���
        roomCompleted = true;

        // �л��� BOSS ս������
        auto scene = FightingScene::createScene();
        Director::getInstance()->replaceScene(scene);
    }

} // namespace MyGame
