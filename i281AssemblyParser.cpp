#include <stdlib.h> 
#include<string>
#include <iostream>
#include <fstream>
#include"i281AssemblyParser.hpp"


void codeNotFoundError(){
    std::cout << "ERROR: Program could not be found. Include .code to denote this";
    exit(1);
}

std::string readFromFile(std::string fileLoc){
    std::string contents;
    std::ifstream readFrom(fileLoc);

    while(readFrom){
         char temp = readFrom.get();
         contents += temp;
    }
   
    readFrom.close();
    return contents;
}

std::string readLine(std::string readFrom, int* cursorLoc){

    bool hitNewLine = false;
    bool foundComment = false;
    std::string line = "";

    while(hitNewLine == false){
        char charAt = readFrom[*cursorLoc];
        if(*cursorLoc == readFrom.length()-1 || charAt == '\n'){
            hitNewLine = true;
        }
        line += readFrom[*cursorLoc];
        (*cursorLoc)++;
    }

    return line;

}

std::string readWord(std::string readFrom,int* currLoc){
    std::string removeBegining = readFrom.substr(*currLoc);
    int spaceLoc = removeBegining.find(' ');

    std::string foundWord = "";
    char currChar = readFrom[*currLoc];

    while(currChar != ' ' && currChar != ',' && currChar != '\n' && *currLoc < readFrom.length()){
        foundWord += currChar;
        (*currLoc)++;
        currChar =  readFrom[*currLoc];
    }

    return foundWord;
}

std::string readWord(std::string readFrom,int currLoc){
    int* temp = (int*) malloc(sizeof temp);
    *temp = currLoc;

    return readWord(readFrom,temp);

    free(temp);
}

std::string readLine(std::string readFrom, int* cursorLoc, int cutOff){
    bool hitNewLine = false;
    bool foundComment = false;
    std::string line = "";

    while(hitNewLine == false){
        char charAt = readFrom[*cursorLoc];
        if(*cursorLoc == readFrom.length()-1 || charAt == '\n'){
            hitNewLine = true;
        }
        line += readFrom[*cursorLoc];

        (*cursorLoc)++;
    }
    line = line.substr(0,line.length()-cutOff);
    return line;

}

std::string formatAdditionAndSubtraction(std::string toFormat){
    //Replaces all '+' chars with " + "
    int currLoc = toFormat.find('+');
    while(currLoc != std::string::npos){
        toFormat.replace(currLoc,1," + ");
        currLoc = toFormat.find('+',currLoc+2);
    }
    //Replaces all '-' chars with " - "
    currLoc = toFormat.find('-');
    while(currLoc != std::string::npos){
        toFormat.replace(currLoc,1," - ");
        currLoc = toFormat.find('-',currLoc+2);
    }

    return toFormat;
}

std::string removeWhiteSpaceAndComments(std::string asmCode){
    int* cursorLoc = (int*) malloc(sizeof *cursorLoc);
    *cursorLoc = 0;

    std::string withoutComments = "";


    //Loops until the cursor is  at the end of the provided string
    while((*cursorLoc) < asmCode.length()){

       std::string currentLine = readLine(asmCode,cursorLoc);
       

        //Checks if the line is a blank line
       if(currentLine[0] == '\n' || currentLine.length() < 2){
        continue;
       }
       //Checks if the line is entirely a comment
       else if(currentLine[0] ==  ';'){
        continue;
       }
       else{
        bool betweenChars =  false;

        //Goes through the line and adds only desired characters to the new string 
        for(int i=0; i<currentLine.length() -1; i++){
            char curChar = currentLine[i];
            if(curChar == ';'){
                i += asmCode.length();
            }
            else if (curChar == ',' || curChar == '['){
               betweenChars = false;
               withoutComments+=curChar;
            }
            else if(curChar != ' ' ){
                betweenChars = true;
                withoutComments+=curChar;
            }
            else if(curChar == ' ' && betweenChars == true){
                withoutComments+=curChar;
                betweenChars = false;
            }
        }
        withoutComments += '\n';
       }
    }
    

    std::free(cursorLoc);
    return withoutComments;

}

partedCode seperateCodeAndData(std::string readFrom){
   int dataBeginMarker = 0;
   int codeBeginMarker = 0;
   int lengthOfData;
   int* cursorLoc = (int*) malloc(sizeof *cursorLoc);
   *cursorLoc =0; 

   std::string currLine = readLine(readFrom , cursorLoc);
   dataBeginMarker = *cursorLoc;
    

   //Gets the postion where the code section begins
   while(currLine != ".code\n"){
    currLine = readLine(readFrom, cursorLoc);
   }

   //Calculates the number of characters in the data section
   lengthOfData = (*cursorLoc) - dataBeginMarker - 6;

   codeBeginMarker = *cursorLoc;
   
   partedCode toReturn;

   //Sets the two sections 
   toReturn.dataSec = readFrom.substr(dataBeginMarker,lengthOfData);
   toReturn.codeSec = readFrom.substr(codeBeginMarker);

   return toReturn;
}

std::string moveJumpAds(std::string asmCode){
    std::string movedString ="";
    int* cursorLoc = (int*) malloc(sizeof *cursorLoc);
   *cursorLoc =0; 

   while(*cursorLoc < asmCode.length()){
    std::string currLine =  readLine(asmCode,cursorLoc);
    int adLoc = currLine.find(':');

    if(adLoc == std::string::npos){
        movedString += currLine ;
    }
    else{
        movedString += currLine.substr(adLoc+2,currLine.length() -3 - adLoc);
        movedString += " :" + currLine.substr(0,adLoc) + '\n';
        
    }
   }
   return movedString;
}

sectionLocs findCodeDataLoc(std::string asmCode){
    bool codeFound =  false;
    int dataLoc = -1;
    int codeLoc;
    int currLineNum = 1;

    int* cursor = (int*) malloc(sizeof  cursor);
    *cursor =0; 


    std::string currLine = readLine(asmCode,cursor,1);
    while(codeFound == false && *cursor < asmCode.length()){
        if(currLine == ".data"){
            dataLoc = currLineNum;
        }
        else if(currLine == ".code"){
            codeLoc = currLineNum;
            codeFound = true;
        }
         currLine = readLine(asmCode,cursor,1);
         currLineNum++;
    }
    if(*cursor >= asmCode.length()){
        codeNotFoundError();
    }

    sectionLocs toReturn;
    toReturn.codeLineNum = codeLoc;
    toReturn.dataLineNum = dataLoc;

    free(cursor);
    return toReturn;
}

partedCode parseCode(std::string asmCode){
    sectionLocs decLocs = findCodeDataLoc(asmCode);
    //This is being refactored to a different section of the program (Formatted within brackets themselves) but is being kept here as an option 
    //asmCode = formatAdditionAndSubtraction(asmCode);
    asmCode =  removeWhiteSpaceAndComments(asmCode);
    partedCode codeInParts = seperateCodeAndData(asmCode);
    codeInParts.codeSec = moveJumpAds(codeInParts.codeSec);
    codeInParts.lineNums.dataLineNum = decLocs.dataLineNum;
    codeInParts.lineNums.codeLineNum = decLocs.codeLineNum;

    return codeInParts;
}

std::string getOpeCode(std::string getFrom){
    int endLoc = getFrom.find(' ');
    return getFrom.substr(0,endLoc);
}

std::string parseBrackets(std::string readFrom, int startLoc){
    //Makes sure  + and - signs are in the desired way
    readFrom = formatAdditionAndSubtraction(readFrom);
    readFrom = removeWhiteSpaceAndComments(readFrom);

    int curLoc = startLoc;
    char curChar =  readFrom[curLoc];
    std::string bracketContents = "";


    while(curChar != '[' && curLoc != readFrom.length() -1 && curChar != '{'){
        curLoc++;
        curChar= readFrom[curLoc];
    }
    if(curLoc == readFrom.length() - 1){
        return "NO_BRACKETS_IN_STRING";
    }
    curLoc ++;
    curChar =  readFrom[curLoc];

    while(curChar != ']' && curLoc < readFrom.length() && curChar != '}'){
        bracketContents += curChar;
        curLoc++;
        curChar =  readFrom[curLoc];
        
    }

    if(curLoc == readFrom.length() && curChar != ']' && curChar != '}'){
        return "BRACKET_WAS_NOT_CLOSED";
    }
    else{
        return bracketContents;
    }

}

std::string parseBrackets(std::string readFrom, int* cursor){
    //Makes sure  + and - signs are in the desired way
    readFrom = formatAdditionAndSubtraction(readFrom);
    readFrom = removeWhiteSpaceAndComments(readFrom);

    char curChar =  readFrom[*cursor];
    std::string bracketContents = "";


    while(curChar != '[' && *cursor != readFrom.length() -1 && curChar != '{'){
        *cursor += 1;
        curChar= readFrom[*cursor];
    }
    if(*cursor == readFrom.length() - 1){
        return "NO_BRACKETS_IN_STRING";
    }
    *cursor += 1;
    curChar =  readFrom[*cursor];

    while(curChar != ']' && *cursor < readFrom.length() && curChar != '}'){
        bracketContents += curChar;
        *cursor += 1;
        curChar =  readFrom[*cursor];
        
    }

    if(*cursor == readFrom.length() && curChar != ']' && curChar != '}'){
        return "BRACKET_WAS_NOT_CLOSED";
    }
    else{
        *cursor += 1;
        return bracketContents;
    }

}
