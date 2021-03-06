/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"
#include "AppMacros.h"
#include "ui/UITextField.h"
#include "ui/UIWebView.h"

USING_NS_CC;


Scene* HelloWorld::scene()
{
     return HelloWorld::create();
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
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

	auto label = Label::createWithSystemFont("abcda123", "arial", TITLE_FONT_SIZE);

    // position the label on the center of the screen
    label->setPosition(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height);
	label->enableOutline(Color4B(255, 0, 0, 255), 1);

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    //auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    //sprite->setPosition(Vec2(visibleSize / 2) + origin);

    // add the sprite as a child to this layer
    //this->addChild(sprite);

	auto textField = ui::TextField::create();
	//textField->setFontName("fonts/HKYuanMini.ttf"); 
	//textField->setFontName("����");
	textField->setCursorEnabled(true);
	textField->setTextHorizontalAlignment(TextHAlignment::LEFT);
	textField->setFontSize(10);

	// position the sprite on the center of the screen
	textField->setPosition(Vec2(visibleSize / 2) + origin);

	// add the sprite as a child to this layer
	this->addChild(textField);

	auto textField1 = ui::TextField::create();
	//textField->setFontName("fonts/HKYuanMini.ttf"); 
	textField1->setFontName("����");
	textField1->setCursorEnabled(true);
	textField1->setTextHorizontalAlignment(TextHAlignment::CENTER);
	textField1->ignoreContentAdaptWithSize(false);
	textField1->setContentSize(Size(100, 20));

	// position the sprite on the center of the screen
	textField1->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 4) + origin);

	// add the sprite as a child to this layer
	this->addChild(textField1);

    auto drawNode = DrawNode::create();
    drawNode->setPosition(Vec2(0, 0));
    addChild(drawNode);

    Rect safeArea = Director::getInstance()->getSafeAreaRect();
    drawNode->drawRect(safeArea.origin, safeArea.origin + safeArea.size, Color4F::BLUE);

	auto webView = experimental::ui::WebView::create();
	webView->setContentSize(Size(200, 100));
	webView->setPosition(Vec2(visibleSize.width / 4, visibleSize.height / 2) + origin);
	addChild(webView);

	webView->loadURL("http://www.k7.cn");

	auto webView2 = experimental::ui::WebView::create();
	webView2->setContentSize(Size(200, 100));
	webView2->setPosition(Vec2(visibleSize.width * 3 / 4, visibleSize.height / 2) + origin);
	addChild(webView2);

	webView2->loadURL("http://www.baidu.com");

    return true;
}

void HelloWorld::menuCloseCallback(Ref* sender)
{
    Director::getInstance()->end();
}
