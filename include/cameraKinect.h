#include "cinder/app/AppBasic.h"
#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"
#include "cinder/imageio.h"
#include "cinder/gl/gl.h"

#include <XnOpenNI.h>
#include "XnCppWrapper.h"
#include <XnHash.h>
#include <XnLog.h>
#include <XnUSB.h> 

// Header for NITE
#include "XnVNite.h"

#include "CinderOpenCV.h"
#include <iostream>
#include <stdio.h> 
#include <time.h> 



#define CHECK_RC(rc, what)                                      \
if (rc != XN_STATUS_OK)                                         \
{                                                               \
printf("%s failed: %s\n", what, xnGetStatusString(rc));     \
}



#define CHECK_ERRORS(rc, errors, what)      \
{                                           \
    if (rc == XN_STATUS_NO_NODE_PRESENT)        \
    {                                           \
        XnChar strError[1024];                  \
        errors.ToString(strError, 1024);        \
        printf("%s\n", strError);               \
        return (rc);                            \
    }                                       \
        CHECK_RC(rc, what)                  \
}                                       \    

#define SAMPLE_XML_CONFIG_PATH "SamplesConfig.xml"
#define SAMPLE_BODY_HAAR_PATH "haarcascade_fullbody.xml"

#define DISPLAY_MODE_OVERLAY	1
#define DISPLAY_MODE_DEPTH		2
#define DISPLAY_MODE_IMAGE		3
#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH

#define MAX_DEPTH 10000
using namespace ci;
using namespace ci::app;
using namespace std;
using namespace xn;



typedef enum
{
	IN_SESSION,
	NOT_IN_SESSION,
	QUICK_REFOCUS
} SessionState;

//static SessionState g_SessionState;


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



class CameraKinect :  public AppBasic
{
public:
    static const int WIDTH = 1280;
	static const int HEIGHT = 720;
    
	static const int KINECT_COLOR_WIDTH = 640;	//1280;
	static const int KINECT_COLOR_HEIGHT = 480;	//1024;
	static const int KINECT_COLOR_FPS = 30;	//15;
	static const int KINECT_DEPTH_WIDTH = 640;
	static const int KINECT_DEPTH_HEIGHT = 480;
	static const int KINECT_DEPTH_FPS = 30;
    
    
    ImageSourceRef getColorSource( uint8_t* pData );
    ImageSourceRef getDepthSource( uint16_t* pData );
    
	void    draw();
	void    update();
    CameraKinect();
    void    updateVideoTexture();
    void    updateDepthTexture();
    bool    Move(int angle);
    void    CleanupExit();
    bool    Open();
    void    Close();
    void    updateBodies();



    enum { MaxDevs = 16 };
    Surface8u getVideoImage();
    Surface16u getDepthImage();
    cv::CascadeClassifier           mBodyCascade;
    vector<Rectf>                   mBodies;
    gl::Texture                     mDepthTexture;
    
    
    //---------------------------------------------------------------------------
    // Globals
    //---------------------------------------------------------------------------
    float* g_pDepthHist;
    XnRGB24Pixel* g_pTexMap;
    unsigned int g_nTexMapX;
    unsigned int g_nTexMapY;
    XnDepthPixel g_nZRes;
    
    unsigned int g_nViewState;
    

    
    //////////////////////////////////////////////////---------------------------------
    // OpenNI objects
    Context g_context;
    ScriptNode g_scriptNode;
    DepthGenerator g_depth;
    ImageGenerator g_image;
    UserGenerator g_UserGenerator;
    DepthMetaData g_depthMD;
    ImageMetaData g_imageMD;
    
   // XnBool g_bNeedPose;
    XnChar g_strPose[20];
/*
    
    float g_pDepthHist[MAX_DEPTH];
    XnRGB24Pixel* g_pTexMap;
    
    unsigned int g_nTexMapX;
    unsigned int g_nTexMapY;
    unsigned int g_nViewState;*/
    static CameraKinect& getInstance()
    {
        // The only instance
        // Guaranteed to be lazy initialized
        // Guaranteed that it will be destroyed correctly
        static CameraKinect instance;
        return instance;
    }

    // NITE objects
    XnVSessionManager* g_pSessionManager;
    
private:
    XN_USB_DEV_HANDLE m_devs[MaxDevs];
    XnUInt32 m_num;
    bool m_isOpen;
    /*
	void setup();
    Vec3f getUserLoc();
    bool hasUser();
	//void mouseDown( MouseEvent event );	
    //void keyDown( KeyEvent event );
    void DeleteUser();
	void update();
	void draw();
    void shutdown();
   
    //Vec2f getUserLoc();
	
	ImageSourceRef getColorImage()
	{
		// register a reference to the active buffer
		uint8_t *activeColor = _device0->getColorMap();
		return ImageSourceRef( new ImageSourceKinectColor( activeColor, KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT ) );
	}
    
	ImageSourceRef getUserColorImage( int id )
	{
		V::OpenNIUserRef user = _manager->getUser(id);
        
		// register a reference to the active buffer
		uint8_t *activeColor = user->getPixels();
		return ImageSourceRef( new ImageSourceKinectColor( activeColor, KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT ) );
	}
    
	ImageSourceRef getDepthImage()
	{
		// register a reference to the active buffer
		uint16_t *activeDepth = _device0->getDepthMap();
		return ImageSourceRef( new ImageSourceKinectDepth( activeDepth, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	} 
    
	ImageSourceRef getDepthImage24()
	{
		// register a reference to the active buffer
		uint8_t *activeDepth = _device0->getDepthMap24();
		return ImageSourceRef( new ImageSourceKinectColor( activeDepth, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	}
    
public:	// Members
	V::OpenNIDeviceManager*	_manager;
	V::OpenNIDevice::Ref	_device0;
    
    Surface                 mDepthSurface;
	gl::Texture				mColorTex;
	gl::Texture				mDepthTex,mCvTexture;
	gl::Texture				mOneUserTex;	
    Vec3f                   userLoc;
    float                   mScale;
	float                   mXOff, mYOff;
    Vec3f                   mTargetPosition;
    */
};