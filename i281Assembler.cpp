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

/**
 * @brief Prints to the console that a variable was declared with a data type other than BYTE
 * 
 * @param lineNum The line number that error happened at
 * @param givenDataType 
 */
void unknownDataTypeError(int lineNum, std::string  givenDataType){
    std::cout << "ERROR on line " << lineNum << ": A variable was declared with the data type " << givenDataType << " expected BYTE";
    exit(1);
}



/**
 * @brief Parses a users variable declaration. Associates its value and location and memory with its name in the usrVar map. 
 * Prints an error if a data type other than BYTE is used
 * 
 * @param lineToParse The line of code to parse the declaration from
 * @param varCounter A running count of the number of variables the user has declared
 */
void parseVarDec(std::string lineToParse, int* varCounter){
    //Initalizes needed variables

    int arrCounter =0;
    std::string counterStr = "";
    int* cursor = (int*) malloc(sizeof cursor);
    *cursor =0;

    //Reads the first word in the declaration and sets the variables name to it
    std::string varName = readWord(lineToParse,cursor);
    *cursor += 1;

    //Reads the second word in the declaration saves it as dataType.
    std::string dataType = readWord(lineToParse,cursor);

    //Ends the program and prints to the console if the data type is anything  besides BYTE
    if(dataType != "BYTE"){
        int lineNum = asmCode.lineNums.dataLineNum + *varCounter;
        unknownDataTypeError(lineNum, dataType);
    }
    *cursor +=1;

    //Loops through the rest of the line until it runs out of variable values - needed in case the user declares an array
    while(* cursor < lineToParse.length()){
        char varVal = lineToParse[*cursor];
        usrVar temp;

        //Checks if the user has declared a number value at this char
        if(varVal != ',' && varVal != '?'){
            temp.val = varVal - '0';
        }
        //Sets the value to zero if a ? is given
        else{
            varVal = 0;
        }

        //If a variable is being declared stores it in the variable map
        if(varVal != ','){
            temp.memoryLoc = *varCounter  -1;
            //If this variable is the second element in an array adds a number to the end of it to differentiate it
            std::string insertName =  varName + counterStr;

            usrVarMap.insert({insertName,temp});

            (*varCounter)++;
            arrCounter++;
            counterStr = (char) ('0' + arrCounter);
        }
        *cursor += 1;
        
    }
    free(cursor);
}

/**
 * @brief Reads through the programs data section and handles all the variable declarations
 * 
 */
void readDataSec(){
    int* cursor = (int*) malloc(sizeof cursor);
    *cursor = 0;
    int* varCounter = (int*) malloc(sizeof varCounter);
    *varCounter = 1;

    while(*cursor < asmCode.dataSec.length()){
         std::string currLine = readLine(asmCode.dataSec,cursor,1);
         parseVarDec(currLine,varCounter);
    }
   
   free(cursor);
   free(varCounter);
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

void  parseINPUTC(std::string  codeLine,int lineNuNum){
    std::string generatedMachineCodeLine = opeCodeMap.at("INPUTC");
    std::string generatedMachineCodeLineRaw = opeCodeMap.at("INPUTC");
    generatedMachineCodeLine += "_00_00_";
    generatedMachineCodeLineRaw += "0000";
    
    int* cursor =  (int*) malloc(sizeof cursor);
    *cursor = 0;
    readWord(codeLine,cursor);
    *cursor+=1;

    std::string bracketContents = parseBrackets(codeLine,*cursor);

    int* bracketCursor =  (int*) malloc(sizeof bracketCursor);
    *bracketCursor = 0;

    std::string varName = readWord(bracketContents,bracketCursor);
    
    try{
        usrVar referencedVar = usrVarMap.at(varName);
    }
    catch(std::out_of_range){
        undefinedVariableError();
    }
    



}


int main(){
    std::string rawCode = readFromFile("TestProgram.txt");

    asmCode = parseCode(rawCode);
    readDataSec();


    std::cout <<asmCode.codeSec;



}
