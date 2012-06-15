#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Font.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"

#include <sstream>
#include "Resources.h"
#include "cinder/Perlin.h"
#include "cinder/Text.h"

#include "cameraKinect.h"
#include "kinectMotor.h"
#include "cinder/params/Params.h"

#include <iostream>
#include <boost/thread.hpp>
//#include "mysql_connector.h"

//#include "particleController.h"
#include "cinder/Camera.h"
#include "nameController.h"

//#include "nameParticle.h"

using namespace ci;
using namespace ci::app;
using namespace std;
//STHeiti Medium
gl::Texture *particleImg;
Font          mFont;
bool          videoSwitcher;
int           counter = 0;
float         widthName = 300;
float         heightName = 350;
float         mThreshold, mBlobMin, mBlobMax;


class earthQuakeProjectApp : public AppBasic {
  public:
    Renderer* prepareRenderer() { return new RendererGl( RendererGl::AA_MSAA_2 ); }

	void setup();
	void mouseDown( MouseEvent event );	
    void prepareSettings( Settings *settings );
    void keyDown(KeyEvent event);
	void update();
	void draw();
    //vector<Vec2i> detectLoc(Surface* surface, Vec2i offset);
    
    //Kinect CAMERA=======================================
    CameraKinect        *mCamera;
    //KinectMotors        KM;
    int                 motorMove;
    bool                hasUser;
    // CAMERA
	CameraPersp			mCam;
	Quatf				mSceneRotation;
	Vec3f				mEye, mCenter, mUp;
	float				mCameraDistance;
    
    //FONT

    //Shape2d    mShape;
    //Vec2i      fontLoc;
    //vector<Vec2i> tttlLoc;
    //vector<vector<Vec2i> > totalLocs;
    
    //Surface    mSurface;
    //gl::Texture mSimpleTexture;
    //particles
   
    //particleController mParticles;
    //Vec2i         mouseLoc;
    //vector<Vec2i> ttl;
    //Perlin sPerlin;
    nameController      loadName;
    
    //background movie
    qtime::MovieGl      mMovie;
    gl::Texture         mFrameTexture;

    
    particleController mParticleController;
	float				zoneRadiusSqrd;
	float				mThresh;

    bool                flatten;
    bool				mCentralGravity;
    
    //PARAMS
    params::InterfaceGl mParams;
    

   
    
};


void earthQuakeProjectApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1280, 720 );
	settings->setFrameRate( 30.0f );
	//settings->setFullScreen( true );
    //settings->setResizable( false );
    //settings->enableSecondaryDisplayBlanking( false );

}
void earthQuakeProjectApp::setup()
{
    Rand::randomize();
    //mysql_connector mc;
    //mc.mysql_connect();
    //
    //boost::thread t(nameController);
    flatten         = false;
    mCentralGravity = true;
    zoneRadiusSqrd  = 200.0f;
	mThresh			= 1.0f;
    hasUser         = false;
    try {
    mMovie = qtime::MovieGl(loadResource("bg.mov"));
    mMovie.setLoop();
    mMovie.play();
    }
    catch( ... ) {
		console() << "Unable to load the movie." << std::endl;
		mMovie.reset();

	}
    mFrameTexture.reset();
    
    loadName.init();
    loadName.addSecParticles();
   // if(!KM.Open())
     //   printf("km is not working\n");
    mCamera = &CameraKinect::getInstance();
    motorMove = 0;
    
    // SETUP CAMERA
	mCameraDistance = 500.0f;
	mEye			= Vec3f( 0.0f, 0.0f, mCameraDistance );
    mCenter			= Vec3f::zero();
    
	mUp				= Vec3f::yAxis();
    mCam = CameraPersp( getWindowWidth(), getWindowHeight(), 75.0f );
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 50.0f, 2000.0f );
    

    videoSwitcher = true;
   // mCamera.setup();

   
    particleImg = new gl::Texture( loadImage( loadResource( RES_PARTICLE ) ) );
    mParams = params::InterfaceGl( "Flocking", Vec2i( 200, 240 ) );
    
	mParams.addParam( "Scene Rotation", &mSceneRotation, "opened=1" );
	mParams.addSeparator();
	mParams.addParam( "Eye Distance", &mCameraDistance, "min=50.0 max=1500.0 step=50.0 keyIncr=s keyDecr=w" );    
    mParams.addParam( "PullToCenter", &mCentralGravity, "keyIncr=g" );
    mParams.addParam( "Flatten", &flatten, "keyIncr=h" );
    mParams.addSeparator();
    mParams.addParam( "KinectTilt", &motorMove, "min=-60, max=60, step=10, keyIncr=x keyDecr=c" );
    mParams.addParam( "NameX", &widthName, "min=-500, max=800, step=25, keyIncr=v keyDecr=b" );
    mParams.addParam( "NameY", &heightName, "min=-500, max=800, step=25, keyIncr=h keyDecr=n" );
    mParams.addSeparator();
    mParams.addParam("Kinect_Threshold", &mThreshold, "min=0.0 max=255.0 step=1.0 keyIncr=u keyDecr=j");
    mParams.addParam( "Blob Minimum Radius", &mBlobMin, "min=1.0 max=500.0 step=1.0 keyIncr=e keyDecr=d" );
    mParams.addParam( "Blob Maximum Radius", &mBlobMax, "min=1.0 max=500.0 step=1.0 keyIncr=r keyDecr=f" );
}

void earthQuakeProjectApp::mouseDown( MouseEvent event )
{
    
}

void earthQuakeProjectApp::keyDown(KeyEvent event)
{
    if( event.getCode() == KeyEvent::KEY_ESCAPE )
	{
        //mCamera.shutdown();
        delete mCamera;
		this->quit();
		this->shutdown();
        
	}
    else if (event.getCode() == KeyEvent::KEY_o)
    {
        setFullScreen(true);
    }
}

void earthQuakeProjectApp::update()
{
    if( mMovie )
		mFrameTexture = mMovie.getTexture();

     mCamera->update();
    //mParticleController.applyForceToParticles( mZoneRadius, mLowerThresh, mHigherThresh, mAttractStrength, mRepelStrength, mOrientStrength);
    //mParticleController.update(flatten);
    loadName.applyForceToNames(	zoneRadiusSqrd*zoneRadiusSqrd,mThresh);
    if(mCentralGravity) loadName.pullToCenter(mCenter);
   /* if(mCamera.hasUser()) 
    {
        //Vec3f loc = mCamera.getUserLoc();
        if(hasUser)
        {
           // loadName.resetNameID();
            //loadName.findUserName(mCamera.getUserLoc());
            hasUser = false;
            printf("HasUser\n");
            //if(mMovie)
            //mMovie.stop();
       }
        
       // Vec3f loc = loadName.getUserNamePos();
    //    loadName.setUserLoc(mCamera.getUserLoc());
    //    loadName.update(flatten,true);
        //the flow of app here is not clear, whether all the particle system shall be halted while a user is detected. 

    }
    else*/
    {
       //mmmCamera.DeleteUser();
       loadName.update(flatten,false);
       hasUser = true;
    loadName.resetExtraInfo();
    

       
    
    }
    mEye = Vec3f( 0.0f, 0.0f, mCameraDistance );
    mCam.lookAt( mEye, mCenter, mUp );
    gl::setMatrices( mCam );
    gl::rotate( mSceneRotation );

       //Move Kinect Motor=========================
  // if(mCamera.Open())
     mCamera->Move(motorMove);
    //KM.Move(motorMove);
   //ci::sleep(1000);

}

void earthQuakeProjectApp::draw()
{
    glClearColor( 0, 0, 0, 0 );
   // gl::clear( Color( 0.5f, 0.5f, 0.5f ) );

     glClear( GL_COLOR_BUFFER_BIT );
     glClear( GL_DEPTH_BUFFER_BIT );
     gl::enableAlphaBlending();

//OPENGL Draw=========================================================================
      glDepthMask( GL_FALSE );
	  glDisable( GL_DEPTH_TEST );
	  glEnable( GL_BLEND );
	  glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    
//Particle System Drawing=====================================================
    //loadName.draw();

    //gl::color(ColorA(1,0,0,1.0f));
    loadName.draw();

    //Kinect SIGNAL================================================
   /* if(mCamera.hasUser())
    {
        mCamera.draw();
        //gl::pushMatrices();
        gl::color(ColorA(0.5,0.5,0.7, 1.0f));
        //Vec3f loc = mCamera.getUserLoc();
        //gl::drawSphere(Vec3f(loc.x-widthName, loc.y-heightName, 400),5.0f,24);
        //gl::popMatrices();
        
    }*/

    gl::setMatricesWindow(getWindowSize());
    gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
    gl::drawString( toString((int) getAverageFps()) + " fps", Vec2f(0.0f, 52.0f));



    //bg drawing
    if( mFrameTexture ) {
		Rectf centeredRect = Rectf( mFrameTexture.getBounds() ).getCenteredFit( getWindowBounds(), true );
       // gl::color( ColorA( 1.0f, 0.5f, 0.5f, 1.0f ) );
        gl::draw( mFrameTexture, Vec2f(0,0)  );
    }
   // mCamera.draw();
    //params gui=================================================
    params::InterfaceGl::draw();
    

}

void renderImage( Vec3f _loc, float _diam, Color _col, float _alpha )
{
	glPushMatrix();
	glTranslatef( _loc.x, _loc.y, _loc.z );
	glScalef( _diam, _diam, _diam );
	glColor4f( _col.r, _col.g, _col.b, _alpha );
	glBegin( GL_QUADS );
    glTexCoord2f(0, 0);    glVertex2f(-.5, -.5);
    glTexCoord2f(1, 0);    glVertex2f( .5, -.5);
    glTexCoord2f(1, 1);    glVertex2f( .5,  .5);
    glTexCoord2f(0, 1);    glVertex2f(-.5,  .5);
	glEnd();
	glPopMatrix();
}


CINDER_APP_BASIC( earthQuakeProjectApp, RendererGl(0) )
