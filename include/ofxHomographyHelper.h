/*
 *  ofxHomographyHelper.cpp
 *  Created by Elliot Woods on 26/11/2010.
 *
 *  Ported to cinder 12/12/2011 by Charlie Whitney
 *
 *  Based entirely on arturo castro's homography implementation
 *  Created 08/01/2010, arturo castro
 *
 *	http://www.openframeworks.cc/forum/viewtopic.php?f=9&t=3121
 */

#pragma once

#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ofxHomographyHelper
{
public:
	static void			gaussian_elimination(float *input, int n);
	
	static void			findHomography(Vec2f src[4], Vec2f dst[4], float homography[16]);
	static Matrix44d	findHomography(float src[4][2], float dst[4][2]);
};

