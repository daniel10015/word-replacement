#include <iostream>
#include <Windows.h>
#include <WinUser.h>
#include <string>
#include <unordered_map>
#include <set>
#include <fstream>
#include <cctype>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>

using namespace std::chrono_literals;
using std::cout;
using std::endl;
using std::string;
using std::fstream;
using std::unordered_map;
using std::vector;


//IMPORTANT NOTES FOR WINDOWS
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
//weird naming conventions so I renamed them
#define SCAN_CODE_FORWARD_SLASH VK_OEM_2
#define SCAN_CODE_COLON VK_OEM_1
#define SCAN_CODE_COMMA VK_OEM_COMMA
#define SCAN_CODE_MINUS VK_OEM_MINUS
#define SCAN_CODE_PERIOD VK_OEM_PERIOD
#define SCAN_CODE_OPEN_BRACKET VK_OEM_4
#define SCAN_CODE_CLOSE_BRACKET VK_OEM_6
#define SCAN_CODE_QUOTE VK_OEM_7

//general functions
//------------
unsigned int getSizeOfChar(char* characters)
{
    unsigned int iterator = 0;
    while(characters[iterator] != '\0')
    {
        iterator++;
    }
    return iterator;
}

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
//--------------
//end of general functions

//class for keeping track of the buffer
//class signatures
class Recorder
{
private:
    unordered_map<string, string> _wordPairs;
    string _L1buffer;
    string _L2buffer;
    int _numberOfWordOnScreen; //negative is error state (reserved for not in use)
    unsigned int _maxSizeOfArray;
    INPUT* _inputArray = nullptr;
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
    //ONLY FOR DEBUGGING
    int lettersTyped;
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

    //ONLY FOR DEBUGGING
    int getLettersTyped();

    //destructor
    ~Recorder();
private:
    //helper functions to modularizing the code
    void type(string word);
    unsigned int importCSV(unsigned int&);
    void setupInputArray();
    void setupStorageString(unsigned int size);
    void handleImpactedBuffer(string word);
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

void Recorder::setupStorageString(unsigned int size)
{
    for(unsigned int i=0; i<size;i++)
    {
        this->_prevCensoredWords[i] = '\0';
    }
}

//class definitions
Recorder::Recorder()
{
    this->_largestCensor = 0;
    //this->word_pairs //getfrom csv
    this->_L1buffer = "";
    this->_L2buffer = "";
    this->_moddingWord = false;
    this->_maxSizeOfArray = this->importCSV(this->_largestCensor); //largestCensor passed by reference
    this->_inputArray = new INPUT[this->_maxSizeOfArray];
    this->_prevCensoredWords = new char[this->_largestCensor + 1];
    this->setupStorageString(this->_largestCensor+1);
    this->_wordCount = 0;
    this->_numberOfWordOnScreen = 0;
}

bool Recorder::isNotModding()
{
    return !(this->_moddingWord);
}

void Recorder::onPress(char letter)
{
    if(letter == ' ') //base case 
    {
        this->evaluate();
    }
    //TODO: handle for ctrl+backspace (removes entire word)
    else if(letter == (char)1) //backspace
    {
        this->handleBackspace();
    }
    //TODO: handle for eval to happen upon enter (like in situations where modding a keybind can be done in game)
    else if(letter == (char)2) //enter
    {
        //later have it evaluate when we figure out how to change the chat bind in LoL
        this->_L1buffer = "";
        this->_L2buffer = "";
    }
    else
    {
        this->_L1buffer += letter;
    }
}

void Recorder::evaluate()
{
    string phrase = this->_prevCensoredWords + this->_L1buffer;
    bool L1bufferTyped = false;
    if((this->_censorWord[_wordCount][_L1buffer] == 'b' || this->_censorWord[_wordCount][_L1buffer] == 's') && this->_wordPairs.find(phrase) != this->_wordPairs.end()) //type something
    {
        L1bufferTyped = true;
        phrase = this->_wordPairs[phrase]; //set phrase from old phrase to new phrase
        this->deleteWord(); //deletes whatever the buffer has stored
        this->typeWord(phrase + " "); //types the appropriate word
    }

    if(this->_censorWord[_wordCount][_L1buffer] == 'b' || this->_censorWord[_wordCount][_L1buffer] == 'm') //store and add word count
    {
        if(this->_prevCensoredWords[0] == '\0')
        {
            appendStrToChar(this->_prevCensoredWords, this->_L1buffer); //cats L1buffer to prev censored words
            //if the index isn't already set then set it to 0, since it would normally get set by typing the word
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
        //if L1 buffer didn't get typed then L2 buffer won't get added to, so add here instead
        if(!L1bufferTyped)
        {
            this->_L2buffer += " " + this->_L1buffer;
        }
    }
    //L2 gets simple appended by L2
    else
    {
        this->setupStorageString(getSizeOfChar(this->_prevCensoredWords)); //reset words
        this->_wordCount = 0; //reset word count since that word is not associated with the middle of a phrase
        this->_numberOfWordOnScreen = -1; //reset index
    }
    //reset L1Buffer 
    //this->_L2buffer += " " + this->_L1buffer;
    this->_L1buffer = "";
}

//TODO see if I can utilize the new method of deleting or if I need to keep this
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

string toLower(string lower)
{
    int lengthOfString = lower.length();
    for(int i=0; i<lengthOfString; i++)
    {
        lower[i] = tolower(lower[i]);
    }
    return lower;
}

//makes GetAsyncKeyState(VK_KEY)
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

void Recorder::type(string word) 
{
    UINT sizeOfWord = word.length(); 
    //DEBUGGING
    this->lettersTyped = (int)sizeOfWord;
    UINT justTyped = 0;
    UINT sizeOfInput = 0;
    for(UINT i=0; i<sizeOfWord-1; i++)
    {
        this->_inputArray[i].type = INPUT_KEYBOARD;
        this->_inputArray[i].ki.wVk = VkKeyScan(word[i]);
        //this is for repeating letters because windows is bad at sending repeated inputs
        if(word[i] == word[i+1])
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

string toUpper(string lower)
{
    int lengthOfString = lower.length();
    for(int i=0; i<lengthOfString; i++)
    {
        lower[i] = toupper(lower[i]);
    }
    return lower;
}

unsigned int Recorder::importCSV(unsigned int& largestCensor)
{
    fstream fi;
    unsigned int count = 0;
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
            //if char is a space
            if(line.substr(i,1) == " ") 
            {
                lastLetter = i;
                //if the word exists and it's not 'm' then we need to make it BOTH
                //if it doesn't exist then we make it 'm'
                //this is so if it's equal to 's' or 'b' then it'll be set to 'b' 
                //if it doesn't exist or is already 'm' then it's still 'm'
                //check if this is the longest phrase
                if(wordPosition > (int)(_censorWord.size() - 1))
                {
                    this->_censorWord.push_back(unordered_map<string, char> ());
                    this->_censorWord[wordPosition][toUpper(line.substr(firstLetter,lastLetter-firstLetter))] = 'm'; //MIDDLE
                }
                else if(this->_censorWord[wordPosition].find(toUpper(line.substr(firstLetter,lastLetter-firstLetter))) != this->_censorWord[wordPosition].end() && this->_censorWord[wordPosition][line.substr(firstLetter,lastLetter-firstLetter)] != 'm')
                {
                    this->_censorWord[wordPosition][toUpper(line.substr(firstLetter,lastLetter-firstLetter))] = 'b'; //STOP AND MIDDLE
                }
                else //doesn't exist yet
                {
                    this->_censorWord[wordPosition][toUpper(line.substr(firstLetter,lastLetter-firstLetter))] = 'm'; //MIDDLE
                }
                wordPosition++; //increment wordPosition
                firstLetter = i+1;
            }

            //is true after censor is fulled parsed
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

                tempLargestCensor = i;
                tempCount = lengthOfLine-i;
                this->_wordPairs[toUpper(line.substr(0, tempLargestCensor))] = line.substr(i+1, tempCount);

                //stop the code from running
                i = lengthOfLine;
            }
        }
        if(tempLargestCensor > largestCensor)
        {
            largestCensor = tempLargestCensor;
        }
        if(tempCount > count)
        {
            count = tempCount;
        }
    }

    return count; //returns mod-count
}

int Recorder::getLettersTyped()
{
    return this->lettersTyped;
}

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
//end of class implementations

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

//writes to 
bool timeForLetter(double milliseconds, int words)
{
    fstream logger;
    bool didLog = true;
    try
    {
        logger.open("timeForLetterMultithread.csv", std::ios::app);
        logger << milliseconds << ';' << words << '\n';
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        didLog = false;
    }

    if(logger.is_open())
        logger.close();
    return didLog;
}



//THE LOOP (with an exit called 'right-ctrl')
int main(int argc, char* argv[])
{
    //DEBUGGING
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    cout << "Running!" << endl;

    Sleep(1000);
    Recorder league;

    char press = (char)0; // error/off state
    bool running = true;

    char alpha1_val;
    char alpha2_val;
    char misc_val;

    while(running)
    {
        if(GetAsyncKeyState(VK_RCONTROL)){
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
        else
        {
            press = alpha1_val;
        }

        //press = checkForKeypress(); //returns 0 if no keypress

        if(press && league.isNotModding())
        {
            //DEBUGGING tracking time
            auto t1 = high_resolution_clock::now();
            league.onPress(press);
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            if(ms_double.count() > 5.0)
                timeForLetter(ms_double.count(), league.getLettersTyped());
        }

    }

    return 0;
}
