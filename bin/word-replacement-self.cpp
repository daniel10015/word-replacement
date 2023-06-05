#include <iostream>
#include <Windows.h>
#include <string>
#include <unordered_map>
#include <fstream>
#include <cctype>

using std::cout;
using std::endl;
using std::string;
using std::fstream;
using std::unordered_map;


//IMPORTANT NOTES FOR WINDOWS

//VK_RETURN	0x0D	ENTER key
//VK_SPACE	0x20	SPACEBAR
//0x41	A key
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





//class for keeping track of the buffer
//class signatures
class Recorder
{
private:
    unordered_map<string, string> _wordPairs;
    string _buffer;
    bool _moddingWord;
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
private:
    //helper functions to modularizing the code
    void type(string word);
    void importCSV();
};

//class definitions
Recorder::Recorder()
{
    //this->word_pairs //getfrom csv
    this->_buffer = "";
    this->_moddingWord = false;
    this->importCSV();
}

bool Recorder::isNotModding()
{
    return !(this->_moddingWord);
}

void Recorder::onPress(char letter)
{
    //cout << "on press: " << letter << endl;
    if(letter == ' ') //base  fuck 
    {
        this->evaluate();
    }
    else if(letter == (char)1) //backspace
    {
        this->handleBackspace();
    }
    else if(letter == (char)2) //enter
    {
        //later have it evaluate when we figure out how to change the chat bind in LoL
        this->_buffer = "";
    }
    else
    {
        this->_buffer += letter;
    }
}

void Recorder::evaluate()
{
    cout << "evaluating: " << _buffer << endl;
    //if the buffer is in the list of words then execute
    if(this->_wordPairs.find(this->_buffer) != this->_wordPairs.end())
    {
        cout << "in list" << endl;
        this->deleteWord(); //deletes whatever the buffer has stored
        this->typeWord(); //types the appropriate word
    }
    this->_buffer = "";
}

void Recorder::deleteWord()
{
    //backspace the current buffer (whatever is on the screen, not what's in the buffer)
    const int length = this->_buffer.length();
    INPUT input = { 0 };
    for(int i=0; i<length+1; i++)
    {
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_BACK;
        SendInput(1, &input, sizeof(input));
        Sleep(10);
    }
}

void Recorder::typeWord(string extension)
{
    this->_moddingWord = true;
    type(this->_wordPairs[this->_buffer] + extension);
    this->_moddingWord = false;
}

void Recorder::handleBackspace()
{
    int length = this->_buffer.length();
    //cout << "backspace handling";
    if(length > 0)
    {
        this->_buffer = this->_buffer.substr(0, length-1); //deletes the last char
    }
}

void Recorder::type(string word) 
{
    for(long long unsigned int i=0; i<word.length(); i++)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VkKeyScan(word[i]);
        if(i>0 && word[i-1] == word[i]){//new code 10/3/2021
            Sleep(75);
        }

        SendInput(1, &input, sizeof(input));
        //Sleep(15); 
    }
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

void Recorder::importCSV()
{
    this->_wordPairs = {
{"GRIEF", "Bursting with jo"},  {"HEARTBREAK", "Excitement fills the ai"},
{"LONELINESS", "Hope sparks the sou"},  {"ANGUISH", "Delightful moments of blis"},
{"BROKEN", "Fulfilled with a grateful hear"},  {"GRIEVED", "Thriving in a state of blis"},
{"HURT", "Uplifted by the power of positivit"},  {"JOYLESS", "Excited for the journey ahea"},
{"MOROSE", "Ecstatic with pure jo"},  {"PATHETIC", "Optimistic about tomorrow's blessing"},
{"WEEPING", "Grateful for the present momen"},  {"SULLEN", "Harmonious unity in diversit"}
};
}


char checkForKeypress()
{
    char output = (char)0; //(default) error state

    if((GetAsyncKeyState(SCAN_CODE_A) & 0x01))
    {
        //cout << "A" << endl;
        output = 'A';
    }
    else if((GetAsyncKeyState(SCAN_CODE_B) & 0x01))
    {
        //cout << "B" << endl;
        output = 'B';
    }
    else if((GetAsyncKeyState(SCAN_CODE_C) & 0x01))
    {
        //cout << "C" << endl;
        output = 'C';
    }
    else if((GetAsyncKeyState(SCAN_CODE_D) & 0x01))
    {
        //cout << "D" << endl;
        output = 'D';
    }
    else if((GetAsyncKeyState(SCAN_CODE_E) & 0x01))
    {
        //cout << "E" << endl;
        output = 'E';
    }
    else if((GetAsyncKeyState(SCAN_CODE_F) & 0x01))
    {
        //cout << "F" << endl;
        output = 'F';
    }
    else if((GetAsyncKeyState(SCAN_CODE_G) & 0x01))
    {
        //cout << "G" << endl;
        output = 'G';
    }
    else if((GetAsyncKeyState(SCAN_CODE_H) & 0x01))
    {
        //cout << "H" << endl;
        output = 'H';
    }
    else if((GetAsyncKeyState(SCAN_CODE_I) & 0x01))
    {
        //cout << "I" << endl;
        output = 'I';
    }
    else if((GetAsyncKeyState(SCAN_CODE_J) & 0x01))
    {
        //cout << "J" << endl;
        output = 'J';
    }
    else if((GetAsyncKeyState(SCAN_CODE_K) & 0x01))
    {
        //cout << "K" << endl;
        output = 'K';
    }
    else if((GetAsyncKeyState(SCAN_CODE_L) & 0x01))
    {
        //cout << "L" << endl;
        output = 'L';
    }
    else if((GetAsyncKeyState(SCAN_CODE_M) & 0x01))
    {
        //cout << "M" << endl;
        output = 'M';
    }
    else if((GetAsyncKeyState(SCAN_CODE_N) & 0x01))
    {
        //cout << "N" << endl;
        output = 'N';
    }
    else if((GetAsyncKeyState(SCAN_CODE_O) & 0x01))
    {
        //cout << "O" << endl;
        output = 'O';
    }
    else if((GetAsyncKeyState(SCAN_CODE_P) & 0x01))
    {
        //cout << "P" << endl;
        output = 'P';
    }
    else if((GetAsyncKeyState(SCAN_CODE_Q) & 0x01))
    {
        //cout << "Q" << endl;
        output = 'Q';
    }
    else if((GetAsyncKeyState(SCAN_CODE_R) & 0x01))
    {
        //cout << "R" << endl;
        output = 'R';
    }
    else if((GetAsyncKeyState(SCAN_CODE_S) & 0x01))
    {
        //cout << "S" << endl;
        output = 'S';
    }
    else if((GetAsyncKeyState(SCAN_CODE_T) & 0x01))
    {
        //cout << "T" << endl;
        output = 'T';
    }
    else if((GetAsyncKeyState(SCAN_CODE_U) & 0x01))
    {
        //cout << "U" << endl;
        output = 'U';
    }
    else if((GetAsyncKeyState(SCAN_CODE_V) & 0x01))
    {
        //cout << "V" << endl;
        output = 'V';
    }
    else if((GetAsyncKeyState(SCAN_CODE_W) & 0x01))
    {
        //cout << "W" << endl;
        output = 'W';
    }
    else if((GetAsyncKeyState(SCAN_CODE_X) & 0x01))
    {
        //cout << "X" << endl;
        output = 'X';
    }
    else if((GetAsyncKeyState(SCAN_CODE_Y) & 0x01))
    {
        //cout << "Y" << endl;
        output = 'Y';
    }
    else if((GetAsyncKeyState(SCAN_CODE_Y) & 0x01))
    {
        //cout << "Z" << endl;
        output = 'Z';
    }
    else if((GetAsyncKeyState(VK_SPACE) & 0x01))
    {
        //cout << "space" << endl;
        output = ' ';
    }
    else if((GetAsyncKeyState(VK_BACK) & 0x01))
    {
        //cout << "backspace" << endl;
        output = (char)1; //backspace 
    }
    else if((GetAsyncKeyState(VK_RETURN) & 0x01))
    {
        //cout << "enter" << endl;
        output = (char)2; //backspace 
    }

    return output;
}



//THE Loop (not to be confused with the one from Chicago, USA)
int main(int argc, char** argv)
{
    cout << "hello jim boe followers!" << endl;

    Sleep(2000);
    Recorder league;



    char press = (char)0; //error state

    while(true)
    {
        if(GetAsyncKeyState(VK_RCONTROL)){
            return 0;
        }

        press = checkForKeypress(); //returns 0 if no keypress

        if(press && league.isNotModding())
        {
            league.onPress(press);
        }

    }

    return 0;
}