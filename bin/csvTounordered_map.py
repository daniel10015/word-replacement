###WARNING THIS IS CURRENTLY SKIPPING EVERY OTHER LINE (im not a pythonista)
#but feel free to debug this!

import csv
with open("bin/Constitution.csv") as file:
    read = csv.reader(file, delimiter=';', lineterminator='\n') 
    #read into a string that will be in the format {"word1", "word2"} 

    output = "{"
    increment = 0

    for row in read:
        #print(row) debugging
        if increment != 0:
            output += ",  "
        if increment % 2 == 0: #line breaks for clarity
             output += '\n'
        badString = str(row[0]).upper()
        goodString = row[1]

        output += '{"' + badString + '", ' + '"' + goodString + '"}'
        badString = file.readline()
        increment+=1

output += "\n}"
print(output)