#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "OBScrollView/OBScrollView.h"

using namespace cocos2d::extension;

class HelloWorld : public cocos2d::CCLayer,
                   public OBScrollViewDelegate
{
public:
    // Method 'init' in cocos2d-x returns bool, instead of 'id' in cocos2d-iphone (an object pointer)
    virtual bool init();

    // there's no 'id' in cpp, so we recommend to return the class instance pointer
    static cocos2d::CCScene* scene();
    
    // a selector callback
    void menuCloseCallback(CCObject* pSender);

    // preprocessor macro for "static create()" constructor ( node() deprecated )
    CREATE_FUNC(HelloWorld);
    
public:
    virtual void scrollViewDidScroll(OBScrollView* view) {};
    virtual void scrollViewDidZoom(OBScrollView* view) {};
    virtual void scrollViewDidClick(CCNode* clickNode);

private:    
    CCNode* createBothDirectionContainer(cocos2d::CCSize& viewSize);
    void testBothDirectionScrollview();
};

#endif // __HELLOWORLD_SCENE_H__
