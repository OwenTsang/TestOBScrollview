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

#include "OBScrollView.h"

NS_CC_EXT_BEGIN

#define SCROLL_DEACCEL_RATE  0.95f
#define SCROLL_DEACCEL_DIST  1.0f
#define BOUNCE_DURATION      0.15f
#define INSET_RATIO          0.2f
#define MOVE_INCH            7.0f/160.0f

/////added By OwenTsang
#define  OB_ACCELERATION   5600
#define  OB_GET_SYMBOL(x)  (x >= 0.0 ? 1.0 : -1.0)
#define  OB_INERTRA_DURATION_RATE   1.5f

static float convertDistanceFromPointToInch(float pointDis)
{
    float factor = ( CCEGLView::sharedOpenGLView()->getScaleX() + CCEGLView::sharedOpenGLView()->getScaleY() ) / 2;
    return pointDis * factor / CCDevice::getDPI();
}


OBScrollView::OBScrollView()
: m_fZoomScale(0.0f)
, m_fMinZoomScale(0.0f)
, m_fMaxZoomScale(0.0f)
, m_pDelegate(NULL)
, m_eDirection(kOBScrollViewDirectionBoth)
, m_bDragging(false)
, m_pContainer(NULL)
, m_bTouchMoved(false)
, m_bBounceable(false)
, m_bClippingToBounds(false)
, m_fTouchLength(0.0f)
, m_pTouches(NULL)
, m_fMinScale(0.0f)
, m_fMaxScale(0.0f)
, m_bInertiaable(true)
, m_beginPosition(CCPointZero)
, m_eScrollingType(kScrollingNone)
, m_horizontalScroller(NULL)
, m_verticalScroller(NULL)
{
}

OBScrollView::~OBScrollView()
{
    m_pTouches->release();
}

OBScrollView* OBScrollView::create(CCSize size, CCNode* container/* = NULL*/)
{
    OBScrollView* pRet = new OBScrollView();
    if (pRet && pRet->initWithViewSize(size, container))
    {
        pRet->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(pRet);
    }
    return pRet;
}

OBScrollView* OBScrollView::create()
{
    OBScrollView* pRet = new OBScrollView();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(pRet);
    }
    return pRet;
}


bool OBScrollView::initWithViewSize(CCSize size, CCNode *container/* = NULL*/)
{
    if (CCLayer::init())
    {
        m_pContainer = container;
        
        if (!this->m_pContainer)
        {
            m_pContainer = CCLayer::create();
            this->m_pContainer->ignoreAnchorPointForPosition(false);
            this->m_pContainer->setAnchorPoint(ccp(0.0f, 0.0f));
        }

        this->setViewSize(size);

        setTouchPriority(kOBScrollViewHandlerPriority);
        setTouchEnabled(true);
        m_pTouches = new CCArray();
        m_pDelegate = NULL;
        m_bBounceable = true;
        m_bClippingToBounds = true;
        m_eDirection  = kOBScrollViewDirectionBoth;
        m_pContainer->setPosition(ccp(0.0f, 0.0f));
        m_fTouchLength = 0.0f;
        
        /////added by owen 
        m_bInertiaable = true;
        m_eScrollingType = kScrollingNone;
        m_horizontalScroller = NULL;
        m_verticalScroller = NULL;

        this->addChild(m_pContainer);
        m_fMinScale = m_fMaxScale = 1.0f;
          
        return true;
    }
    return false;
}

bool OBScrollView::init()
{
    return this->initWithViewSize(CCSizeMake(200, 200), NULL);
}

void OBScrollView::registerWithTouchDispatcher()
{
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, getTouchPriority(), false);
}

bool OBScrollView::isNodeVisible(CCNode* node)
{
    const CCPoint offset = this->getContentOffset();
    const CCSize  size   = this->getViewSize();
    const float   scale  = this->getZoomScale();
    
    CCRect viewRect;
    
    viewRect = CCRectMake(-offset.x/scale, -offset.y/scale, size.width/scale, size.height/scale); 
    
    return viewRect.intersectsRect(node->boundingBox());
}

void OBScrollView::pause(CCObject* sender)
{
    if (m_horizontalScroller) {
        m_horizontalScroller->pauseSchedulerAndActions();
    }
    if (m_verticalScroller) {
        m_verticalScroller->pauseSchedulerAndActions();
    }
    
    m_pContainer->pauseSchedulerAndActions();

    CCObject* pObj = NULL;
    CCArray* pChildren = m_pContainer->getChildren();

    CCARRAY_FOREACH(pChildren, pObj)
    {
        CCNode* pChild = (CCNode*)pObj;
        pChild->pauseSchedulerAndActions();
    }
}

void OBScrollView::resume(CCObject* sender)
{
    CCObject* pObj = NULL;
    CCArray* pChildren = m_pContainer->getChildren();

    CCARRAY_FOREACH(pChildren, pObj)
    {
        CCNode* pChild = (CCNode*)pObj;
        pChild->resumeSchedulerAndActions();
    }

    m_pContainer->resumeSchedulerAndActions();
    
    if (m_horizontalScroller) {
        m_horizontalScroller->resumeSchedulerAndActions();
    }
    if (m_verticalScroller) {
        m_verticalScroller->resumeSchedulerAndActions();
    }
}

void OBScrollView::setTouchEnabled(bool e)
{
    CCLayer::setTouchEnabled(e);
    if (!e)
    {
        m_bDragging = false;
        m_bTouchMoved = false;
        m_pTouches->removeAllObjects();
    }
}

void OBScrollView::setContentOffset(CCPoint offset, bool animated/* = false*/)
{
    if (animated)
    { //animate scrolling
        this->setContentOffsetInDuration(offset, BOUNCE_DURATION,kScrollingBounce);
    }
    else
    { //set the container position directly
        if (!m_bBounceable)
        {
            const CCPoint minOffset = this->minContainerOffset();
            const CCPoint maxOffset = this->maxContainerOffset();
            
            offset.x = MAX(minOffset.x, MIN(maxOffset.x, offset.x));
            offset.y = MAX(minOffset.y, MIN(maxOffset.y, offset.y));
        }
        m_pContainer->setPosition(offset);
        updateScrollerPosition(m_verticalScroller,offset);
        updateScrollerPosition(m_horizontalScroller,offset);
        
        
        if (m_pDelegate != NULL)
        {
            m_pDelegate->scrollViewDidScroll(this);   
        }
    }
}

void OBScrollView::stopByTouchEnd()
{
    if (m_eScrollingType == kScrollingInteria) {
        if (m_verticalScroller) {
            m_verticalScroller->m_sScroller->stopAllActions();
        }
        if (m_horizontalScroller) {
            m_horizontalScroller->m_sScroller->stopAllActions();
        }
        if (m_pContainer) {
            m_pContainer->stopAllActions();
            stoppedAnimatedScroll(m_pContainer);
        }
        
        this->schedule(schedule_selector(OBScrollView::deaccelerateScrolling));   
    }
}

void OBScrollView::setContentOffsetInDuration(CCPoint offset, float dt,EScrollingType scrollingType)
{
    m_eScrollingType = scrollingType;
    
    CCFiniteTimeAction *scroll, *expire;
    
//    scroll = CCMoveTo::create(dt, offset);
    scroll = CCEaseOut::create(CCMoveTo::create(dt, offset), 1) ;
    expire = CCCallFuncN::create(this, callfuncN_selector(OBScrollView::stoppedAnimatedScroll));
    m_pContainer->runAction(CCSequence::create(scroll, expire, NULL));
    this->schedule(schedule_selector(OBScrollView::performedAnimatedScroll));
    
    updateScrollerPosition(m_verticalScroller,offset,dt);
    updateScrollerPosition(m_horizontalScroller,offset,dt);
    
}

CCPoint OBScrollView::getContentOffset()
{
    return m_pContainer->getPosition();
}

void OBScrollView::setZoomScale(float s)
{
    if (m_pContainer->getScale() != s)
    {
        CCPoint oldCenter, newCenter;
        CCPoint center;
        
        if (m_fTouchLength == 0.0f) 
        {
            center = ccp(m_tViewSize.width*0.5f, m_tViewSize.height*0.5f);
            center = this->convertToWorldSpace(center);
        }
        else
        {
            center = m_tTouchPoint;
        }
        
        oldCenter = m_pContainer->convertToNodeSpace(center);
        m_pContainer->setScale(MAX(m_fMinScale, MIN(m_fMaxScale, s)));
        newCenter = m_pContainer->convertToWorldSpace(oldCenter);
        
        const CCPoint offset = ccpSub(center, newCenter);
        if (m_pDelegate != NULL)
        {
            m_pDelegate->scrollViewDidZoom(this);
        }
        this->setContentOffset(ccpAdd(m_pContainer->getPosition(),offset));
    }
}

float OBScrollView::getZoomScale()
{
    return m_pContainer->getScale();
}

void OBScrollView::setZoomScale(float s, bool animated)
{
    if (animated)
    {
        this->setZoomScaleInDuration(s, BOUNCE_DURATION);
    }
    else
    {
        this->setZoomScale(s);
    }
}

void OBScrollView::setZoomScaleInDuration(float s, float dt)
{
    if (dt > 0)
    {
        if (m_pContainer->getScale() != s)
        {
            CCActionTween *scaleAction;
            scaleAction = CCActionTween::create(dt, "zoomScale", m_pContainer->getScale(), s);
            this->runAction(scaleAction);
        }
    }
    else
    {
        this->setZoomScale(s);
    }
}

void OBScrollView::setViewSize(CCSize size)
{
    m_tViewSize = size;
    CCLayer::setContentSize(size);
}

CCNode * OBScrollView::getContainer()
{
    return this->m_pContainer;
}

void OBScrollView::setContainer(CCNode * pContainer)
{
    this->removeAllChildrenWithCleanup(true);

    if (!pContainer) return;

    this->m_pContainer = pContainer;

    this->m_pContainer->ignoreAnchorPointForPosition(false);
    this->m_pContainer->setAnchorPoint(ccp(0.0f, 0.0f));

    this->addChild(this->m_pContainer);

    this->setViewSize(this->m_tViewSize);
}

void OBScrollView::relocateContainer(bool animated)
{
    CCPoint oldPoint, min, max;
    float newX, newY;
    
    min = this->minContainerOffset();
    max = this->maxContainerOffset();
    
    oldPoint = m_pContainer->getPosition();
    
    newX     = oldPoint.x;
    newY     = oldPoint.y;
    if (m_eDirection == kOBScrollViewDirectionBoth || m_eDirection == kOBScrollViewDirectionHorizontal)
    {
        newX     = MAX(newX, min.x);
        newX     = MIN(newX, max.x);
    }
    
    if (m_eDirection == kOBScrollViewDirectionBoth || m_eDirection == kOBScrollViewDirectionVertical)
    {
        newY     = MIN(newY, max.y);
        newY     = MAX(newY, min.y);
    }
    
    if (newY != oldPoint.y || newX != oldPoint.x)
    {
        this->setContentOffset(ccp(newX, newY), animated);
    }
}

CCPoint OBScrollView::maxContainerOffset()
{
    return ccp(0.0f, 0.0f);
}

CCPoint OBScrollView::minContainerOffset()
{
    return ccp(m_tViewSize.width - m_pContainer->getContentSize().width*m_pContainer->getScaleX(), 
               m_tViewSize.height - m_pContainer->getContentSize().height*m_pContainer->getScaleY());
}

void OBScrollView::deaccelerateScrolling(float dt)
{
    if (m_bDragging)
    {
        this->unschedule(schedule_selector(OBScrollView::deaccelerateScrolling));
        return;
    }
    
    float newX, newY;
    CCPoint maxInset, minInset;
    
    m_pContainer->setPosition(ccpAdd(m_pContainer->getPosition(), m_tScrollDistance));

    if (m_bBounceable)
    {
        maxInset = m_fMaxInset;
        minInset = m_fMinInset;
    }
    else
    {
        maxInset = this->maxContainerOffset();
        minInset = this->minContainerOffset();
    }
    
    //check to see if offset lies within the inset bounds
    newX     = MIN(m_pContainer->getPosition().x, maxInset.x);
    newX     = MAX(newX, minInset.x);
    newY     = MIN(m_pContainer->getPosition().y, maxInset.y);
    newY     = MAX(newY, minInset.y);
    
    newX = m_pContainer->getPosition().x;
    newY = m_pContainer->getPosition().y;
    
    m_tScrollDistance     = ccpSub(m_tScrollDistance, ccp(newX - m_pContainer->getPosition().x, newY - m_pContainer->getPosition().y));
    m_tScrollDistance     = ccpMult(m_tScrollDistance, SCROLL_DEACCEL_RATE);
    this->setContentOffset(ccp(newX,newY));
    
    if ((fabsf(m_tScrollDistance.x) <= SCROLL_DEACCEL_DIST &&
         fabsf(m_tScrollDistance.y) <= SCROLL_DEACCEL_DIST) ||
        newY > maxInset.y || newY < minInset.y ||
        newX > maxInset.x || newX < minInset.x ||
        newX == maxInset.x || newX == minInset.x ||
        newY == maxInset.y || newY == minInset.y)
    {
        this->unschedule(schedule_selector(OBScrollView::deaccelerateScrolling));
        this->relocateContainer(true);
    }
}

void OBScrollView::stoppedAnimatedScroll(CCNode * node)
{
    m_eScrollingType = kScrollingNone;
    this->unschedule(schedule_selector(OBScrollView::performedAnimatedScroll));
    // After the animation stopped, "scrollViewDidScroll" should be invoked, this could fix the bug of lack of tableview cells.
    if (m_pDelegate != NULL)
    {
        m_pDelegate->scrollViewDidScroll(this);
    }
}

void OBScrollView::performedAnimatedScroll(float dt)
{
    if (m_bDragging)
    {
        this->unschedule(schedule_selector(OBScrollView::performedAnimatedScroll));
        return;
    }

    if (m_pDelegate != NULL)
    {
        m_pDelegate->scrollViewDidScroll(this);
    }
}


const CCSize& OBScrollView::getContentSize()
{
	return m_pContainer->getContentSize();
}

void OBScrollView::setContentSize(const CCSize & size)
{
    if (this->getContainer() != NULL)
    {
        this->getContainer()->setContentSize(size);
		this->updateInset();
    }
}

void OBScrollView::updateInset()
{
	if (this->getContainer() != NULL)
	{
		m_fMaxInset = this->maxContainerOffset();
		m_fMaxInset = ccp(m_fMaxInset.x + m_tViewSize.width * INSET_RATIO,
			m_fMaxInset.y + m_tViewSize.height * INSET_RATIO);
		m_fMinInset = this->minContainerOffset();
		m_fMinInset = ccp(m_fMinInset.x - m_tViewSize.width * INSET_RATIO,
			m_fMinInset.y - m_tViewSize.height * INSET_RATIO);
	}
}

/**
 * make sure all children go to the container
 */
void OBScrollView::addChild(CCNode * child, int zOrder, int tag)
{
    child->ignoreAnchorPointForPosition(false);
    child->setAnchorPoint(ccp(0.0f, 0.0f));
    if (m_pContainer != child) {
        m_pContainer->addChild(child, zOrder, tag);
    } else {
        CCLayer::addChild(child, zOrder, tag);
    }
}

void OBScrollView::addChild(CCNode * child, int zOrder)
{
    this->addChild(child, zOrder, child->getTag());
}

void OBScrollView::addChild(CCNode * child)
{
    this->addChild(child, child->getZOrder(), child->getTag());
}

/**
 * clip this view so that outside of the visible bounds can be hidden.
 */
void OBScrollView::beforeDraw()
{
    if (m_bClippingToBounds)
    {
		CCRect frame = getViewRect();
        
        glEnable(GL_SCISSOR_TEST);
        
        CCEGLView::sharedOpenGLView()->setScissorInPoints(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
    }
}

/**
 * retract what's done in beforeDraw so that there's no side effect to
 * other nodes.
 */
void OBScrollView::afterDraw()
{
    if (m_bClippingToBounds)
    {
        glDisable(GL_SCISSOR_TEST);
    }
}

void OBScrollView::visit()
{
	// quick return if not visible
	if (!isVisible())
    {
		return;
    }

	kmGLPushMatrix();
	
    if (m_pGrid && m_pGrid->isActive())
    {
        m_pGrid->beforeDraw();
        this->transformAncestors();
    }

	this->transform();
    this->beforeDraw();

	if(m_pChildren)
    {
		ccArray *arrayData = m_pChildren->data;
		unsigned int i=0;
		
		// draw children zOrder < 0
		for( ; i < arrayData->num; i++ )
        {
			CCNode *child =  (CCNode*)arrayData->arr[i];
			if ( child->getZOrder() < 0 )
            {
				child->visit();
			}
            else
            {
				break;
            }
		}
		
		// this draw
		this->draw();
		
		// draw children zOrder >= 0
		for( ; i < arrayData->num; i++ )
        {
			CCNode* child = (CCNode*)arrayData->arr[i];
			child->visit();
		}
        
	}
    else
    {
		this->draw();
    }

    this->afterDraw();
	if ( m_pGrid && m_pGrid->isActive())
    {
		m_pGrid->afterDraw(this);
    }

	kmGLPopMatrix();
}

bool OBScrollView::ccTouchBegan(CCTouch* touch, CCEvent* event)
{
    if (!this->isVisible())
    {
        return false;
    }
    
    CCRect frame = getViewRect();

    //dispatcher does not know about clipping. reject touches outside visible bounds.
    if (m_pTouches->count() > 2 ||
        m_bTouchMoved          ||
        !frame.containsPoint(m_pContainer->convertToWorldSpace(m_pContainer->convertTouchToNodeSpace(touch))))
    {
        return false;
    }
    
    if (m_eScrollingType != kScrollingNone) {
        return true;
    }

    if (!m_pTouches->containsObject(touch))
    {
        m_pTouches->addObject(touch);
    }

    if (m_pTouches->count() == 1)
    { // scrolling
        m_tTouchPoint     = this->convertTouchToNodeSpace(touch);
        m_bTouchMoved     = false;
        m_bDragging     = true; //dragging started
        m_tScrollDistance = ccp(0.0f, 0.0f);
        m_fTouchLength    = 0.0f;
    }
    else if (m_pTouches->count() == 2)
    {
        m_tTouchPoint  = ccpMidpoint(this->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(0)),
                                   this->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(1)));
        m_fTouchLength = ccpDistance(m_pContainer->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(0)),
                                   m_pContainer->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(1)));
        m_bDragging  = false;
    }
    if (m_bInertiaable) {
        m_beginPosition = m_pContainer->getPosition();
        CCTime::gettimeofdayCocos2d(&m_beginTime, NULL);
    }
    return true;
}

void OBScrollView::ccTouchMoved(CCTouch* touch, CCEvent* event)
{
    if (!this->isVisible())
    {
        return;
    }
    if (m_eScrollingType != kScrollingNone) {
        return;
    }

    if (m_pTouches->containsObject(touch))
    {
        if (m_pTouches->count() == 1 && m_bDragging)
        { // scrolling
            CCPoint moveDistance, newPoint, maxInset, minInset;
            CCRect  frame;
            float newX, newY;
            
            frame = getViewRect();

            newPoint     = this->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(0));
            moveDistance = ccpSub(newPoint, m_tTouchPoint);
            
            float dis = 0.0f;
            if (m_eDirection == kOBScrollViewDirectionVertical)
            {
                dis = moveDistance.y;
            }
            else if (m_eDirection == kOBScrollViewDirectionHorizontal)
            {
                dis = moveDistance.x;
            }
            else
            {
                dis = sqrtf(moveDistance.x*moveDistance.x + moveDistance.y*moveDistance.y);
            }

            if (!m_bTouchMoved && fabs(convertDistanceFromPointToInch(dis)) < MOVE_INCH )
            {
                //CCLOG("Invalid movement, distance = [%f, %f], disInch = %f", moveDistance.x, moveDistance.y);
                return;
            }
            
            if (!m_bTouchMoved)
            {
                moveDistance = CCPointZero;
            }
            
            m_tTouchPoint = newPoint;
            m_bTouchMoved = true;
            
            if (frame.containsPoint(this->convertToWorldSpace(newPoint)))
            {
                switch (m_eDirection)
                {
                    case kOBScrollViewDirectionVertical:
                        moveDistance = ccp(0.0f, moveDistance.y);
                        break;
                    case kOBScrollViewDirectionHorizontal:
                        moveDistance = ccp(moveDistance.x, 0.0f);
                        break;
                    default:
                        break;
                }
                
                maxInset = m_fMaxInset;
                minInset = m_fMinInset;

                newX     = m_pContainer->getPosition().x + moveDistance.x;
                newY     = m_pContainer->getPosition().y + moveDistance.y;

                m_tScrollDistance = moveDistance;
                this->setContentOffset(ccp(newX, newY));
            }
        }
        else if (m_pTouches->count() == 2 && !m_bDragging)
        {
            const float len = ccpDistance(m_pContainer->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(0)),
                                            m_pContainer->convertTouchToNodeSpace((CCTouch*)m_pTouches->objectAtIndex(1)));
            this->setZoomScale(this->getZoomScale()*len/m_fTouchLength);
        }
    }
}

void OBScrollView::ccTouchEnded(CCTouch* touch, CCEvent* event)
{
    if (!this->isVisible())
    {
        return;
    }
    if (m_eScrollingType != kScrollingNone) {
        stopByTouchEnd();
        return;
    }
    
    if (m_pTouches->containsObject(touch))
    {
        if (m_pTouches->count() == 1)
        {
            if (m_bTouchMoved) {
                float intertraDuration = judgeInertiaWhenTouchEnded();
                this->schedule(schedule_selector(OBScrollView::deaccelerateScrolling),0.0,kCCRepeatForever,intertraDuration);   
            }
            else
            {
                if (m_pDelegate) {
                    nodeDidClick(m_pContainer->convertTouchToNodeSpace(touch));
                }
            }
        }
        m_pTouches->removeObject(touch);
    }

    if (m_pTouches->count() == 0)
    {
        m_bDragging = false;    
        m_bTouchMoved = false;
    }
}



float OBScrollView::judgeInertiaWhenTouchEnded()
{
    if (! m_bInertiaable) {
        return 0.0f;
    }
    float inertiaDuration = 0.0f;

    CCPoint curPosition = m_pContainer->getPosition();
    CCPoint minOffset = minContainerOffset();
    CCPoint maxOffset = maxContainerOffset();
    
    ////check edge
    if ( curPosition.x < minOffset.x || curPosition.x > maxOffset.x
      || curPosition.y < minOffset.y || curPosition.y > maxOffset.y) {
        return inertiaDuration;
    }
    
    CCPoint delta = ccpSub(m_pContainer->getPosition(), m_beginPosition);
    if (m_eDirection == kOBScrollViewDirectionHorizontal)
    {
        delta.y = 0.0f;
    }
    else if (m_eDirection == kOBScrollViewDirectionVertical)
    {
        delta.x = 0.0f;
    }
    
    if (fabsf(delta.x) > 20.f || fabsf(delta.y) > 20.0f ) /////check position delta
    {
        cc_timeval end;
        CCTime::gettimeofdayCocos2d(&end, NULL);
        float duration = CCTime::timersubCocos2d(&m_beginTime, &end)/1000.0;

        if (duration > 0.05f) ////check time
        {
            CCPoint v = ccp(2*delta.x/duration, 2*delta.y/duration);
            if (fabsf(v.x) > 1000.0f || fabsf(v.y) >1000.0f) {  ////check speed
                
                CCPoint deltaOffset = ccp(OB_GET_SYMBOL(delta.x)*(v.x*v.x/(2*OB_ACCELERATION)),
                                          OB_GET_SYMBOL(delta.y)*(v.y*v.y/(2*OB_ACCELERATION)));
                
                CCPoint offset = ccpAdd(m_pContainer->getPosition(),deltaOffset);
                
                offset.x = MAX(offset.x, minOffset.x - m_tViewSize.width/2);
                offset.y = MAX(offset.y, minOffset.y - m_tViewSize.height/2);
                
                offset.x = MIN(offset.x, maxOffset.x + m_tViewSize.width/2);
                offset.y = MIN(offset.y, maxOffset.y + m_tViewSize.height/2);
                
                float durationRate = OB_INERTRA_DURATION_RATE;
                if (offset.x < minOffset.x || offset.y < minOffset.y
                    || offset.x > maxOffset.x || offset.y > maxOffset.y) { /////bounce happened
                    durationRate = 1.0;
                }
                
                inertiaDuration = durationRate*MAX(fabsf(v.x)/OB_ACCELERATION,fabsf(v.y)/OB_ACCELERATION);
                setContentOffsetInDuration(offset, inertiaDuration,kScrollingInteria);
            }
        }
    }
    return inertiaDuration;
}

void OBScrollView::nodeDidClick(const CCPoint& clickPos)
{
    if (!m_pDelegate) {
        return;
    }
    CCObject* pObj = NULL;
    CCArray* pChildren = m_pContainer->getChildren();
    
    CCARRAY_FOREACH(pChildren, pObj)
    {
        CCNode* pChild = (CCNode*)pObj;
        if(pChild)
        {
            if (pChild->boundingBox().containsPoint(clickPos))
            {
                if (m_verticalScroller != pChild && m_horizontalScroller != pChild) {
                    m_pDelegate->scrollViewDidClick(pChild);
                }
                return;
            }
        }
    }
}

void OBScrollView::ccTouchCancelled(CCTouch* touch, CCEvent* event)
{
    if (!this->isVisible())
    {
        return;
    }
    m_pTouches->removeObject(touch);
    if (m_pTouches->count() == 0)
    {
        m_bDragging = false;
        m_bTouchMoved = false;
    }
}

CCRect OBScrollView::getViewRect()
{
    CCPoint screenPos = this->convertToWorldSpace(CCPointZero);
    
    float scaleX = this->getScaleX();
    float scaleY = this->getScaleY();
    
    for (CCNode *p = m_pParent; p != NULL; p = p->getParent()) {
        scaleX *= p->getScaleX();
        scaleY *= p->getScaleY();
    }
    
    return CCRectMake(screenPos.x, screenPos.y, m_tViewSize.width*scaleX, m_tViewSize.height*scaleY);
}

void OBScrollView::setScroller(OBScroller* scroller)
{
    if (scroller == NULL) {
        return;
    }
    if (scroller->getDirection() == kOBScrollViewDirectionVertical) {
        if (m_verticalScroller) {
            m_verticalScroller->removeFromParent();
        }
        m_verticalScroller = scroller;
        m_verticalScroller->setAnchorPoint(ccp(1,0));
        m_verticalScroller->setPosition(ccp(m_tViewSize.width,0));
        CCLayer::addChild(m_verticalScroller,INT32_MAX, INT32_MAX);

        updateScrollerSize(m_verticalScroller);
        updateScrollerPosition(m_verticalScroller,m_pContainer->getPosition());
        
    }
    else if (scroller->getDirection() == kOBScrollViewDirectionHorizontal) {
        if (m_horizontalScroller) {
            m_horizontalScroller->removeFromParent();
        }
        m_horizontalScroller = scroller;
        m_horizontalScroller->setAnchorPoint(ccp(0,0));
        m_horizontalScroller->setPosition(ccp(0,0));
        CCLayer::addChild(m_horizontalScroller,INT32_MAX-1, INT32_MAX-1);

        updateScrollerSize(m_horizontalScroller);
        updateScrollerPosition(m_horizontalScroller,m_pContainer->getPosition());
        
    }
}


void OBScrollView::updateScrollerPosition(OBScroller* scroller,const CCPoint& targetPos, float duration)
{
    if (scroller)
    {
        if (scroller->getDirection() == kOBScrollViewDirectionVertical)
        {
           scroller->updateOffset(ccp(0,targetPos.y/minContainerOffset().y),duration);
        }
        else if(scroller->getDirection() == kOBScrollViewDirectionHorizontal)
        {
            scroller->updateOffset(ccp(targetPos.x/minContainerOffset().x,0),duration);
        }
    }
}

void OBScrollView::updateScrollerSize(OBScroller* scroller)
{
    if (scroller)
    {
        if (scroller->getDirection() == kOBScrollViewDirectionVertical)
        {
            if (m_pContainer && m_pContainer->getContentSize().height > 0) {
                scroller->updateScrollerSize(CCSizeMake(1.0f, m_tViewSize.height/m_pContainer->getContentSize().height));
            }
        }
        else if(scroller->getDirection() == kOBScrollViewDirectionHorizontal)
        {
            if (m_pContainer && m_pContainer->getContentSize().width > 0) {
                scroller->updateScrollerSize(CCSizeMake(m_tViewSize.width/m_pContainer->getContentSize().width,1.0f));
            }
        }
    }
}

NS_CC_EXT_END
