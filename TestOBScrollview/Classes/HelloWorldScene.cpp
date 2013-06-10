#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

using namespace cocos2d;
using namespace cocos2d::extension;
using namespace CocosDenshion;

CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
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
    if ( !CCLayer::init() )
    {
        return false;
    }

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    CCMenuItemImage *pCloseItem = CCMenuItemImage::create(
                                        "CloseNormal.png",
                                        "CloseSelected.png",
                                        this,
                                        menu_selector(HelloWorld::menuCloseCallback) );
    pCloseItem->setPosition( ccp(CCDirector::sharedDirector()->getWinSize().width - 20, 20) );

    // create menu, it's an autorelease object
    CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
    pMenu->setPosition( CCPointZero );
    this->addChild(pMenu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    CCLabelTTF* pLabel = CCLabelTTF::create("Hello World", "Thonburi", 34);

    // ask director the window size
    CCSize size = CCDirector::sharedDirector()->getWinSize();

    // position the label on the center of the screen
    pLabel->setPosition( ccp(size.width / 2, size.height - 20) );

    // add the label as a child to this layer
    this->addChild(pLabel, 1);

    // add "HelloWorld" splash screen"
    CCSprite* pSprite = CCSprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    pSprite->setPosition( ccp(size.width/2, size.height/2) );

    // add the sprite as a child to this layer
    this->addChild(pSprite, 0);
    
    testBothDirectionScrollview();
    
    return true;
}

void HelloWorld::menuCloseCallback(CCObject* pSender)
{
    CCDirector::sharedDirector()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

#define verticalScrollerWidth  20 
#define horizontalScrollerHeight  20 
CCNode* HelloWorld::createBothDirectionContainer(CCSize& viewSize)
{
    CCTexture2D* texture = CCTextureCache::sharedTextureCache()->addImage("item.png");
    CCSize itemSize = texture->getContentSize();
    CCSize itemSizeWithMargin = CCSizeMake((int)(itemSize.width*9/8), (int)(itemSize.height*9/8));
    
    const int row = 8;
    const int column = 8;
    
    CCNode* container = CCNode::create();
    CCSize containerSize = CCSizeMake(column*itemSizeWithMargin.width+verticalScrollerWidth,
                                      row*itemSizeWithMargin.height+horizontalScrollerHeight);
    container->setContentSize(containerSize);
    
    CCSprite* item = NULL;
    CCLabelTTF* lable = NULL;
    int posX = 0;
    int posY = horizontalScrollerHeight + itemSizeWithMargin.height;
    
    char text[64] = {0};
    int tag = 0;
    for(int i = 0; i< column; ++i)
    {
        for (int j = 0; j < row; ++j)
        {
            tag = i*row +j+1;
            item = new CCSprite();
            item->initWithTexture(texture);
            item->setAnchorPoint(ccp(0, 1));
            item->setPosition(ccp(posX, posY));
            container->addChild(item,2,tag);
            
            sprintf(text, "%d",tag);
            lable = new CCLabelTTF();
            lable->initWithString(text, "Thonburi", 20);
            lable->setColor(ccBLACK);
            lable->setAnchorPoint(ccp(0.5f,0.5f));
            lable->setPosition(ccp(itemSize.width/2, itemSize.height/2));
            item->addChild(lable);
            
            lable->release();
            item->release();
            
            posY += itemSizeWithMargin.height;
        }
        
        posY = horizontalScrollerHeight + itemSizeWithMargin.height;
        posX += itemSizeWithMargin.width;
    }
    
    viewSize = CCSizeMake(4*itemSizeWithMargin.width + verticalScrollerWidth, 4*itemSizeWithMargin.height + horizontalScrollerHeight);
    return container;
    
}

void HelloWorld::testBothDirectionScrollview()
{
    CCSize viewSize;
    CCNode* container = createBothDirectionContainer(viewSize);
    OBScrollView* scrollView = OBScrollView::create(viewSize,container);
    scrollView->setDirection(kOBScrollViewDirectionBoth);
    scrollView->setDelegate(this);
    scrollView->setPosition( ccp(CCDirector::sharedDirector()->getWinSize().width / 4,
                                 CCDirector::sharedDirector()->getWinSize().height/ 6));
    this->addChild(scrollView);
    scrollView->setContentOffset(ccp(0,scrollView->minContainerOffset().y));
    
    OBScroller* scroller = new OBScroller(kOBScrollViewDirectionVertical,CCSizeMake(verticalScrollerWidth, viewSize.height));
    scroller->setSpriteScroller("scroller9.png",CCRectMake(0,0,20,40),CCRectMake(6, 6, 6, 6));
    //    scroller->setSpriteScroller("Scroller.png");
    scrollView->setScroller(scroller);
    scroller->release();
    
    scroller = new OBScroller(kOBScrollViewDirectionHorizontal,CCSizeMake(viewSize.width, horizontalScrollerHeight));
    scroller->setSpriteScroller("horizontalScroller9.png",CCRectMake(0,0,40,20),CCRectMake(6, 6, 6, 6));
    //    scroller->setSpriteScroller("horizontalScroller.png");
    scrollView->setScroller(scroller);
    scroller->release();
    
}


void HelloWorld::scrollViewDidClick(CCNode* clickNode)
{
    CCLOG("click node id = %d",clickNode->getTag());
    CCTexture2D* texture1 = CCTextureCache::sharedTextureCache()->addImage("item.png");
    CCTexture2D* texture2 = CCTextureCache::sharedTextureCache()->addImage("itemS.png");
    CCSprite* item = (CCSprite*)clickNode;
    if (item->getTexture() == texture1) {
        item->setTexture(texture2);
    }
    else{
        item->setTexture(texture1);
    }    
}

