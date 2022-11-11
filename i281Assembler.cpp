#include <stdlib.h> 
#include<string>
#include <iostream>
#include <unordered_map>
#include "i281AssemblyParser_H.h"


/**
 * @brief Stores information about a user provided variable. Stores the location memory and user provided value together.
 * 
 */
struct usrVar{
    int memoryLoc;
    int val;
};


//Global Variables

//Maps Ope Codes to there 4 bit machine code identifier. 
const std::unordered_map<std::string,std::string> opeCodeMap = 
{
    {"NOOP","0000"},
    {"INPUTC","0001"},
    {"INPUTCF","001"},
    {"INPUTD","0001"},
    {"INPUTDF","0001"},
    {"MOVE","0010"},
    {"LOADI","0011"},
    {"LOADP","0011"},
    {"ADD","0100"},
    {"ADDI","0101"},
    {"SUB","0110"},
    {"SUBI","0111"},
    {"LOAD","1000"},
    {"LOADF","1001"},
    {"STORE","1010"},
    {"STOREF","1011"},
    {"SHIFTL","1100"},
    {"SHIFTR","1100"},
    {"CMP","1101"},
    {"JUMP","1110"},
    {"BRE","1111"},
    {"BRZ","1111"},
    {"BRNE","1111"},
    {"BRNZ","1111"},
    {"BRG","1111"},
    {"BRGE","1111"},
};
//Stores the addresses(Line number) of points the program can jump to 
std::unordered_map<std::string,int> jumpAddressesMap;
//Stores the code that is being operated on as a string
partedCode asmCode;
//Stores the current line of the program the assembler is operating on
int currLineNum;
//Stores the generated machine code as a string
std::string machineCode = "";
//Stores the generated machine code as a string as only the binary bits with no additonal formating
std::string machineCodeUnformmated = "";
//Stores the value of a user provided vairable and its location in memory associated with its name
std::unordered_map<std::string,usrVar> usrVarMap;


void readDataSec(){
    int* cursor = (int*) malloc(sizeof cursor);
    *cursor = 0;

    while(*cursor < asmCode.dataSec.length()){
         std::string currLine = readLine(asmCode.dataSec,cursor,1);
    }
   



}

void parseVarDec(std::string lineToParse){
    std::string varName = "";
    char currChar = 'x';

    while(currChar != ' '){

    }

}

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
    std::string generatedMachineCodeLineRaw = opeCodeMap.at("NOOP");
    generatedMachineCodeLine += "_00_00_00000000\n";
    generatedMachineCodeLineRaw += "000000000000\n";
    machineCode += generatedMachineCodeLine;
    machineCodeUnformmated += generatedMachineCodeLineRaw;
}

void  parseINPUTC(std::string  codeLine){
    std::string generatedMachineCodeLine = opeCodeMap.at("INPUTC");
    std::string generatedMachineCodeLineRaw = opeCodeMap.at("INPUTC");
    generatedMachineCodeLine += "_00_00_";
    generatedMachineCodeLineRaw += "0000";
    
}


int main(){
    std::string rawCode = readFromFile("TestProgram.txt");
    asmCode = parseCode(rawCode);
    parseNOPE();
    std::cout <<  machineCode;


}
