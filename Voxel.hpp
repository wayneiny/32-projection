//
//  Voxel.hpp
//  LungNoduleSynthesizer
//
//  Created by Weicheng on 2017-08-19.
//  Copyright © 2017 Weicheng Cao. All rights reserved.
//

#ifndef Voxel_hpp
#define Voxel_hpp

#include "Coordinate.hpp"
#include "Geometry.hpp"

class Voxel : public Geometry
{
    
public:
    /// properties
    double attenuation;                 /// each voxel has its own attenuation value
    static Scale scale;
    
    /// constructors
    Voxel(double _attenuation, double _x, double _y, double _z): Geometry(_x, _y, _z), attenuation(_attenuation) {};
    
    /// methods
    double GetIntersectLineLen();   /// override abstract
    
};

#endif /* Voxel_hpp */
