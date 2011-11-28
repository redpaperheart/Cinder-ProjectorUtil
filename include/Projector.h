//
//  ProjectorUtil.h
//  skewTest
//
//  Created by Charlie Whitney on 11/24/11.
//  Copyright (c) 2011 Red Paper Heart. All rights reserved.
//

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"

#include "Resources.h"
#include "CinderOpenCV.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class ProjectorUtil {
  public:
    ProjectorUtil();
    
    void setup( int width, int height );
    void begin();
    void end();
    void update();
    void draw();
    
    void showHandles( bool show=true );
    void showBlending( bool show=true );
    void mouseDown( MouseEvent event );
    void mouseUp( MouseEvent event );
    void mouseDrag( MouseEvent event );
    
  protected:
    void updateHomography();
    
    vector<Vec2f> handles;
    
    int         dragging;
    bool        bDrawHandles, bApplyBlend;
    
    gl::GlslProg	mShader;
    gl::Fbo         mFbo;
    gl::Texture     output, tex;
    
    // for warping
    cv::Point2f	mSource[4];
    cv::Point2f	mDestination[4];
    Matrix44d	mTransform;
    
    
};