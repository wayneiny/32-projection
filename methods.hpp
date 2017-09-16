//
//  methods.hpp
//  LungNoduleSynthesizer
//
//  Created by Weicheng on 2017-08-24.
//  Copyright © 2017 Weicheng Cao. All rights reserved.
//

#ifndef methods_hpp
#define methods_hpp

#include <algorithm>    /// to find common elements in 2 vectors
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <thread>
#include "Constants.h"
#include "ProjectionPlane.hpp"
#include "Voxel.hpp"

/// top level wrapper function
void SynthesizeXRayFromCTScan();

void CreateCtVoxels(vector<vector<vector<Voxel>>> &ctVoxels);
void WriteResultToFile(ProjectionPlane &plane, vector<SimulatedRay*> &rays);

/// create a projection plane and set extensions
ProjectionPlane CreateProjectionPlane(Coordinate &planeCenter, Coordinate &viewpoint, vector<vector<vector<Voxel>>> &ctMatrix);

/// partition the projection plane into a row * column 2D matrix with small voxels with height equal to 0
void PartitionProjectionPlane(ProjectionPlane &plane, double row, double column);

void GetVoxelsOnRayHelper(vector<vector<vector<Voxel>>> &ctVoxels, int view, int index, double intercept, double slope, vector<Voxel*> &result, double rightBound, double leftBound);

/// wrapper function
vector<Voxel*> GetVoxelsOnRay(vector<vector<vector<Voxel>>>& ctVoxels, SimulatedRay* ray);

/// calculate remaining intensity from ray
/// 1. ray: ray to be calculated upon
void CalculateRemainingIntensity(SimulatedRay* ray, vector<vector<vector<Voxel>>> &ctVoxels);

double CalculateLinearFunction(double k, double b, double x);
double CalculateXFromPoint(double k, double b, double y);

void GetBoundingVoxelsIndices(double totalLen, double projectionIndex, double sourceIndex, double &higherBound, double &lowerBound);

void AssignVoxelIntercepts(SimulatedRay* ray);
void AssignVoxelInterceptsHelper(bool &foundFirst, bool &foundSecond, double &x, double &y, double &z, int &face, Voxel* voxel);

#endif /* methods_hpp */
