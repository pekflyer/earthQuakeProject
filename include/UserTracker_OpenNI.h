//
//  UserTracker_OpenNI.h
//  earthQuakeProject
//
//  Created by Allan Yong on 12-06-26.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef earthQuakeProject_UserTracker_OpenNI_h
#define earthQuakeProject_UserTracker_OpenNI_h


#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"
#include "cinder/imageio.h"
#include "cinder/Thread.h"

#include <XnOpenNI.h>
#include "XnCppWrapper.h"
#include <XnHash.h>
#include <XnLog.h>

// Header for NITE
#include "XnVNite.h"

#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace xn;

#define CHECK_RC(rc, what)                                      \
if (rc != XN_STATUS_OK)                                         \
{                                                               \
printf("%s failed: %s\n", what, xnGetStatusString(rc));     \
}

#define CHECK_ERRORS(rc, errors, what)		\
if (rc == XN_STATUS_NO_NODE_PRESENT)        \
{                                           \
XnChar strError[1024];                  \
errors.ToString(strError, 1024);        \
printf("%s\n", strError);               \
return (rc);                            \
}

#define SAMPLE_XML_CONFIG_PATH "Sample-Tracking.xml"
#define GESTURE_CLICK "Click"
#define GESTURE_WAVE "Wave"
#define GESTURE_RAISE_HAND "RaiseHand"

//#define GL_WIN_SIZE_X 1024
//#define GL_WIN_SIZE_Y 768

#define DISPLAY_MODE_OVERLAY	1
#define DISPLAY_MODE_DEPTH		2
#define DISPLAY_MODE_IMAGE		3

typedef enum
{
	NORMAL_VIEW,
    VIDEO_VIEW,
    OPTICALFLOW_VIEW,
    VECTORFIELD_VIEW,
    DEBUG_VIEW
} VIEW_STATE;

static int CURR_VIEW_STATE = 0;

static const int    MAX_TIME_WARMING  = 300; //30fps * 60s/min * 5 min
static const float  MAX_TIME_DIVISOR  = 1.0f/(float)MAX_TIME_WARMING;

//App settings
static const int    APP_RES_X       = 1024;
static const int    APP_RES_Y       = 768;
static const float  APP_FRAMERATE   = 30.0f;
static const bool   APP_RESIZEABLE  = false;
static const bool   APP_FULLSCREEN  = false;


#define MAX_DEPTH 10000

struct HandPoint
{
	HandPoint()
	{
		id = -1;
        position = ci::Vec3f::zero();
        prevPosition = ci::Vec3f::zero();
	}
	
	~HandPoint(){};
	
	int id;
    ci::Vec3f position;
    ci::Vec3f prevPosition;
};

typedef enum
{
	IN_SESSION,
	NOT_IN_SESSION,
	QUICK_REFOCUS
} SessionState;

class ImageSourceKinectColor : public ImageSource 
{
public:
    ImageSourceKinectColor( uint8_t *buffer, int width, int height ) : ImageSource(), mData( buffer ), _width(width), _height(height)
    {
        setSize( _width, _height );
        setColorModel( ImageIo::CM_RGB );
        setChannelOrder( ImageIo::RGB );
        setDataType( ImageIo::UINT8 );
    }
    
    ~ImageSourceKinectColor()
    {
        // mData is actually a ref. It's released from the device. 
        /*if( mData ) {
         delete[] mData;
         mData = NULL;
         }*/
    }
    
    virtual void load( ImageTargetRef target )
    {
        ImageSource::RowFunc func = setupRowFunc( target );
        
        for( uint32_t row	 = 0; row < _height; ++row )
            ((*this).*func)( target, row, mData + row * _width * 3 );
    }
    
protected:
    uint32_t					_width, _height;
    uint8_t						*mData;
};


class ImageSourceKinectDepth : public ImageSource 
{
public:
    ImageSourceKinectDepth( uint16_t *buffer, int width, int height ) : ImageSource(), mData( buffer ), _width(width), _height(height)
    {
        setSize( _width, _height );
        setColorModel( ImageIo::CM_GRAY );
        setChannelOrder( ImageIo::Y );
        setDataType( ImageIo::UINT16 );
    }
    
    ~ImageSourceKinectDepth()
    {
        // mData is actually a ref. It's released from the device. 
        /*if( mData ) {
         delete[] mData;
         mData = NULL;
         }*/
    }
    
    virtual void load( ImageTargetRef target )
    {
        ImageSource::RowFunc func = setupRowFunc( target );
        
        for( uint32_t row = 0; row < _height; ++row )
            ((*this).*func)( target, row, mData + row * _width );
    }
    
protected:
    uint32_t					_width, _height;
    uint16_t					*mData;
};

static int testNum = 23;
static SessionState g_SessionState;

class UserTracker_OpenNI 
{
    
private:
    // Private Constructor
    UserTracker_OpenNI();
    // Stop the compiler generating methods of copy the object
    UserTracker_OpenNI(UserTracker_OpenNI const& copy);            // Not Implemented
    UserTracker_OpenNI& operator=(UserTracker_OpenNI const& copy); // Not Implemented
    
    //Surface8u   mVideoTexture;
    //Surface16u  mDepthTexture;
    
public:
    static UserTracker_OpenNI& getInstance()
    {
        // The only instance
        // Guaranteed to be lazy initialized
        // Guaranteed that it will be destroyed correctly
        static UserTracker_OpenNI instance;
        return instance;
    }
    
public:
    
    static vector<HandPoint> handpoints;
    
    //    Surface8u mDepthImage; // initialized elsewhere
    //    Surface8u mVideoImage; // initialized elsewhere
    ImageSourceRef getColorSource( uint8_t* pData );
    ImageSourceRef getDepthSource( uint16_t* pData );
    
	void    draw();
	void    update();
    
    void    updateVideoTexture();
    void    updateDepthTexture();
    
    Surface8u getVideoImage();
    Surface16u getDepthImage();
    
    vector<HandPoint> *getHandPoints()
    {
        return &handpoints;
    }
    
    //---------------------------------------------------------------------------
    // Globals
    //---------------------------------------------------------------------------
    float g_pDepthHist[MAX_DEPTH];
    XnRGB24Pixel* g_pTexMap;
    
    unsigned int g_nTexMapX;
    unsigned int g_nTexMapY;
    unsigned int g_nViewState;
    
    XnBoundingBox3D* boundingBox;
    GestureGenerator g_GestureGenerator;
    HandsGenerator g_HandsGenerator;
    
    // OpenNI objects
    Context context;
    DepthGenerator g_DepthGenerator;
    ImageGenerator g_ImageGenerator; //!!
    DepthMetaData g_depthMD;
    ImageMetaData g_imageMD;
    
    // NITE objects
    XnVSessionManager* g_pSessionManager;
    
    static void    addHand( int id, ci::Vec3f position );
    static void    removeHand( int id );
    static int     findHandPointIndex(int id);
    
	static void XN_CALLBACK_TYPE Gesture_Recognized(GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie);
	static void XN_CALLBACK_TYPE Gesture_Process(GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pPosition, XnFloat fProgress, void* pCookie);
	static void XN_CALLBACK_TYPE Hand_Update(HandsGenerator& generator, XnUserID nId, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie);
	static void XN_CALLBACK_TYPE Hand_Create(HandsGenerator& generator, XnUserID nId, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie);
	static void XN_CALLBACK_TYPE Hand_Destroy(HandsGenerator& generator, XnUserID nId, XnFloat fTime, void* pCookie);
	static void XN_CALLBACK_TYPE NoHands(void* UserCxt);
	static void XN_CALLBACK_TYPE FocusProgress(const XnChar* strFocus, const XnPoint3D& ptPosition, XnFloat fProgress, void* UserCxt);
	static void XN_CALLBACK_TYPE SessionStarting(const XnPoint3D& ptPosition, void* UserCxt);
	static void XN_CALLBACK_TYPE SessionEnding(void* UserCxt);
    static void XN_CALLBACK_TYPE onErrorStateChanged(XnStatus errorState, void* pCookie);
    
};


#endif
