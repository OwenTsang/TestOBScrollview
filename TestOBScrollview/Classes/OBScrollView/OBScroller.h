
#ifndef __OBScroller_H__
#define __OBScroller_H__

#include "cocos2d.h"
#include "cocos-ext.h"

NS_CC_EXT_BEGIN

typedef enum {
	kOBScrollViewDirectionNone = -1,
    kOBScrollViewDirectionHorizontal = 0,
    kOBScrollViewDirectionVertical,
    kOBScrollViewDirectionBoth
} OBScrollViewDirection;

class OBScrollView;
/**
 * OBCCScroller support for cocos2d for iphone.
 * It provides scroll view functionalities to cocos2d projects natively.
 */
class OBScroller : public CCNode
{
public:
    OBScroller(OBScrollViewDirection eDirection,const CCSize& contentSize);
    virtual ~OBScroller();
    
    OBScrollViewDirection getDirection(){    return m_eDirection;   }
    
    void setSpriteScroller(const char* scrollerFile, CCRect rect,  CCRect capInsets);
    
    void setSpriteScroller(const char* scrollerFile);
//    void setColorScroller(ccColor3B color);
    
    void setScrollerBg(const char* bgFile);
    
    GLubyte getOpacity()
    {
        if(m_sScroller)
            m_sScroller->getOpacity();
        return 0;
    }
    void setOpacity(GLubyte opacity)
    {
        if(m_sScroller)
            m_sScroller->setOpacity(opacity);
    }
    
private:
    void updateScrollerSize(const CCSize& scale);
    
    void updateOffset(const CCPoint& scale,float duration = 0.0f);
    
private:
    CCSprite*               m_sBg;
    CCNodeRGBA*             m_sScroller;
    OBScrollViewDirection   m_eDirection;
    friend class            OBScrollView;
};


NS_CC_EXT_END

#endif /* __OBScroller_H__ */
