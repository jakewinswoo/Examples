"# Examples" 
README
author: Jacob Link

CS3013 Project 3

OVERVIEW:
This program simulates rats running through a maze of rooms. The rats are 
simulated by threads, and the rooms are protected with mutual exclusion, allowing
only the number of rats specified in the rooms file at any one time. 

The user may specify the number of rats, any amount from 1-5, which will be running
the maze. This is done by adding an integer on the command line.

The program can run either in distributed or in-order modes, specified with an 
i or a d when calling the function. 

Sample program call:

"maze 3 i"

A typescript file is included with a sample run of the program. 

BASIC OBJECTIVE:
This program compiles by running "make". It compiles into the executable "maze".
It can be run by running "maze 3 i", with an i or d, and 5 or less as the integer.
Higher integers will throw an error as they should, and incorrect arguments will
exit and prompt the user to enter the correct arguments.

Distributed and in-order both function correctly, and an example of the program 
running and correctly responding to invalid input is included in the typescript file.

The source code file is called "proj3.c", and the file to input room parameters is 
called "rooms".

A room is a single line from the rooms file, in the format "int1 int2", where int1 is the
room capacity, and int2 is the delay. Spacing per line should not affect the program's ability 
to read them in. 
