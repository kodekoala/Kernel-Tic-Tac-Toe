#include <stdio.h>

int main(){
    // Char arrays are declared like so:
    char array[] = {'h', 'o', 'w', 'd', 'y', '\n'};

    // Open a file for writing. 
    // (This will replace any existing file. Use "w+" for appending)
    FILE *file = fopen("/dev/tictactoe", "w");

    int results = fputs(array, file);
    if (results == EOF) {
        // Failed to write do error code here.
    }
    fclose(file);
    return 0;
}