/****************************************************************************
 Copyright (c) 2012 cocos2d-x.org
 Copyright (c) 2010 Sangwoo Im
 
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

#include "OBScroller.h"

NS_CC_EXT_BEGIN

OBScroller::OBScroller(OBScrollViewDirection eDirection,const CCSize& contentSize)
:CCNode()
,m_sBg(NULL)
,m_sScroller(NULL)
,m_eDirection(eDirection)
{
    CCAssert(m_eDirection == kOBScrollViewDirectionVertical
             || m_eDirection == kOBScrollViewDirectionHorizontal, "OBScroller::setDirection error");
    setContentSize(contentSize);
}

OBScroller::~OBScroller()
{
    
}

void OBScroller::setSpriteScroller(const char* scrollerFile, CCRect rect,  CCRect capInsets)
{
    CCAssert(scrollerFile, "scrollerFile error");
    if(m_sScroller)
    {
        m_sScroller->removeFromParent();
    }
    m_sScroller = CCScale9Sprite::create(scrollerFile,rect,capInsets);
    m_sScroller->setAnchorPoint(CCPointZero);
    m_sScroller->setPosition(CCPointZero);
    this->addChild(m_sScroller,1);
}

void OBScroller::setSpriteScroller(const char* scrollerFile)
{
    CCAssert(scrollerFile, "scrollerFile error");
    if(m_sScroller)
    {
        m_sScroller->removeFromParent();
    }
    m_sScroller = CCSprite::create(scrollerFile);
    m_sScroller->setAnchorPoint(CCPointZero);
    m_sScroller->setPosition(CCPointZero);
    this->addChild(m_sScroller,1);
}

void OBScroller::setScrollerBg(const char* bgFile)
{
    CCAssert(bgFile, "bgFile error");
    if(m_sBg)
    {
        m_sBg->removeFromParent();
    }
    m_sBg = CCSprite::create(bgFile);
    m_sBg->setAnchorPoint(CCPointZero);
    m_sBg->setPosition(CCPointZero);
    this->addChild(m_sBg);
}

void OBScroller::updateScrollerSize(const CCSize& scale)
{
    if (m_sScroller) {
        const CCSize& allSize = getContentSize();
        if (dynamic_cast<CCSprite*>(m_sScroller))
        {
            ((CCSprite*)m_sScroller)->setTextureRect(CCRectMake(0, 0, allSize.width*scale.width, allSize.height*scale.height));
        }
        else //if (dynamic_cast<CCScale9Sprite*>(m_sScroller))
        {
            m_sScroller->setContentSize(CCSizeMake(allSize.width*scale.width, allSize.height*scale.height));
        }
    }
}

void OBScroller::updateOffset(const CCPoint& scale,float duration)
{
    if (m_sScroller)
    {
        const CCSize& allSize = getContentSize();
        const CCSize& scrollerSize = m_sScroller->getContentSize();
        CCPoint targetPos = ccp((allSize.width - scrollerSize.width)*scale.x, (allSize.height - scrollerSize.height)*scale.y);
        if (duration <= 0.0f)
        {
            m_sScroller->setPosition(targetPos);
        }
        else
        {
            m_sScroller->runAction(CCMoveTo::create(duration, targetPos));
        }
    }
}

NS_CC_EXT_END