//
//  ProjectorUtil.cpp
//  skewTest
//
//  Created by Charlie Whitney on 11/24/11.
//  Copyright (c) 2011 Red Paper Heart. All rights reserved.
//

#include "ProjectorUtil.h"

ProjectorUtil::ProjectorUtil()
{
    blendDirection = ProjectorUtil::BLEND_RIGHT;
    mBlendPct = 0.2;  
}

ProjectorUtil::ProjectorUtil( App *app )
    : mApp( app )
{    
    mApp->registerMouseDown( this, &ProjectorUtil::mouseDown );
    mApp->registerMouseUp( this, &ProjectorUtil::mouseUp );
    mApp->registerMouseDrag( this, &ProjectorUtil::mouseDrag );
    
    blendDirection = ProjectorUtil::BLEND_RIGHT;
    mBlendPct = 0.2;
}

bool ProjectorUtil::loadXml(){
    
    string str = ci::app::App::getResourcePath().string() + "/projectorSettings.xml";
    fs::path pth( str );

    try{
        XmlTree doc = XmlTree( loadFile( pth.string() ) );
        
        XmlTree &firstPoint = doc.getChild( "root/handles/" );
        for( XmlTree::Iter child = firstPoint.begin(); child != firstPoint.end(); ++child ){        
            float xx = child->getChild("x").getValue<float>();
            float yy = child->getChild("y").getValue<float>();
            
            handles.push_back( Vec2f( xx, yy ) );
        }

    }catch(...){
        return false;
    }    
    
    return true;
}

void ProjectorUtil::saveXml(){    
    XmlTree library( "root", "" );
    XmlTree handleXml( "handles", "");
    
    for( vector<Vec2f>::iterator it = handles.begin(); it<handles.end(); ++it){
        XmlTree p( "handle","" );
        
        float xx = Vec2f(*it).x;
        float yy = Vec2f(*it).y;
        
        XmlTree x( "x", toString(xx) );
        XmlTree y( "y", toString(yy) );
        
        p.push_back( x );
        p.push_back( y );
        
        handleXml.push_back( p );
    }
    
    library.push_back( handleXml );
    
    string str = ci::app::App::getResourcePath().string() + "/projectorSettings.xml";
    fs::path pth( str );
    
    library.write( writeFile( pth.string() ) );
    cout << "ProjectorUtil :: data written to " << pth.string() << endl;
}

void ProjectorUtil::setup( int width, int height)
{    
    // setup fbo
    mFbo = gl::Fbo( width, height );
    
    console() << "SETUP :: " << width << " " << height << endl;
    
    // set up handles for skewing
    bDrawHandles = false;
    bApplyBlend = false;
    
    Vec2f v1( 0, 0 );
    Vec2f v2( width, 0 );
    Vec2f v3( width, height );
    Vec2f v4( 0, height );
    
    if( loadXml() ){
        // handles were loaded. yay.
        console() << "ProjectorUtil :: Handles successfully loaded" << endl;
    }else{
        console() << "ProjectorUtil :: Couldn't load handles" << endl;
        
        // handles added in reverse order to compensate for flipped fbo texture
        handles.push_back( v4 );
        handles.push_back( v3 );
        handles.push_back( v2 );
        handles.push_back( v1 );
    }
    
    mSource[0] = toOcv( v1 );
    mSource[1] = toOcv( v2 );
    mSource[2] = toOcv( v3 );
    mSource[3] = toOcv( v4 );
    
    dragging = -1;
    
    
    // set up edge blending
    try {
		mShader = gl::GlslProg( loadResource( RES_VERT_GLSL ), loadResource( RES_FRAG_GLSL ) );
        std::cout << "ProjectorUtil :: Blend shader loaded successfully" << std::endl;
	}
	catch( gl::GlslProgCompileExc &exc ) {
		std::cout << "ProjectorUtil :: Shader compile error: " << std::endl;
		std::cout << exc.what();
	}
	catch( ... ) {
		std::cout << "ProjectorUtil :: Unable to load shader" << std::endl;
	}
    
    
    console() << "HANDLE SIZE " << handles.size() << endl;
    
    updateHomography( mSource, handles );
}

void ProjectorUtil::resetHandles(){
    
    
    handles[0] = fromOcv( mSource[3] );
    handles[1] = fromOcv( mSource[2] );
    handles[2] = fromOcv( mSource[1] );
    handles[3] = fromOcv( mSource[0] );
    
    /*
    handles[0] = fromOcv( mSource[0] );
    handles[1] = fromOcv( mSource[1] );
    handles[2] = fromOcv( mSource[2] );
    handles[3] = fromOcv( mSource[3] );
    */
    updateHomography( mSource, handles );
    saveXml();
}

void ProjectorUtil::setup( Vec2i size ){
    setup( size.x, size.y );
}

void ProjectorUtil::begin()
{
//    gl::setMatricesWindow( mFbo.getSize() );
    
    mFbo.bindFramebuffer();
    gl::clear( Color(0, 0, 0) );
}

void ProjectorUtil::end()
{
    mFbo.unbindFramebuffer();
    
//    gl::setMatricesWindow( getWindowSize() );
}

void ProjectorUtil::update(){
    
}

void ProjectorUtil::draw()
{
    gl::Texture tex = mFbo.getTexture();
    //    tex.setFlipped( true );
    
    gl::pushModelView();{
        gl::multModelView( mTransform );
        gl::draw( tex );
    }gl::popModelView();
    
    // draw handles
    if( !bDrawHandles )
        return;
    
    gl::color( ColorA(255, 0, 255, 0.2) );
    for(vector<Vec2f>::iterator it = handles.begin(); it < handles.end(); ++it){
        gl::drawSolidCircle( *it, 20);
    }
}

void ProjectorUtil::draw( Rectf rect )
{
    gl::Texture tex = mFbo.getTexture();
    //    tex.setFlipped( true );
    
    gl::pushModelView();{
        gl::multModelView( mTransform );
        gl::draw( tex, rect );
    }gl::popModelView();
    
    // draw handles
    if( !bDrawHandles )
        return;
    
    gl::color( ColorA(255, 0, 255, 0.2) );
    for(vector<Vec2f>::iterator it = handles.begin(); it < handles.end(); ++it){
        gl::drawSolidCircle( *it, 20);
    }
}


Matrix44d ProjectorUtil::updateHomography( vector<Vec2f> source, vector<Vec2f> dest )
{
    cv::Point2f src[4];
    src[0] = toOcv( source[0] );
    src[1] = toOcv( source[1] );
    src[2] = toOcv( source[2] );
    src[3] = toOcv( source[3] );
    
    return updateHomography( src, dest);
}

Matrix44d ProjectorUtil::updateHomography( cv::Point2f source[4], vector<Vec2f> dest )
{    
    for( int i=0; i<4; i++ )
        mDestination[i] = toOcv( dest[i] );
    
    // calculate warp matrix
    cv::Mat	warp = cv::getPerspectiveTransform( source, mDestination );
    
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
    
    return mTransform;
}


void ProjectorUtil::showHandles( bool show )
{
    bDrawHandles = show;
}

void ProjectorUtil::showBlending( bool show )
{
    bApplyBlend = show;
}

void ProjectorUtil::setBlendDirection( int blendDir ){
    blendDirection = blendDir;
}

void ProjectorUtil::setBlendAmount( float pct ){
    mBlendPct = pct;
}

bool ProjectorUtil::mouseDown( MouseEvent event )
{
    int count = 0;
    for(vector<Vec2f>::iterator it = handles.begin(); it < handles.end(); ++it){
        
        Vec2f v = event.getPos() - *it;
        
        if( v.length() < 100 ){
            dragging = count;
        }
        
        ++count;
    }
    
    return false;
}

bool ProjectorUtil::mouseUp( MouseEvent event )
{    
    if(dragging != -1){
        saveXml();
    }
    
    dragging = -1;
    
    return false;
}

bool ProjectorUtil::mouseDrag( MouseEvent event )
{
    if( dragging != -1 ){
        handles[ dragging ] = event.getPos();
        updateHomography( mSource, handles );
    }
    
    return false;
}



