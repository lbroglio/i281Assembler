#include <stdlib.h> 
#include<string>
#include <iostream>
#include <fstream>


/**
 * @brief Stores the line numbers of the .code and .data declarators together
 * 
 */
struct sectionLocs {
    int dataLineNum;
    int codeLineNum;
};

/**
 * @brief Stores the two parts of the code together but as seperate strings along with the line number they are each at within the program as a sectionLoc struct.
 * 
 */
struct partedCode{
    std::string dataSec;
    std::string codeSec;
    sectionLocs lineNums;
    
};

/**
 * @brief Prints to the console that the program could not find the code section and ends the program.
 * 
 */
void codeNotFoundError(){
    std::cout << "ERROR: Program could not be found. Include .code to denote this";
    exit(1);
}

/**
 * @brief Reads through the contents of a file at the provided path and returns a string containing the contents of the file.
 * 
 * @param fileLoc A string containing the path to the file  to read from.
 * @return The contents of the files as a string.
 */
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


/**
 * @brief Reads from the provided string until it hits a newline (\\n) characters or the end of the file. 
 * Adds the content it reads to the returned string.
 * 
 * @param readFrom The string to read a line from.
 * @param cursorLoc A pointer to the location in the read string to start reading from. The pointer will be incremented until the functions 
 * finds a new line or reaches the end of the string.
 * @return A string contianing the contents of readFrom from cursorLoc to a newline character (or the end of the file). 
 */
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



/**
 * @brief Reads the next word from the provided string. Starts at the value of provided pointer and ends at a space, new line character, or the end of the provided string. 
 * Increments the value at the provided cursor to the index after the end of the word
 * 
 * @param readFrom The string to read a word fro,
 * @param currLoc A pointer to the location to start reading the word from. The pointer will be icremented to after the end of the word
 * @return The word read  from the string
 */
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

/**
 * @brief Reads the next word from the provided string. Starts at the value of provided pointer and ends at a space, new line character, or the end of the provided string. 
 * Increments the value at the provided cursor to the index after the end of the word
 * 
 * @param readFrom The string to read a word fro,
 * @param currLoc The location within that string to start reading at
 * @return The word read  from the string
 */
std::string readWord(std::string readFrom,int currLoc){
    int* temp = (int*) malloc(sizeof temp);
    *temp = currLoc;

    return readWord(readFrom,temp);

    free(temp);
}

/**
 * @brief Reads from the provided string until it hits a newline (\\n) characters or the end of the file. 
 * Adds the content it reads to the returned string.
 * 
 * @param readFrom The string to read a line from.
 * @param cursorLoc A pointer to the location in the read string to start reading from. The pointer will be incremented until the functions 
 * finds a new line or reaches the end of the string.
 * @param cutOff The number of characters from the end of the string to stop reading (ie If curtOff == 1 than the last character will not be read)
 * @return A string contianing the contents of readFrom from cursorLoc to a newline character (or the end of the file). 
 */
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

/**
 * @brief Takes in a string containing an i281 Assembly program. Removes the comments and unwanted whitespace from it.
 * 
 * @param asmCode The string to remove comments and white space from. 
 * @return The contents of asmCode with the comments and  whitespace removed.
 */
std::string removeWhiteSpaceAndComments(std::string asmCode){
    int* cursorLoc = (int*) malloc(sizeof *cursorLoc);
    *cursorLoc = 0;

    std::string withoutComments = "";


    //Loops until the cursor is  at the end of the provided string
    while((*cursorLoc) < asmCode.length()){

       std::string currentLine = readLine(asmCode,cursorLoc);
       

        //Checks if the line is a blank line
       if(currentLine == "\n"){
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


/**
 * @brief Seperates the .data and .code sections of the program into there own strings stored together as a struct
 * 
 * @param readFrom The string to extract the parts from.
 * @return A struct storing the two parts of the code 
 */
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

/**
 * @brief Moves the provided jump addresses (Formmated as Name:) to behind the rest of the code.(Fromamted as :Name)
 * 
 * @param asmCode The code to move the jump addresses within
 * @return The code with  the addresses moved
 */
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


/**
 * @brief Finds the line numbers of the .data and .code declarators returns them in a sectionLocs struct
 * 
 * @param asmCode The code to find the declarators within
 * @return The line numbers stored together in a struct 
 */
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

/**
 * @brief Parses the code from a provided string. Returns the data and code sections in a struct with the comments and white removed and the jump addresses moved to the end.
 * 
 * @param asmCode A string containing the code to parse
 * @return A struct containing the parsed code 
 */
partedCode parseCode(std::string asmCode){
    //std::string code = readFromFile(asmCode);
    sectionLocs decLocs = findCodeDataLoc(asmCode);
    asmCode = formatAdditionAndSubtraction(asmCode);
    asmCode =  removeWhiteSpaceAndComments(asmCode);
    partedCode codeInParts = seperateCodeAndData(asmCode);
    codeInParts.codeSec = moveJumpAds(codeInParts.codeSec);
    codeInParts.lineNums.dataLineNum = decLocs.dataLineNum;
    codeInParts.lineNums.codeLineNum = decLocs.codeLineNum;

    return codeInParts;
}

/**
 * @brief Finds the ope code of the given line
 * 
 * @param getFrom The line to get the ope code from 
 * @return The ope code
 */
std::string getOpeCode(std::string getFrom){
    int endLoc = getFrom.find(' ');
    return getFrom.substr(0,endLoc);
}


/**
 * @brief Reads the first set of brackets after the given index startLoc. Returns everything between the two brackets.
 * If the brackets are not closed returns BRACKET_WAS_NOT_CLOSED
 * 
 * @param readFrom The string to read the bracket statement from
 * @param startLoc The index to start looking for brackets at
 * @return Everything within the set of brackets or BRACKET_WAS_NOT_CLOSED if they aren't closed 
 */
std::string parseBrackets(std::string readFrom, int startLoc){
    int curLoc = startLoc;
    char curChar =  readFrom[curLoc];
    std::string bracketContents = "";


    while(curChar != '['){
        curLoc++;
        curChar= readFrom[curLoc];
    }
    curLoc ++;
    curChar =  readFrom[curLoc];

    while(curChar != ']' && curLoc < readFrom.length()){
        bracketContents += curChar;
        curLoc++;
        curChar =  readFrom[curLoc];
        
    }

    if(curLoc == readFrom.length() && curChar != ']'){
        return "BRACKET_WAS_NOT_CLOSED";
    }
    else{
        return bracketContents;
    }

}
int main(){
    std::string test  = readFromFile("TestProgram.txt");
    partedCode code = parseCode(test);
    std::cout << code.codeSec;
}