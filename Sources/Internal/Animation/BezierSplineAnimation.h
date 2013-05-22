/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#ifndef __DAVAENGINE_BEZIER_SPLINE_ANIMATION_H__
#define __DAVAENGINE_BEZIER_SPLINE_ANIMATION_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "FileSystem/Logger.h"
#include "Animation/Animation.h"
#include "Animation/AnimatedObject.h"

namespace DAVA 
{
	
	
class BezierSplineAnimation2 : public Animation
{
public:
	BezierSplineAnimation2(AnimatedObject * _owner, Vector2 * _var, BezierSpline2 * _spline, float32 _animationTimeLength, Interpolation::FuncType _iType);
	virtual void Update(float32 timeElapsed);
	virtual void OnStart();
private:
	Vector2 * var;//i hate you boroda
	BezierSpline2 * spline;
};
	

class BezierSplineAnimation3 : public Animation
{
public:
	BezierSplineAnimation3(AnimatedObject * _owner, Vector3 * _var, BezierSpline3 * _spline, float32 _animationTimeLength, Interpolation::FuncType _iType);
	virtual void Update(float32 timeElapsed);
	virtual void OnStart();
private:
	Vector3 * var;//i hate you boroda
	BezierSpline3 * spline;
};

};
#endif // __DAVAENGINE_BEZIER_SPLINE_ANIMATION_H__