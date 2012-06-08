//
//  nameParticle.cpp
//  earthQuakeProject
//
//  Created by Allan Yong on 12-03-19.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "nameParticle.h"
#include "cinder/Rand.h"
#include "cinder/ip/Flip.h"
#include "Resources.h"


extern int counter;
extern float widthName;
extern float heightName;

using namespace cinder::ip;
using namespace ci;
using namespace ci::app;

const int normal_offset = 1;

nameParticle::nameParticle()
{
    
    
    
}
nameParticle::nameParticle(ci::Vec3f _loc, ci::Vec3f _vel, string name )
{
    mPos            = _loc;
    mVel            = _vel;
    mVelNormal		= Vec3f::yAxis();
	mAcc			= Vec3f::zero();
    mTitleAlpha     = 0.0f;
    mTitleColor		= Color( 0.7f, 0.7f, 0.7f ); 
    
    mNeighborPos	= Vec3f::zero();
	mNumNeighbors	= 0;
	mMaxSpeed		= Rand::randFloat( 2.5f, 3.0f );
	mMaxSpeedSqrd	= mMaxSpeed * mMaxSpeed;
	mMinSpeed		= Rand::randFloat( 1.0f, 1.5f );
	mMinSpeedSqrd	= mMinSpeed * mMinSpeed;
    
    mDecay			= 0.99f;
	mRadius			= 1.0f;
	mLength			= 5.0f;
	mFear			= 1.0f;
	mCrowdFactor	= 1.0f;
    
    //temp.setFont(Font("Hei", 50));
    temp.setFont(Font(loadResource( "华文黑体.ttf" ), 20));
    
    temp.setColor(Color(1.0f, 0.1f, 0.1f));
    temp.setText(name);
    
    tempSur         = temp.render();
    cinder::ip::flipVertical(&tempSur);
    particleAMT = (int) detectLoc(&tempSur, mPos, normal_offset).size();
    //printf("particleAMT is %d\n", particleAMT);
    if(tempSur !=  NULL){
        mParticleController.addParticles( particleAMT);
    }
    
    flatten         = true;
    mZoneRadius		= 80.0f;
	mLowerThresh	= 0.4f;
	mHigherThresh	= 0.75f;
	mAttractStrength	= 0.005f;
	mRepelStrength		= 0.01f;
	mOrientStrength		= 0.01f;

}

void nameParticle::findPerlin()
{
 	}



void nameParticle::selectUser(string info, Vec3f userLoc)
{
    
    float dur1 = 0.2f;
    float dur2 = 15.0f;
    
    extraInfo.setFont(Font("Hei", 20));
    extraInfo.setColor(Color(0.7f, 0.7f, 0.7f));
    extraInfo.setText(info);
    extraInfoSur = extraInfo.render();
    
    // title color
    timeline().apply( &mTitleColor, Color( 1, 1, 1 ), dur1, EaseInQuad() );
    // title alpha
    //timeline().apply( &mTitleAlpha, 0.0f, dur1, EaseInAtan( 10 ) );
    timeline().apply( &mTitleAlpha, 0.7f, dur2, EaseOutAtan( 10 ) );
    
    //cout << "mTitleAlpha " << mTitleAlpha << " value.\n";
    //Vec3f dirToCenter = userLoc - Vec3f::zero();
    Vec3f temp = Vec3f(userLoc.x-widthName, userLoc.y - heightName, 400.0f);
    Vec3f go2 = temp - mPos;
    
    float distToTemp = go2.length();
  

    //printf("userLoc.x is %f; userLoc.y is %f\n", userLoc.x, userLoc.y);

    

        if(distToTemp > 2)
        {
            limitSpeed();
            go2.safeNormalize();
            mPos = mPos+go2;
            
            hasAdd = true;
        }
        else
        {
            if(tempSur !=  NULL){
               if(hasAdd)
               {
                   //pullParticle = detectLoc(&tempSur, mPos);
                   //;
                   hasAdd = false;
               }
                pullParticle = detectLoc(&tempSur, mPos,normal_offset);
                mParticleController.pullToName(pullParticle);
            }
        }
}

void nameParticle::clearExtraInfo()
{
    float dur1 = 0.2f;
    float dur2 = 15.0f;
    if (extraInfoSur)
    {
    cinder::ip::flipVertical(&extraInfoSur);
        // title color
	timeline().apply( &mTitleColor, Color( 1, 1, 1 ), dur1, EaseInAtan( 10 ) );
	timeline().appendTo( &mTitleColor, Color( 0.7f, 0.7f, 0.7f ), 0.05f, EaseOutAtan( 10 ) );
        // title alpha
        //timeline().apply( &mTitleAlpha, 0.0f, dur1, EaseInAtan( 10 ) );
    timeline().apply( &mTitleAlpha, 0.0f, dur1, EaseInAtan( 10 ) );
	timeline().appendTo( &mTitleAlpha, 1.0f, dur2, EaseOutAtan( 10 ) );
        
       // cout << "mTitleAlpha " << mTitleAlpha << " value.\n";
    }

}






void nameParticle::update(bool flatten, bool findUser){
  
    if(findUser)
    {
        if(tempSur !=  NULL){
            pullParticle = detectLoc(&tempSur, mPos, normal_offset);
            mParticleController.pullToName(pullParticle);

        }
        //printf("it is called\n");
    }
    else
    {
        if( flatten ) mAcc.z = 0.0f;
        mVel += mAcc;
        mVelNormal = mVel.normalized();
        
        limitSpeed();
        
        mPos += mVel;
        
        
        if( flatten ) mPos.z = 0.0f;
        
        mVel *= mDecay;
        mAcc = Vec3f::zero();

        
        if(tempSur !=  NULL){
            pullParticle = detectLoc(&tempSur, mPos, normal_offset);
            mParticleController.pullToName(pullParticle);
           // printf("x is %f, y is %f, z is %f\n", pullParticle.at(0).x, pullParticle.at(0).y, pullParticle.at(0).z);
        }   
        
        

    }
    //mParticleController.applyForceToParticles( mZoneRadius, mLowerThresh, mHigherThresh, mAttractStrength, mRepelStrength, mOrientStrength );
    //mParticleController.pullToName(pullParticle);
   //mParticleController.update( flatten );
       //printf("z is %f\n", mPos.z);
}

void nameParticle::draw(){
    //gl::setMatricesWindow(getWindowSize());
    mParticleController.draw();
    if(tempSur)
    {
        gl::pushModelView();
        gl::pushMatrices();
    
     
        gl::draw(gl::Texture(tempSur), Vec2f(mPos.x, mPos.y/-1));
        gl::popMatrices();
        gl::popModelView();
        //cout <<"i am here \n" << std::endl;     
    }
    if (extraInfoSur) {
        //extraInfoSur
        gl::pushModelView();
        gl::pushMatrices();
            gl::color( ColorA( mTitleColor(), mTitleAlpha() ) );
            cinder::ip::flipVertical(&extraInfoSur);
            gl::draw(gl::Texture(extraInfoSur), Vec2f(mPos.x, mPos.y/-1));
        gl::popMatrices();
        gl::popModelView();
    }
}


void nameParticle::limitSpeed(){
	float vLengthSqrd = mVel.lengthSquared();
	if( vLengthSqrd > mMaxSpeedSqrd ){
		mVel = mVelNormal * mMaxSpeed;
		
	} else if( vLengthSqrd < mMinSpeedSqrd ){
		mVel = mVelNormal * mMinSpeed;
	}
}

void nameParticle::pullToCenter( const ci::Vec3f &center )
{
	Vec3f dirToCenter = mPos - center;
	float distToCenter = dirToCenter.length();
	float maxDistance = 100.0f;
	
	if( distToCenter > maxDistance ){
		dirToCenter.normalize();
		float pullStrength = 0.0001f;
		mVel -= dirToCenter * ( ( distToCenter - maxDistance ) * pullStrength );
	}

}
void nameParticle::addNeighborPos(ci::Vec3f pos)
{
	mNeighborPos += pos;
	mNumNeighbors ++;
}

vector<Vec3f> nameParticle::detectLoc(Surface* surface, Vec3f offset, int perOffset)
 {
 vector<Vec3f> ttlLocation;
  
     Surface::Iter iterttl = surface->getIter(surface->getBounds());
 
         while (iterttl.line()) 
         {
             while(iterttl.pixel())
             {
                 if( (iterttl.x()%perOffset==0) && (iterttl.y()%perOffset==0))
                 {
                     Vec2f ttlLoc = iterttl.getPos();
                     
                     int red = iterttl.r();
                     //cout<< "red" << red <<std::endl;
                     
                     if ( red >  100)
                     {    
                         //cout<< "red" << red <<std::endl;
                         ttlLoc.x += offset.x;
                         ttlLoc.y += offset.y; //offset  
                         //cout << "r: "<< (float)iterttl.r()  << "g: "<< iterttl.g()  << "b: "<<iterttl.b() << "a: "<<iterttl.a()<< std::endl;
                         ttlLocation.push_back(  Vec3f(ttlLoc.x, ttlLoc.y, offset.z) );
                        // printf("loc.x is %f, loc.y is %f, loc.z is %f\n", ttlLoc.x, ttlLoc.y,offset.z);
                         
                     }
                    }
            
             }
         }
     
 return ttlLocation;
 }


/*vector<int> nameParticle::randGen()
{
    //array to hold data
    vector<int> in;
    
    //arrary to hold randomly selected data
    vector<int> out;
    
    
    for(int i = 0; i < totalRandNum; i++)
    {
        in.push_back(i);
    }
    
    int nRands = 0;
    srand ( time(NULL) );
    for(int i=0; i<totalRandNum;i++)
    {
        int currentDataSize = totalRandNum - nRands;
        //printf("\ncurrentDataSize: %d\n",currentDataSize);
        // for(int j=0; j < currentDataSize; j++)
        //   printf("%d\n",in[j]);
        int r = rand.randInt()%currentDataSize;
        //save it in out
        out.push_back(in[r]);
        
        //printf("random %d: %d\n",i,out[nRands]);
        //remove the selected data from input array by shifting data down
        for(int j=r+1; j < currentDataSize; j++)
            in[j-1] = in[j];
        //
        nRands++;
    }
    return out;
}*/

