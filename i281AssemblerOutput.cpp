#include <stdlib.h> 
#include <iostream>
#include <fstream>
#include <vector>
#include"i281AssemblerOutput.hpp"


void declareVerilogOutputs(int numOutputs,int outputBusSize, std::ofstream* outputFile){
    for(int i =0; i <  numOutputs; i++){
        *outputFile << "output [" << outputBusSize-1 << ":0]b" << i <<"I;\n";
    }
}


void outputUserCode(std::vector<std::string> instructionList, int startLoc,std::string filePath,std::string moduleName){
    std::ofstream codeFile;
    codeFile.open(filePath + moduleName + ".v");

    //Write the first line to the file
    codeFile << "module " << moduleName << "(b0I,b1I,b2I,b3I,b4I,b5I,b6I,b7I,b8I,b9I,b10I,b11I,b12I,b13I,b14I,b15I);\n\n";
    
    //Declare the outputs
    declareVerilogOutputs(16,16,&codeFile);
    codeFile << '\n';
    
    //Write the assign statements until all the instructions are used or the length is 16
    int i = 0;
    while(i + startLoc < instructionList.size()){
        codeFile << "assign b" << i << "I[15:0] = 16\'b" << instructionList[startLoc + i] <<";\n";
        i++;
    }
    //Adds empty instructions if less than 16 
    while(i < 16){
        codeFile << "assign b" << i << "I[15:0] = 16\'b" <<"0000_00_00_00000000" <<";\n";
        i++; 
    }  

    codeFile << "endmodule";
    codeFile.close();
}   

void outputUserData(std::vector<usrVarOutput> usrVars,std::string filePath){
    std::ofstream dataFile;
    dataFile.open(filePath + "User_Data.v");

    
    //Write the first line to the file
    dataFile << "module User_Data.v" << "(b0I,b1I,b2I,b3I,b4I,b5I,b6I,b7I,b8I,b9I,b10I,b11I,b12I,b13I,b14I,b15I);\n\n";
    
    //Declare the outputs
    declareVerilogOutputs(16,7,&dataFile);
    dataFile << '\n';

    //Write the assign statements until all the instructions are used or the length is 16
    int i = 0;
    int lineCounter = 0;
    while(i < usrVars.size()){
        dataFile << "assign b" << i << "I[7:0] = 8\'b" << usrVars[i].val << "; //" << usrVars[i].name << "\n";
        i++;
    }
    //Adds empty instructions if less than 16 
    while(i < 16){
        dataFile << "assign b" << i << "I[7:0] = 8\'b" <<"00000000" <<";\n";
        i++;
        
    }

    dataFile << "endmodule";
    dataFile.close();
} 

void outputBinFile(std::string rawCode,std::string machineCode,std::string programName,std::string filePath,std::vector<int> varsList,std::vector<branchLocOutput> branchLocs){
    std::ofstream binFile;
    binFile.open(filePath + programName + ".bin");

    binFile << rawCode;

    //Writes the machine code to file
    binFile << "\n\n-----MACHINE CODE-----\n" << machineCode;

    //Writes the data segment values to the file
    binFile << "\n-----DATA SEGMENT-----\n[";

    if(varsList.size() != 0){
          binFile << varsList[0];
    }

    for(int i=1; i < varsList.size(); i++){
        binFile << ", "<< varsList[i];
    }
    binFile << "]\n";

    //Writes the Branch Destinations to the file
    binFile << "\n----------Branch Destinations--------\n{";

    if(branchLocs.size() != 0){
        binFile << branchLocs[0].name << "=" << branchLocs[0].loc;
    }

    for(int i=1; i < branchLocs.size(); i++){
        binFile << ", "<< branchLocs[i].name << "=" << branchLocs[i].loc;
    }
    binFile << "}\n";

    binFile.close();

}

void outputToConsole(std::string rawCode,std::string machineCode,std::vector<int> varsList,std::vector<branchLocOutput> branchLocs){
    std::cout << rawCode;

    //Writes the machine code to file
    std::cout << "\n\n-----MACHINE CODE-----\n" << machineCode;

    //Writes the data segment values to the file
    std::cout << "\n-----DATA SEGMENT-----\n[";

    if(varsList.size() != 0){
          std::cout << varsList[0];
    }

    for(int i=1; i < varsList.size(); i++){
        std::cout << ", "<< varsList[i];
    }
    std::cout << "]\n";

    //Writes the Branch Destinations to the file
    std::cout << "\n----------Branch Destinations--------\n{";

    if(branchLocs.size() != 0){
        std::cout << branchLocs[0].name << "=" << branchLocs[0].loc;
    }

    for(int i=1; i < branchLocs.size(); i++){
        std::cout << ", "<< branchLocs[i].name << "=" << branchLocs[i].loc;
    }
    std::cout << "}\n";

}