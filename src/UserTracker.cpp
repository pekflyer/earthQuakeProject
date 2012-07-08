//
//  UserTracker.cpp
//  earthQuakeProject
//
//  Created by Allan Yong on 12-07-07.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "UserTracker.h"



UserTracker::UserTracker()
{
    g_pTexMap = NULL;
    g_nTexMapX = 0;
    g_nTexMapY = 0;
    g_nViewState = DISPLAY_MODE_DEPTH;
    g_SessionState = NOT_IN_SESSION;
    g_bNeedPose = false;
    // g_strPose[20] = "";
    
    ////// Gesture Tracking ///////////////////////////////////////////////////////
	XnStatus rc = XN_STATUS_OK;
	EnumerationErrors errors;
    
	// Initialize OpenNI
	rc = m_Context.InitFromXmlFile(SAMPLE_XML_CONFIG_PATH,m_ScriptNode, &errors);
    
	CHECK_RC(rc, "InitFromXmlFile");
    
    rc = m_Context.FindExistingNode(XN_NODE_TYPE_USER, m_UserGenerator);
    if(rc != XN_STATUS_OK)
    {
        rc = m_UserGenerator.Create(m_Context);
       CHECK_RC(rc, "Find User Generator");
    }
    rc = m_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, m_DepthGenerator);
    if (rc != XN_STATUS_OK)
	{
		printf("No depth generator found. Using a default one...");
        CHECK_RC(rc ,"Find Depth generator");
    }
    
    
    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected, hCalibrationInProgress, hPoseInProgress;
    
    if(!m_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
    {
        printf("Supplied user generator dosn't support skeleton\n");
        exit(1);
    }
    
    rc = m_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, (xn::UserGenerator *) &m_UserGenerator, hUserCallbacks);
    CHECK_RC(rc, "Register to user callbacks");
	rc = m_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, (xn::UserGenerator *) &m_UserGenerator, hCalibrationStart);
	CHECK_RC(rc, "Register to calibration start");
	rc = m_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, (xn::UserGenerator *) &m_UserGenerator, hCalibrationComplete);
	CHECK_RC(rc, "Register to calibration complete");
    
    if (m_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
	{
		g_bNeedPose = TRUE;
		if (!m_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
		{
			printf("Pose required, but not supported\n");
		}
		rc = m_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, (xn::UserGenerator *) &m_UserGenerator, hPoseDetected);
		CHECK_RC(rc, "Register to Pose Detected");
		m_UserGenerator.GetSkeletonCap().GetCalibrationPose(g_strPose);
        
		rc = m_UserGenerator.GetPoseDetectionCap().RegisterToPoseInProgress(MyPoseInProgress, (xn::UserGenerator *) &m_UserGenerator, hPoseInProgress);
		CHECK_RC(rc, "Register to pose in progress");
	}
    
	
    //m_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
    m_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_UPPER);
    
	rc = m_UserGenerator.GetSkeletonCap().RegisterToCalibrationInProgress(MyCalibrationInProgress, NULL, hCalibrationInProgress);
	CHECK_RC(rc, "Register to calibration in progress");
    
	rc = m_Context.StartGeneratingAll();
	CHECK_RC(rc, "StartGenerating");
    
    g_pSessionManager = new XnVSessionManager;
    // XN_SKEL_TORSO
    
    glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
    
    
}



void UserTracker::addUser(int nId, ci::Vec3f position)
{
    UserPoint newUser = UserPoint();
    newUser.id = nId;
    newUser.position = position;
    newUser.prevPosition = position;
    //for(int i = 0; i<userPoints.size();++i)
    //{
    if(userPoints.size() < 1)
    {
        userPoints.push_back(newUser);
    }   
    // }
    cout << userPoints.size() << endl;
}

void    UserTracker::removeUser(int nId)
{
    int index = findUserPointIndex(nId);
    userPoints.erase(userPoints.begin()+index);
}

int     UserTracker::findUserPointIndex(int nId)
{
    int index = -1;
    UserPoint tempUser;
    for (int i = 0; i < userPoints.size(); i++) {
        tempUser = userPoints.at(i);
        if(tempUser.id == nId)
        {
            index = i;
            break;
        }
    }
    return index;
}
void UserTracker::setUserID(XnUserID nId)
{
    trackID = nId;
}
//void UserTracker::mouseDown( MouseEvent event )
//{
//}


void XN_CALLBACK_TYPE UserTracker::User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
    XnUInt32 epochTime = 0;
	xnOSGetEpochTime(&epochTime);
	printf("%d New User %d\n", epochTime, nId);
    UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);

	// New user found
	if (g_bNeedPose)
	{
		userGenerator->GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
	}
	else
	{
       if(userGenerator)
		userGenerator->GetSkeletonCap().RequestCalibration(nId, TRUE);
	}
    
    
}

void XN_CALLBACK_TYPE UserTracker::User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
    XnUInt32 epochTime = 0;
	xnOSGetEpochTime(&epochTime);
	printf("%d Lost user %d\n", epochTime, nId);
    removeUser(nId);
}

void XN_CALLBACK_TYPE UserTracker::UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
    XnUInt32 epochTime = 0;
	xnOSGetEpochTime(&epochTime);
	//printf("%d Pose %s detected for user %d\n", epochTime, strPose, nId);
    UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
    if(userGenerator)
    {
        userGenerator->GetPoseDetectionCap().StopPoseDetection(nId);
        userGenerator->GetSkeletonCap().RequestCalibration(nId, TRUE); 
    }
    
}


void XN_CALLBACK_TYPE UserTracker::UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
    XnUInt32 epochTime = 0;
	xnOSGetEpochTime(&epochTime);
	printf("%d Calibration started for user %d\n", epochTime, nId);
    
}


void XN_CALLBACK_TYPE UserTracker::UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie)
{
    
    XnUInt32 epochTime = 0;
	xnOSGetEpochTime(&epochTime);
    XnStatus status;
    xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
	if (eStatus == XN_CALIBRATION_STATUS_OK)
	{
		// Calibration succeeded
		printf("%d Calibration complete, start tracking user %d\n", epochTime, nId);
        if(userGenerator)
		status = userGenerator->GetSkeletonCap().StartTracking(nId);
        cout << xnGetStatusString(status);

        //setUserID(nId);
        //        if(m_UserGenerator.GetSkeletonCap().IsTracking(nId))
        //        {
        XnSkeletonJointPosition Torso;
        if(userGenerator)
        userGenerator->GetSkeletonCap().GetSkeletonJointPosition(nId, XN_SKEL_TORSO, Torso);
        printf("%d: (%f,%f,%f) [%f]\n", nId,Torso.position.X, Torso.position.Y, Torso.position.Z,Torso.fConfidence);
        //printf("this is called");
        
        addUser( nId, ci::Vec3f((Torso.position.X + APP_RES_X/2.0f) * APP_RES_X/640.0f, (-Torso.position.Y + APP_RES_Y/2.0f) * APP_RES_Y/320.0f, Torso.position.Z));
        //        }
        
	}
	else
	{
		// Calibration failed
		//printf("%d Calibration failed for user %d\n", epochTime, nId);
        if(eStatus==XN_CALIBRATION_STATUS_MANUAL_ABORT)
        {
            printf("Manual abort occured, stop attempting to calibrate!");
            return;
        }
		if (g_bNeedPose)
		{
            if(userGenerator)
			userGenerator->GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
		}
		else
		{
            if(userGenerator)
			userGenerator->GetSkeletonCap().RequestCalibration(nId, TRUE);
		}
	}
}
void XN_CALLBACK_TYPE UserTracker::MyCalibrationInProgress(xn::SkeletonCapability& capability, XnUserID id, XnCalibrationStatus calibrationError, void* pCookie)
{
    m_Errors[id].first = calibrationError;
    
    
    
}
void XN_CALLBACK_TYPE UserTracker::MyPoseInProgress(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID id, XnPoseDetectionStatus poseError, void* pCookie)
{
	m_Errors[id].second = poseError;
    printf("this is called everytime right");
}


void UserTracker::update()
{
    
    //XnStatus rc = m_Context.WaitAnyUpdateAll();
     XnStatus rc = m_Context.WaitOneUpdateAll(m_UserGenerator);
    
    CHECK_RC(rc, "Wait and Update");
    
    m_DepthGenerator.GetMetaData(depthMD);
    m_UserGenerator.GetUserPixels(0, sceneMD);
    
    //m_UserGenerator.GetUsers(aUsers[1], 1);
    
    XnUInt16 nUsers = 1; 
    m_UserGenerator.GetUsers(aUsers, nUsers); 
    for (int i = 0; i < nUsers; ++i) 
    { 
        XnPoint3D com; 
        m_UserGenerator.GetCoM(aUsers[i], com); 
        
        if (com.Z == 0) //this could be your check for useful data 
        { 
            
        } 
    } 
    
    if(m_UserGenerator.GetSkeletonCap().IsTracking(1))
    {
        XnSkeletonJointPosition Torso;
        m_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(1, XN_SKEL_TORSO, Torso);
        printf("%d: (%f,%f,%f) [%f]\n", 1,Torso.position.X+ APP_RES_X/2.0f, -(-Torso.position.Y+ APP_RES_Y/2.0f), Torso.position.Z,Torso.fConfidence);
        //printf("this is called\n");
        if(userPoints.size()>0)
        {
            userPoints.at(0).prevPosition = userPoints.at(0).position;
            //userPoints.at(0).position = ci::Vec3f( Torso.position.X + APP_RES_X/2.0f, -(-Torso.position.Y + APP_RES_Y/2.0f), Torso.position.Z );
            userPoints.at(0).position = ci::Vec3f( Torso.position.X + APP_RES_X/3.0f, -(-Torso.position.Y + APP_RES_Y/2.0f), Torso.position.Z );
        }
        
    }
}

void UserTracker::draw()
{
	// clear out the window with black
	//gl::clear( Color( 0, 0, 0 ) ); 
    //gl::setMatricesWindow(getWindowSize());
    
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 10.0f);
    if(userPoints.size()>0)
    {
        //for ( int i = 0; i < userPoints.size(); ++i )
        // {
        gl::drawSolidCircle( ci::Vec2f( userPoints.at(0).position.x, userPoints.at(0).position.y), 40.0f);
        //printf("%d: (%f,%f,%f)\n", 1,userPoints.at(0).position.x, userPoints.at(0).position.y, userPoints.at(0).position.z);
        //}
    }
    glPopMatrix();
   // glColor3f(1.0f, 1.0f, 1.0f);
    
}