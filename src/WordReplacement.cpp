#include <iostream>
#include <Windows.h>
#include <WinUser.h>
#include <string>
#include <unordered_map>
#include <set>
#include <fstream>
#include <cctype>
#include <vector>
#include <thread>

using std::cout;
using std::endl;
using std::string;
using std::fstream;
using std::unordered_map;
using std::vector;


//IMPORTANT MACROS FOR WINDOWS
#define SCAN_CODE_A 0x41
#define SCAN_CODE_B 0x42
#define SCAN_CODE_C 0x43
#define SCAN_CODE_D 0x44
#define SCAN_CODE_E 0x45
#define SCAN_CODE_F 0x46
#define SCAN_CODE_G 0x47
#define SCAN_CODE_H 0x48
#define SCAN_CODE_I 0x49
#define SCAN_CODE_J 0x4A
#define SCAN_CODE_K 0x4B
#define SCAN_CODE_L 0x4C
#define SCAN_CODE_M 0x4D
#define SCAN_CODE_N 0x4E
#define SCAN_CODE_O 0x4F
#define SCAN_CODE_P 0x50
#define SCAN_CODE_Q 0x51
#define SCAN_CODE_R 0x52
#define SCAN_CODE_S 0x53
#define SCAN_CODE_T 0x54
#define SCAN_CODE_U 0x55
#define SCAN_CODE_V 0x56
#define SCAN_CODE_W 0x57
#define SCAN_CODE_X 0x58
#define SCAN_CODE_Y 0x59
#define SCAN_CODE_Z 0x5A
// different naming conventions so I renamed them for this use case
#define SCAN_CODE_FORWARD_SLASH VK_OEM_2
#define SCAN_CODE_COLON VK_OEM_1
#define SCAN_CODE_COMMA VK_OEM_COMMA
#define SCAN_CODE_MINUS VK_OEM_MINUS
#define SCAN_CODE_PERIOD VK_OEM_PERIOD
#define SCAN_CODE_OPEN_BRACKET VK_OEM_4
#define SCAN_CODE_CLOSE_BRACKET VK_OEM_6
#define SCAN_CODE_QUOTE VK_OEM_7


// utility functions
// ------------
// gets size of char* string
unsigned int getSizeOfChar(char* characters)
{
    unsigned int iterator = 0;
    while(characters[iterator] != '\0')
    {
        iterator++;
    }
    return iterator;
}

// appends <string> to the a <char*> 
// it's ensured that this will never result in overflow 
// because of program flow
void appendStrToChar(char* characters, string strToAppend)
{
    unsigned int lengthOffset = getSizeOfChar(characters);
    unsigned int sizeOfString = strToAppend.size();
    for(unsigned int i=lengthOffset; i< lengthOffset + sizeOfString; i++)
    {
        characters[i] = strToAppend[i-lengthOffset];
    }

    characters[lengthOffset + sizeOfString] = ' ';
}

// returns a copy of the string that's lowercase using tolower(char)
string toLower(string lower)
{
    int lengthOfString = lower.length();
    for(int i=0; i<lengthOfString; i++)
    {
        lower[i] = tolower(lower[i]);
    }
    return lower;
}

// returns a copy of the string that's uppercase using toupper(char)
string toUpper(string lower)
{
    int lengthOfString = lower.length();
    for(int i=0; i<lengthOfString; i++)
    {
        lower[i] = toupper(lower[i]);
    }
    return lower;
}
// --------------
// end of utility functions

// class for keeping track of the buffer
// class declarations
class Recorder
{
private:
    unordered_map<string, string> _wordPairs; // stores Constitution pre->post change
    string _L1buffer; // keeps track of current word getting typed 
    string _L2buffer; // keeps track of words that are presumed to be on the screen, and fair game to be used
    int _numberOfWordOnScreen; //negative is error state (reserved for not in use)
    unsigned int _maxSizeOfArray; // stores max size of possible output according to "post" in 'Constitution.csv'
    INPUT* _inputArray = nullptr; // dynamically allocated based on _maxSizeOfArray
    bool _moddingWord;
    vector<unordered_map<string, char>> _censorWord;
    //dictionary has 3 outputs
    //output0: 's' -> stop (last occurence of word in phrase)
    //  in implementation we'll reset the counter to 0
    //output1: 'm' -> middle (increment counter, don't mod)
    //  in implementation we'll increment 
    //output2: 'b' -> both (increment counter and mod)
    //  in implementation we'll increment

    char* _prevCensoredWords = nullptr; //this will keep track of word1, word2, etc
    unsigned int _largestCensor; //keeps track of size of prevCensoredWords
    //only gets used onto _wordsPairs when mapping censor->mod
    //and gets updated when a new word gets matched
    unsigned int _wordCount; //tracks current censored word in phrase
public:
    //constructors
    Recorder(); //creates class

    //methods
    //getters
    bool isNotModding();
    //logic (deciders)
    void onPress(char); //action on key press
    void onRelease(char); //action on key release
    void evaluate(); //decide if/what to type
    //actions
    void deleteWord(); //delete what should be on the screen (ordering to buffer)
    void typeWord(string extension = " "); //whatever is translated from buffer
    void handleBackspace(); //remove last char from buffer


    //destructor
    ~Recorder();
private:
    //helper functions to modularizing the code
    void type(string word); // types the word
    unsigned int importCSV(unsigned int&); // handles preprocessing of importing from 'Constitution.csv' and setting up word-analysis 
    void setupInputArray(); // sets up array that sends output 
    void setupStorageString(unsigned int size); // sets up char* that stores previous words when applicable
    void handleImpactedBuffer(string word); // 
};
//end of forward declaratins

//setup the specifications
//NOTE: we preset the type to keyboard and place temp vals  
void Recorder::setupInputArray()
{
    for(int i=0; i < (int)(this->_maxSizeOfArray); i++)
    {
        this->_inputArray[i].type = INPUT_KEYBOARD;
    }
}

// sets up char* to be empty 
void Recorder::setupStorageString(unsigned int size)
{
    for(unsigned int i=0; i<size;i++)
    {
        this->_prevCensoredWords[i] = '\0';
    }
}

//class definitions
// NOTE: word_pairs gets initialized inside of Recorder::importCSV()
Recorder::Recorder()
{
    this->_largestCensor = 0; // largest possible pre-phrase 
    this->_L1buffer = ""; // resets buffer
    this->_L2buffer = ""; // resets buffer
    this->_moddingWord = false; 
    this->_maxSizeOfArray = this->importCSV(this->_largestCensor); //largestCensor passed by reference
    this->_inputArray = new INPUT[this->_maxSizeOfArray]; // dynamically allocates memory for input characters
    this->_prevCensoredWords = new char[this->_largestCensor + 1]; //  dynamically allocates memory for possible previous words (in the context of word state position)
    this->setupStorageString(this->_largestCensor+1); // initializes all characters to terminating string '\0'
    this->_wordCount = 0; 
    this->_numberOfWordOnScreen = 0; 
}

// returns true if word is currently not being typed
// important in case a form of multitasking occurs, 
// however, in this implementation it won't be of issue
bool Recorder::isNotModding()
{
    return !(this->_moddingWord);
}

// handles different types of keypresses
// 1. space: calls evaluate()
// 2. backspace: calls handleBackspace()
// 3. enter: reset buffer (presumed loss of buffer data) 
// 4. regular char: append character to L1 buffer 
void Recorder::onPress(char letter)
{
    if(letter == ' ') //space 
    {
        this->evaluate();
    }
    // TODO: handle for ctrl+backspace (removes entire word)
    else if(letter == (char)1) // backspace
    {
        this->handleBackspace();
    }
    // TODO: handle for eval to happen upon enter (like in situations where modding a keybind can be done in game)
    else if(letter == (char)2) // enter
    {
        // later have it evaluate when we figure out how to change the chat bind in LoL
        this->_L1buffer = "";
        this->_L2buffer = "";
    }
    else
    {
        this->_L1buffer += letter; // append char to L1 buffer
    }
}

// evaluates whether or not L1buffer should result in a replaced phrase
// if word is apart of a phrase but not the end then it's stored in 'this->_prevCensoredWords'
// if word is apart of the end of a phrase then it gets typed  
void Recorder::evaluate()
{
    string phrase = this->_prevCensoredWords + this->_L1buffer;
    bool L1bufferTyped = false;
    // if word in its respective state (word1, word2, etc), then it should be typed
    if((this->_censorWord[_wordCount][_L1buffer] == 'b' || this->_censorWord[_wordCount][_L1buffer] == 's') && this->_wordPairs.find(phrase) != this->_wordPairs.end())
    {
        L1bufferTyped = true; // L1buffer results in typed word
        phrase = this->_wordPairs[phrase]; //set phrase from old phrase to new phrase
        this->deleteWord(); //deletes whatever the buffer has stored
        this->typeWord(phrase + " "); //types the appropriate word
    }

    // if the word is not the end 
    if(this->_censorWord[_wordCount][_L1buffer] == 'b' || this->_censorWord[_wordCount][_L1buffer] == 'm') //store and add word count
    {
        if(this->_prevCensoredWords[0] == '\0') // if first word in phrase
        {
            appendStrToChar(this->_prevCensoredWords, this->_L1buffer); //cats L1buffer to prev censored words

            // if the index isn't already set then set it to 0, since it would normally get set in Recorder::typeWord(string)
            // NOTE: this is redudant step because it should be updated (X>0) in Recorder::typeWord(string)
            if(this->_numberOfWordOnScreen == -1)
            {
                this->_numberOfWordOnScreen = 0;
            }
        }
        else
        {
            appendStrToChar(this->_prevCensoredWords, this->_L1buffer);
        }
        _wordCount++; //increment word count 
        // iff L1 buffer didn't get typed then add to L2 buffer 
        // if L1 buffer gets typed then it would be handled in Recorder::typeWord(string) 
        // where the <string> gets appended to L2buffer because that's what is on the screen
        if(!L1bufferTyped)
        {
            this->_L2buffer += " " + this->_L1buffer;
        }
    }
    // L2 gets appended by L1
    else
    {
        this->setupStorageString(getSizeOfChar(this->_prevCensoredWords)); // reset words
        this->_wordCount = 0; // reset word count since that word is not associated with the middle of a phrase
        this->_numberOfWordOnScreen = -1; // reset index
    }
    
    this->_L1buffer = ""; // reset L1buffer
}

// handles deleting the words on the screen to make appropiate room for the word to be typed
// if words (written by program, not user) from previous phrase is on screen then delete that
// uses single backspaces instead of 'ctrl+backspace' for applications where 'ctrl+backspace' won't work
void Recorder::deleteWord()
{
    //backspace the current buffer (whatever is on the screen, not what's in the buffer)
    int length;
    //new code
    length = this->_L1buffer.length(); 
    if(this->_numberOfWordOnScreen >= 0 && (int)(this->_L2buffer.size()) >= this->_numberOfWordOnScreen)
    {
        //difference between first character and starting index of prev word to get start of modded phrase
        length += (int)(this->_L2buffer.size()) - this->_numberOfWordOnScreen + 1; //+1 for the space between prevWords and L1buffer
    }
    
    INPUT input = { 0 };
    for(int i=0; i<length+1; i++)
    {
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_BACK;
        SendInput(1, &input, sizeof(input));
        Sleep(1);
    }
    GetAsyncKeyState(VK_BACK); //resets function call so the program doesn't think this is done by the user
}


// handles byproduct of Recorder::typeWord(string) that results in 
// a set of those key presses to be logged in the system
// this method removes those logs from the system
// removes the possibility of a nested trail as well as unnecesary overhead
void Recorder::handleImpactedBuffer(string word)
{
    int size = (int)word.length();
    std::set<char> lettersForImpactedBuffer;
    word = toLower(word); //standardize the word to lowercase
    for(int i=0; i<size; i++)
    {
        if(lettersForImpactedBuffer.insert(word[i]).second) // returns true if insertion is successful
        {
            GetAsyncKeyState(VkKeyScan(word[i])); //does call
        }
    }
}

// handles all that's related to typing words
// calls Recorder::type(string) which types to screen
// handles number of words on screen as a result of Recorder::type(string)
// updates L2buffer 
void Recorder::typeWord(string extension)
{
    this->_moddingWord = true;
    type(extension);
    this->_moddingWord = false;
    //appends L1 (extension) to L2 as well as replaces any modded words 
    if(this->_numberOfWordOnScreen > 0)
    {
        this->_L2buffer = this->_L2buffer.substr(0,this->_numberOfWordOnScreen) + " "+ extension;//stays the same
    }
    else
    {
        this->_numberOfWordOnScreen = this->_L2buffer.length(); //L2buffer size is extension's first char, which is the new output 
        //this->_L2buffer = this->_L2buffer + extension;
        this->_L2buffer += " " + extension;
    }
    handleImpactedBuffer(extension);
}

// handles effects of backspace on buffer states
// will remove from L1 when L1 is not empty
// will remove from L2 if L1 is empty
// won't do anything if all buffers are empty
void Recorder::handleBackspace()
{
    int L1bufferLength = this->_L1buffer.length();
    int L2BufferLength = this->_L2buffer.length();
    if(L1bufferLength > 0)
    {
        this->_L1buffer = this->_L1buffer.substr(0, L1bufferLength-1); //deletes the last char from L1
    }
    else if(L2BufferLength > 0)
    {
        this->_prevCensoredWords[0] = '\0';
        this->_L2buffer = this->_L2buffer.substr(0, L2BufferLength-1); //deletes the last char from L2
        if(this->_numberOfWordOnScreen > L2BufferLength)
        {
            this->_numberOfWordOnScreen = -1;
        }
    }
}

// types word using SendInput(size, address, sizeof(type))
// attempts to minimize calls to SendInput()
// SendInput will only press adjacent characters if they're unique 
// ex: SendInput("abc") prints "abc", however SendInput("aab") will print "ab"  
void Recorder::type(string word) 
{
    UINT sizeOfWord = word.length(); 
    UINT justTyped = 0;
    UINT sizeOfInput = 0;
    for(UINT i=0; i<sizeOfWord-1; i++)
    {
        this->_inputArray[i].type = INPUT_KEYBOARD;
        this->_inputArray[i].ki.wVk = VkKeyScan(word[i]);
        //this is for repeating letters because windows is bad at sending repeated inputs
        if(word[i] == word[i+1]) // are adjacent character non-unique
        {
            sizeOfInput = i-justTyped+1;
            SendInput( sizeOfInput, (this->_inputArray + justTyped), sizeof(this->_inputArray[0]));
            justTyped = i+1;
        }
    }
    //final letter
    this->_inputArray[sizeOfWord-1].type = INPUT_KEYBOARD;
    this->_inputArray[sizeOfWord-1].ki.wVk = VkKeyScan(word[sizeOfWord-1]); 
    
    sizeOfInput = sizeOfWord-justTyped;
    
    //this also accounts for the offset
    SendInput( sizeOfInput, (this->_inputArray + justTyped), sizeof(this->_inputArray[0]));
}


// imports 'Constitution.csv' and sets up map of input phrase to output phrase
// if a word is typed the buffers and states react to the given word according to state arrays mapping position states
// state arrays with maps to corresponding possible position states, for example:
// Suppose a word is located at a state. It contains the possible position states {'s', 'm', 'b'} 
//  's' -> stop (last occurence of word in phrase)
//    in implementation we'll reset the counter to 0
//  'm' -> middle (increment counter, don't mod)
//    in implementation we'll increment 
//  'b' -> both (increment counter and mod)
//    in implementation we'll increment 
// largestCensor keeps track of size of every pre-phrase 
unsigned int Recorder::importCSV(unsigned int& largestCensor)
{
    fstream fi;
    unsigned int count = 0; // keeps track of size of every pre-phrase
    fi.open("Constitution.csv", std::ios::in); //csv is ready for input
    int lengthOfLine;
    string line;
    unsigned int tempCount = 0;
    unsigned int tempLargestCensor; 

    int firstLetter;
    int lastLetter;
    int wordPosition;
    while(getline(fi, line))
    {
        lengthOfLine = line.length();
        firstLetter = 0;
        lastLetter = 0;   
        wordPosition = 0; 
        //this for-loop ends after 'i = lengthOfLine;' this line gets ran
        //which is after the censored word/phrase has been fully parsed
        for(int i=0; i<lengthOfLine; i++)
        {
            //if char is a space, then there is a word
            if(line.substr(i,1) == " ") 
            {
                lastLetter = i;
                //if the word exists and it's not 'm' then we need to make it BOTH
                //if it doesn't exist then we make it 'm'
                //this is so if it's equal to 's' or 'b' then it'll be set to 'b' 
                //if it doesn't exist or is already 'm' then it's still 'm'
                //check if this is the longest phrase

                // if word state is higher than current highest word state (1st position in state is always unique) 
                if(wordPosition > (int)(_censorWord.size() - 1))
                {
                    this->_censorWord.push_back(unordered_map<string, char> ()); // declare new map at index
                    this->_censorWord[wordPosition][toUpper(line.substr(firstLetter,lastLetter-firstLetter))] = 'm'; //MIDDLE
                }
                // if word state position is not unique
                else if(this->_censorWord[wordPosition].find(toUpper(line.substr(firstLetter,lastLetter-firstLetter))) != this->_censorWord[wordPosition].end() && this->_censorWord[wordPosition][line.substr(firstLetter,lastLetter-firstLetter)] != 'm')
                {
                    this->_censorWord[wordPosition][toUpper(line.substr(firstLetter,lastLetter-firstLetter))] = 'b'; //STOP AND MIDDLE
                }
                // if word state position is unique
                else 
                {
                    this->_censorWord[wordPosition][toUpper(line.substr(firstLetter,lastLetter-firstLetter))] = 'm'; //MIDDLE
                }
                wordPosition++; //increment wordPosition
                firstLetter = i+1;
            }

            //is true after censor is fully parsed
            if(line.substr(i,1) == ";")
            {
                lastLetter = i;
                //check if this is the longest phrase
                if((int)wordPosition > (int)(_censorWord.size() - 1))
                {
                    this->_censorWord.push_back(unordered_map<string, char> ());
                    this->_censorWord[wordPosition][toUpper(line.substr(firstLetter,lastLetter-firstLetter))] = 's'; //STOP
                }
                else if(this->_censorWord[wordPosition].find(line.substr(firstLetter,lastLetter-firstLetter)) != this->_censorWord[wordPosition].end() && this->_censorWord[wordPosition][line.substr(firstLetter,lastLetter-firstLetter)] != 'm')
                {
                    this->_censorWord[wordPosition][toUpper(line.substr(firstLetter,lastLetter-firstLetter))] = 'b'; //STOP AND MIDDLE
                }
                else //doesn't exist yet
                {
                    this->_censorWord[wordPosition][toUpper(line.substr(firstLetter,lastLetter-firstLetter))] = 's'; //STOP/END
                }

                tempLargestCensor = i; // update sizes
                tempCount = lengthOfLine-i; // update sizes
                this->_wordPairs[toUpper(line.substr(0, tempLargestCensor))] = line.substr(i+1, tempCount); // phrase to map

                // increment loop, move onto next pair of phrases
                i = lengthOfLine; 
            }
        }

        if(tempLargestCensor > largestCensor)
        {
            largestCensor = tempLargestCensor; // tracks largest pre-phrase
        }
        if(tempCount > count)
        {
            count = tempCount; // tracks largest post-phrase
        }
    }

    return count; //returns mod-count
}

// deallocates memory
Recorder::~Recorder()
{
    if(this->_inputArray)
    {
        delete[] this->_inputArray;
    }
    if(this->_prevCensoredWords)
    {
        delete[] this->_prevCensoredWords;
    }
}
// end of class implementations

// checks for first part of alphabet
void alpha1(char& output)
{
    output = (char)0;
    if((GetAsyncKeyState(SCAN_CODE_A) & 0x01))
    {
        output = 'A';
    }
    else if((GetAsyncKeyState(SCAN_CODE_B) & 0x01))
    {
        output = 'B';
    }
    else if((GetAsyncKeyState(SCAN_CODE_C) & 0x01))
    {
        output = 'C';
    }
    else if((GetAsyncKeyState(SCAN_CODE_D) & 0x01))
    {
        output = 'D';
    }
    else if((GetAsyncKeyState(SCAN_CODE_E) & 0x01))
    {
        output = 'E';
    }
    else if((GetAsyncKeyState(SCAN_CODE_F) & 0x01))
    {
        output = 'F';
    }
    else if((GetAsyncKeyState(SCAN_CODE_G) & 0x01))
    {
        output = 'G';
    }
    else if((GetAsyncKeyState(SCAN_CODE_H) & 0x01))
    {
        output = 'H';
    }
    else if((GetAsyncKeyState(SCAN_CODE_I) & 0x01))
    {
        output = 'I';
    }
    else if((GetAsyncKeyState(SCAN_CODE_J) & 0x01))
    {
        output = 'J';
    }
    else if((GetAsyncKeyState(SCAN_CODE_K) & 0x01))
    {
        output = 'K';
    }
    else if((GetAsyncKeyState(SCAN_CODE_L) & 0x01))
    {
        output = 'L';
    }
    else if((GetAsyncKeyState(SCAN_CODE_M) & 0x01))
    {
        output = 'M';
    }
}

// checks for second part of alphabet
void alpha2(char& output)
{
    output = (char)0;
    if((GetAsyncKeyState(SCAN_CODE_N) & 0x01))
    {
        output = 'N';
    }
    else if((GetAsyncKeyState(SCAN_CODE_O) & 0x01))
    {
        output = 'O';
    }
    else if((GetAsyncKeyState(SCAN_CODE_P) & 0x01))
    {
        output = 'P';
    }
    else if((GetAsyncKeyState(SCAN_CODE_Q) & 0x01))
    {
        output = 'Q';
    }
    else if((GetAsyncKeyState(SCAN_CODE_R) & 0x01))
    {
        output = 'R';
    }
    else if((GetAsyncKeyState(SCAN_CODE_S) & 0x01))
    {
        output = 'S';
    }
    else if((GetAsyncKeyState(SCAN_CODE_T) & 0x01))
    {
        output = 'T';
    }
    else if((GetAsyncKeyState(SCAN_CODE_U) & 0x01))
    {
        output = 'U';
    }
    else if((GetAsyncKeyState(SCAN_CODE_V) & 0x01))
    {
        output = 'V';
    }
    else if((GetAsyncKeyState(SCAN_CODE_W) & 0x01))
    {
        output = 'W';
    }
    else if((GetAsyncKeyState(SCAN_CODE_X) & 0x01))
    {
        output = 'X';
    }
    else if((GetAsyncKeyState(SCAN_CODE_Y) & 0x01))
    {
        output = 'Y';
    }
    else if((GetAsyncKeyState(SCAN_CODE_Y) & 0x01))
    {
        output = 'Z';
    }
}

// checks for misc characters
void misc(char& output)
{
    output = (char)0;
    if((GetAsyncKeyState(SCAN_CODE_FORWARD_SLASH) & 0x01))
    {
        output = '/';
    }
    else if((GetAsyncKeyState(SCAN_CODE_COLON) & 0x01))
    {
        output = ';';
    }
    else if((GetAsyncKeyState(SCAN_CODE_COMMA) & 0x01))
    {
        output = ',';
    }
    else if((GetAsyncKeyState(SCAN_CODE_MINUS) & 0x01))
    {
        output = '-';
    }
    else if((GetAsyncKeyState(SCAN_CODE_PERIOD) & 0x01))
    {
        output = '.';
    }
    else if((GetAsyncKeyState(SCAN_CODE_OPEN_BRACKET) & 0x01))
    {
        output = '[';
    }
    else if((GetAsyncKeyState(SCAN_CODE_CLOSE_BRACKET) & 0x01))
    {
        output = ']';
    }
    else if((GetAsyncKeyState(SCAN_CODE_QUOTE) & 0x01))
    {
        output = '"';
    }
    else if((GetAsyncKeyState(VK_SPACE) & 0x01))
    {
        output = ' ';
    }
    else if((GetAsyncKeyState(VK_BACK) & 0x01))
    {
        output = (char)1; //backspace 
    }
    else if((GetAsyncKeyState(VK_RETURN) & 0x01))
    {
        output = (char)2; //backspace 
    }
}


//THE LOOP (with an exit by pressing 'right-ctrl')
int main(int argc, char* argv[])
{
    cout << "Running!" << endl;

    Sleep(1000); // buffer before starting
    Recorder league;

    char press = (char)0; // error/off state
    bool running = true;

    // stores current state of keypress for different division of alphabet
    char alpha1_val; // 1st part of alphabet
    char alpha2_val; // 2st part of alphabet
    char misc_val; // misc characters

    while(running)
    {
        if(GetAsyncKeyState(VK_RCONTROL)){ // end condition: right-ctrl
            running = false;
        }

        //takes references of all these vals and writes to memory
        std::thread t_alpha1(alpha1, std::ref(alpha1_val));
        std::thread t_alpha2(alpha2, std::ref(alpha2_val));
        std::thread t_misc(misc, std::ref(misc_val));

        //join the three functions together
        t_alpha1.join();
        t_alpha2.join();
        t_misc.join();

        if((int)misc_val != 0)
        {
            press = misc_val;
        }
        else if((int)alpha2_val != 0)
        {
            press = alpha2_val;
        }
        else // press needs to get reset and if alpha1_val is in error state (0) then press won't be recognized 
        {
            press = alpha1_val;
        }

        
        // execute if there's a press and currently not replacing phrase
        if(press && league.isNotModding())
        {
            league.onPress(press);
        }

    }

    return 0;
}
