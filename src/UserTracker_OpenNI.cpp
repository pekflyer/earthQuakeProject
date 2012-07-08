#include "UserTracker_OpenNI.h"

UserTracker_OpenNI::UserTracker_OpenNI()
{   
    g_pTexMap = NULL;
    g_nTexMapX = 0;
    g_nTexMapY = 0;
    g_nViewState = DISPLAY_MODE_DEPTH;
    g_SessionState = NOT_IN_SESSION;
    
    ////// Gesture Tracking ///////////////////////////////////////////////////////
	XnStatus rc = XN_STATUS_OK;
	EnumerationErrors errors;
    
	// Initialize OpenNI
	rc = context.InitFromXmlFile(SAMPLE_XML_CONFIG_PATH, &errors);
	CHECK_RC(rc, "InitFromXmlFile");
    
	rc = context.FindExistingNode(XN_NODE_TYPE_HANDS, g_HandsGenerator);
	CHECK_RC(rc, "Find hands generator");
    
	// Create NITE objects
	g_pSessionManager = new XnVSessionManager;
    //	rc = g_pSessionManager->Initialize(&context, "Click,Wave", "RaiseHand");
    rc = g_pSessionManager->Initialize(&context, GESTURE_RAISE_HAND, GESTURE_RAISE_HAND);
	CHECK_RC(rc, "SessionManager::Initialize");
    
	g_pSessionManager->RegisterSession(NULL, SessionStarting, SessionEnding, FocusProgress);
    
    XnStatus nRetVal = XN_STATUS_OK;
    
    nRetVal = g_GestureGenerator.Create(context);
    nRetVal = g_HandsGenerator.Create(context);
    CHECK_RC(nRetVal, "Create Gesture & Hands Generators");
    
    // Register to callbacks
    XnCallbackHandle h1, h2;
    g_GestureGenerator.RegisterGestureCallbacks(Gesture_Recognized, Gesture_Process, NULL, h1);
    g_HandsGenerator.RegisterHandCallbacks(Hand_Create, Hand_Update, Hand_Destroy, NULL, h2);
    
    //error checking
    XnCallbackHandle hDummy;
    context.RegisterToErrorStateChange(onErrorStateChanged, NULL, hDummy);
    
    //Make it start generating data
    nRetVal = context.StartGeneratingAll();
    CHECK_RC(nRetVal, "Start Generating All Data");
    //nRetVal = g_GestureGenerator.AddGesture(GESTURE_RAISE_HAND, boundingBox);
    //nRetVal = g_GestureGenerator.AddGesture(GESTURE_CLICK, boundingBox);
    //nRetVal = g_GestureGenerator.AddGesture(GESTURE_WAVE, boundingBox);
    ///////////////////////////////////////////////////////////////////////////////
    
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
    //    mDepthImage = Surface8u(g_depthMD.FullXRes(), g_depthMD.FullYRes(), true);
    //    mVideoImage = Surface8u(g_depthMD.FullXRes(), g_depthMD.FullYRes(), true);
    //mVideoTexture = Surface8u();
    //mDepthTexture = Surface16u();
    
    //!!
    //printf("Res's: %i %i %i %i", GL_WIN_SIZE_X, GL_WIN_SIZE_Y, g_depthMD.FullXRes(), g_depthMD.FullYRes());
    
	g_nTexMapX = (((unsigned short)(g_depthMD.FullXRes()-1) / 512) + 1) * 512;
	g_nTexMapY = (((unsigned short)(g_depthMD.FullYRes()-1) / 512) + 1) * 512;
	g_pTexMap = (XnRGB24Pixel*)malloc(g_nTexMapX * g_nTexMapY * sizeof(XnRGB24Pixel));
    
    glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
    ///////////////////////////////////////////////////////////////////////////////
    
    g_DepthGenerator.GetAlternativeViewPointCap().SetViewPoint(g_ImageGenerator);
    g_HandsGenerator.SetSmoothing(0.1f);
}

void UserTracker_OpenNI::updateVideoTexture() 
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

void UserTracker_OpenNI::updateDepthTexture() 
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

void UserTracker_OpenNI::update() 
{
	XnStatus nRetVal = context.WaitAnyUpdateAll();
    CHECK_RC(nRetVal, "Wait and Update"); //!!here is where to check for errors
	g_pSessionManager->Update(&context);
    
    g_DepthGenerator.GetMetaData(g_depthMD);
	g_ImageGenerator.GetMetaData(g_imageMD);
}

void UserTracker_OpenNI::draw() 
{    
    // Copied from SimpleViewer
	// Clear the OpenGL buffers
	//glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //	// Display the OpenGL texture map
	//glColor4f(1,1,1,1);
    //    
    //	glBegin(GL_QUADS);
    //    
    //	int nXRes = g_depthMD.FullXRes();
    //	int nYRes = g_depthMD.FullYRes();
    //    
    //	// upper left
    //	glTexCoord2f(0, 0);
    //	glVertex2f(0, 0);
    //	// upper right
    //	glTexCoord2f((float)nXRes/(float)g_nTexMapX, 0);
    //	glVertex2f(GL_WIN_SIZE_X, 0);
    //	// bottom right
    //	glTexCoord2f((float)nXRes/(float)g_nTexMapX, (float)nYRes/(float)g_nTexMapY);
    //	glVertex2f(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
    //	// bottom left
    //	glTexCoord2f(0, (float)nYRes/(float)g_nTexMapY);
    //	glVertex2f(0, GL_WIN_SIZE_Y);
    //    
    //	glEnd();
    
    
    //    switch (g_nViewState) 
    //    {
    //        case DISPLAY_MODE_DEPTH:
    //            gl::draw(mDepthTexture);
    //            break;
    //        case DISPLAY_MODE_IMAGE:
    //gl::draw( mVideoTexture );
    //            break;
    //        case DISPLAY_MODE_OVERLAY:
    //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    //gl::draw( getVideoImage(), getWindowBounds() );
    //gl::draw( getDepthImage(), getWindowBounds() );
    //            break;    
    //        default:
    //            break;
    //    }
    
    //printf("%i \n", handpoints.size() );
    
    glColor4f(1, 1, 1, 0.6);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 10.0f);
    for ( int i = 0; i < handpoints.size(); i++ )
    {
        //gl::drawCube(mHandPoints.at(i).position, ci::Vec3f(30.0f, 30.f, 30.0f));
        gl::drawSolidCircle( ci::Vec2f( handpoints.at(i).position.x, handpoints.at(i).position.y), 40.0f);
    }
    glPopMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    
	// Swap the OpenGL display buffers
	//glutSwapBuffers();
}

//
//
//void UserTracker_OpenNI::keyDown( KeyEvent event ) 
//{
//    switch (event.getCode())
//	{
//		case 27:
//			exit (1);
//		case '1':
//			g_nViewState = DISPLAY_MODE_OVERLAY;
//			//g_DepthGenerator.GetAlternativeViewPointCap().SetViewPoint(g_ImageGenerator);
//			break;
//		case '2':
//			g_nViewState = DISPLAY_MODE_DEPTH;
//			//g_DepthGenerator.GetAlternativeViewPointCap().ResetViewPoint();
//			break;
//		case '3':
//			g_nViewState = DISPLAY_MODE_IMAGE;
//			//g_DepthGenerator.GetAlternativeViewPointCap().ResetViewPoint();
//			break;
//		case 'm':
//			context.SetGlobalMirror(!context.GetGlobalMirror());
//			break;
//	}
//}

void UserTracker_OpenNI::addHand( int id, ci::Vec3f position )
{
    HandPoint newHand = HandPoint();
    newHand.id = id;
    newHand.position = position;
    newHand.prevPosition = position;
    
    handpoints.push_back( newHand );
}

void UserTracker_OpenNI::removeHand( int id )
{
    int index = findHandPointIndex(id);
    handpoints.erase( handpoints.begin() + index );
}

int UserTracker_OpenNI::findHandPointIndex(int id)
{
    int index = -1;
    HandPoint tempHand;
    for (int i = 0; i < handpoints.size(); i++)
    {
        tempHand = handpoints.at(i);
        
        if(tempHand.id == id)
        {
            index = i;
            break;
        }
    }
    
    return index;
}

Surface8u UserTracker_OpenNI::getVideoImage()
{
    //    mVideoTexture = Surface8u( getColorSource( (uint8_t *)g_imageMD.Data() ) );
    //    return mVideoTexture;
    
    return Surface8u( getColorSource( (uint8_t *)g_imageMD.Data() ) );
}

Surface16u UserTracker_OpenNI::getDepthImage()
{
    //    mDepthTexture = Surface16u( getDepthSource( (uint16_t *)g_depthMD.Data() ) );
    //    return mDepthTexture;
    
    return Surface16u( getDepthSource( (uint16_t *)g_depthMD.Data() ) );
}

ImageSourceRef UserTracker_OpenNI::getColorSource( uint8_t* pData )
{
    // register a reference to the active buffer
    //uint8_t *activeColor = _device0->getColorMap();
    return ImageSourceRef( new ImageSourceKinectColor( pData, g_imageMD.FullXRes(), g_imageMD.FullYRes() ) );
}

ImageSourceRef UserTracker_OpenNI::getDepthSource( uint16_t* pData )
{
    // register a reference to the active buffer
    //uint16_t *activeDepth = _device0->getDepthMap();
    return ImageSourceRef( new ImageSourceKinectDepth( pData, g_imageMD.FullXRes(), g_imageMD.FullYRes() ) );
}

void XN_CALLBACK_TYPE UserTracker_OpenNI::Gesture_Recognized(GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie)
{
	printf("Gesture recognized: %s\n", strGesture);
	//g_GestureGenerator.RemoveGesture(strGesture);
    
    if ( string(strGesture).compare( string( GESTURE_WAVE ) ) == 0 )
    {
        //g_HandsGenerator.StartTracking(*pEndPosition);
    }
}

void XN_CALLBACK_TYPE UserTracker_OpenNI::Gesture_Process(GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pPosition, XnFloat fProgress, void* pCookie) {
	//printf("Gesture process: %s\n", strGesture);
    printf("Gesture Process: %s @ (%f,%f,%f)\n",strGesture, pPosition->X, pPosition->Y, pPosition->Z);
}

void XN_CALLBACK_TYPE UserTracker_OpenNI::Hand_Create(HandsGenerator& generator, XnUserID nId, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie)
{
	printf("New Hand: %d @ (%f,%f,%f)\n", nId, pPosition->X, pPosition->Y, pPosition->Z);
    
    //g_DepthGenerator.ConvertProjectiveToRealWorld(XnUInt32(1), { XnPoint3D(pPosition->X, pPosition->Y, pPosition->Z) }, aRealWorld[]);
    //Point3D pt = depthGen.convertRealWorldToProjective(realPt);
    addHand( nId, ci::Vec3f((pPosition->X + APP_RES_X/2.0f) * APP_RES_X/640.0f, (-pPosition->Y + APP_RES_Y/2.0f) * APP_RES_Y/320.0f, pPosition->Z));
}

void XN_CALLBACK_TYPE UserTracker_OpenNI::Hand_Update(HandsGenerator& generator, XnUserID nId, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie) 
{
    //printf("Update Hand: %d @ (%f,%f,%f)\n", nId, pPosition->X, pPosition->Y, pPosition->Z);
    
    int index = findHandPointIndex( nId );
    handpoints[index].prevPosition = handpoints[index].position;
    handpoints[index].position = ci::Vec3f( pPosition->X + APP_RES_X/2.0f, -pPosition->Y + APP_RES_Y/2.0f, pPosition->Z );
}
void XN_CALLBACK_TYPE UserTracker_OpenNI::Hand_Destroy(HandsGenerator& generator, XnUserID nId, XnFloat fTime, void* pCookie)
{
	printf("Lost Hand: %d\n", nId);
    
    removeHand( nId );
    //g_HandsGenerator.StopTracking( nId ); //already calls this apparently
	//g_GestureGenerator.AddGesture(GESTURE_WAVE, NULL);
}

// Callback for when the focus is in progress
void XN_CALLBACK_TYPE UserTracker_OpenNI::FocusProgress(const XnChar* strFocus, const XnPoint3D& ptPosition, XnFloat fProgress, void* UserCxt)
{
	printf("Focus progress: %s @(%f,%f,%f): %f\n", strFocus, ptPosition.X, ptPosition.Y, ptPosition.Z, fProgress);
}

// callback for session start
void XN_CALLBACK_TYPE UserTracker_OpenNI::SessionStarting(const XnPoint3D& ptPosition, void* UserCxt)
{
	printf("Session start: (%f,%f,%f)\n", ptPosition.X, ptPosition.Y, ptPosition.Z);
	g_SessionState = IN_SESSION;
}

// Callback for session end
void XN_CALLBACK_TYPE UserTracker_OpenNI::SessionEnding(void* UserCxt)
{
	printf("Session end\n");
	g_SessionState = NOT_IN_SESSION;
}

void XN_CALLBACK_TYPE UserTracker_OpenNI::NoHands(void* UserCxt)
{
	if (g_SessionState != NOT_IN_SESSION)
	{
		printf("Quick refocus\n");
		g_SessionState = QUICK_REFOCUS;
	}
}

void XN_CALLBACK_TYPE UserTracker_OpenNI::onErrorStateChanged(XnStatus errorState, void* pCookie) 
{ 
    if (errorState != XN_STATUS_OK) 
    { 
        printf("Error: %s\n", xnGetStatusString(errorState)); 
    } 
} 
