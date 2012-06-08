//
//  particleController.cpp
//  earthQuakeProject
//
//  Created by Allan Yong on 12-03-12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//


#include "cinder/gl/gl.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "particleController.h"


using namespace ci;
using namespace std;

extern gl::Texture *particleImg;
extern void renderImage( Vec3f _loc, float _diam, Color _col, float _alpha );


particleController::particleController()
{
 
}
void particleController::applyForceToParticles( float zoneRadius, float lowerThresh, float higherThresh, float attractStrength, float repelStrength, float alignStrength )
{
    float twoPI = M_PI * 2.0f;
	mParticleCentroid = Vec3f::zero();
	mNumParticles = mParticles.size();
	
	for( vector<particle>::iterator p1 = mParticles.begin(); p1 != mParticles.end(); ++p1 ){
        
		vector<particle>::iterator p2 = p1;
		for( ++p2; p2 != mParticles.end(); ++p2 ) {
			Vec3f dir = p1->mPos - p2->mPos;
			float distSqrd = dir.lengthSquared();
			float zoneRadiusSqrd = zoneRadius * zoneRadius;
			
			if( distSqrd < zoneRadiusSqrd ){			// Neighbor is in the zone
				float percent = distSqrd/zoneRadiusSqrd;
                //p1->addNeighborPos( p2->mPos );
				//p2->addNeighborPos( p1->mPos );
                
				if( percent < lowerThresh ){			// Separation
					float F = ( lowerThresh/percent - 1.0f ) * repelStrength;
					dir = dir.normalized() * F;
                    
                    
					p1->mAcc += dir;
					p2->mAcc -= dir;
				} else if( percent < higherThresh ){	// Alignment
					float threshDelta		= higherThresh - lowerThresh;
					float adjustedPercent	= ( percent - lowerThresh )/threshDelta;
					float F					= ( 1.0 - ( cos( adjustedPercent * twoPI ) * -0.5f + 0.5f ) ) * alignStrength;
					
					p1->mAcc += p2->mVelNormal * F;
					p2->mAcc += p1->mVelNormal * F;
					
				} else {								// Cohesion
					float threshDelta		= 1.0f - higherThresh;
					float adjustedPercent	= ( percent - higherThresh )/threshDelta;
					float F					= ( 1.0 - ( cos( adjustedPercent * twoPI ) * -0.5f + 0.5f ) ) * attractStrength;
                    
					dir.normalize();
					dir *= F;
                    
					p1->mAcc -= dir;
					p2->mAcc += dir;
				}
			}
		}
		
		mParticleCentroid += p1->mPos;
	}
    
    
    
	mParticleCentroid /= (float)mNumParticles;

}
void particleController::pullToCenter( const ci::Vec3f &center )
{
	for( vector<particle>::iterator p = mParticles.begin(); p != mParticles.end(); ++p ){
		p->pullToCenter( center );
	}
}

void particleController::pullToName( const vector<ci::Vec3f> &center)
{
    //printf("%d the total size\n",(int)center.size());
    int count = 0;
	for( vector<particle>::iterator p = mParticles.begin(); p != mParticles.end(); ){
       // if(count >= center.size()) 
            //printf("1111%d the total size\n",(int)center.size());
        if(count < (int)center.size())
        {
           p->pullToName(center.at(count));
          count++;
          ++p; 
           // printf("%d the total size\n",count);
        }
        else
        {
            count = 0;
       }
       
       //printf("%d the total size\n",count);
        
	}
}
void particleController::update( bool flatten  )
{
    for( vector<particle>::iterator p = mParticles.begin(); p != mParticles.end(); ++p ){
		p->update( flatten );
	}
    
    glDisable( GL_TEXTURE_2D );
}

void particleController::draw()
{
    glEnable( GL_TEXTURE_2D );
    
	particleImg->bind();
 	
	for( vector<particle>::iterator p = mParticles.begin(); p != mParticles.end(); ++p ){
		p->render();
	}
    particleImg->unbind();
}

void particleController::addParticles( int amt)
{
	for( int i=0; i<amt; i++ )
	{
		Vec3f randVec = Rand::randVec3f();
		Vec3f pos = randVec * Rand::randFloat( 50.0f, 250.0f );
		Vec3f vel = randVec * 2.0f;
        //check speed!!!
        //printf("vel.x is %f, vel.y is %f\n", vel.x, vel.y);
		mParticles.push_back( particle( pos,vel));
	}
    //printf("amount of particles is %d, how many are created %d\n", amt, (int)mParticles.size());
}

void particleController::removeParticles( int amt )
{
    for( int i=0; i<amt; i++ )
    {
        mParticles.pop_back();
    }
}
void particleController::iterateListExist()
{
	
    
}

