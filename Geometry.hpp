//
//  Geometry.hpp
//  LungNoduleSynthesizer
//
//  Created by Weicheng on 2017-08-20.
//  Copyright © 2017 Weicheng Cao. All rights reserved.
//

#ifndef Geometry_hpp
#define Geometry_hpp

#include <stdio.h>
#include "Coordinate.hpp"
#include "Constants.h"
#include "Scale.hpp"

enum CubeFace
{
    TOP,
    BOTTOM,
    FRONT,
    BACK,
    RIGHT,
    LEFT,
    END
};

/// abstract class
class Geometry
{
    
public:
    /// properties
    Coordinate center;  /// the coordinates of center, servers as the id of this voxel
    
    Coordinate firstIntercept;
    CubeFace firstFace;
    Coordinate secondIntercept;
    CubeFace secondFace;
    
    /// constructors
    Geometry(double _x, double _y, double _z)
    {
        center = Coordinate(_x, _y, _z);
    };
    
    /// methods
    virtual double GetIntersectLineLen() = 0;

};


#endif /* Geometry_hpp */
