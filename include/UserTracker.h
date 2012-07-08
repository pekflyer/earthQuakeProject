//
//  UserTracker.h
//  earthQuakeProject
//
//  Created by Allan Yong on 12-07-07.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Cinder.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rect.h"
#include "cinder/Utilities.h"
#include "cinder/Camera.h"

#include <XnOpenNI.h>
#include "XnCppWrapper.h"
#include <XnHash.h>
#include <XnLog.h>
#include <XnUSB.h> 

// Header for NITE
#include "XnVNite.h"

#include <iostream>
#include <stdio.h> 
#include <time.h> 

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace xn;
static const int    APP_RES_X       = 1280;
static const int    APP_RES_Y       = 720;
static const float  APP_FRAMERATE   = 30.0f;


static XnChar g_strPose[20] = "";
static XnBool g_bNeedPose;
//static Context g_context;
//static DepthGenerator g_DepthGenerator;
//static ImageGenerator g_ImageGenerator; //!!
//static UserGenerator g_UserGenerator;
//static ScriptNode     g_scriptNode;
//static DepthMetaData  g_depthMD;
//static ImageMetaData  g_imageMD;
//static Player         g_Player;
static std::map<XnUInt32, std::pair<XnCalibrationStatus, XnPoseDetectionStatus> > m_Errors;
static SceneMetaData sceneMD;
static DepthMetaData depthMD;
static  XnUserID    trackID;
#define CHECK_RC(rc, what)                                      \
if (rc != XN_STATUS_OK)                                         \
{                                                               \
printf("%s failed: %s\n", what, xnGetStatusString(rc));     \
}



#define CHECK_ERRORS(rc, errors, what)              \
{                                                   \
if (rc == XN_STATUS_NO_NODE_PRESENT)            \
{                                           \
XnChar strError[1024];                  \
errors.ToString(strError, 1024);        \
printf("%s\n", strError);               \
return (rc);                            \
}                                           \
CHECK_RC(rc, what)                              \
}                                                     

#define SAMPLE_XML_CONFIG_PATH "SamplesConfig.xml"


#define DISPLAY_MODE_OVERLAY	1
#define DISPLAY_MODE_DEPTH		2
#define DISPLAY_MODE_IMAGE		3


#define MAX_DEPTH 10000


struct UserPoint
{
	UserPoint()
	{
		id = -1;
        position = ci::Vec3f::zero();
        prevPosition = ci::Vec3f::zero();
	}
	
	~UserPoint(){};
	
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


class UserTracker {
    
private:
    // Private Constructor
    UserTracker();
    // Stop the compiler generating methods of copy the object
    //  UserTracker(UserTracker const& copy);            // Not Implemented
     // UserTracker& operator=(UserTracker const& copy); // Not Implemented
    
    //Surface8u   mVideoTexture;
    //Surface16u  mDepthTexture;
    
public:
       static UserTracker& getInstance()
       {
         // The only instance
         // Guaranteed to be lazy initialized
            // Guaranteed that it will be destroyed correctly
          static UserTracker instance;
          return instance;
      }
    
public:
    
    
     static vector<UserPoint> userPoints;
    //    Surface8u mDepthImage; // initialized elsewhere
    //    Surface8u mVideoImage; // initialized elsewhere
    // static vector<UserPoint> userPoints;
    
    vector<UserPoint> *getUserPoints()
    {
        return &userPoints;
    }
    ImageSourceRef getColorSource( uint8_t* pData );
    ImageSourceRef getDepthSource( uint16_t* pData );
    
    
	void    draw();
	void    update();
    
    void    updateVideoTexture();
    void    updateDepthTexture();

     void setUserID(XnUserID nId);
    Surface8u getVideoImage();
    Surface16u getDepthImage();
   

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
    
    XnUserID aUsers[15];
    
    //    // OpenNI objects
    xn::Context         m_Context;
    xn::DepthGenerator  m_DepthGenerator;
    xn::ImageGenerator  m_ImageGenerator; //!!
    xn::UserGenerator   m_UserGenerator;
    xn::ScriptNode      m_ScriptNode;
    xn::DepthMetaData   m_DepthMD;
    xn::ImageMetaData   m_ImageMD;
    xn::Player          m_Player;


    
    // NITE objects
    XnVSessionManager* g_pSessionManager;
    
    /*
    static void    addUser(int nId, ci::Vec3f  position);
    static void    removeUser(int nId);
    static int     findUserPointIndex(int nId);
    static void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
    static void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
    static void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie);
    static void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie);
    static void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie);
    static void XN_CALLBACK_TYPE MyCalibrationInProgress(xn::SkeletonCapability& capability, XnUserID id, XnCalibrationStatus calibrationError, void* pCookie);
    static void XN_CALLBACK_TYPE MyPoseInProgress(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID id, XnPoseDetectionStatus poseError, void* pCookie);
    */
    static void    addUser(int nId, ci::Vec3f  position);
    static void    removeUser(int nId);
    static int     findUserPointIndex(int nId);
    static void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
    static void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
    static void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie);
    static void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie);
    static void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie);
    static void XN_CALLBACK_TYPE MyCalibrationInProgress(xn::SkeletonCapability& capability, XnUserID id, XnCalibrationStatus calibrationError, void* pCookie);
    static void XN_CALLBACK_TYPE MyPoseInProgress(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID id, XnPoseDetectionStatus poseError, void* pCookie);
    
    
    
};
    

