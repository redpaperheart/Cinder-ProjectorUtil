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
    
    string pth = getAppPath();
    int pos = pth.find_last_of("/");
    pth = pth.substr( 0, pos );

    XmlTree doc;
    
    try{
        doc = XmlTree( loadFile( toString(pth) + "/projectorSettings.xml" ) );
    }catch(...){
        return false;
    }    
    
    XmlTree firstPoint = doc.getChild( "root/handles/" );
    for( XmlTree::Iter child = firstPoint.begin(); child != firstPoint.end(); ++child ){        
        float xx = child->getChild("x").getValue<float>();
        float yy = child->getChild("y").getValue<float>();
        
        handles.push_back( Vec2f( xx, yy ) );
    }
    
    
    return true;
}

void ProjectorUtil::saveXml(){
    string pth = getAppPath();
    int pos = pth.find_last_of("/");
    pth = pth.substr( 0, pos );
    
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
    
    library.write( writeFile( toString(pth) + "/projectorSettings.xml" ) );
    cout << "ProjectorUtil :: data written to " << toString(pth) + "/projectorSettings.xml" << endl;
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
    
    if( loadXml() ){
        // handles were loaded. yay.
    }else{
        
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
    
    updateHomography( mSource, handles );
}

void ProjectorUtil::resetHandles(){
    
    /*
    handles[0] = fromOcv( mSource[3] );
    handles[1] = fromOcv( mSource[2] );
    handles[2] = fromOcv( mSource[1] );
    handles[3] = fromOcv( mSource[0] );
    */
    
    handles[0] = fromOcv( mSource[0] );
    handles[1] = fromOcv( mSource[1] );
    handles[2] = fromOcv( mSource[2] );
    handles[3] = fromOcv( mSource[3] );
    
    updateHomography( mSource, handles );
    saveXml();
}

void ProjectorUtil::setup( Vec2i size ){
    setup( size.x, size.y );
}

void ProjectorUtil::begin()
{
    gl::setMatricesWindow( mFbo.getSize() );
    
    mFbo.bindFramebuffer();
    glClear(GL_DEPTH_BUFFER_BIT );
    gl::clear( Color(0, 0, 0) );
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
        
        if( false && bApplyBlend && mShader ){
            tex.enableAndBind();
                        mShader.bind();

                        mShader.uniform( "sampler2D", 0 );
                        mShader.uniform( "exponent", float(2.0) );
                        mShader.uniform( "luminance", Vec3f(0.5, 0.5, 0.5) );
                        mShader.uniform( "gamma", Vec3f(1.8, 1.5, 1.2) );

                        if( blendDirection == ProjectorUtil::BLEND_RIGHT){
                            mShader.uniform( "edges", Vec4f(0.0, 0.0, mBlendPct, 0.0) );
                        }else if( blendDirection == ProjectorUtil::BLEND_LEFT){
                            mShader.uniform( "edges", Vec4f(mBlendPct, 0.0, 0.0, 0.0) );
                        }
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



