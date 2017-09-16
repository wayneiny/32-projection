//
//  SimulatedRay.hpp
//  LungNoduleSynthesizer
//
//  Created by Weicheng on 2017-08-19.
//  Copyright © 2017 Weicheng Cao. All rights reserved.
//

#ifndef SimulatedRay_hpp
#define SimulatedRay_hpp

#include <vector>
#include "Voxel.hpp"

using namespace std;

class SimulatedRay
{
    
public:
    /// properties
    Coordinate* viewpoint;          /// ray source
    vector<Voxel*> voxels;         /// an array of voxels this ray passes through
    Voxel* projectedVoxel;          /// pointer to the projected voxel
    Coordinate* lineVector;         /// vector from viewpoint to projected voxel: line = viewpoint.coordinate + lineVector
    double remainingRay;
    const double initialRay = 1;
    
    /// constructors & destructors
    SimulatedRay() { remainingRay = initialRay; };
    SimulatedRay(Coordinate* _viewpoint, Voxel* _projection);
    
    ~SimulatedRay()
    {
        delete viewpoint;
        delete lineVector;
        /// projected voxel will be deleted by projection plane to avoid double deletion
    };
    
    /// methods
    double GetTrueAzimuth();    /// taken scale into account
    double GetTrueElevation();  /// the angle between ground to a vector of x, y, z
    
};

#endif /* SimulatedRay_hpp */
