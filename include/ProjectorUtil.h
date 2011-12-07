//
//  ProjectorUtil.h
//  skewTest
//
//  Created by Charlie Whitney on 11/24/11.
//  Copyright (c) 2011 Red Paper Heart. All rights reserved.
//

#pragma once

#include "cinder/Xml.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIO.h"

#include "Resources.h"
#include "CinderOpenCV.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class ProjectorUtil {
public:
    ProjectorUtil( ci::app::App *app );
    
    void setup( int width, int height );
    void setup( Vec2i size );
    void begin();
    void end();
    void update();
    void draw();
    
    void resetHandles();
    void showHandles( bool show=true );
    void showBlending( bool show=true );
    
    bool mouseDown( MouseEvent event );
    bool mouseUp( MouseEvent event );
    bool mouseDrag( MouseEvent event );
    
    
    
protected:
    void updateHomography( vector<Vec2f> points );
    bool loadXml();
    void saveXml();
    
    vector<Vec2f> handles;
    
    int         dragging;
    bool        bDrawHandles, bApplyBlend;
    ci::app::App	*mApp;
    gl::GlslProg	mShader;
    gl::Fbo         mFbo;
    gl::Texture     output, tex;
    
    // for warping
    cv::Point2f	mSource[4];
    cv::Point2f	mDestination[4];
    Matrix44d	mTransform;
    
    
};