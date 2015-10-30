#include "HelloWorldScene.h"
#include "AppMacros.h"
#include "ui/CocosGUI.h"//UI相关的头文件
#include"cocostudio/CocoStudio.h"//在CocosStudio.h 头文件中已经包含了Studio所需要的各个头文件(除CocosGUI)因此我们使用Studio仅需要包含他就可以

USING_NS_CC;
using namespace ui;


Scene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    HelloWorld *layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                        "CloseNormal.png",
                                        "CloseSelected.png",
                                        CC_CALLBACK_1(HelloWorld::menuCloseCallback,this));
    
    closeItem->setPosition(origin + Vec2(visibleSize) - Vec2(closeItem->getContentSize() / 2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
    
    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = Label::createWithTTF("Hello World", "fonts/arial.ttf", TITLE_FONT_SIZE);
    
    // position the label on the center of the screen
    label->setPosition(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height);

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize / 2) + origin);

    // add the sprite as a child to this layer
    this->addChild(sprite);

	FileUtils::getInstance()->addSearchPath("TexasPoker");
	FileUtils::getInstance()->addSearchPath("TexasPoker/ui");
	FileUtils::getInstance()->addSearchPath("TexasPoker/images");
	FileUtils::getInstance()->addSearchPath("TexasPoker/images/game");

// 	FileUtils::getInstance()->addSearchPath("TexasPoker/images");
// 	Widget* rootNode = cocostudio::GUIReader::getInstance()->widgetFromJsonFile("main_ui.json");

	Node* rootNode = CSLoader::createNode("main_ui.csb");
	addChild(rootNode);

// 	rootNode = CSLoader::createNode("result_ui.csb");
// 	addChild(rootNode);
	auto layout = (Layout*)rootNode->getChildByName("main_panel");
	cocos2d::ui::Widget* m_pBtnWidget[3];
	for (int i = 0; i < 3; ++i)
	{
		m_pBtnWidget[i] = (cocos2d::ui::Widget*)Helper::seekWidgetByName(layout, StringUtils::format("btns_panel_%d", i));
		m_pBtnWidget[i]->setVisible(false);
	}
	m_pBtnWidget[2]->setVisible(true);

	Button* pBtn = (Button*)Helper::seekWidgetByName(m_pBtnWidget[2], "btn_fold");
	Vec2 pt(0,0);
	if (pBtn)
	{
		pBtn->loadTextureHot("btn_fold_2.png");
		pt = pBtn->getPosition();
	}

// 	m_pEditBk = (Widget*)m_pBtnWidget[2]->getChildByName("edit_bk");
// 	m_pBetEdit = (TextField*)m_pEditBk->getChildByName("edit_bet");
	TextField* m_pBetEdit = (TextField*)Helper::seekWidgetByName(m_pBtnWidget[2], "edit_bet");
	m_pBetEdit->setPlaceHolderColor(Color3B(92, 64, 0));
// 	m_pBetEdit->addEventListener(CC_CALLBACK_2(MainScene::onBetEditChanged, this));

	auto m_pSlider = (Slider*)Helper::seekWidgetByName(m_pBtnWidget[2], "slider_bet");
// 	m_pSlider->addEventListener(CC_CALLBACK_2(MainScene::onSliderBetChanged, this));
    
    return true;
}

void HelloWorld::menuCloseCallback(Ref* sender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
