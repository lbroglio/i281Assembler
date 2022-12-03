#include <stdlib.h> 
#include<string>
#include <iostream>
#include <unordered_map>
#include <bitset>
#include <vector>
#include "i281AssemblyParser_H.hpp"
#include "i281AssemblerOutput_H.hpp"


/**
 * @brief Stores information about a user provided variable. Stores the location memory and user provided value together.
 * 
 */
struct usrVar{
    int memoryLoc;
    int val;
};


//Global Variables

//Maps OpCodes to there 4 bit machine code identifier. 
const std::unordered_map<std::string,std::string> opCodeMap = 
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
//Stores  he current line within the .code section the assembler is operation on
int codeSecLineNum = 1;
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
//Stores a list of the values of user variables for use in outputing
std::vector<int> varVals;
//Stores the names of the variables in order
std::vector<std::string> varNames;
//Stores the code imediately after it is read from the file without any parsing peformed on it
std::string rawCode;
//Stores the name of the users program
std::string programName;
//Stores the path to the users program
std::string userFilePath;

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
     if(loc < 0  || loc > 63){
        std::cout << "On line " << currLineNum << " the program includes a memory access or write with an offset from a register. Depending on the value of the register at the time this could be out of bounds.";
        std::cout << "\nIf the offset is zero the access memory location will be " << loc << "\n\n";
    }
    
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
    int shiftBy;

    //Converts the offSetNum to an integer throws an assembler error if it cannot 
    try{
        shiftBy = stoi(offSetNum);
    }
    catch(std::invalid_argument){
        throwAssemblerError(offSetNum +" is not a valid constant to offset a memory location by.");
    }
    
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
        std::string varVal = readWord(lineToParse,cursor);
        usrVar temp;

        //Checks if the user has declared a number value at this char
        if(varVal != "?"){
            temp.val = stoi(varVal);
        }
        //Sets the value to zero if a ? is given
        else {
            temp.val = 0;
        }

        //Stores the variable in the map and the name list
        temp.memoryLoc = *varCounter  -1;
        //If this variable is the second element in an array adds a number to the end of it to differentiate it
        std::string insertName =  varName + counterStr;
        varNames.push_back(insertName);
        usrVarMap.insert({insertName,temp});

        (*varCounter)++;
        arrCounter++;
        counterStr = "[";
        counterStr += (char) ('0' + arrCounter);
        counterStr += "]";
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

    if(*varCounter > 15){
        throwAssemblerError("You cannot have more than 16 BYTES in your data segment");
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
std::string formatMachineCodeLine(std::string toFormat){
    return toFormat.substr(0,4) + "_" + toFormat.substr(4,2) + "_" + toFormat.substr(6,2) + "_" + toFormat.substr(8);
}


/**
 * @brief Gets the imdevalue bits needed for a code line with the given jump address as a 2s complement binary number
 * 
 * @param jumpAddress - The name of the address to jump to
 * @return The 2s complement binary number to use as the imedvalue stored as a string
 */
std::string getLineJumpImedVal(std::string jumpAddress){
     //Gets the line number of the jump address
    int jumpAddressLineNum = jumpAddressesMap.at(jumpAddress);

    //Gets the difference between the current line num and the address
    int diffInAdds = jumpAddressLineNum - codeSecLineNum;

    //Converts the difference to 2's complement binary
    std::bitset<8> bnrAddress = diffInAdds;

    return bnrAddress.to_string();
}


/**
 * @brief Converts a User Inputed constant to a 2's complement binary imedvalue
 * 
 * @param givenConstant The constant provided by the user
 * @return The 2's complement binary representation of the constant stored as a string
 */
std::string parseConstantImedValue(int givenConstant){
    //Converts the constant to 2's complement binary
    std::bitset<8> bnrImedVal = givenConstant;

    return bnrImedVal.to_string();
}

/**
 * @brief Parses a line of code with the OpCpde NOOP
 * 
 * @return The machine code instruction of this line
 */
std::string parseNOOP(){
    return "000000000000";
}

/**
 * @brief Parses a line of code with the OpCodes INPUTC, INPUTCF, INPUTD, INPUTDF. 
 * 
 * @param codeLine The line of code to parse
 * @param opCode This line of codes opCode
 * 
 * @return The machine code instruction of this line
 */
std::string parseINPUT(std::string asmCodeLine, std::string opCode){
    std::string generatedMachineCode = "";
   //Cursor to read through line
    int* cursor =  (int*) malloc(sizeof cursor);
    *cursor = 0;

    //Reads the contents of the bracket and gets the contents
    std::string bracketContents = parseBrackets(asmCodeLine,*cursor);
    if(bracketContents == "BRACKET_WAS_NOT_CLOSED" || bracketContents == "NO_BRACKETS_IN_STRING"){
        throwAssemblerError("Expected well formed brackets");
    }

    //If the OpCpde is INPUTC or INPUTD sets the next 4 bits to there values
    if(opCode == "INPUTC" ){
        generatedMachineCode += "0000";
    }
    else if(opCode == "INPUTD"){
        generatedMachineCode += "0010";
    }
    //If the OpCpde is INPUTCF or INPUTDF gets the offset register and then sets the next four bits
    else{
        //Reads and removes the register to offset by
        std::string* contentsPointer = &bracketContents;
        char offsetRegister = readRegisterOffset(contentsPointer);
        generatedMachineCode += registerNameMap.at(offsetRegister);

        //Sets the next two bits depending on the OpCpde
        if(opCode == "INPUTCF"){
            generatedMachineCode += "01";
        }
        else{
            generatedMachineCode += "11";
        }
    }

    int referencedMemLoc = readVarReference(bracketContents);
    
    //Checks if the given location is out of bounds
    if(opCode == "INPUTC"){
         checkOutOfBoundsCodeMem(referencedMemLoc);
    }
    else if (opCode == "INPUTD"){
         checkOutOfBoundsDataMem(referencedMemLoc);
    }

    //Warns the user that if the offset could go outside the code memory
    if(opCode == "INPUTCF" || opCode == "INPUTDF"){
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
 * @brief Parses a line of code with the OpCpde MOVE
 * 
 * @param codeLine The line of code to parse
 * 
 * @return The machine code instruction of this line
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
 * @brief Parses a line of code with the OpCpde LOADI
 * 
 * @param codeLine The line of code to parse
 * 
 * @return The machine code instruction of this line
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
   
    std::string imedValue = parseConstantImedValue(imedValInt);

    //Adds the immediate value to the rest of the code
    generatedMachineCode += imedValue;

    free(cursor);
    return generatedMachineCode;

}

/**
 * @brief Parses a line of code with the OpCpde LOADP
 * 
 * @param codeLine The line of code to parse
 * 
 * @return The machine code instruction of this line
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
    return generatedMachineCode;
}

/**
 * @brief Parses a line of code with the OpCpde ADD, SUB, or CMP
 *
 * @param codeLine The line of code to parse
 * 
 * @return The machine code instruction of this line
 */
std::string parseADDxSUBxCMP(std::string asmCodeLine){
    //Gets the two regiesters to add or subtract together
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
 * @brief Parses a line of code with the OpCpde ADDI or SUBI
 *
 * @param codeLine The line of code to parse
 * 
 * @return The machine code instruction of this line
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
    std::string imedVal =  parseConstantImedValue(imedValInt);

    //Adds the immediate value to the rest of the code
    generatedMachineCode += imedVal;

    return generatedMachineCode;

}

/**
 * @brief Parses a line of code with the OpCodes LOAD, LOADF
 * 
 * @param codeLine The line of code to parse
 * @param opCode This line of codes opCode
 * 
 * @return The machine code instruction of this line
 */
std::string parseLOAD(std::string asmCodeLine, std::string opCode){
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

    //If the OpCpde is LOAD sets the next 2 bits as don't cares
    if(opCode == "LOAD"){
        generatedMachineCode += "00";
    }
    //If the OpCpde is LOADF sets the offset register and then sets the next two bits
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
    if(opCode == "LOADF"){
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
 * @brief Parses a line of code with the OpCodes STORE
 * 
 * @param codeLine The line of code to parse
 * @param opCode This line of codes opCode
 * 
 * @return The machine code instruction of this line
 */
std::string parseSTORE(std::string asmCodeLine, std::string opCode){
    std::string generatedMachineCode = "";
   //Cursor to read through line
    int* cursor =  (int*) malloc(sizeof cursor);
    *cursor = 0;

    //Reads the contents of the bracket and gets the contents
    std::string bracketContents = parseBrackets(asmCodeLine,cursor);
    *cursor += 1;


    if(bracketContents == "BRACKET_WAS_NOT_CLOSED" || bracketContents == "NO_BRACKETS_IN_STRING"){
        throwAssemblerError("Expected well formed brackets");
    }

    //Gets the register to have date stored in/loaded from 
    char accessRegister = readWord(asmCodeLine,cursor)[0];

    //Makes sure the access register is a valid register
    checkRegisterValid(accessRegister);

    //Converts the access register to its binary  identifier 
    std::string accessRegIndentifier = registerNameMap.at(accessRegister);

    //Adds the indentifier to the machine code
    generatedMachineCode += accessRegIndentifier;


    //If the OpCpde is STORE sets the next 2 bits as don't cares
    if(opCode == "STORE"){
        generatedMachineCode += "00";
    }
    //If the OpCpde is STOREF gets the offset register and then sets the next two bits
    else{
        //Reads and removes the register to offset by
        std::string* contentsPointer = &bracketContents;
        char offsetRegister = readRegisterOffset(contentsPointer);
        generatedMachineCode += registerNameMap.at(offsetRegister);
    }

    //Reads the variables referenced within the brackets
    int referencedMemLoc = readVarReference(bracketContents);
    
    //Checks if the given location is out of bounds
     checkOutOfBoundsDataMem(referencedMemLoc);
    
    //Warns the user that if the offset could go outside the data memory
    if(opCode == "STOREF"){
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
 * @brief Parses a line of code with the OpCodes SHIFTL, or SHIFTR
 * 
 * @param codeLine The line of code to parse
 * @param opCode This line of codes opCode
 * 
 * @return The machine code instruction of this line
 */
std::string parseSHIFT(std::string asmCodeLine, std::string opCode){
    std::string generatedMachineCode = "";
   //Cursor to read through line
    int* cursor =  (int*) malloc(sizeof cursor);
    *cursor = 0;

    //Gets the register to shift
    char regToShift = readWord(asmCodeLine,cursor)[0];
    *cursor += 1;

    //Makes sure the access register is a valid register
    checkRegisterValid(regToShift);

    //Converts the access register to its binary  identifier 
    std::string shiftRegIndentifier = registerNameMap.at(regToShift);

    //Adds the indentifier to the machine code
    generatedMachineCode += shiftRegIndentifier;

    if(opCode ==  "SHIFTL"){
        generatedMachineCode += "0000000000";
    }
    else{
        generatedMachineCode += "0100000000";
    }

    free(cursor);
    return generatedMachineCode;
}
/**
 * @brief Parses a line of code with the OpCpde JUMP
 * 
 * @param codeLine The line of code to parse
 * 
 * @return The machine code instruction of this line
 */
std::string parseJUMP(std::string asmCodeLine){
    std::string generatedMachineCode = "0000";
    
    //Gets the name of the location to jump to 
    std::string jumpAddress = readWord(asmCodeLine,0);

    //Gets the Imedval from the jump address 
    generatedMachineCode += getLineJumpImedVal(jumpAddress);

    return generatedMachineCode;
   
   
}
/**
 * @brief Parses a line of code with the OpCodes BRE, BRZ, BRNE, BRNZ, BRG, or BRGE
 * 
 * @param codeLine The line of code to parse
 * @param opCode This line of codes opCode
 * 
 * @return The machine code instruction of this line
 */
std::string parseBRANCH(std::string asmCodeLine, std::string opCode){
     std::string generatedMachineCode = "00";

    //Adds the next two bits depending on the OpCpde
     if(opCode == "BRE" || opCode == "BRZ"){
        generatedMachineCode += "00";
    }
    else if(opCode == "BRNE" || opCode == "BRNZ"){
         generatedMachineCode += "01";
    }
    else if(opCode == "BRG"){
        generatedMachineCode += "10";
    }
    else{
        generatedMachineCode += "11";
    }

    //Gets the name of the location to branch to 
    std::string jumpAddress = readWord(asmCodeLine,0);

    //Gets the Imedval from the branch address 
    generatedMachineCode += getLineJumpImedVal(jumpAddress);

    return generatedMachineCode;


}

/**
 * @brief Loops through the Assembly Program parses each line and generates its machine code
 * 
 */
void convertAsmCode(){
    //Creates a cursor to read through the code
    int* cursor = (int*) malloc(sizeof cursor);
    *cursor = 0;

    while(*cursor < asmCode.codeSec.length()){
        //Sets the line number
        currLineNum = codeSecLineNum + asmCode.lineNums.codeLineNum;

         //Makes sure the program isn't to long
        if(codeSecLineNum > 32){
            throwAssemblerError("The providec code is to long. The maximum length is 32 instructions.");
        }
    
        //Gets the current line of code 
        std::string currLine = readLine(asmCode.codeSec,cursor,1);

        //Gets a cursor to read through the line
        int* lineCursor = (int*) malloc(sizeof cursor);
        *lineCursor = 0;

        //Reads the OpCpde of the current line
        std::string opCode = readWord(currLine,lineCursor);
        *lineCursor +=1;

        if(opCodeMap.count(opCode) == 0){
            throwAssemblerError("The provided OpCpde " + opCode + " is not valid");
        }

        //Gets the current  line without the OpCpde to be sent to the parsing functions
        std::string lineToParse = currLine.substr(*lineCursor);
       
        //Creates the line of machine code and adds the OpCpde's binary identifier to it
        std::string machineCodeLine = opCodeMap.at(opCode);

        //Parses the code based on the OpCpde
        if(opCode == "NOOP"){
            machineCodeLine += parseNOOP();
        }
        else if(opCode == "INPUTC" || opCode == "INPUTCF" || opCode == "INPUTD" || opCode == "INPUTDF"){
            machineCodeLine += parseINPUT(lineToParse,opCode);
        }
        else if(opCode == "MOVE"){
            machineCodeLine += parseMOVE(lineToParse);
        }
        else if(opCode == "LOADI"){
            machineCodeLine += parseLOADI(lineToParse);
        }
        else if(opCode == "LOADP"){
            machineCodeLine += parseLOADP(lineToParse);
        }
        else if(opCode == "ADD" || opCode == "SUB" || opCode == "CMP"){
            machineCodeLine += parseADDxSUBxCMP(lineToParse);
        }
        else if(opCode == "ADDI" || opCode == "SUBI"){
            machineCodeLine += parseADDIxSUBI(lineToParse);
        }
        else if(opCode == "LOAD" || opCode == "LOADF"){
            machineCodeLine += parseLOAD(lineToParse,opCode);
        }
        else if(opCode == "STORE" || opCode == "STOREF"){
            machineCodeLine += parseSTORE(lineToParse,opCode);
        }
        else if(opCode == "SHIFT"){
            machineCodeLine += parseSHIFT(lineToParse,opCode);
        }
        else if(opCode == "JUMP"){
            machineCodeLine += parseJUMP(lineToParse);
        }
        else if(opCode == "BRE" || opCode == "BRZ" || opCode == "BRNE" || opCode == "BRNZ" || opCode == "BRG" || opCode == "BRGE" ){
            machineCodeLine += parseBRANCH(lineToParse,opCode);
        }
        //Increases the code line number by one
        codeSecLineNum += 1;

        //Adds the created code to the rest of the program
        machineCodeProgram += formatMachineCodeLine(machineCodeLine) + "\n";
        instructionList.push_back(formatMachineCodeLine(machineCodeLine));

        free(lineCursor);
        
    }

    free(cursor);
}

/**
 * @brief Formats the variables into a list of structs used by the output functions
 * 
 * @return A list of the information needed for outputting variables
 */
std::vector<usrVarOutput> formatVariablesOutput(){
    std::vector<usrVarOutput> outputVars;
    std::vector<std::string> varNamesOutput;

    //Does additonal formatting on any declared arrays and adds all variables to a new list
    int prevBracket = varNames[0].find('[');
    for(int i =1; i <varNames.size(); i++){
        int currBracket = varNames[i].find('[');

        if(prevBracket == std::string::npos && currBracket != std::string::npos){
            varNamesOutput.push_back(varNames[i-1] + "[0]");
        }
        else{
            varNamesOutput.push_back(varNames[i-1]);
        }

        prevBracket = currBracket;
    }
    varNamesOutput.push_back(varNames[varNames.size() -1 ]);

    //Stores the name and value into structs and then returns them as a list
    for(int i =0; i <varNames.size(); i++){
        usrVarOutput temp;
        temp.name = varNamesOutput[i];
        temp.val = usrVarMap[varNames[i]].val;
        varVals.push_back(usrVarMap[varNames[i]].val);
        outputVars.push_back(temp);
    }
    return outputVars;

}

/**
 * @brief  Formats the Branch locations into a list of structs used by the output functions
 * 
 * @return  A list of the information needed for outputting branch locations
 */
 std::vector<branchLocOutput> formatBranchLocsOutput(){
       std::vector<branchLocOutput> outputBranches;

    //Get a list of the keys (variable names)
    std::vector<std::string> keys;
    for (auto it = jumpAddressesMap.begin(); it != jumpAddressesMap.end(); it++) {
        keys.push_back(it->first);
    }

    //Stores the name and value into structs and then returns them as a list
    for(int i =0; i <keys.size(); i++){
        branchLocOutput temp;
        temp.name = keys[i];
        temp.loc = jumpAddressesMap[keys[i]];

        outputBranches.push_back(temp);
    }
    return outputBranches;

 }

/**
 * @brief Outputs the Assembled code to necessary Verilog and Bin files
 * 
 */
void outputCode(){
    std::vector<usrVarOutput> outputVars = formatVariablesOutput();
    std::vector<branchLocOutput> outputBranches = formatBranchLocsOutput();

    //Writes the users code to verilog files
    outputUserCode(instructionList,0,userFilePath,"User_Code_Low");
    outputUserCode(instructionList,16,userFilePath,"User_Code_High");

    //Writes the users data to a verilog files
    outputUserData(outputVars,userFilePath);

    //Writes the .bin file
    outputBinFile(rawCode,machineCodeProgram,programName,userFilePath,varVals,outputBranches);

    //Writes to the console
    outputToConsole(rawCode,machineCodeProgram,varVals,outputBranches);
}

/**
 * @brief Prompts the user for a file and handles the users input
 * 
 * @return The users inputed file path 
 */
std::string getUserFile(){
    //Asks the user for input and saves it to a variable
    std::cout << "Enter the file containing code to assemble: \n";
    std::string usrInput;
    std::cin >> usrInput;

    //Gets the location within the input of different parts of the filepath
    int nameStartLocation = -1;
    int extensionLocation = -1;
    for(int i = usrInput.length(); i >= 0; i -= 1){
        char curChar = usrInput[i];

        if(curChar == '.' && extensionLocation == -1){
            extensionLocation == i;
        }

        if(curChar == '/' && extensionLocation == -1 || curChar == '\\' && extensionLocation == -1){
            nameStartLocation == i +1;
        }
    }

    //Sets variables for the file path and name
    programName = usrInput.substr(nameStartLocation,extensionLocation-nameStartLocation);
    userFilePath = usrInput.substr(0,usrInput.length() - nameStartLocation); 

    return usrInput;

}

int main(){
    rawCode = readFromFile("AsmPrograms/SelectionSort/SelectionSort.asm");
    asmCode = parseCode(rawCode);
    readDataSec();
    setJumpAddreses();
    convertAsmCode();
    userFilePath = "";
    programName = "TestProgram";
    outputCode();

}
