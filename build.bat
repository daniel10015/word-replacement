:: builds the source and outputs executable to ~/bin
:: DEBUG option builds the debug version which also prints out information and logs letter/time  
:: usage: 
:: build <DEBUG> c

:begin
@echo off
IF "%1" == "DEBUG" ( 
  g++ src/WordReplacementDEBUG.cpp -o bin/WordReplacementDEBUG.exe 
  ) ELSE ( 
  g++ src/WordReplacement.cpp -o bin/WordReplacement.exe )
:end