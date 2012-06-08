//
//  particleController.h
//  earthQuakeProject
//
//  Created by Allan Yong on 12-03-12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//


#pragma once
#include "particle.h"

#include "Resources.h"


class particleController {
public:
    
    particleController();
    void addParticles( int amt );    
    void iterateListExist();
	void applyForceToParticles( float zoneRadius, float lowerThresh, float higherThresh, float attractStrength, float repelStrength, float orientStrength );
	void pullToCenter( const ci::Vec3f &center );
    void pullToName( const vector<ci::Vec3f> &center);
	void update( bool flatten );
	void draw();
	void removeParticles( int amt );
    
    
    vector<particle> mParticles;
    ci::Vec3f mParticleCentroid;
	int mNumParticles;
 
    //destination        =destination;
};

