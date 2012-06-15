//
//  nameController.h
//  earthQuakeProject
//
//  Created by Allan Yong on 12-03-17.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef earthQuakeProject_nameController_h
#define earthQuakeProject_nameController_h
#pragma once
#include <iostream>
#include <boost/thread.hpp>

#include "cinder/Rand.h"
#include "cinder/Perlin.h"


#include "mysql_connector.h"
#include "nameParticle.h"




class nameController
{
    public:
    nameController();
 
    void init();
    void addNames(vector<string> names);
    void draw();
    void applyForceToNames(float zoneRadiusSqrd, float thresh );
    void pullToCenter( const ci::Vec3f &center );
    void findUserName(ci::Vec3f userLoc);
    Vec3f getUserNamePos();
    void update(bool flatten, bool findUser);
    void setUserLoc(Vec3f _userLoc);
    void resetNameID();
    void resetExtraInfo();
    void addSecParticles();
    
    vector<int>             randGen();
    vector<Surface>         *getNames();
    vector<Vec2f>           *getNameLoc();
    
    
    //Varible
    Surface                 tempSur;
    ci::Vec3f               mParticleCentroid;
    int                     mNumParticles;
    vector<nameParticle>    mNameParticles;
    vector<Surface>         nameSurface;
    vector<string>          nameStrings;
    vector<string>          extraInfoStrings;
    vector<Vec2f>           nameLoc;
    TextBox                 temp;
    ci::Vec3f               userNamePos;    
    ci::Perlin              mPerlin;
    ci::Vec3f               userLoc;
    float                   mDecay;
	float                   mRadius;
	float                   mLength;
    float                   mMaxSpeed, mMaxSpeedSqrd;
	float                   mMinSpeed, mMinSpeedSqrd;
    
    float                   mZoneRadius;
	float                   mLowerThresh, mHigherThresh;
	float                   mAttractStrength, mRepelStrength, mOrientStrength;
    float                   mFear;
	float                   mCrowdFactor;
    bool                    flatten;

    int totalRandNum;
    Rand rand;
    int nameID;
    mysql_connector mc;
    particleController  mParticleControllerSec;
    //    ci::Vec3f nLoc;
//    ci::Vec3f nVel;
//    ci::Color nColor;
//    float nScale;
//    int mNumNames;
};


#endif
