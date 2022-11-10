#include <stdlib.h> 
#include<string>
#include <iostream>
#include <unordered_map>
#include "i281AssemblyParser_H.h"


struct usrVar{
    std::string name;
    int val;
};


//Global Variables

//Maps Ope Codes to there 4 bit machine code identifier. 
const std::unordered_map<std::string,std::string> opeCodeMap = 
{
    {"NOOP","0000_"},
    {"INPUTC","0001_"},
    {"INPUTCF","001_"},
    {"INPUTD","0001_"},
    {"INPUTDF","0001_"},
    {"MOVE","0010_"},
    {"LOADI","0011_"},
    {"LOADP","0011_"},
    {"ADD","0100_"},
    {"ADDI","0101_"},
    {"SUB","0110_"},
    {"SUBI","0111_"},
    {"LOAD","1000_"},
    {"LOADF","1001_"},
    {"STORE","1010_"},
    {"STOREF","1011_"},
    {"SHIFTL","1100_"},
    {"SHIFTR","1100_"},
    {"CMP","1101_"},
    {"JUMP","1110_"},
    {"BRE","1111_"},
    {"BRZ","1111_"},
    {"BRNE","1111_"},
    {"BRNZ","1111_"},
    {"BRG","1111_"},
    {"BRGE","1111_"},
};
//Stores the addresses(Line number) of points the program can jump to 
std::unordered_map<std::string,int> jumpAddressesMap;
//Stores the code that is being operated on as a string
partedCode asmCode;
//Stores the current line of the program the assembler is operating on
int currLineNum;
//Stores the generated machine code as a string
std::string machineCode = "";
//Stores the locations of the user provided variables in memory 
std::unordered_map<usrVar,int> usrVarMap;


/**
 * @brief Finds the line number for the jump addreses in the code and associates it with the address in the jumpAddressesMap
 * 
 */
void setJumpAddreses(){
    int* cursor =  (int*) malloc(sizeof cursor);
    *cursor =0;

    currLineNum =0;
    while(*cursor <  asmCode.codeSec.length()){
        std::string currLine = readLine(asmCode.codeSec,cursor,1);
        int jumpAdrPos =  currLine.find(':');

        if(jumpAdrPos != std::string::npos){
            std::string jumpAdrName = currLine.substr(jumpAdrPos+1);
            jumpAddressesMap.insert({jumpAdrName,currLineNum});
        }

        currLineNum++;
    }


    free(cursor);
}

void parseNOPE(){
    std::string generatedMachineCodeLine = opeCodeMap.at("NOOP");
    generatedMachineCodeLine += "00_00_00000000\n";
    machineCode += generatedMachineCodeLine;
}

void  parseINPUTC(std::string  codeLine){
    std::string generatedMachineCodeLine =  opeCodeMap.at("INPUTC");
    generatedMachineCodeLine += "00_00_";
}


int main(){
    asmCode = parseCode("TestProgram.txt");
    parseNOPE();
    std::cout <<  machineCode;


}
