//
//  nameParticle.h
//  earthQuakeProject
//
//  Created by Allan Yong on 12-03-19.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef earthQuakeProject_nameParticle_h
#define earthQuakeProject_nameParticle_h
#pragma once
#include "cinder/app/AppBasic.h"
#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/gl.h"
#include "cinder/ip/Flip.h"
#include "cinder/Font.h"
#include "cinder/Text.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include <vector>
#include "particleController.h"
#include "cinder/Timeline.h"



using namespace ci;
using namespace cinder::ip;
using namespace ci::app;
using namespace std;


class nameParticle
{

public: 
    nameParticle();
    nameParticle(ci::Vec3f _loc, ci::Vec3f _vel , string name);
    void pullToCenter( const ci::Vec3f &center );
    void update(bool flatten,bool findUser);
    void draw();
    void limitSpeed();
    void addNeighborPos(ci::Vec3f pos);
    void findPerlin();
    vector<Vec3f> detectLoc(Surface* surface, Vec3f offset,int perOffset);
    
    void selectUser(string info, Vec3f userLoc);
    void clearExtraInfo();
    vector<Vec3f>* getPullParticle();
    
    //int checkSecParticleSize(){return mParticleControllerSec.);
    
    
    vector<int> randGen();
    
    std::vector<ci::Vec3f> loc;
    ci::Vec3f  mPos;
    ci::Vec2f  mPosExtraInfo;
   
    ci::Vec3f  mVel; 
    ci::Vec3f  mAcc;
    ci::Vec3f  mVelNormal;
    ci::Vec3f  perlin;       // perlin noise vector
    
    ci::Vec3f   mNeighborPos;
    int         mNumNeighbors;
    
    ci::Color   mColor;
    
    int         particleAMT;
    float		mDecay;
	float		mRadius;
	float		mLength;
    float		mMaxSpeed, mMaxSpeedSqrd;
	float		mMinSpeed, mMinSpeedSqrd;
    
    float				mZoneRadius;
	float				mLowerThresh, mHigherThresh;
	float				mAttractStrength, mRepelStrength, mOrientStrength;
    float               mFear;
	float               mCrowdFactor;
    

    bool                flatten;
    particleController  mParticleController;
    particleController  mParticleControllerSec;
    bool                hasAdd;
    
    Surface         tempSur;
    Surface         extraInfoSur;
 
    vector<string>  nameStrings;
    vector<Vec3f>   pullParticle;
    vector<Surface> nameSurfaces;
    
    TextBox temp;
    TextBox extraInfo;
    
    ci::Anim<ci::Color> mTitleColor;
	ci::Anim<float>		mTitleAlpha;
};

#endif
