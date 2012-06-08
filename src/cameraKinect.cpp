//
//  camera.cpp
//  earthQuakeProject
//
//  Created by Allan Yong on 12-03-15.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#include "cinder/app/AppBasic.h"
#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"

#include "cameraKinect.h"

using namespace ci;
using namespace ci::app;
using namespace std;

extern bool                videoSwitcher;

void CameraKinect::setup()
{
    _manager = V::OpenNIDeviceManager::InstancePtr();
	_device0 = _manager->createDevice( "data/configIR.xml", true );
	if( !_device0 ) 
	{
		DEBUG_MESSAGE( "(App)  Couldn't init device0\n" );
		exit( 0 );
	}
	_device0->setPrimaryBuffer( V::NODE_TYPE_DEPTH );
	_manager->start();
    
    
    
	gl::Texture::Format format;
	gl::Texture::Format depthFormat;
	mColorTex = gl::Texture( KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT, format );
	mDepthTex = gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, depthFormat );
	mOneUserTex = gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, format );
}

void CameraKinect::update()
{
    // Update textures
	//mColorTex.update( getColorImage() );
	//mDepthTex.update( getDepthImage24() );	// Histogram

	// Uses manager to handle users.
	if( _manager->hasUsers() && _manager->hasUser(1) ) 
    {
        //mOneUserTex.update( getUserColorImage(1) );
       
       // _manager->renderJoints(3);
       
        userLoc.x= _manager->getUser(1)->getCenterOfMass()[0]*1.5;
        userLoc.y= _manager->getUser(1)->getCenterOfMass()[1]*1.5;
        userLoc.z= _manager->getUser(1)->getCenterOfMass()[2]*1.5;
       // printf("center ponits x is %f, y is %f, z is %f\n", userLoc.x, userLoc.y, userLoc.z);
        //_manager->pause();
    }
    else if( _manager->hasUsers() && _manager->hasUser(2) ) 
    {
        //mOneUserTex.update( getUserColorImage(1) );
        
        // _manager->renderJoints(3);
        
        userLoc.x= _manager->getUser(2)->getCenterOfMass()[0]*1.5;
        userLoc.y= _manager->getUser(2)->getCenterOfMass()[1]*1.5;
        userLoc.z= _manager->getUser(2)->getCenterOfMass()[2]*1.5;
        //printf("center ponits x is %f, y is %f, z is %f\n", userLoc.x, userLoc.y, userLoc.z);
        //_manager->pause();
    }


}

bool CameraKinect::hasUser()
{
    if(_manager->hasUsers() && _manager->hasUser(1) )
    {return true;}
    if(_manager->hasUsers() && _manager->hasUser(2) )
    {return true;}
    else
    {return false;}
}
Vec3f  CameraKinect::getUserLoc()
{

    return userLoc;
}

void CameraKinect::DeleteUser()
{
    //_manager->removeUser(1);
    if(_manager->hasUsers())
    { //_device0->removeAllUser();
    
   // printf("All Users are removed\n");
    }
}
void CameraKinect::shutdown()
{
    _manager->destroyAll();
    _device0->release();

}

void CameraKinect::draw()
{
    
  
    // clear out the window with black
	//gl::clear( Color( 0, 0, 0 ), true ); 
    
    
	gl::setMatricesWindow(getWindowSize() );
    
//	float sx = 320/2;
//	float sy = 240/2;
//	float xoff = 10;
//	float yoff = 10;
//	glEnable( GL_TEXTURE_2D );
	//gl::color( cinder::ColorA(1, 1, 1, 1) );
	if( _manager->hasUsers() && _manager->hasUser(1) ) //gl::draw( mOneUserTex, Rectf( 0, 0, getWindowWidth(), getWindowHeight()) );
	//gl::draw( mDepthTex, Rectf( xoff, yoff, xoff+sx, yoff+sy) );
	//gl::draw( mColorTex, Rectf( xoff+sx*1, yoff, xoff+sx*2, yoff+sy) );
    
    
	if( _manager->hasUsers() && _manager->hasUser(1) )
	{
		// Render skeleton if available
		//_manager->renderJoints( 3 );
        
		// Get list of available bones/joints
		// Do whatever with it
		//V::UserBoneList boneList = _manager->getUser(1)->getBoneList();

    }

    
}