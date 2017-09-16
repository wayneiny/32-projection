//
//  ProjectionPlane.hpp
//  LungNoduleSynthesizer
//
//  Created by Weicheng on 2017-08-19.
//  Copyright © 2017 Weicheng Cao. All rights reserved.
//

#ifndef ProjectionPlane_hpp
#define ProjectionPlane_hpp

#include <stdio.h>
#include <unordered_map>
#include <vector>
#include "Coordinate.hpp"
#include "SimulatedRay.hpp"
#include "Voxel.hpp"

using namespace std;

class ProjectionPlane
{
    
public:
    /// properties
    Coordinate viewpoint;                   /// where xray point source is
    Coordinate topLeft;
    Coordinate topRight;
    Coordinate bottomLeft;
    Coordinate bottomRight;                 /// define the plane
    vector<vector<Voxel>> intensities;      /// xray projection intensities, Voxel here is a plane
    
    /// constructors
    ProjectionPlane(Coordinate &_viewpoint): viewpoint(_viewpoint) {};
    
};

#endif /* ProjectionPlane_hpp */
