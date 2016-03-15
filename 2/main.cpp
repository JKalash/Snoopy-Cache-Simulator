//
//  main.cpp
//  2
//
//  Created by Joseph Kalash on 2/29/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#include <libiomp/omp.h>
#include <iostream>
#include <stdio.h>
#include <vector>

#include "Helper.h"
#include "Processor.h"

#define N_CORES 4
#define FILE_PATH "ENTER_INSTRUCTIONS_FILE_PATH"

int main(int argc, const char * argv[]) {
    
    //One thread per core.
    omp_set_num_threads(N_CORES);
    
    //Read instructions from text file
    std::vector<RWInstruction> instructions = Helper::readIntructionsFile(FILE_PATH);
    
    //Bus is a shared variable holding the value of the message that its currently transmitting
    Message bus;
    
    //Create (Quad-core) processors
    std::vector<Processor> processors;
    for(int i = 0; i < N_CORES; i++)
        processors.push_back(Processor(i, instructions, bus));
    
    typedef enum : int {
        ReadingRequest,
        DoingSomething,
        Snooping,
    } Iteration;
    
    //Task is complete once all instructions have been processed
    while (!Helper::taskComplete(processors)) {
        /*
         1. Processor initially Idle
         2. Figure out what request to make. -> state becomes Read New Request
         3. Based on Priority (code in main inside parallel) -> Snoop or Do the Request
         4. If did request -> Increment instructionsProcessed & Go back to idle
         5. Barrier until all processes have equal intructionsProcessed
         */
        #pragma omp parallel
        {
            
            int cpuid = omp_get_thread_num();
            Processor* p = nullptr;
            for(int i = 0; i < processors.size(); i++)
                if(cpuid == processors[i].cpuid)
                    p = &processors[i];
            
            //Prepare new request to process
            if(p != nullptr)
                p->prepareNewRequest();
            
            #pragma omp barrier
            
            if(p != nullptr)
                //Do a request & go back to idle
                if(Helper::isProcessorPriority(cpuid, processors))
                    p->doSomething();
            
            #pragma omp barrier
            
            //Snoop
            if(p != nullptr)
                if(p->state != Processor::Executed)
                    p->snoopBus();
            
            #pragma omp barrier
            //Reset States
            if(p != nullptr)
                p->state = Processor::Idle;
        }
    }
    
    cout << "Task Complete!\n";
    
    return 0;
}
