//
//  main.cpp
//  LungNoduleSynthesizer
//
//  Created by Weicheng on 2017-08-17.
//  Copyright © 2017 Weicheng Cao. All rights reserved.
//

/// 1. voxels are static so other objects will use pointer to reference them
/// 2. 3D array will start from (0, 0, 0) and height is 256

#include "methods.hpp"

int main(int argc, const char * argv[]) {
    
    SynthesizeXRayFromCTScan();

    cout << endl;
    return 0;
}
