//
//  Scale.hpp
//  LungNoduleSynthesizer
//
//  Created by Weicheng on 2017-08-20.
//  Copyright © 2017 Weicheng Cao. All rights reserved.
//

#ifndef Scale_hpp
#define Scale_hpp

#include <stdio.h>
#include "Constants.h"

class Scale
{

public:
    /// properties
    double scaleX;
    double scaleY;
    double scaleZ;
    
    /// constructors
    Scale(): scaleX(VOXEL_LENGTH), scaleY(VOXEL_WIDTH), scaleZ(VOXEL_HEIGHT) {};    /// default

};

#endif /* Scale_hpp */
