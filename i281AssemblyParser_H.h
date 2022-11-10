#include <stdlib.h> 
#include<string>
#include <iostream>
#include <fstream>



/**
 * @brief Stores the two parts of the code together but as seperate strings
 * 
 */
struct partedCode{
    std::string dataSec;
    std::string codeSec;
};

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
   free(cursorLoc);
   return movedString;
}

/**
 * @brief Parses the code from a provided file. Returns the data and code sections in a struct with the comments and white removed and the jump addresses moved to the end.
 * 
 * @param asmCode A string containing the code to parse
 * @return A struct containing the parsed code 
 */
partedCode parseCode(std::string asmCode){
    //std::string code = readFromFile(asmCode);
    asmCode =  removeWhiteSpaceAndComments(asmCode);
    partedCode codeInParts = seperateCodeAndData(asmCode);
    codeInParts.codeSec = moveJumpAds(codeInParts.codeSec);

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

