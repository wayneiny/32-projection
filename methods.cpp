//
//  methods.cpp
//  LungNoduleSynthesizer
//
//  Created by Weicheng on 2017-08-24.
//  Copyright © 2017 Weicheng Cao. All rights reserved.
//

#include "methods.hpp"  // 299330

/// initialize static member of class Voxel
Scale Voxel::scale = Scale();
int numberOfRaysFinished = 0;
const int maxThreads = 4;
int semaphore = 0;
mutex mtx;

void CreateCtVoxels(vector<vector<vector<Voxel>>> &ctVoxels)
{
    string line;
    ifstream infile("/Users/weicheng/Documents/xcode/LungNoduleSynthesizer/dicom_matrix_front.txt", ifstream::in);
    
    ctVoxels.resize(512);
    for (unsigned int h=0; h<ctVoxels.size(); h++)
    {
        ctVoxels[h].resize(512);
    }

    int x=0, y=0, z=0;  /// indices in matrix
    double attenuation;

    while(getline(infile, line))
    {
        stringstream linestream(line);
        string value;
        
        x = 0;
        y = 0;

        while(getline(linestream, value, ','))
        {
//            attenuation = (1+stod(value))*WATER_ATTENUATION;
            attenuation = (1000+stod(value))*WATER_ATTENUATION/1000;
            ctVoxels[z][y].push_back(Voxel(attenuation, double(x+VOXEL_SPACING), double(y+VOXEL_SPACING), double(z+VOXEL_SPACING)));
            
            if (x++ / 255 == 1)
            {
                x = 0;
                y++;
            }
        }
        
        if (line == "")
        {
            continue;
        }
        
        z++;
    }
}

void WriteResultToFile(ProjectionPlane &plane, vector<SimulatedRay*> &rays)
{
    ofstream outfile;
    outfile.open("/Users/weicheng/Documents/xcode/LungNoduleSynthesizer/xray_result_750_375_x10_front.txt");

    for (unsigned int i=0; i<plane.intensities.size(); i++)
    {
        for (unsigned int j=0; j<plane.intensities[i].size(); j++)
        {
            outfile << rays[int(i*plane.intensities[0].size()+j)]->remainingRay << ",";
            plane.intensities[i][j].attenuation = rays[int(i*plane.intensities[0].size()+j)]->remainingRay;
        }
        outfile << endl;
    }
    
    outfile.close();
}

void SynthesizeXRayFromCTScan()
{
    Coordinate source (127.99999, POINT_SOURCE_Y, 256);
    Coordinate projectionCenter (127.99999, 512, 256);

    vector<vector<vector<Voxel>>> ctVoxels;
    CreateCtVoxels(ctVoxels);
    
    /// test plane
    ProjectionPlane plane = CreateProjectionPlane(projectionCenter, source, ctVoxels);
    
    cout << setw(15) << "Top left: (" << plane.topLeft.x << ", " << plane.topLeft.y << ", " << plane.topLeft.z << ")" << endl;
    cout << setw(15) << "Top Right: (" << plane.topRight.x << ", " << plane.topRight.y << ", " << plane.topRight.z << ")" << endl;
    cout << setw(15) << "Bottom left: (" << plane.bottomLeft.x << ", " << plane.bottomLeft.y << ", " << plane.bottomLeft.z << ")" << endl;
    cout << setw(15) << "Bottom Right: (" << plane.bottomRight.x << ", " << plane.bottomRight.y << ", " << plane.bottomRight.z << ")" << endl;
    cout << endl;
    
    /// partition projection plane
    PartitionProjectionPlane(plane, 750, 375);  /// 200,200 5minutes
    
    /// test 2 rays with multi-threading
    vector<thread> workingThreads;
    vector<SimulatedRay*> rays;

    for (unsigned int i=0; i<plane.intensities.size(); i++)
    {
        for (unsigned int j=0; j<plane.intensities[0].size(); j++)
        {
            SimulatedRay* tmp = new SimulatedRay(&source, &plane.intensities[i][j]);
            rays.push_back(tmp);
        }
    }
    
    /// check sanity
    for (unsigned int i=0; i<rays.size(); i++)
    {
//        cout << rays[i]->lineVector->x << "," << rays[i]->lineVector->y << "," << rays[i]->lineVector->z << ")" << i << endl;
    }
    
    int index;
    for (unsigned int i=0; i<plane.intensities.size(); i++)
    {
        for (unsigned int j=0; j<plane.intensities[0].size(); j++)
        {
            index = int(i*plane.intensities[0].size()+j);
//            CalculateRemainingIntensity(rays[i], ctVoxels);
            mtx.lock();
            if (semaphore < maxThreads)
            {
                workingThreads.push_back(thread(CalculateRemainingIntensity, rays[index], ref(ctVoxels)));
                semaphore++;
            }
            else
            {
                j--;
            }
            mtx.unlock();
        }
    }
    
    /// join all threads
    for (unsigned int i=0; i<workingThreads.size(); i++)
    {
        workingThreads[i].join();
    }

    cout << endl << "Threads joined: calculating remaining intensity finished" << endl;
    workingThreads.clear();
    
    WriteResultToFile(plane, rays);
    
    return;
}

void AssignVoxelInterceptsHelper(bool &foundFirst, bool &foundSecond, double &x, double &y, double &z, int &face, Voxel* voxel)
{
    if (!foundFirst)
    {
        voxel->firstIntercept = Coordinate(x, y, z);
        voxel->firstFace = (CubeFace)face;
        foundFirst = true;
    }
    else if (!foundSecond)
    {
        if (voxel->firstIntercept.x != x ||
            voxel->firstIntercept.y != y ||
            voxel->firstIntercept.z != z)
        {
            voxel->secondIntercept = Coordinate(x, y, z);
            voxel->secondFace = (CubeFace)face;
            foundSecond = true;
        }
    }
}

void AssignVoxelIntercepts(SimulatedRay* ray)
{
    for (int i=0; i<ray->voxels.size(); i++)
    {
        bool foundFirst = false, foundSecond = false;
        Coordinate* viewpoint = ray->viewpoint;
        Coordinate currVoxelCenter = ray->voxels[i]->center;
        Coordinate* lineVector = ray->lineVector;
        
        /// loop through enums
        for (int face=TOP; face!=END; face++)
        {
            double lamda;
            double x, y, z;

            switch (face) {
                case TOP:
                case BOTTOM:
                    z = currVoxelCenter.z + (face == TOP ? VOXEL_SPACING : -VOXEL_SPACING);
                    lamda = (z - viewpoint->z) / lineVector->z;
                    x = lineVector->x*lamda+viewpoint->x;
                    y = lineVector->y*lamda+viewpoint->y;

                    if (x <= currVoxelCenter.x+VOXEL_SPACING &&
                        x >= currVoxelCenter.x-VOXEL_SPACING &&
                        y <= currVoxelCenter.y+VOXEL_SPACING &&
                        y >= currVoxelCenter.y-VOXEL_SPACING)
                    {
                        AssignVoxelInterceptsHelper(foundFirst, foundSecond, x, y, z, face, ray->voxels[i]);
                    }
                    break;
                case RIGHT:
                case LEFT:
                    if (foundFirst && foundSecond) continue;

                    x = currVoxelCenter.x + (face == RIGHT ? VOXEL_SPACING : -VOXEL_SPACING);
                    lamda = (x - viewpoint->x) / lineVector->x;
                    z = lineVector->z*lamda+viewpoint->z;
                    y = lineVector->y*lamda+viewpoint->y;
                    
                    if (z <= currVoxelCenter.z+VOXEL_SPACING &&
                        z >= currVoxelCenter.z-VOXEL_SPACING &&
                        y <= currVoxelCenter.y+VOXEL_SPACING &&
                        y >= currVoxelCenter.y-VOXEL_SPACING)
                    {
                        AssignVoxelInterceptsHelper(foundFirst, foundSecond, x, y, z, face, ray->voxels[i]);
                    }
                    break;
                case FRONT:
                case BACK:
                    if (foundFirst && foundSecond) continue;

                    y = currVoxelCenter.y + (face == FRONT ? -VOXEL_SPACING : VOXEL_SPACING);
                    lamda = (y - viewpoint->y) / lineVector->y;
                    x = lineVector->x*lamda+viewpoint->x;
                    z = lineVector->z*lamda+viewpoint->z;
                    
                    if (z <= currVoxelCenter.z+VOXEL_SPACING &&
                        z >= currVoxelCenter.z-VOXEL_SPACING &&
                        x <= currVoxelCenter.x+VOXEL_SPACING &&
                        x >= currVoxelCenter.x-VOXEL_SPACING)
                    {
                        AssignVoxelInterceptsHelper(foundFirst, foundSecond, x, y, z, face, ray->voxels[i]);
                    }

                    break;
                default:
                    break;
            }
        }
        
        if (!foundFirst || !foundSecond)
        {
            ray->voxels.erase(ray->voxels.begin()+i);
        }
    }
}

void CalculateRemainingIntensity(SimulatedRay* ray, vector<vector<vector<Voxel>>> &ctVoxels)
{
    ray->voxels = GetVoxelsOnRay(ctVoxels, ray);
    AssignVoxelIntercepts(ray);
    
    double len, attenuation, power = 1.0, totalLen = 0;
    for (int i=0; i<ray->voxels.size(); i++)
    {
        len = ray->voxels[i]->GetIntersectLineLen();
        attenuation = ray->voxels[i]->attenuation;
//        cout << "Voxel at (" << ray->voxels[i]->center.x << "," << ray->voxels[i]->center.y << "," << ray->voxels[i]->center.z << "): " << len << endl;
        power += len*attenuation;
        totalLen += len;
    }
    
    mtx.lock();
//    if (power <0)
//    {
//        power = 0;
//    }
    ray->remainingRay *= exp(-1.0*power);
    
    if (numberOfRaysFinished % 1000 == 0)
    {
        cout << numberOfRaysFinished << ":" << ray->remainingRay << endl;
//        cout << "Total length by summing is " << totalLen << endl;
    }
    semaphore--;
    numberOfRaysFinished++;
    mtx.unlock();
}

vector<Voxel*> GetVoxelsOnRay(vector<vector<vector<Voxel>>> &ctVoxels, SimulatedRay* ray)
{
    vector<Voxel*> top, right, front, common;
    
    double xlen = double(ctVoxels[0][0].size());
    double ylen = double(ctVoxels[0].size());
    double zlen = double(ctVoxels.size());
    
    double rightRightBound, rightLeftBound, rightTopBound, rightBottomBound;
    double topRightBound, topLeftBound, topTopBound, topBottomBound;
    double frontRightBound, frontLeftBound, frontTopBound, frontBottomBound;
    
    GetBoundingVoxelsIndices(zlen, ray->projectedVoxel->center.z, ray->viewpoint->z, rightTopBound, rightBottomBound);
    GetBoundingVoxelsIndices(ylen, ray->projectedVoxel->center.y, ray->viewpoint->y, rightRightBound, rightLeftBound);
    GetBoundingVoxelsIndices(xlen, ray->projectedVoxel->center.x, ray->viewpoint->x, topRightBound, topLeftBound);
    GetBoundingVoxelsIndices(ylen, ray->projectedVoxel->center.y, ray->viewpoint->y, topTopBound, topBottomBound);
    GetBoundingVoxelsIndices(zlen, ray->projectedVoxel->center.z, ray->viewpoint->z, frontTopBound, frontBottomBound);
    GetBoundingVoxelsIndices(xlen, ray->projectedVoxel->center.x, ray->viewpoint->x, frontRightBound, frontLeftBound);
    
    if (topTopBound == INVALID || topBottomBound == INVALID || topRightBound == INVALID || topLeftBound == INVALID ||
        rightRightBound == INVALID || rightLeftBound == INVALID || rightTopBound == INVALID || rightBottomBound == INVALID ||
        frontRightBound == INVALID || frontLeftBound == INVALID || frontTopBound == INVALID || frontBottomBound == INVALID)
    {
        return common;
    }
    
    /// front
    double frontLineFuncSlope = (ray->projectedVoxel->center.z - ray->viewpoint->z) / (ray->projectedVoxel->center.x - ray->viewpoint->x);
    double frontLineFuncIntercept = ray->viewpoint->z - ray->viewpoint->x * frontLineFuncSlope;
    
    for (int z=frontBottomBound; z<=frontTopBound; z++)
        GetVoxelsOnRayHelper(ctVoxels, FRONT_VIEW, z, frontLineFuncIntercept, frontLineFuncSlope, front, frontRightBound, frontLeftBound);
    
    /// top
    double topLineFuncSlope = (ray->projectedVoxel->center.y - ray->viewpoint->y) / (ray->projectedVoxel->center.x - ray->viewpoint->x);
    double topLineFuncIntercept = ray->viewpoint->y - ray->viewpoint->x * topLineFuncSlope;
    
    for (int y=topBottomBound; y<=topTopBound; y++)
        GetVoxelsOnRayHelper(ctVoxels, TOP_VIEW, y, topLineFuncIntercept, topLineFuncSlope, top, topRightBound, topLeftBound);
    
    /// right
    double rightLineFuncSlope = (ray->projectedVoxel->center.z - ray->viewpoint->z) / (ray->projectedVoxel->center.y - ray->viewpoint->y);
    double rightLineFuncIntercept = ray->viewpoint->z - ray->viewpoint->y*rightLineFuncSlope;
    
    for (int z=rightBottomBound; z<=rightTopBound; z++)
        GetVoxelsOnRayHelper(ctVoxels, RIGHT_VIEW, z, rightLineFuncIntercept, rightLineFuncSlope, right, rightRightBound, rightLeftBound);
    
    vector<Voxel*> fronttop;
    sort(right.begin(), right.end());
    sort(front.begin(), front.end());
    sort(top.begin(), top.end());
    set_intersection(right.begin(), right.end(), front.begin(), front.end(), back_inserter(fronttop));
    sort(fronttop.begin(), fronttop.end());
    set_intersection(top.begin(), top.end(), fronttop.begin(), fronttop.end(), back_inserter(common));
    
    return common;
}

void GetVoxelsOnRayHelper(vector<vector<vector<Voxel>>> &ctVoxels, int view, int index, double intercept, double slope, vector<Voxel*> &result, double rightBound, double leftBound)
{
    double xlen = double(ctVoxels[0][0].size());
    double ylen = double(ctVoxels[0].size());
    double zlen = double(ctVoxels.size());
    
    bool foundIntercept = false, foundConsecutiveIntercept = false;
    
    double bottom = CalculateXFromPoint(slope, intercept, index);
    double top = CalculateXFromPoint(slope, intercept, index+1);

    for (int i=leftBound; i<=rightBound; i++)
    {
        double left = CalculateLinearFunction(slope, intercept, i);
        bool isLeft = slope > 0 ? left >= index && left < index+1 : left > index && left <= index+1;
        bool isBottom = bottom >= i && bottom < i+1;
        bool isTop = top >= i && top < i+1;

        if (isLeft || isBottom || isTop)
        {
            if (view == FRONT_VIEW)
                for (int y=0; y<ylen; y++)
                    result.push_back(&ctVoxels[index][y][i]);
            else if (view == TOP_VIEW)
                for (int z=0; z<zlen; z++)
                    result.push_back(&ctVoxels[z][index][i]);
            else
                for (int x=0; x<xlen; x++)
                    result.push_back(&ctVoxels[index][i][x]);
            
            if (foundIntercept) foundConsecutiveIntercept = true;
            foundIntercept = true;
            continue;
        }
        
        if (foundIntercept && !foundConsecutiveIntercept) break;
        foundConsecutiveIntercept = false;
    }
}

double CalculateLinearFunction(double k, double b, double x)
{
    return k*x + b;
}

double CalculateXFromPoint(double k, double b, double y)
{
    return (y-b) / k;
}

/**
 *  Note: planeCenter and viewpoint must have the same x and z coordinates.
 */
ProjectionPlane CreateProjectionPlane(Coordinate &planeCenter, Coordinate &viewpoint, vector<vector<vector<Voxel>>> &ctMatrix)
{
    ProjectionPlane plane (viewpoint);
    
    /// useful variables
    double xlen = double(ctMatrix[0][0].size());    /// length
    double ylen = double(ctMatrix[0].size());       /// width
    double zlen = double(ctMatrix.size());          /// height
    
    double viewpointToVoxels = -viewpoint.y;
    double voxelsToProjection = planeCenter.y - ylen;
    
    double proportion = (viewpointToVoxels + voxelsToProjection + ylen) / viewpointToVoxels;
    
    double extLeft     = viewpoint.x < 0       ? 0 : viewpoint.x * proportion;
    double extRight    = viewpoint.x > xlen    ? 0 : (xlen - viewpoint.x) * proportion;
    double extTop      = viewpoint.z > zlen    ? 0 : (zlen - viewpoint.z) * proportion;
    double extBottom   = viewpoint.z < 0       ? 0 : viewpoint.z * proportion;
    
    plane.topLeft = Coordinate(planeCenter.x - extLeft, planeCenter.y, planeCenter.z + extTop);
    plane.topRight = Coordinate(planeCenter.x + extRight, planeCenter.y, planeCenter.z + extTop);
    plane.bottomLeft = Coordinate(planeCenter.x - extLeft, planeCenter.y, planeCenter.z - extBottom);
    plane.bottomRight = Coordinate(planeCenter.x + extRight, planeCenter.y, planeCenter.z - extBottom);
    
    return plane;
}

void PartitionProjectionPlane(ProjectionPlane& plane, double row, double column)
{
    double leftMost = plane.topLeft.x;
    double topMost = plane.topLeft.z;
    double widthSpacing = fabs(plane.topLeft.z - plane.bottomLeft.z) / row;
    double lengthSpacing = fabs(plane.topRight.x - plane.topLeft.x) / column;
    
    plane.intensities.resize(row);
    
    for (int j=0; j<row; j++)
    {
        for (int i=0; i<column; i++)
        {
            plane.intensities[j].push_back(Voxel(0, leftMost+i*lengthSpacing+lengthSpacing/2, plane.bottomLeft.y, topMost-j*widthSpacing-widthSpacing/2));
//            cout << "(" << setw(4) << plane.intensities[j][i].center.x << "," << setw(4) << plane.intensities[j][i].center.y << "," << setw(4) << plane.intensities[j][i].center.z << ")     " ;
        }
//        cout << endl;
    }
//    cout << endl;
}

void GetBoundingVoxelsIndices(double totalLen, double projectionIndex, double sourceIndex, double &higherBound, double &lowerBound)
{
    if ((projectionIndex >= totalLen && sourceIndex >= totalLen) ||
        (projectionIndex <= 0 && sourceIndex <= 0))
    {
        higherBound = INVALID;
        lowerBound = INVALID;
        return;
    }
    
    if (projectionIndex > sourceIndex)
    {
        higherBound = min(fmod(projectionIndex,1)==0 ? (int)projectionIndex/1-1 : (int)projectionIndex/1, (int)totalLen-1);
        lowerBound = max((int)sourceIndex/1, 0);
    }
    else
    {
        higherBound = min(fmod(sourceIndex,1)==0 ? (int)sourceIndex/1-1 : (int)sourceIndex/1, (int)totalLen-1);
        lowerBound = max((int)projectionIndex/1, 0);
    }
}
