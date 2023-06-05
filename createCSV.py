import pandas as pd

badWord = []
goodWords = []

readBad = open("list-of-words-to-change.txt","r")
readGood = open("list-of-happy-phrases.txt","r")

badString = readBad.readline()
goodString = readGood.readline()

increment = 0

while badString and goodString:
    badWord.append(badString.strip()) #these indecies may need to be edited if there are punctuations at the end
    goodWords.append(goodString.strip())

    increment+=1
    badString = readBad.readline()
    goodString = readGood.readline()

#increment is now the length of the lists 
df = pd.DataFrame({'Bad Words': badWord,
                   'Good Words': goodWords})
df.to_csv('src/Constitution.csv', sep=';', index=False, header=False)
df.to_csv('bin/Constitution.csv', sep=';', index=False, header=False)
print("Done transfering text files to csv!")