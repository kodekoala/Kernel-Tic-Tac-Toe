project2-part1-kodekoala

This is a kernel character module implementation of the common tic-tac-toe game (3 by 3 board).
The user program is to write to the device file /dev/tictactoe, following the instructions and specs for project 2 part 1:

"00 X/O\n" will begin a new game and set the user's mark to what is given, an X or O.
"01\n" will return a char * for the board array, which will list starting with the top left, progressing row by row to the bottom right.
"02 X Y\n" specifies a move for the player, where X is column and Y is row. 
"03\n" asks the computer to make a move and place its mark on the board.

There are several test programs written, under the test directory. I will be adding a makefile to the test directory sometime soon. 

There is also a driver directory that will hopefully include driver programs, along with a Makefile.

The module source code lies under the module directory. There is a Makefile that can be run to build the code.
