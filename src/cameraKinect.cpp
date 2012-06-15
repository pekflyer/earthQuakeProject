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
#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;

extern bool                videoSwitcher;
extern float         mThreshold, mBlobMin, mBlobMax;


CameraKinect::CameraKinect()
{
    g_pTexMap = NULL;
    g_nTexMapX = 0;
    g_nTexMapY = 0;
    g_nViewState = DISPLAY_MODE_DEPTH;
    g_SessionState = NOT_IN_SESSION;
    Open();
	XnStatus rc = XN_STATUS_OK;
	EnumerationErrors errors;
    
	// Initialize OpenNI
	rc = context.InitFromXmlFile(SAMPLE_XML_CONFIG_PATH, &errors);
	CHECK_RC(rc, "InitFromXmlFile");
    
    XnStatus nRetVal = XN_STATUS_OK;
    //Make it start generating data
    nRetVal = context.StartGeneratingAll();
    CHECK_RC(nRetVal, "Start Generating All Data");

    
    
    ////// Getting Image and Depth data ///////////////////////////////////////////
    rc = context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
	CHECK_RC(rc, "Find depth generator");
    rc = context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_ImageGenerator);
	CHECK_RC(rc, "Find image generator");
    g_DepthGenerator.GetMetaData(g_depthMD);
	g_ImageGenerator.GetMetaData(g_imageMD);
    
    
    
    // Hybrid mode isn't supported in this sample
	if (g_imageMD.FullXRes() != g_depthMD.FullXRes() || g_imageMD.FullYRes() != g_depthMD.FullYRes())
	{
		printf ("The device depth and image resolution must be equal!\n");
		//return 1;
	}
    
	// RGB is the only image format supported.
	if (g_imageMD.PixelFormat() != XN_PIXEL_FORMAT_RGB24)
	{
		printf("The device image format must be RGB24\n");
		//return 1;
	}
    // Texture map init
//        mDepthImage = Surface8u(g_depthMD.FullXRes(), g_depthMD.FullYRes(), true);
//        mVideoImage = Surface8u(g_depthMD.FullXRes(), g_depthMD.FullYRes(), true);
//        mVideoTexture = Surface8u();
//        mDepthTexture = Surface16u();
    g_nTexMapX = (((unsigned short)(g_depthMD.FullXRes()-1) / 512) + 1) * 512;
	g_nTexMapY = (((unsigned short)(g_depthMD.FullYRes()-1) / 512) + 1) * 512;
	g_pTexMap = (XnRGB24Pixel*)malloc(g_nTexMapX * g_nTexMapY * sizeof(XnRGB24Pixel));
    
    //glDisable(GL_DEPTH_TEST);
	//glEnable(GL_TEXTURE_2D);


}
bool CameraKinect::Open()
{
    const XnUSBConnectionString *paths;
    XnUInt32 count;
    XnStatus res;
    
    // Init OpenNI USB
    res = xnUSBInit();
    if (res != XN_STATUS_OK)
    {
        xnPrintError(res, "xnUSBInit failed");
        return false;
    }
    
    // Open all "Kinect motor" USB devices
    res = xnUSBEnumerateDevices(0x045E /* VendorID */  , 0x02B0 /*ProductID*/, &paths, &count);
    if (res != XN_STATUS_OK)
    {
        xnPrintError(res, "xnUSBEnumerateDevices failed");
        return false;
    }
    
    // Open devices
    for (XnUInt32 index = 0; index < count; ++index)
    {
        res = xnUSBOpenDeviceByPath(paths[index], &m_devs[index]);
        if (res != XN_STATUS_OK) {
            xnPrintError(res, "xnUSBOpenDeviceByPath failed");
            return false;
        }
    }
    
    m_num = count;
    XnUChar buf[1]; // output buffer
    
    // Init motors
    for (XnUInt32 index = 0; index < m_num; ++index)
    {
        res = xnUSBSendControl(m_devs[index], (XnUSBControlType) 0xc0, 0x10, 0x00, 0x00, buf, sizeof(buf), 0);
        if (res != XN_STATUS_OK) {
            xnPrintError(res, "xnUSBSendControl failed");
            Close();
            return false;
        }
        
        res = xnUSBSendControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x06, 0x01, 0x00, NULL, 0, 0);
        if (res != XN_STATUS_OK) {
            xnPrintError(res, "xnUSBSendControl failed");
            Close();
            return false;
        }
    }
    
    m_isOpen = true;

}
void CameraKinect::updateDepthTexture() 
{
    const XnDepthPixel* pDepth = g_depthMD.Data();
    
	// Calculate the accumulative histogram (the yellow display...)
	xnOSMemSet(g_pDepthHist, 0, MAX_DEPTH*sizeof(float));
    
	unsigned int nNumberOfPoints = 0;
	for (XnUInt y = 0; y < g_depthMD.YRes(); ++y)
	{
		for (XnUInt x = 0; x < g_depthMD.XRes(); ++x, ++pDepth)
		{
			if (*pDepth != 0)
			{
				g_pDepthHist[*pDepth]++;
				nNumberOfPoints++;
			}
		}
	}
	for (int nIndex=1; nIndex<MAX_DEPTH; nIndex++)
	{
		g_pDepthHist[nIndex] += g_pDepthHist[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (int nIndex=1; nIndex<MAX_DEPTH; nIndex++)
		{
			g_pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (g_pDepthHist[nIndex] / nNumberOfPoints)));
		}
	}
    
    // check if we need to draw depth frame to texture
	if (g_nViewState == DISPLAY_MODE_OVERLAY ||
		g_nViewState == DISPLAY_MODE_DEPTH)
	{
		const XnDepthPixel* pDepthRow = g_depthMD.Data();
		XnRGB24Pixel* pTexRow = g_pTexMap + g_depthMD.YOffset() * g_nTexMapX;
        
		for (XnUInt y = 0; y < g_depthMD.YRes(); ++y)
		{
			const XnDepthPixel* pDepth = pDepthRow;
			XnRGB24Pixel* pTex = pTexRow + g_depthMD.XOffset();
            
			for (XnUInt x = 0; x < g_depthMD.XRes(); ++x, ++pDepth, ++pTex)
			{
				if (*pDepth != 0)
				{
					int nHistValue = g_pDepthHist[*pDepth];
					pTex->nRed = nHistValue;
					pTex->nGreen = nHistValue;
					pTex->nBlue = 0;
				}
			}
            
			pDepthRow += g_depthMD.XRes();
			pTexRow += g_nTexMapX;
		}
	}
    
    //    pColorData = pTexRow;
    //    
    //	// Create the OpenGL texture map
    //	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    //	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGB, g_nTexMapX, g_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, g_pTexMap);
    //    
    //   mDepthTexture = gl::Texture( GL_TEXTURE_2D, 1, g_nTexMapX, g_nTexMapY, false );
    
    //mDepthTexture = gl::Texture( getDepthImage( g_pTexMap ) );
}


void CameraKinect::updateVideoTexture() 
{
	xnOSMemSet(g_pTexMap, 0, g_nTexMapX*g_nTexMapY*sizeof(XnRGB24Pixel));
    
    const XnRGB24Pixel* pImageRow = g_imageMD.RGB24Data();
    XnRGB24Pixel* pTexRow = g_pTexMap + g_imageMD.YOffset() * g_nTexMapX;
    
    for (XnUInt y = 0; y < g_imageMD.YRes(); ++y)
    {
        const XnRGB24Pixel* pImage = pImageRow;
        XnRGB24Pixel* pTex = pTexRow + g_imageMD.XOffset();
        
        for (XnUInt x = 0; x < g_imageMD.XRes(); ++x, ++pImage, ++pTex)
        {
            *pTex = *pImage;
        }
        
        pImageRow += g_imageMD.XRes();
        pTexRow += g_nTexMapX;
    }
    
    //	// Create the OpenGL texture map
    //	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    //	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_nTexMapX, g_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, g_pTexMap);
    //    
    //    //printf("%i %i \n", g_imageMD.XRes(), g_imageMD.YRes());
    //    
    //    mVideoTexture = gl::Texture( GL_TEXTURE_2D, 0, g_nTexMapX, g_nTexMapY, false );
    //    mDepthTexture = gl::Texture( GL_TEXTURE_2D, 1, g_nTexMapX, g_nTexMapY, false ); //!!need this here for some reason for mVideoTexture to work??
    
    //mVideoTexture = gl::Texture( getColorImage( g_imageMD.GetData() ) );
}
void CameraKinect::update() 
{
	//XnStatus nRetVal = context.WaitAnyUpdateAll();
    //CHECK_RC(nRetVal, "Wait and Update"); //!!here is where to check for errors
	//g_pSessionManager->Update(&context);
    
    g_DepthGenerator.GetMetaData(g_depthMD);
	//g_ImageGenerator.GetMetaData(g_imageMD);
}

Surface8u CameraKinect::getVideoImage()
{
    //    mVideoTexture = Surface8u( getColorSource( (uint8_t *)g_imageMD.Data() ) );
    //    return mVideoTexture;
    
    return Surface8u( getColorSource( (uint8_t *)g_imageMD.Data() ) );
}

Surface16u CameraKinect::getDepthImage()
{
    //    mDepthTexture = Surface16u( getDepthSource( (uint16_t *)g_depthMD.Data() ) );
    //    return mDepthTexture;
    
    return Surface16u( getDepthSource( (uint16_t *)g_depthMD.Data() ) );
}

ImageSourceRef CameraKinect::getColorSource( uint8_t* pData )
{
    // register a reference to the active buffer
    //uint8_t *activeColor = _device0->getColorMap();
    return ImageSourceRef( new ImageSourceKinectColor( pData, g_imageMD.FullXRes(), g_imageMD.FullYRes() ) );
}

ImageSourceRef CameraKinect::getDepthSource( uint16_t* pData )
{
    // register a reference to the active buffer
    //uint16_t *activeDepth = _device0->getDepthMap();
    return ImageSourceRef( new ImageSourceKinectDepth( pData, g_imageMD.FullXRes(), g_imageMD.FullYRes() ) );
}

void CameraKinect::draw()
{
    gl::draw(gl::Texture(getDepthImage()));  
}


bool CameraKinect::Move(int angle) 
{ 
    XnStatus res;
    
    // Send move control requests
    for (XnUInt32 index = 0; index < m_num; ++index)
    {
        res = xnUSBSendControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x31, angle, 0x00, NULL, 0, 0);
        
        if (res != XN_STATUS_OK)
        {
            xnPrintError(res, "xnUSBSendControl failed");
            return false;
        }
    }
    return true;
} 

void CameraKinect::Close()
{
    if (m_isOpen) {
        for (XnUInt32 index = 0; index < m_num; ++index) {
            xnUSBCloseDevice(m_devs[index]);
        }
        m_isOpen = false;
    }
}
/*

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
    mTargetPosition = Vec3f::zero();
    mThreshold  =   80.0f;
    mBlobMin    =   200.0f;
    mBlobMax    =   200.0f;

}

void CameraKinect::update()
{
    // Update textures
	//mColorTex.update( getColorImage() );
	mDepthTex = getDepthImage();	// Histogram
    mDepthSurface = getDepthImage();

    if(mDepthTex)
    {
        cv::Mat input( toOcv( Channel8u( mDepthSurface )  ) ), blurred, thresholded, thresholded2, output;    
    
        cv::blur(input, blurred, cv::Size(10,10));
        // make two thresholded images one to display and one
        // to pass to find contours since its process alters the image
        cv::threshold( blurred, thresholded, mThreshold, 255,  CV_THRESH_BINARY);
        cv::threshold( blurred, thresholded2, mThreshold, 255,  CV_THRESH_BINARY);
        
        // 2d vector to store the found contours
        vector<vector<cv::Point> > contours;
        // find em
        cv::findContours(thresholded, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        
        // convert theshold image to color for output
        // so we can draw blobs on it
        cv::cvtColor( thresholded2, output, CV_GRAY2RGB );
        
        // loop the stored contours
        for (vector<vector<cv::Point> >::iterator it=contours.begin() ; it < contours.end(); it++ ){
            
            // center abd radius for current blob
            cv::Point2f center;
            float radius;
            // convert the cuntour point to a matrix 
            vector<cv::Point> pts = *it;
            cv::Mat pointsMatrix = cv::Mat(pts);
            // pass to min enclosing circle to make the blob 
            cv::minEnclosingCircle(pointsMatrix, center, radius);
            
            cv::Scalar color( 0, 255, 0 );
            
            if (radius > mBlobMin && radius < mBlobMax) {
                // draw the blob if it's in range
                cv::circle(output, center, radius, color);
                
                //update the target position
                mTargetPosition.x = 640 - center.x;
                mTargetPosition.y = center.y;
                mTargetPosition.z = 0;
            }
            
            
        }
        
        mCvTexture = gl::Texture( fromOcv( output ) );
    }

    /*
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
*/

//}

   /*bool CameraKinect::hasUser()
{
 if(_manager->hasUsers() && _manager->hasUser(1) )
    {return true;}
    if(_manager->hasUsers() && _manager->hasUser(2) )
    {return true;}
    else
    {return false;}

}*/
/*
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
    
    
//	gl::setMatricesWindow(getWindowSize() );
    if(mDepthTex)
    {   cout << "DepthTex Ready" << endl;
        //gl::color( cinder::ColorA(1, 1, 1, 1) );
        gl::draw( mDepthTex, Vec2i(0,0) );
        
    }
    if ( mCvTexture )
    {
        gl::draw( mCvTexture,Vec2i(640, 0));
        cout << "CvTexture Ready" << endl;
    }
    gl::color(Colorf(1.0f, 1.0f, 1.0f));
    gl::drawSphere(mTargetPosition, 10.0f);
    float sx = 320;
	float sy = 240;
	float xoff = 100;
	float yoff = 100;
	//glEnable( GL_TEXTURE_2D );
	//gl::color( cinder::ColorA(1, 1, 1, 1) );
	//if( _manager->hasUsers() && _manager->hasUser(1) ) //gl::draw( mOneUserTex, Rectf( 0, 0, getWindowWidth(), getWindowHeight()) );
	//gl::draw( mColorTex, Rectf( xoff+sx*1, yoff, xoff+sx*2, yoff+sy) );
    
    gl::disableDepthWrite();
    gl::disableDepthRead();

    glPushMatrix();
    gl::scale(Vec3f(-0.5, 0.5, 1));
    if(mDepthTex)
    {   cout << "DepthTex Ready" << endl;
        //gl::color( cinder::ColorA(1, 1, 1, 1) );
        gl::draw( mDepthTex, Vec2i(0,0) );

    }
    if ( mCvTexture )
    {
        gl::draw( mCvTexture,Vec2i(500, 500));
        cout << "CvTexture Ready" << endl;
    }
    glPopMatrix();
    
    gl::enableDepthWrite();
    gl::enableDepthRead();
    
    gl::color(Colorf(1.0f, 1.0f, 1.0f));
    gl::drawSphere(mTargetPosition, 10.0f);
	if( _manager->hasUsers() && _manager->hasUser(1) )
	{
		// Render skeleton if available
		//_manager->renderJoints( 3 );
        
		// Get list of available bones/joints
		// Do whatever with it
		//V::UserBoneList boneList = _manager->getUser(1)->getBoneList();

    }

    
}*/