//
//  particle.cpp
//  earthQuakeProject
//
//  Created by Allan Yong on 12-03-12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "particle.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/app/AppBasic.h"


extern int counter;
extern void renderImage( Vec3f _loc, float _diam, Color _col, float _alpha );

particle::particle()
{
}    


particle::particle( Vec3f pos, Vec3f vel)
{
    
    mPos			= pos;
	mVel			= vel;
	mVelNormal		= Vec3f::yAxis();
	mAcc			= Vec3f::zero();
	
	mMaxSpeed		= Rand::randFloat( 2.5f, 3.0f );
	mMaxSpeedSqrd	= mMaxSpeed * mMaxSpeed;
	mMinSpeed		= Rand::randFloat( 1.0f, 1.5f );
	mMinSpeedSqrd	= mMinSpeed * mMinSpeed;
	
    mNeighborPos	= Vec3f::zero();
	mNumNeighbors	= 0;
	mDecay			= 0.99f;
	mRadius			= 10.0f;
	mLength			= 10.0f;
	mCrowdFactor	= 1.0f;    

     //destination        = destination;
	mColor			= ColorA( 0.4f, 0.4f, 0.2f, 1.0f );

    
}    

void particle::update(bool flatten)
{   
    mCrowdFactor -= ( mCrowdFactor - ( 1.0f - mNumNeighbors * 0.01f ) ) * 0.1f;

    if( flatten ) mAcc.z = 0.0f;
    mVel += mAcc;
	mVelNormal = mVel.normalized();
	
	limitSpeed();
	
	mPos += mVel;
	
	if( flatten ) mPos.z = 0.0f;
    
	mVel *= mDecay;
	mAcc = Vec3f::zero();
    
    //float c = mNumNeighbors/50.0f;
	//mColor = ColorA( 0.5f,0.5f,0.9f, 0.7f );
	
	mNeighborPos = Vec3f::zero();
	mNumNeighbors = 0;
    //mRadius = 5;

}
void particle::pullToCenter( const Vec3f &center )
{
	Vec3f dirToCenter = mPos - center;
	float distToCenter = dirToCenter.length();
	float maxDistance = 300.0f;
	
	if( distToCenter > maxDistance ){
		dirToCenter.normalize();
		float pullStrength = 0.0001f;
		mVel -= dirToCenter * ( ( distToCenter - maxDistance ) * pullStrength );
	}
}
void particle::pullToName( const Vec3f &center )
{
    Vec3f go2 = center - mPos;
    float distToTemp = go2.length();
    
    if(distToTemp > 0.5)
    {
        //printf("distToTemp is %f\n",distToTemp);
        go2.safeNormalize();
        mPos = mPos+go2;
    }
    //printf("Done\n");

}

void particle::limitSpeed()
{
    float vLengthSqrd = mVel.lengthSquared();
	if( vLengthSqrd > mMaxSpeedSqrd ){
		mVel = mVelNormal * mMaxSpeed;
		
	} else if( vLengthSqrd < mMinSpeedSqrd ){
		mVel = mVelNormal * mMinSpeed;
	}
}

void particle::render()
{
    
    renderImage( mPos, mRadius, mColor, 1.0f );
}
void particle::draw()
{
    
   //gl::color( mColor );
   // gl::drawSolidRect( Rectf( mLoc, mLoc + Vec2f( mRadius * mAgePer, mRadius * mAgePer) ) );
 //  gl::drawSphere( mPos, mRadius,12 );
    //gl::drawSolidCircle(Vec2f( mPos.x,mPos.y), 100 );
}

void particle::addNeighborPos( Vec3f pos )
{
	mNeighborPos += pos;
	mNumNeighbors ++;
}