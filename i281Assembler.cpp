#include <stdlib.h> 
#include<string>
#include <iostream>
#include <unordered_map>
#include <bitset>
#include <vector>
#include "i281AssemblyParser_H.hpp"


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
std::string machineCodeProgram = "";
//Stores a list of unformatted Machine Code Instructions
std::vector<std::string> instructionList;
//Stores the value of a user provided vairable and its location in memory associated with its name
std::unordered_map<std::string,usrVar> usrVarMap;
//Maps the four registers (A,B,C,D) to there binary identifiers (00,01,10,11)
const std::unordered_map<char,std::string> registerNameMap = 
{
    {'A',"00"},
    {'B',"01"},
    {'C',"10"},
    {'D',"11"}

};

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
 * @brief Checks if a provided memory address is out of bounds of the code memory
 * 
 * @param loc The given memory location
 */
void checkOutOfBoundsCodeMem(int loc){
    if(loc < 0  || loc > 63){
        throwAssemblerError("Index is out of bounds. Values must be between 0 - 63 inclusively");
        exit(1);
    }
}

/**
 * @brief Checks if a provided memory address is out of bounds of the data memory
 * 
 * @param loc The given memory location
 */
void checkOutOfBoundsDataMem(int loc){
    if(loc < 0  || loc > 15){
        throwAssemblerError("Index is out of bounds. Values must be between 0 - 15 inclusively");
        exit(1);
    }
}

/**
 * @brief Warns the user than an offset memory location might be out of bounds
 * 
 * @param loc The location without the offset
 */
void offSetOutOfBoundsWarning(int loc){
    std::cout << "On line " << currLineNum << " the program includes a memory access or write with an offset from a register. Depending on the value of the register at the time this could be out of bounds.";
    std::cout << "\nIf the offset is zero the access memory location will be " << loc << '\n';
}
/**
 * @brief Checks if the given value for a register is a legal one
 * 
 * @param givenRegister The given identifier for the register as a string
 */
void checkRegisterValid(std::string givenRegister){
    if(givenRegister != "A" && givenRegister != "B" && givenRegister != "C" && givenRegister != "D"){
        throwAssemblerError("The given value " + givenRegister + " is not a valid register. The value must be A, B, C, or D");
    }
}
/**
 * @brief Checks if the given value for a register is a legal one
 * 
 * @param givenRegister The given identifier for the register as a character
 */
void checkRegisterValid(char givenRegister){
    std::string givenRegisterS = "";
    givenRegisterS += givenRegister;
    if(givenRegisterS != "A" && givenRegisterS != "B" && givenRegisterS != "C" && givenRegisterS != "D"){
        throwAssemblerError("The given value " + givenRegisterS + " is not a valid register. The value must be A, B, C, or D");
    }
}


/**
 * @brief Locates an offset by a regitser within a given string. Removes the offset reference from the string and returns the name of the register.
 * 
 * @param readFrom The string containing the offset by register
 * @return The name of the register 
 */
char readRegisterOffset(std::string* readFrom){
    int* cursor =  (int*) malloc(sizeof cursor);
    *cursor = 0;
    std::string newContents = "";

    while(*cursor < (*readFrom).length()){
       char curr = (*readFrom)[*cursor];
        if(curr == '+'){
            char val = (*readFrom)[*cursor + 2];
            checkRegisterValid(val);
            if(*cursor + 4 < (*readFrom).length()){
                newContents += ((*readFrom).substr(*cursor +4));
            }
            *readFrom = newContents;
            
            return(val);
        }
        *cursor += 1;
        newContents += curr;
    }
    free(cursor);
    throwAssemblerError("Expected to find a register offset. Found none");

    //The program will never reach this line
    return 'x';
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

    *cursor += 1;
    //Returns if this is the end of the reference
    if(*cursor >= reference.length()){
        free(cursor);
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
    if(*cursor >= reference.length()){
        throwAssemblerError("Expected an integer after " + decOperator);
    }
    std::string offSetNum = reference.substr(*cursor);

    int shiftBy = stoi(offSetNum);
    if(decOperator == '+'){
        free(cursor);
        return varMemLoc + shiftBy;
    }
    else{
        free(cursor);
        return varMemLoc -shiftBy;
    }
    
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
        else {
            temp.val = 0;
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

/**
 * @brief Adds '_' characters to a machine code line to make it easier for humans to read it
 * 
 * @param toFormat - The machine code line to format
 * @return - The formatted machine code
 */
std::string formatMachineCodeLineReadable(std::string toFormat){
    return toFormat.substr(0,4) + "_" + toFormat.substr(4,2) + "_" + toFormat.substr(6,2) + "_" + toFormat.substr(8);
}

//NOT IN USE
std::string parseOPECode(std::string codeLine,int* cursor){
    *cursor = 0;
    std::string opeCode = readWord(codeLine,cursor);
    *cursor+=1;
    return opeCode;
}
/* SAVED FOR LATER
    machineCode += generatedMachineCodeLine;
    instructionList.push_back(generatedMachineCodeLineRaw) ;
*/

/**
 * @brief Parses a line of code with the Ope code NOOP
 * 
 */
std::string parseNOPE(){
    return "000000000000";
}

/**
 * @brief Parses a line of code with the Ope codes INPUTC, INPUTCF, INPUTD, INPUTDF. 
 * 
 * @param codeLine The line of code to parse
 */
std::string parseINPUT(std::string asmCodeLine, std::string opeCode){
    std::string generatedMachineCode = "";
   //Cursor to read through line
    int* cursor =  (int*) malloc(sizeof cursor);
    *cursor = 0;

    //Reads the contents of the bracket and gets the contents
    std::string bracketContents = parseBrackets(asmCodeLine,*cursor);
    if(bracketContents == "BRACKET_WAS_NOT_CLOSED" || bracketContents == "NO_BRACKETS_IN_STRING"){
        throwAssemblerError("Expected well formed brackets");
    }

    //If the Ope code is INPUTC or INPUTD sets the next 4 bits to there values
    if(opeCode == "INPUTC" ){
        generatedMachineCode += "0000";
    }
    else if(opeCode == "INPUTD"){
        generatedMachineCode += "0010";
    }
    //If the Ope code is INPUTCF or INPUTDF gets the offset register and then sets the next four bits
    else{
        //Reads and removes the register to offset by
        std::string* contentsPointer = &bracketContents;
        char offsetRegister = readRegisterOffset(contentsPointer);
        generatedMachineCode += registerNameMap.at(offsetRegister);

        //Sets the next two bits depending on the Ope code
        if(opeCode == "INPUTCF"){
            generatedMachineCode += "01";
        }
        else{
            generatedMachineCode += "11";
        }
    }

    int referencedMemLoc = readVarReference(bracketContents);
    
    //Checks if the given location is out of bounds
    if(opeCode == "INPUTC" || opeCode== "INPUTCF"){
         checkOutOfBoundsCodeMem(referencedMemLoc);
    }
    else{
         checkOutOfBoundsDataMem(referencedMemLoc);
    }

    //Warns the user that if the offset could go outside the code memory
    if(opeCode == "INPUTCF" || opeCode == "INPUTDF"){
        offSetOutOfBoundsWarning(referencedMemLoc);
    }
    
    //Converts the location to binary
    std::bitset<8> asBinary;
    asBinary = referencedMemLoc;

    //Adds the location to the generated code
    generatedMachineCode += asBinary.to_string();

    free(cursor);
    return generatedMachineCode;
}
/**
 * @brief Parses a line of code with the Ope code MOVE
 * 
 * @param codeLine The line of code to parse
 */
std::string parseMOVE(std::string asmCodeLine){
    std::string generatedMachineCode = "";
    //Cursor to read through line
    int* cursor =  (int*) malloc(sizeof cursor);
    *cursor = 0;

    //Gets both of the given registers
    std::string reg1 = readWord(asmCodeLine,cursor);
    *cursor+=1;
    std::string reg2 = readWord(asmCodeLine,cursor);

    //Checks to make sure the registers are valid 
    checkRegisterValid(reg1);
    checkRegisterValid(reg2);

    //Gets the binary identifiers for the registers
    std::string reg1Identifier = registerNameMap.at(reg1[0]);
    std::string reg2Identifier = registerNameMap.at(reg2[0]);
    
    //Adds the rest of the machine code 
    generatedMachineCode += reg1Identifier + reg2Identifier + "00000000" ;

    free(cursor);
    return generatedMachineCode;
}

/**
 * @brief Parses a line of code with the Ope code LOADI
 * 
 * @param codeLine The line of code to parse
 */
std::string parseLOADI(std::string asmCodeLine){
    std::string generatedMachineCode = "";
    //Cursor to read through line
    int* cursor =  (int*) malloc(sizeof cursor);
    *cursor = 0;

    //Gets the register to load the value into 
    std::string loadRegister = readWord(asmCodeLine,cursor);
    *cursor+=1;

    //Checks to make sure the register is valid 
    checkRegisterValid(loadRegister);



    //Gets the binary identifiers for the registers
    std::string regIdentifier = registerNameMap.at(loadRegister[0]);
    
    //Adds register and the don't care bits 
    generatedMachineCode += regIdentifier + "00";

    //Gets the immediate value and converts it to binary
    std::string imedValStr = readWord(asmCodeLine,cursor);
    int imedValInt = stoi(imedValStr);
    std::bitset<8> imedValBnr;
    imedValBnr = imedValInt;

    //Adds the immediate value to the rest of the code
    generatedMachineCode += imedValBnr.to_string();

    free(cursor);
    return generatedMachineCode;

}

/**
 * @brief Parses a line of code with the Ope code LOADP
 * 
 * @param codeLine The line of code to parse
 */
std::string parseLOADP(std::string asmCodeLine){
    std::string generatedMachineCode = "";
    //Cursor to read through line
    int* cursor =  (int*) malloc(sizeof cursor);
    *cursor = 0;

    //Gets the register to load the value into 
    std::string loadRegister = readWord(asmCodeLine,cursor);
    *cursor+=1;

    //Checks to make sure the register is valid 
    checkRegisterValid(loadRegister);

    //Gets the binary identifiers for the registers
    std::string regIdentifier = registerNameMap.at(loadRegister[0]);
    
    //Adds register and the don't care bits 
   generatedMachineCode += regIdentifier + "00";

    //Gets the pointer value and converts it to binary
    std::string bracketContents = parseBrackets(asmCodeLine,*cursor);
    if(bracketContents == "BRACKET_WAS_NOT_CLOSED" || bracketContents == "NO_BRACKETS_IN_STRING"){
        throwAssemblerError("Expected well formed brackets");
    }
    int pointerLoc = readVarReference(bracketContents);
    std::bitset<8> pointerLocBnr;
    pointerLocBnr = pointerLoc;

    //Adds the immediate value to the rest of the code
    generatedMachineCode += pointerLocBnr.to_string();

    free(cursor);
    std::cout << generatedMachineCode << "\n";
    return generatedMachineCode;
}

/**
 * @brief Parses a line of code with the Ope code ADD or SUB
 *
 * @param codeLine The line of code to parse
 */
std::string parseADDxSUB(std::string asmCodeLine){
    //Gets the two regiesters to add together
    char reg1 = asmCodeLine[0];
    char reg2 = asmCodeLine[2];

    //Checks to make sure the registers are valid 
    checkRegisterValid(reg1);
    checkRegisterValid(reg2);

    //Converts the registers to there binary indentifiers
    std::string reg1Indentifier = registerNameMap.at(reg1);
    std::string reg2Indentifier = registerNameMap.at(reg2);

    //Returns the machine code line
    return reg1Indentifier + reg2Indentifier + "00000000";
}

/**
 * @brief Parses a line of code with the Ope code ADDI or SUBI
 *
 * @param codeLine The line of code to parse
 */
std::string parseADDIxSUBI(std::string asmCodeLine){
    std::string generatedMachineCode = "";

    //Gets the regiester to add to the immedval
    char additionRegister = asmCodeLine[0];

    //Checks to make sure the register is valid 
    checkRegisterValid(additionRegister);

    //Converts the register to its binary indentifier
    std::string regIndentifier = registerNameMap.at(additionRegister);

    //Adds the indentifiers and don't cares to the line
    generatedMachineCode += regIndentifier + "00";

     //Gets the immediate value and converts it to binary
    std::string imedValStr = readWord(asmCodeLine,2);
    int imedValInt = stoi(imedValStr);
    std::bitset<8> imedValBnr;
    imedValBnr = imedValInt;

    //Adds the immediate value to the rest of the code
    generatedMachineCode += imedValBnr.to_string();

    return generatedMachineCode;

}

/**
 * @brief Parses a line of code with the Ope codes LOAD, LOADF, STORE, STOREF. 
 * 
 * @param codeLine The line of code to parse
 */
std::string parseLOADxSTORE(std::string asmCodeLine, std::string opeCode){
    std::string generatedMachineCode = "";
   //Cursor to read through line
    int* cursor =  (int*) malloc(sizeof cursor);
    *cursor = 0;

    //Gets the register to have date stored in/loaded from 
    char accessRegister = readWord(asmCodeLine,cursor)[0];
    *cursor += 1;

    //Makes sure the access register is a valid register
    checkRegisterValid(accessRegister);

    //Converts the access register to its binary  identifier 
    std::string accessRegIndentifier = registerNameMap.at(accessRegister);

    //Adds the indentifier to the machine code
    generatedMachineCode += accessRegIndentifier;

    //Reads the contents of the bracket and gets the contents
    std::string bracketContents = parseBrackets(asmCodeLine,*cursor);
    if(bracketContents == "BRACKET_WAS_NOT_CLOSED" || bracketContents == "NO_BRACKETS_IN_STRING"){
        throwAssemblerError("Expected well formed brackets");
    }

    //If the Ope code is LOAD or STORES sets the next 2 bits as don't cares
    if(opeCode == "LOAD" || opeCode == "STORE"){
        generatedMachineCode += "00";
    }
    //If the Ope code is LOADF or STOREF gets the offset register and then sets the next two bits
    else{
        //Reads and removes the register to offset by
        std::string* contentsPointer = &bracketContents;
        char offsetRegister = readRegisterOffset(contentsPointer);
        generatedMachineCode += registerNameMap.at(offsetRegister);
    }

    int referencedMemLoc = readVarReference(bracketContents);
    
    //Checks if the given location is out of bounds
     checkOutOfBoundsDataMem(referencedMemLoc);
    
    //Warns the user that if the offset could go outside the data memory
    if(opeCode == "LOADF" || opeCode == "STOREF"){
        offSetOutOfBoundsWarning(referencedMemLoc);
    }
    
    //Converts the location to binary
    std::bitset<8> asBinary;
    asBinary = referencedMemLoc;

    //Adds the location to the generated code
    generatedMachineCode += asBinary.to_string();

    free(cursor);
    return generatedMachineCode;
}

int main(){
    std::string rawCode = readFromFile("TestProgram.txt");
    asmCode = parseCode(rawCode);
    readDataSec();

    std::string test = opeCodeMap.at("LOAD") + parseLOADxSTORE("B,[last + C]","STOREF");

    std::cout << formatMachineCodeLineReadable(test);

    /*
    asmCode = parseCode(rawCode);
    readDataSec();
    */

    //std::cout <<asmCode.codeSec;



}
