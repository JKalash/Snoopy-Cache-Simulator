//
//  main.cpp
//  CreateRandomRWFile
//
//  Created by Joseph Kalash on 3/4/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <math.h>

int main(int argc, const char * argv[]) {
    
    srand((unsigned)time(0));
    std::ofstream myfile;
    myfile.open("data.txt");
    
    for(int i = 0; i < 5000; i++) {
        int cpuid = rand() % 4;
        
        
        //Rand Mem Address Creation
        //Tag & index random value between 0 & 511
        int tag = rand() % 10;
        int index = rand() % 512;
        int offset = rand() % 64;
        int memAddress = (tag << 15) + (index << 6) + offset;
        
        int read = rand() % 2;
        
        myfile << cpuid << " " << memAddress << " ";
        if (read == 0)
            myfile << "r" << "\n";
        else myfile << "w" << "\n";
    }
    
    myfile.close();
    
    return 0;
}
