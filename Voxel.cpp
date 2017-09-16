//
//  Voxel.cpp
//  LungNoduleSynthesizer
//
//  Created by Weicheng on 2017-08-19.
//  Copyright © 2017 Weicheng Cao. All rights reserved.
//

#include "Voxel.hpp"
#include <cmath>

double Voxel::GetIntersectLineLen()
{
    return sqrt(pow((firstIntercept.x - secondIntercept.x)*Voxel::scale.scaleX, 2) +
                pow((firstIntercept.y - secondIntercept.y)*Voxel::scale.scaleY, 2) +
                pow((firstIntercept.z - secondIntercept.z)*Voxel::scale.scaleZ, 2));
}
