/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "UI/UIScrollViewContainer.h"

namespace DAVA 
{
	
const int32 DEFAULT_TOUCH_TRESHOLD = 15;  // Default value for finger touch tresshold

UIScrollViewContainer::UIScrollViewContainer(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
:	UIControl(rect, rectInAbsoluteCoordinates),
	mainTouch(-1),
	touchTreshold(DEFAULT_TOUCH_TRESHOLD),
	enableHorizontalScroll(true),
	enableVerticalScroll(true),
	newPos(0.f, 0.f),
	oldPos(0.f, 0.f),
	lockTouch(false),
	state(STATE_NONE)
{
	this->SetInputEnabled(true);
	this->SetMultiInput(true);
}

UIScrollViewContainer::~UIScrollViewContainer()
{
}

UIControl* UIScrollViewContainer::Clone()
{
	UIScrollViewContainer *t = new UIScrollViewContainer(GetRect());
	t->CopyDataFrom(this);
	return t;
}
	
void UIScrollViewContainer::CopyDataFrom(UIControl *srcControl)
{
	UIControl::CopyDataFrom(srcControl);
}

void UIScrollViewContainer::SetRect(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
{
	UIControl::SetRect(rect, rectInAbsoluteCoordinates);
	
	UIControl *parent = this->GetParent();
	if (parent)
	{
		Rect parentRect = parent->GetRect();
		// We should not allow scrolling when content rect is less than or is equal ScrollView "window"
		enableHorizontalScroll = rect.dx > parentRect.dx;
		enableVerticalScroll = rect.dy > parentRect.dy;
	}
}

void UIScrollViewContainer::SetTouchTreshold(int32 holdDelta)
{
	touchTreshold = holdDelta;
}
int32 UIScrollViewContainer::GetTouchTreshold()
{
	return touchTreshold;
}

void UIScrollViewContainer::Input(UIEvent *currentTouch)
{
	Vector<UIEvent> touches = UIControlSystem::Instance()->GetAllInputs();
	
	if(1 == touches.size())
	{
		newPos = currentTouch->point;
		
		switch(currentTouch->phase)
		{
			case UIEvent::PHASE_BEGAN:
			{
				scrollTouch = *currentTouch;
				scrollStartInitialPosition = currentTouch->point;
				scrollStartMovement = false;
				state = STATE_SCROLL;
				lockTouch = true;
				oldPos = newPos;
			}
			break;
			case UIEvent::PHASE_DRAG:
			{
				if(state == STATE_SCROLL)
				{
					if(currentTouch->tid == scrollTouch.tid)
					{
						scrollStartMovement = true;
					}
				}
			}
			break;
			case UIEvent::PHASE_ENDED:
			{
				lockTouch = false;
				state = STATE_DECCELERATION;
			}
			break;
		}
	}
}

bool UIScrollViewContainer::SystemInput(UIEvent *currentTouch)
{
	if(!inputEnabled || !visible || controlState & STATE_DISABLED)
	{
		return false;
	}

	bool systemInput = UIControl::SystemInput(currentTouch);
	if (currentTouch->GetInputHandledType() == UIEvent::INPUT_HANDLED_HARD)
	{
		// Can't scroll - some child control already processed this input.
		return systemInput;
	}

	if(currentTouch->phase == UIEvent::PHASE_BEGAN)
	{
		if(IsPointInside(currentTouch->point))
		{
			mainTouch = currentTouch->tid;
			PerformEvent(EVENT_TOUCH_DOWN);
			Input(currentTouch);
		}
	}
	else if(currentTouch->tid == mainTouch && currentTouch->phase == UIEvent::PHASE_DRAG)
	{
		// Don't scroll if touchTreshold is not exceeded 
		if ((abs(currentTouch->point.x - scrollStartInitialPosition.x) > touchTreshold) ||
			(abs(currentTouch->point.y - scrollStartInitialPosition.y) > touchTreshold))
		{
			UIControlSystem::Instance()->SwitchInputToControl(mainTouch, this);
			Input(currentTouch);
		}
	}
	else if(currentTouch->tid == mainTouch && currentTouch->phase == UIEvent::PHASE_ENDED)
	{
		Input(currentTouch);
		mainTouch = -1;
	}

	if (scrollStartMovement && currentTouch->tid == mainTouch)
	{
		return true;
	}
	
	return systemInput;
}

void UIScrollViewContainer::Update(float32 timeElapsed)
{
	if (state == STATE_NONE)
	{
		return;
	}
	
	UIScrollView *scrollView = dynamic_cast<UIScrollView*>(this->GetParent());
	if (scrollView)
	{
		Rect contentRect = this->GetRect();
	
		Vector2 posDelta = newPos - oldPos;
		oldPos = newPos;
	
		ScrollHelper *horizontalScroll = scrollView->GetHorizontalScroll();
		ScrollHelper *verticalScroll = scrollView->GetVerticalScroll();
		// Get scrolls positions and change scroll container relative position
		if (horizontalScroll && enableHorizontalScroll)
		{
			contentRect.x = horizontalScroll->GetPosition(posDelta.x, SystemTimer::FrameDelta(), lockTouch);
		}
		if (verticalScroll && enableVerticalScroll)
		{
			contentRect.y = verticalScroll->GetPosition(posDelta.y, SystemTimer::FrameDelta(), lockTouch);
		}

		this->SetRect(contentRect);
		// Change state when scrolling is not active
		if (!lockTouch && (horizontalScroll->GetCurrentSpeed() == 0) && (verticalScroll->GetCurrentSpeed() == 0))
		{
			state = STATE_NONE;
		}
	}
}

YamlNode * UIScrollViewContainer::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);
	
    // Control Type
	SetPreferredNodeType(node, "UIScrollViewContainer");
	// Save scroll view container childs including all sub-childs
    loader->SaveChildren(this, node);
    
    return node;
}

};