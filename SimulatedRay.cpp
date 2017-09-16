//
//  SimulatedRay.cpp
//  LungNoduleSynthesizer
//
//  Created by Weicheng on 2017-08-19.
//  Copyright © 2017 Weicheng Cao. All rights reserved.
//

#include <cmath>
#include <stdio.h>
#include "Constants.h"
#include "SimulatedRay.hpp"

/// assign pointer and coordinate
SimulatedRay::SimulatedRay(Coordinate* _viewpoint, Voxel* _projection)
{
    viewpoint = _viewpoint;
    projectedVoxel = _projection;
    lineVector = new Coordinate(_projection->center.x-_viewpoint->x, _projection->center.y-_viewpoint->y, _projection->center.z-_viewpoint->z);
    remainingRay = initialRay;
}

/// Azimuth is a polar angle in the x-y plane, with positive angles indicating counterclockwise rotation of the viewpoint.
double SimulatedRay::GetTrueAzimuth()
{
    /// 1. calculate the acute angle of azimuth and disregard the relative positions of dest and start
    double deltaY = projectedVoxel->center.y - viewpoint->y;
    double deltaX = projectedVoxel->center.x - viewpoint->x;
    
    /// this assumes length is along x-axis, width is along y-axis and slices are spaced along z-axis
    double acuteAngle = atan(fabs(deltaY * VOXEL_WIDTH) / fabs(deltaX * VOXEL_LENGTH)) * 180 / PI;
    double result = acuteAngle;
    
    /// 2. adjust angle by taking the positions into account
    /// if viewpoint's y is less than destination point's
    if (deltaY > 0)
    {
        result += 90;
    }
    
    /// if viewpoint's x is less than destination point's
    if (deltaX < 0)
    {
        result = result + 180 - acuteAngle;
    }
    
    return result;
}   /// verified


/// Elevation is the angle above (positive angle) or below (negative angle) the x-y plane.
double SimulatedRay::GetTrueElevation()
{
    /// 1. calculate the acute angle of elevation and disregard the relative positions of dest and start
    double deltaZ = projectedVoxel->center.z - viewpoint->z;
    double deltaX = projectedVoxel->center.x - viewpoint->x;
    double deltaY = projectedVoxel->center.y - viewpoint->y;
    
    /// this assumes length is along x-axis, width is along y-axis and slices are spaced along z-axis
    double acuteAngle = atan(fabs(deltaZ*VOXEL_HEIGHT) / fabs(sqrt(pow(deltaX*VOXEL_LENGTH,2) + pow(deltaY*VOXEL_WIDTH, 2)))) * 180 / PI;
    
    /// 2. adjust angle by taking the positions into account
    /// if viewpoint's z is less than destination point's, elevation is negative
    return deltaZ < 0 ? -acuteAngle : acuteAngle;
}   /// verified

