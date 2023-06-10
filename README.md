# Word Replacement
The perfect software for an angry Online Gamer with 80wpm typing speed... Inspiration for this from [this video](https://www.youtube.com/watch?v=on0UTFHNFMg&ab_channel=MartinHavelka)

This repository contains source code, binaries (Windows x64AMD), and python scripts to automate some of the csv work.
## Table of Contents
- [Getting Started](#getting-started)
- [Usage](#usage)
    - [Make Your Own List Of Words](#make-your-own-list-of-words)
- [How It Works](#how-it-works)
- [Bug Report](#bug-report)



## Getting Started
### Pre reqs
- Windows (x64)
    - I'm using Windows.h for the keypresses
- A python3 interpreter (optional if you want to use the python scripts)


Either you [don't have a C++ Compiler](#you-don't-have-a-c++-compiler) or [you do have one](#you-have-a-c++-compiler)

### You don't have a C++ Compiler
You will need to 
1. download the executable 
``` ./WordReplacement/bins/Word-Replace.exe ```
2. Windows will warn you that it's dangerous (because I don't have any security in the software), just say "keep it"
3. Everytime you run the application it will ask if you're sure, just click "run anyways"

If you don't want to deal with this, then build it yourself.


### You have a C++ Compiler
If you have a C++ compiler then you all you'll need to compile is any variation of the ```WordReplace.cpp``` source code file. The DEBUG version will print out program states into the console, and self versions won't rely on an external csv file during runtime (since it matches the pairs of words at compile time). For example if you're in the parent directory and using g++ then run this command:
```
g++ ./WordReplacement/src/WordReplace.cpp -o "Word Replace"
```



## Usage
1. Run the program
    - You can make it an app that can you click so you don't have to open up the folder everytime.
2. Type a word
3. Press the spacebar to evaluate the word 
4. If the word is set to change according to the settings then it will get modified
5. r-ctrl to exit the program (or close the application window)

### Make Your Own List Of Words
If you'd like to create your own constitution then there's a python script ```createCSV.py``` that will take two text files, one called ```word-to-replace.txt``` and ```words-to-add.txt``` and will do a one-to-one matchup between the two. 

```word-to-replace.txt``` should only have 1 word per line since the app currently only works on singular words.

```words-to-add.txt``` should only have 1 word or phrase per line to matchup with the same line in ```word-to-replace.txt```

You can also just delete or add to the csv file manually. 

If you're you're using ```Word-Replace-self``` (the one that doesn't pull from a csv everytime it runs) then you'll need to download the executable ```Word-Replace.exe```. Go back to [getting started](#getting-started) for directions.



## How It Works
```words-to-add.txt``` and ```words-to-remove.txt``` are phrases generated from ChatGPT when asked to create a list of 100 "sad words" and "phrases in 5 words or less".

I'll finish this later if it gets more complicated. Well it's more complicated, click [here](https://jamboard.google.com/d/1Lc5qBSnu2zR7ofjS1GFtHWSp0otIEuCCNbBpaViyiQs/viewer?f=2) to checkout the different designs! 


### source code difference
Every executable works the same, however ```Word-Replace-self``` implements the unordered_map inside the source code instead of pulling from a csv everytime to make it so you don't have to download the csv and put it in the same folder during setup. So modifying it from the source looks like the code block below. This is instead of just opening up the csv in a text editor or an excel sheet (prefered)

```
Word-Replace-self.cpp (Original)
    map = {{"test", "loop"}, {"wam", "bam"}};
Word-Replace-self.cpp (Adding a key/value pair)
    map = {{"test", "loop"}, {"wam", "bam"}, {"foo", "bar"}};
```


## Contributing
I'm new so add me on discord if you're interested in improving this!


## Bug Report
If you encounter any bugs add me on discord Daniel0192837465#5940
