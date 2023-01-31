#include <stdlib.h> 
#include<string>


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
void codeNotFoundError();

/**
 * @brief Reads through the contents of a file at the provided path and returns a string containing the contents of the file.
 * 
 * @param fileLoc A string containing the path to the file  to read from.
 * @return The contents of the files as a string.
 */
std::string readFromFile(std::string fileLoc);


/**
 * @brief Reads from the provided string until it hits a newline (\\n) characters or the end of the file. 
 * Adds the content it reads to the returned string.
 * 
 * @param readFrom The string to read a line from.
 * @param cursorLoc A pointer to the location in the read string to start reading from. The pointer will be incremented until the functions 
 * finds a new line or reaches the end of the string.
 * @return A string contianing the contents of readFrom from cursorLoc to a newline character (or the end of the file). 
 */
std::string readLine(std::string readFrom, int* cursorLoc);



/**
 * @brief Reads the next word from the provided string. Starts at the value of provided pointer and ends at a space, new line character, or the end of the provided string. 
 * Increments the value at the provided cursor to the index after the end of the word
 * 
 * @param readFrom The string to read a word fro,
 * @param currLoc A pointer to the location to start reading the word from. The pointer will be icremented to after the end of the word
 * @return The word read  from the string
 */
std::string readWord(std::string readFrom,int* currLoc);

/**
 * @brief Reads the next word from the provided string. Starts at the value of provided pointer and ends at a space, new line character, or the end of the provided string. 
 * Increments the value at the provided cursor to the index after the end of the word
 * 
 * @param readFrom The string to read a word fro,
 * @param currLoc The location within that string to start reading at
 * @return The word read  from the string
 */
std::string readWord(std::string readFrom,int currLoc);

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
std::string readLine(std::string readFrom, int* cursorLoc, int cutOff);

/**
 * @brief Formats addition and subtraction operations to the desired state by the assembler
 * 
 * @param toFormat The string to format addition and subtraction within
 * @return The string with guranteed spaceing between + and  - symbols 
 */
std::string formatAdditionAndSubtraction(std::string toFormat);

/**
 * @brief Takes in a string containing an i281 Assembly program. Removes the comments and unwanted whitespace from it.
 * 
 * @param asmCode The string to remove comments and white space from. 
 * @return The contents of asmCode with the comments and  whitespace removed.
 */
std::string removeWhiteSpaceAndComments(std::string asmCode);

/**
 * @brief Seperates the .data and .code sections of the program into there own strings stored together as a struct
 * 
 * @param readFrom The string to extract the parts from.
 * @return A struct storing the two parts of the code 
 */
partedCode seperateCodeAndData(std::string readFrom);

/**
 * @brief Moves the provided jump addresses (Formmated as Name:) to behind the rest of the code.(Fromamted as :Name)
 * 
 * @param asmCode The code to move the jump addresses within
 * @return The code with  the addresses moved
 */
std::string moveJumpAds(std::string asmCode);

/**
 * @brief Finds the line numbers of the .data and .code declarators returns them in a sectionLocs struct
 * 
 * @param asmCode The code to find the declarators within
 * @return The line numbers stored together in a struct 
 */
sectionLocs findCodeDataLoc(std::string asmCode);

/**
 * @brief Parses the code from a provided string. Returns the data and code sections in a struct with the comments and white removed and the jump addresses moved to the end.
 * 
 * @param asmCode A string containing the code to parse
 * @return A struct containing the parsed code 
 */
partedCode parseCode(std::string asmCode);

/**
 * @brief Finds the ope code of the given line
 * 
 * @param getFrom The line to get the ope code from 
 * @return The ope code
 */
std::string getOpeCode(std::string getFrom);


/**
 * @brief Reads the first set of brackets after the given index startLoc. Returns everything between the two brackets.
 * If the brackets are not closed returns BRACKET_WAS_NOT_CLOSED.
 * If no brackets are opened than returns NO_BRACKETS_IN_STRING
 * 
 * @param readFrom The string to read the bracket statement from
 * @param startLoc The index to start looking for brackets at
 * @return Everything within the set of brackets, BRACKET_WAS_NOT_CLOSED if they aren't closed, or NO_BRACKETS_IN_STRING if they aren't opened
 */
std::string parseBrackets(std::string readFrom, int startLoc);

/**
 * @brief Reads the first set of brackets after the given index stored at the pointer cursor. Returns everything between the two brackets. 
 * If the brackets are not closed returns BRACKET_WAS_NOT_CLOSED.
 * If no brackets are opened than returns NO_BRACKETS_IN_STRING.
 * Cursor will be set to the location after the closing bracket.
 * 
 * @param readFrom The string to read the bracket statement from
 * @param startLoc The index to start looking for brackets at
 * @return Everything within the set of brackets, BRACKET_WAS_NOT_CLOSED if they aren't closed, or NO_BRACKETS_IN_STRING if they aren't opened
 */
std::string parseBrackets(std::string readFrom, int* cursor);
