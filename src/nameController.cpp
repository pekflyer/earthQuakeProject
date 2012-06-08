//
//  name.cpp
//  earthQuakeProject
//
//  Created by Allan Yong on 12-03-15.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "time.h" 

#include "nameController.h"

extern Font mFont;
extern float widthName;
extern float heightName;
using namespace ci;

#include <iostream>
#include <boost/thread.hpp>

nameController::nameController()
{
}
void nameController::init()
{

   nameStrings.push_back("陈思成");
   nameStrings.push_back("苟玉玲");
   nameStrings.push_back("邓晨雨");
   nameStrings.push_back("黄蓉");
   nameStrings.push_back("李春龙");
   nameStrings.push_back("文金亮");

   
    extraInfoStrings.push_back("男，1999年4月30日, 9岁 - 2012年4月30日, 13岁");
    extraInfoStrings.push_back("女，2001年4月30日, 7岁 - 2012年4月30日, 11岁");
    extraInfoStrings.push_back("女，2003年4月30日, 5岁 - 2012年4月30日, 9岁");
    extraInfoStrings.push_back("女，1992年4月30日, 16岁 - 2012年4月30日, 17岁");
    extraInfoStrings.push_back("男，1993年4月30日, 15岁 - 2012年4月30日, 19岁");
    extraInfoStrings.push_back("男，2000年4月30日, 8岁 - 2012年4月30日, 12岁");

    
    rand.randomize();
    //totalRandNum = (int) nameStrings.size();
    userLoc = Vec3f::zero();
    
    addNames(nameStrings);
    mPerlin = Perlin( 4 );
    nameID  = -1;
    
    flatten         = true;
    mZoneRadius		= 80.0f;
	mLowerThresh	= 0.4f;
	mHigherThresh	= 0.75f;
	mAttractStrength	= 0.005f;
	mRepelStrength		= 0.01f;
	mOrientStrength		= 0.01f;

	mMaxSpeed		= Rand::randFloat( 2.5f, 3.0f );
	mMaxSpeedSqrd	= mMaxSpeed * mMaxSpeed;
	mMinSpeed		= Rand::randFloat( 1.0f, 1.5f );
	mMinSpeedSqrd	= mMinSpeed * mMinSpeed;
    
    mDecay			= 0.99f;
	mRadius			= 1.0f;
	mLength			= 5.0f;
	mFear			= 1.0f;
	mCrowdFactor	= 1.0f;
    

}



void nameController::applyForceToNames( float zoneRadiusSqrd, float thresh)
{
	float twoPI = M_PI * 2.0f;
	for( vector<nameParticle>::iterator p1 = mNameParticles.begin(); p1 != mNameParticles.end(); ++p1 ){
        
		vector<nameParticle>::iterator p2 = p1;
		for( ++p2; p2 != mNameParticles.end(); ++p2 ) {
			Vec3f dir = p1->mPos - p2->mPos;
			float distSqrd = dir.lengthSquared();
			
			if( distSqrd < zoneRadiusSqrd ){	// Neighbor is in the zone
				float percent = distSqrd/zoneRadiusSqrd;
				
				if( percent < thresh ){			// Separation
					float F = ( thresh/percent - 1.0f ) * 0.01f;
					dir.normalize();
					dir *= F;
                    
					p1->mAcc += dir;
					p2->mAcc -= dir;
					
				} else {						// Cohesion
					float threshDelta = 1.0f - thresh;
					float adjustedPercent = ( percent - thresh )/threshDelta;
					float F = ( 1.0 - ( cos( adjustedPercent * twoPI ) * -0.5f + 0.5f ) ) * 0.05f;
					
					// INTERESTING BUG
					// Use this F instead and lower the thresh to 0.2 after flattening the scene ('f' key)
					// float F = ( 0.5f - ( cos( adjustedPercent * twoPI ) * 0.5f + 0.5f ) ) * 0.15f;
                    
					dir.normalize();
					dir *= F;
                    
					p1->mAcc -= dir;
					p2->mAcc += dir;
					
				}
			}
		}
        
      	}
     
		//mParticleCentroid /= (float)mNumParticles;
	
}
void nameController::findUserName(Vec3f userLoc)
{
    
    float db = FLT_MAX;
    //printf("I was here\n");
    for( int i = 0; i < mNameParticles.size(); i++ )
     {
         
         Vec3f dir = userLoc - mNameParticles.at(i).mPos;
         
         float d = dir.lengthSquared();
         if(db > d)
         {
             db = d;
             nameID = i;
             //printf("I was here\n");
             //userNamePos = mNameParticles.at(i).mPos;
         }
     }
    printf("NAME ID is %d\n", nameID);
}

Vec3f nameController::getUserNamePos()
{
    if(nameID != -1){
     return mNameParticles.at(nameID).mPos;
    }
    else
    {
        return Vec3f::zero();
    }
}

void nameController::setUserLoc(Vec3f _userLoc)
{
    userLoc = _userLoc;
}
void nameController::resetNameID()
{
    nameID = Rand::randInt(0,9);
}
void nameController::resetExtraInfo()
{
    if(nameID != -1)
    {
        mNameParticles.at(nameID).clearExtraInfo();
    }
}


void nameController::update(bool flatten, bool findUser)
{
    
    
    // mNameParticles.at(nameID).update(flatten, findUser);
            
    
    for( vector<nameParticle>::iterator p = mNameParticles.begin(); p != mNameParticles.end(); ++p )
    {
            p->update( flatten, findUser);
    }
        
    if(findUser && nameID != -1)
    {
        if(userLoc != Vec3f::zero())
        {
            mNameParticles.at(nameID).selectUser(extraInfoStrings.at(nameID), userLoc);
            mParticleControllerSec.pullToName(mNameParticles.at(nameID).detectLoc(&mNameParticles.at(nameID).tempSur, mNameParticles.at(nameID).mPos, 2));
            
        }
    }
   
   if(!findUser)
   {
       mParticleControllerSec.applyForceToParticles( mZoneRadius, mLowerThresh, mHigherThresh, mAttractStrength, mRepelStrength, mOrientStrength );
       mParticleControllerSec.update(flatten);
       mParticleControllerSec.pullToCenter(Vec3f::zero());
       //cout <<"i am here \n" << std::endl; 
   }
//    else if(nameID == -1)
//    {
//        for( vector<nameParticle>::iterator p = mNameParticles.begin(); p != mNameParticles.end(); ++p )
//        {
//            p->update( flatten, false);
//        }
//    }

}

void nameController::pullToCenter( const ci::Vec3f &center )
{
    for( vector<nameParticle>::iterator p = mNameParticles.begin(); p != mNameParticles.end(); ++p ){
		p->pullToCenter( center );
	}
}


void nameController::addNames(vector<string> names)
{   
       
    for(int i = 0; i < names.size(); i++)
    {
  		Vec3f pos = Rand::randVec3f() * Rand::randFloat( 100.0f, 200.0f );
		Vec3f vel = Rand::randVec3f();
        
        nameParticle p = nameParticle( pos, vel, names.at(i) ); 
                mNameParticles.push_back( p );
    }
}


vector<Surface> * nameController::getNames()
{

    return &nameSurface;
}
vector<Vec2f> * nameController::getNameLoc()
{
    
    return &nameLoc;
}
void nameController::draw()
{
    
   // gl::draw(surface,  offset);
    for( vector<nameParticle>::iterator p = mNameParticles.begin(); p != mNameParticles.end(); ++p ){
            p->draw();
	}
    mParticleControllerSec.draw();

}
void nameController::addSecParticles()
{
    mParticleControllerSec.addParticles(500);
}


/*
vector<int> nameController::randGen()
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