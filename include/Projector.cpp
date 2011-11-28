//
//  ProjectorUtil.cpp
//  skewTest
//
//  Created by Charlie Whitney on 11/24/11.
//  Copyright (c) 2011 Red Paper Heart. All rights reserved.
//

#include "ProjectorUtil.h"

ProjectorUtil::ProjectorUtil(){
    
}

void ProjectorUtil::setup( int width, int height)
{    
    // setup fbo
    mFbo = gl::Fbo( width, height );
    
    // set up handles for skewing
    bDrawHandles = false;
    bApplyBlend = true;
    
    Vec2f v1( 0, 0 );
    Vec2f v2( width, 0 );
    Vec2f v3( width, height );
    Vec2f v4( 0, height );
    
    handles.push_back( v1 );
    handles.push_back( v2 );
    handles.push_back( v3 );
    handles.push_back( v4 );
    
    mSource[0] = toOcv( handles[0] );
    mSource[1] = toOcv( handles[1] );
    mSource[2] = toOcv( handles[2] );
    mSource[3] = toOcv( handles[3] );

    
    dragging = -1;
    
    
    // set up edge blending
    try {
		mShader = gl::GlslProg( loadResource( RES_VERT_GLSL ), loadResource( RES_FRAG_GLSL ) );
        std::cout << "Blend shader loaded successfully" << std::endl;
	}
	catch( gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
	}
	catch( ... ) {
		std::cout << "Unable to load shader" << std::endl;
	}
    
    updateHomography();
}

void ProjectorUtil::begin()
{
    gl::setMatricesWindow( mFbo.getSize() );
    
    mFbo.bindFramebuffer();
        gl::clear( Color(255, 0, 0) );
}

void ProjectorUtil::end()
{
    mFbo.unbindFramebuffer();
    
    gl::setMatricesWindow( getWindowSize() );
}

void ProjectorUtil::update(){
    
}

void ProjectorUtil::draw()
{
    gl::Texture tex = mFbo.getTexture();
//    tex.setFlipped( true );
    
    gl::pushModelView();{
        gl::multModelView( mTransform );
        
        if( bApplyBlend ){
            tex.enableAndBind();
                mShader.bind();
                
                mShader.uniform( "sampler2D", 0 );
                mShader.uniform( "exponent", float(2.0) );
                mShader.uniform( "luminance", Vec3f(0.5, 0.5, 0.5) );
                mShader.uniform( "gamma", Vec3f(1.8, 1.5, 1.2) );
                mShader.uniform( "edges", Vec4f(0.0, 0.0, 0.2, 0.0) );
                
                gl::drawSolidRect( Rectf( Vec2f::zero(), tex.getSize() ) );
                mShader.unbind();
            tex.unbind();
        }else{
            gl::draw( tex );
        }
    
    }gl::popModelView();
    
    // draw handles
    int size = 5;
    gl::color( Color(255, 0, 255) );
    for(vector<Vec2f>::iterator it = handles.begin(); it < handles.end(); ++it){
        gl::drawSolidRect( Rectf( it->x-size, it->y-size, it->x + size, it->y + size) );
    }
}

void ProjectorUtil::updateHomography()
{    
    for( int i=0; i<4; i++ )
        mDestination[i] = toOcv( handles[i] );
    
    // calculate warp matrix
    cv::Mat	warp = cv::getPerspectiveTransform( mSource, mDestination );
    
    // convert to OpenGL matrix
    mTransform.setToIdentity();
    
    mTransform[0]	= warp.ptr<double>(0)[0]; 
    mTransform[4]	= warp.ptr<double>(0)[1]; 
    mTransform[12]	= warp.ptr<double>(0)[2]; 
    
    mTransform[1]	= warp.ptr<double>(1)[0]; 
    mTransform[5]	= warp.ptr<double>(1)[1];
    mTransform[13]	= warp.ptr<double>(1)[2]; 
    
    mTransform[3]	= warp.ptr<double>(2)[0]; 
    mTransform[7]	= warp.ptr<double>(2)[1];
    mTransform[15]	= warp.ptr<double>(2)[2];
}


void ProjectorUtil::showHandles( bool show )
{
    bDrawHandles = show;
}

void ProjectorUtil::showBlending( bool show )
{
    bApplyBlend = show;
}

void ProjectorUtil::mouseDown( MouseEvent event )
{
    int count = 0;
    for(vector<Vec2f>::iterator it = handles.begin(); it < handles.end(); ++it){
        Vec2f v = event.getPos() - *it;
        
        if( v.length() < 100 ){
            dragging = count;
        }
        
        ++count;
    }
}

void ProjectorUtil::mouseUp( MouseEvent event )
{
    dragging = -1;
}

void ProjectorUtil::mouseDrag( MouseEvent event )
{
    if( dragging != -1 ){
        handles[ dragging ] = event.getPos();
        updateHomography();
    }
}



