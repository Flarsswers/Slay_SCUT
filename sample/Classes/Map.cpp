// Map.cpp
#include "Map.h"
#include "FightingScene.h"
#include "question.h"
#include "Rest.h"
#include <random>

USING_NS_CC;

namespace MyGame {

    // 存储地图数据（房间类型和位置信息，但不存储指针）
    struct RoomInfo {
        RoomType type;
        cocos2d::Vec2 position;
    };
    static std::vector<std::vector<RoomInfo>> staticMapInfo;

    // 存储连线数据
    static std::vector<ConnectionInfo> staticConnectionInfo;

    // 存储当前可访问的最高层级（静态变量，在场景切换时保持）
    static int maxAccessibleLayer = 0;

    // 是否刚刚完成了一个房间（用于从房间场景返回地图时判断）
    static bool roomCompleted = false;

    // 记录当前选择的房间位置（用于判断下一层哪些房间可以访问）
    static cocos2d::Vec2 currentRoomPosition;

    // 添加重置游戏状态的函数实现
    void resetGameState() {
        // 清空地图信息
        staticMapInfo.clear();

        // 清空连线信息
        staticConnectionInfo.clear();

        // 重置当前房间位置
        currentRoomPosition = Vec2::ZERO;

        // 重置可访问层级
        maxAccessibleLayer = 0;

        // 重置房间完成状态
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

        // 初始化当前层和当前房间
        currentLayer = 0;
        currentRoom = -1;

        // 创建 ScrollView
        scrollView = ui::ScrollView::create();
        scrollView->setDirection(ui::ScrollView::Direction::VERTICAL);
        scrollView->setContentSize(visibleSize);
        scrollView->setInnerContainerSize(Size(visibleSize.width, visibleSize.height * 3));  // 设置滚动范围
        scrollView->setPosition(origin);
        this->addChild(scrollView);

        // 添加地图背景图片
        auto background = Sprite::create("map_background.png");
        if (background)
        {
            background->setPosition(Vec2(visibleSize.width / 2, scrollView->getInnerContainerSize().height / 2));
            scrollView->addChild(background, 0);  // 将背景图片添加到最底层
        }

        // 检查是否已经生成了地图数据
        if (staticMapInfo.empty())
        {
            // 生成多层地图
            int layers = 18;  // 假设有10层
            int roomsPerLayer = 5;  // 每层有3个房间
            generateRandomMap(layers, roomsPerLayer);
        }
        else
        {
            // 使用已存储的地图数据重新生成地图
            generateMapFromSavedInfo();
        }

        // 如果刚刚完成了一个房间，增加可访问的层级
        if (roomCompleted)
        {
            maxAccessibleLayer++;
            roomCompleted = false;
        }

        // 更新房间的可访问性
        updateRoomAccessibility();

        // 添加层级提示标签
        createLayerLabels();

        return true;
    }

    // 创建层级标签
    void Map::createLayerLabels()
    {
        auto visibleSize = Director::getInstance()->getVisibleSize();

        // 添加当前可访问层级提示
        auto accessLabel = Label::createWithTTF(
            "可访问层级: " + std::to_string(maxAccessibleLayer + 1),
            "fonts/Marker Felt.ttf",
            32
        );
        accessLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 50));
        this->addChild(accessLabel, 3);
    }

    // 更新房间的可访问性
    void Map::updateRoomAccessibility()
    {
        // 先将所有房间设为禁用状态
        for (int i = 0; i < mapLayers.size(); i++)
        {
            for (auto& room : mapLayers[i])
            {
                room.item->setEnabled(false);
                room.item->setColor(Color3B(100, 100, 100)); // 灰色
            }
        }

        // 特殊情况：游戏开始时，第一层所有房间都可访问
        if (maxAccessibleLayer == 0 && currentRoomPosition == Vec2::ZERO)
        {
            for (auto& room : mapLayers[0])
            {
                room.item->setEnabled(true);
                room.item->setColor(Color3B(255, 255, 255)); // 正常颜色
            }
            return;
        }

        // 仅启用可访问层级中通过路径连接的房间
        int accessibleLayer = maxAccessibleLayer;

        // 如果当前没有选中房间，但已经通过了一些层级
        if (currentRoomPosition == Vec2::ZERO && maxAccessibleLayer > 0) {
            // 启用新解锁层的所有房间
            for (auto& room : mapLayers[maxAccessibleLayer]) {
                room.item->setEnabled(true);
                room.item->setColor(Color3B(255, 255, 255)); // 正常颜色
            }
            return;
        }

        // 遍历连线信息，找出从当前房间位置可以到达的下一层房间
        for (auto& connection : staticConnectionInfo) {
            // 检查连线的起点是否与当前房间位置匹配
            if (connection.start.equals(currentRoomPosition)) {
                // 找到该连线终点所对应的房间
                for (auto& room : mapLayers[maxAccessibleLayer]) {
                    // 如果房间位置与连线终点匹配，则启用该房间
                    if (room.item->getPosition().equals(connection.end)) {
                        room.item->setEnabled(true);
                        room.item->setColor(Color3B(255, 255, 255)); // 正常颜色
                    }
                }
            }
        }
    }

    RoomType Map::getRandomRoomType()
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());

        // 更新概率分布，添加ELITE类型
        // START(0), BATTLE(1), QUESTION(2), REST(3), BOSS(4), ELITE(5)
        static std::discrete_distribution<> dis({ 0, 45, 30, 10, 0, 15 });  // 给ELITE 15%的概率

        // 获取随机值并转换为RoomType
        int randomType = dis(gen);
        return static_cast<RoomType>(randomType);
    }

    void Map::generateRandomMap(int layers, int roomsPerLayer)
    {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        // 随机数生成器
        static std::random_device rd;
        static std::mt19937 gen(rd());

        // 添加开始点图标
        auto startItem = MenuItemImage::create(
            "start.png",  // 使用特殊图案
            "start_selected.png",
            [](Ref*) {}  // 开始点不需要回调函数
        );
        if (startItem)
        {
            startItem->setPosition(Vec2(
                visibleSize.width / 2,
                scrollView->getInnerContainerSize().height - 100  // 设置开始点位置
            ));
            startItem->setScale(0.5);  // 调整图标大小
            auto menu = Menu::create(startItem, nullptr);
            menu->setPosition(Vec2::ZERO);
            scrollView->addChild(menu, 1);  // 确保图标在背景图片之上
        }

        // 清空数据
        mapLayers.clear();
        staticMapInfo.clear();

        // 计算最小房间数
        int minRooms = std::max(3, roomsPerLayer - 2); // 最低数量为3或设定值-2

        for (int i = 0; i < layers; ++i)
        {
            std::vector<Room> layer;
            std::vector<RoomInfo> layerInfo;  // 存储当前层的房间信息
            int roomCount;  // 每层的房间数量

            // 根据不同层级设置不同的房间数量和类型
            if (i == layers - 1) {
                // 最后一层（Boss层）只有1个房间
                roomCount = 1;
            }
            else if (i == layers - 2) {
                // Boss层前一层至少有3个房间（都是休息房间）
                roomCount = roomsPerLayer;
            }
            else {
                // 其他层随机生成房间数量：最小值到roomsPerLayer之间
                std::uniform_int_distribution<> roomDist(minRooms, roomsPerLayer);
                roomCount = roomDist(gen);
            }

            for (int j = 0; j < roomCount; ++j)
            {
                RoomType roomType;

                // 根据不同层级设置不同的房间类型
                if (i == layers - 1) {
                    roomType = RoomType::BOSS;  // 最后一层是 BOSS 房间
                }
                else if (i == layers - 2) {
                    roomType = RoomType::REST;  // Boss层前一层全部是休息房间
                }
                else {
                    roomType = getRandomRoomType();  // 其他层随机生成房间类型
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
                        "boss.png",  // 使用特殊图案
                        "boss_selected.png",
                        CC_CALLBACK_1(Map::menuBossCallback, this)
                    );
                    break;

                case RoomType::ELITE:
                    roomItem = MenuItemImage::create(
                        "elite_normal.png",  // 需要准备这些资源图片
                        "elite_selected.png",
                        CC_CALLBACK_1(Map::menuEliteCallback, this)
                    );
                    break;
                }
                if (roomItem)
                {
                    // 根据房间数量调整位置
                    Vec2 position;
                    if (roomCount == 1) {
                        // 如果只有一个房间，放在中间
                        position = Vec2(
                            visibleSize.width / 2,
                            scrollView->getInnerContainerSize().height - (i + 1) * 200  // 根据层数调整位置
                        );
                    }
                    else {
                        // 根据房间数量均匀分布
                        float spacing = visibleSize.width / (roomCount + 1);
                        position = Vec2(
                            origin.x + spacing * (j + 1),
                            scrollView->getInnerContainerSize().height - (i + 1) * 200  // 根据层数调整位置
                        );
                    }

                    roomItem->setPosition(position);
                    roomItem->setScale(0.5);  // 调整按钮大小
                    // 存储层级信息
                    roomItem->setTag(i);

                    auto menu = Menu::create(roomItem, nullptr);
                    menu->setPosition(Vec2::ZERO);
                    scrollView->addChild(menu, 1);  // 确保菜单在背景图片之上

                    layer.push_back({ roomType, roomItem });

                    // 保存位置信息以便后续重建
                    layerInfo.push_back({ roomType, position });
                }
            }
            mapLayers.push_back(layer);
            staticMapInfo.push_back(layerInfo);  // 存储该层信息
        }

        connectRooms(mapLayers);
    }


    void Map::generateMapFromSavedInfo()
    {
        // 清空现有地图数据
        mapLayers.clear();

        // 根据保存的地图信息重新创建房间
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
                        "boss.png",  // 使用特殊图案
                        "boss_selected.png",
                        CC_CALLBACK_1(Map::menuBossCallback, this)
                    );
                    break;
                case RoomType::ELITE:
                    roomItem = MenuItemImage::create(
                        "elite_normal.png",  // 需要准备这些资源图片
                        "elite_selected.png",
                        CC_CALLBACK_1(Map::menuEliteCallback, this)
                    );
                    break;
                }

                if (roomItem)
                {
                    roomItem->setPosition(roomInfo.position);
                    roomItem->setScale(0.5);  // 调整按钮大小
                    // 存储层级信息
                    roomItem->setTag(i);

                    auto menu = Menu::create(roomItem, nullptr);
                    menu->setPosition(Vec2::ZERO);
                    scrollView->addChild(menu, 1);  // 确保菜单在背景图片之上

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
        scrollView->addChild(drawNode, 2);  // 确保连接线在按钮之上

        // 如果已经有保存的连线信息，直接使用
        if (!staticConnectionInfo.empty()) {
            for (const auto& connection : staticConnectionInfo) {
                drawNode->drawLine(connection.start, connection.end, Color4F::WHITE);
            }
            return;
        }

        // 清空连线信息以便重新生成
        staticConnectionInfo.clear();

        for (size_t i = 0; i < map.size() - 1; ++i)
        {
            const auto& currentLayer = map[i];
            const auto& nextLayer = map[i + 1];

            if (nextLayer.empty() || currentLayer.empty()) {
                continue;  // 确保层不为空
            }

            // 处理BOSS层的特殊情况（只有一个房间）
            if (nextLayer.size() == 1) {
                // 所有当前层房间都连接到下一层的BOSS房间
                auto bossPos = nextLayer[0].item->getPosition();
                for (const auto& currentRoom : currentLayer) {
                    auto startPos = currentRoom.item->getPosition();
                    drawNode->drawLine(startPos, bossPos, Color4F::WHITE);
                    // 保存连线信息
                    staticConnectionInfo.push_back({ startPos, bossPos });
                }
                continue;
            }

            // 记录下一层房间已连接状态
            std::vector<bool> nextRoomConnected(nextLayer.size(), false);

            // 为每个当前层的房间创建连接，允许跨越一个房间
            for (size_t j = 0; j < currentLayer.size(); j++)
            {
                auto startPos = currentLayer[j].item->getPosition();

                // 确定当前房间在下一层可能连接的房间索引范围
                // 允许横向跨越一个房间，但不能跨越两个
                int maxConnections = 2; // 限制每个房间最多连接下一层的房间数
                int connectionsCount = 0;

                // 根据当前层和下一层的房间数量关系确定连接策略
                if (nextLayer.size() <= 2) {
                    // 如果下一层房间少于3个，所有房间都可以互相连接
                    for (size_t k = 0; k < nextLayer.size() && connectionsCount < maxConnections; k++) {
                        auto endPos = nextLayer[k].item->getPosition();
                        drawNode->drawLine(startPos, endPos, Color4F::WHITE);
                        // 保存连线信息
                        staticConnectionInfo.push_back({ startPos, endPos });
                        nextRoomConnected[k] = true;
                        connectionsCount++;
                    }
                }
                else {
                    // 为每个房间找到"合理"的连接位置
                    // 首先尝试正下方的位置索引
                    int baseIndex;

                    if (currentLayer.size() <= nextLayer.size()) {
                        // 当前层房间少于等于下一层
                        baseIndex = j * nextLayer.size() / currentLayer.size();
                    }
                    else {
                        // 当前层房间多于下一层
                        baseIndex = j * nextLayer.size() / currentLayer.size();
                        baseIndex = std::min(baseIndex, (int)nextLayer.size() - 1);
                    }

                    // 可连接范围：允许跨越一个房间
                    int minIndex = std::max(0, baseIndex - 1); // 往左最多一个
                    int maxIndex = std::min((int)nextLayer.size() - 1, baseIndex + 1); // 往右最多一个

                    // 随机连接范围内的1-2个房间
                    std::vector<int> connectIndexes;

                    // 首先总是连接正下方或最接近的房间
                    connectIndexes.push_back(baseIndex);
                    nextRoomConnected[baseIndex] = true;
                    connectionsCount++;

                    // 50%几率添加第二个连接
                    if (std::rand() % 2 == 0 && connectionsCount < maxConnections) {
                        // 在剩余范围内随机选择
                        std::vector<int> remainingIndexes;
                        for (int k = minIndex; k <= maxIndex; k++) {
                            if (k != baseIndex) { // 不包括已经连接的baseIndex
                                remainingIndexes.push_back(k);
                            }
                        }

                        if (!remainingIndexes.empty()) {
                            int randomIdx = remainingIndexes[std::rand() % remainingIndexes.size()];
                            connectIndexes.push_back(randomIdx);
                            nextRoomConnected[randomIdx] = true;
                            connectionsCount++;
                        }
                    }

                    // 绘制所有连接
                    for (int idx : connectIndexes) {
                        auto endPos = nextLayer[idx].item->getPosition();
                        drawNode->drawLine(startPos, endPos, Color4F::WHITE);
                        // 保存连线信息
                        staticConnectionInfo.push_back({ startPos, endPos });
                    }
                }
            }

            // 确保所有下层房间都有连接
            for (size_t j = 0; j < nextLayer.size(); j++) {
                if (!nextRoomConnected[j]) {
                    // 找到相对位置最近的上层房间
                    int closestRoomIndex = 0;
                    float minDistance = FLT_MAX;

                    for (size_t k = 0; k < currentLayer.size(); k++) {
                        float xDist = currentLayer[k].item->getPosition().x - nextLayer[j].item->getPosition().x;
                        float dist = fabs(xDist);  // 只考虑水平距离

                        if (dist < minDistance) {
                            minDistance = dist;
                            closestRoomIndex = k;
                        }
                    }

                    auto startPos = currentLayer[closestRoomIndex].item->getPosition();
                    auto endPos = nextLayer[j].item->getPosition();
                    drawNode->drawLine(startPos, endPos, Color4F::WHITE);
                    // 保存连线信息
                    staticConnectionInfo.push_back({ startPos, endPos });
                    nextRoomConnected[j] = true;
                }
            }
        }
    }

    void Map::menuBattleCallback(Ref* pSender)
    {
        auto item = static_cast<MenuItemImage*>(pSender);
        int layerIndex = item->getTag();

        // 检查是否可以访问这个层级
        if (layerIndex > maxAccessibleLayer)
        {
            // 创建提示标签
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto label = Label::createWithTTF("unable to enter now", "fonts/Marker Felt.ttf", 32);
            label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
            this->addChild(label, 5);

            // 2秒后移除提示
            label->runAction(Sequence::create(
                DelayTime::create(2.0f),
                RemoveSelf::create(),
                nullptr
            ));

            return;
        }

        // 设置当前选中的层和房间
        currentLayer = layerIndex;
        currentRoom = 0;

        // 记录当前房间位置
        currentRoomPosition = item->getPosition();

        // 标记为已完成房间
        roomCompleted = true;

        // 切换到战斗场景
        auto scene = FightingScene::createScene();
        Director::getInstance()->replaceScene(scene);
    }

    void Map::menuQuestionCallback(Ref* pSender)
    {
        auto item = static_cast<MenuItemImage*>(pSender);
        int layerIndex = item->getTag();

        // 检查是否可以访问这个层级
        if (layerIndex > maxAccessibleLayer)
        {
            // 创建提示标签
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto label = Label::createWithTTF("unable to enter now", "fonts/Marker Felt.ttf", 32);
            label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
            this->addChild(label, 5);

            // 2秒后移除提示
            label->runAction(Sequence::create(
                DelayTime::create(2.0f),
                RemoveSelf::create(),
                nullptr
            ));

            return;
        }

        // 设置当前选中的层和房间
        currentLayer = layerIndex;
        currentRoom = 0;

        // 记录当前房间位置
        currentRoomPosition = item->getPosition();

        // 标记为已完成房间
        roomCompleted = true;

        // 切换到问号场景
        auto scene = Question::createScene();
        Director::getInstance()->replaceScene(scene);
    }

    void Map::menuRestCallback(Ref* pSender)
    {
        auto item = static_cast<MenuItemImage*>(pSender);
        int layerIndex = item->getTag();

        // 检查是否可以访问这个层级
        if (layerIndex > maxAccessibleLayer)
        {
            // 创建提示标签
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto label = Label::createWithTTF("unable to enter now", "fonts/Marker Felt.ttf", 32);
            label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
            this->addChild(label, 5);

            // 2秒后移除提示
            label->runAction(Sequence::create(
                DelayTime::create(2.0f),
                RemoveSelf::create(),
                nullptr
            ));

            return;
        }

        // 设置当前选中的层和房间
        currentLayer = layerIndex;
        currentRoom = 0;

        // 记录当前房间位置
        currentRoomPosition = item->getPosition();

        // 标记为已完成房间
        roomCompleted = true;

        // 切换到休息场景
        auto scene = Rest::createScene();
        Director::getInstance()->replaceScene(scene);
    }

    void Map::menuBossCallback(Ref* pSender)
    {
        auto item = static_cast<MenuItemImage*>(pSender);
        int layerIndex = item->getTag();

        // 检查是否可以访问这个层级
        if (layerIndex > maxAccessibleLayer)
        {
            // 创建提示标签
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto label = Label::createWithTTF("Unable to enter now", "fonts/Marker Felt.ttf", 32);
            label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
            this->addChild(label, 5);

            // 2秒后移除提示
            label->runAction(Sequence::create(
                DelayTime::create(2.0f),
                RemoveSelf::create(),
                nullptr
            ));

            return;
        }

        // 设置当前选中的层和房间
        currentLayer = layerIndex;
        currentRoom = 0;

        // 记录当前房间位置
        currentRoomPosition = item->getPosition();

        // 标记为已完成房间
        roomCompleted = true;

        // 切换到 BOSS 战斗场景
        auto scene = FightingScene::createScene();
        Director::getInstance()->replaceScene(scene);
    }

    void Map::menuEliteCallback(Ref* pSender)
    {
        auto item = static_cast<MenuItemImage*>(pSender);
        int layerIndex = item->getTag();

        // 检查是否可以访问这个层级
        if (layerIndex > maxAccessibleLayer)
        {
            // 创建提示标签
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto label = Label::createWithTTF("无法进入此房间", "fonts/Marker Felt.ttf", 32);
            label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
            this->addChild(label, 5);

            // 2秒后移除提示
            label->runAction(Sequence::create(
                DelayTime::create(2.0f),
                RemoveSelf::create(),
                nullptr
            ));

            return;
        }

        // 设置当前选中的层和房间
        currentLayer = layerIndex;
        currentRoom = 0;

        // 记录当前房间位置
        currentRoomPosition = item->getPosition();

        // 标记为已完成房间
        roomCompleted = true;

        // 切换到战斗场景 - 使用与其他回调相同的方式
        auto scene = FightingScene::createScene();
        Director::getInstance()->replaceScene(scene);
    }

}