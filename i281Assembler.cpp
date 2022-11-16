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
    {"INPUTCF","0001"},
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
 * @brief Prints to the console that an error occured with a given message. Ends the program
 * 
 * @param lineNum The line number that error happened at
 * @param errorMessage What to print to the console along with the error
 */
void throwAssemblerError(std::string  errorMessage){
    std::cout << "ERROR on line " << currLineNum << ": " << errorMessage;
    exit(1);
}


/**
 * @brief Reads the contents of bracket [] and gets the location of the variable reference including any offset included in the brackets
 * 
 * @param reference The string to read the variable reference from
 * @return The location in memory referenced by the code
 */
int readVarReference(std::string reference){
    //Creates a cursor to read through the reference with 
    int* cursor =  (int*) malloc(sizeof cursor);
    *cursor = 0;

    //Reads the first word in the referense assigns saves it as the name of the referenced variable
    std::string varName = readWord(reference,cursor);

    usrVar referencedVar;
    //Gets the information about the variable from the map. Throws an error if this variable wasn't declared in the data section
    try{
         referencedVar = usrVarMap.at(varName);
    }
    catch(std::out_of_range){
        throwAssemblerError("The variable " + varName +" was referenced without a declaration.");
    }

 
    //Gets the location of the variable in from the retrieved struct
    int varMemLoc = referencedVar.memoryLoc;

    *cursor += 2;
    //Returns if this is the end of the reference
    if(*cursor == reference.length()){
        return varMemLoc;
    }

    //Makes sure the current character is a + or -. 
    char decOperator = reference[*cursor];
    if(decOperator != '+' && decOperator != '-'){
        std::string temp = "";
        temp += decOperator;
        throwAssemblerError("Received illegal operator "+ temp + " expected + or -");
    }


    *cursor += 2;
    //Throws an error if there is no integer after the given operator
    if(*cursor == reference.length()){
        throwAssemblerError("Expected an integer after " + decOperator);
    }

    //Returns the offset memory value
    int shiftBy = (int) reference[*cursor];
    if(decOperator == '+'){
        return varMemLoc + shiftBy;
    }
    else{
        return varMemLoc -shiftBy;
    }
    free(cursor);
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
        currLineNum = asmCode.lineNums.dataLineNum + *varCounter;
        throwAssemblerError("A variable was declared with the data type " +  dataType + " expected BYTE.");
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

void  parseINPUTC(std::string  codeLine){
    std::string generatedMachineCodeLine = opeCodeMap.at("INPUTC");
    std::string generatedMachineCodeLineRaw = opeCodeMap.at("INPUTC");
    generatedMachineCodeLine += "_00_00_";
    generatedMachineCodeLineRaw += "0000";
    
    int* cursor =  (int*) malloc(sizeof cursor);
    *cursor = 0;
    readWord(codeLine,cursor);
    *cursor+=1;

    std::string bracketContents = parseBrackets(codeLine,*cursor);

}


int main(){
    std::string rawCode = readFromFile("TestProgram.txt");

    asmCode = parseCode(rawCode);
    readDataSec();


    std::cout <<asmCode.codeSec;



}
