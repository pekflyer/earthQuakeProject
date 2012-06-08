//
//  particle.h
//  earthQuakeProject
//
//  Created by Allan Yong on 12-03-12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/cairo/Cairo.h"
#include "cinder/Font.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Perlin.h"
#include <sstream>
#include "Resources.h"
using namespace ci;
using namespace ci::app;
using namespace std;


class particle {
public:
    void update(bool flatten);
	void draw();
    particle();
    particle( Vec3f loc, Vec3f vel);
    void render();
    void limitSpeed();
    void pullToCenter( const ci::Vec3f &center );
    void pullToName( const Vec3f &name );
    void addNeighborPos( ci::Vec3f pos );
 
    
    ci::Vec3f  mPos;
    ci::Vec3f  mVel; 
    ci::Vec3f  mAcc;
    ci::Vec3f  mVelNormal;
    
	float		mDecay;
	float		mRadius;
	float		mLength;
	float		mMaxSpeed, mMaxSpeedSqrd;
	float		mMinSpeed, mMinSpeedSqrd;
    float		mCrowdFactor;
    
    ci::Vec3f	mNeighborPos;
	int			mNumNeighbors;
    
    
    Color mColor;
  
};
