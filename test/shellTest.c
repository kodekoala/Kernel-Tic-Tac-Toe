#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

volatile sig_atomic_t stop;

void inthand(int signum) {
    stop = 1;
}

int main(){
    // Char arrays are declared like so:
    static char respbuf[256];
    char *entries = NULL;
    ssize_t rs;
    ssize_t entriesLen = 0;
    int i = 0, receivedLen = 0;

    // Open a file for writing. 
    // (This will replace any existing file. Use "w+" for appending)
    int fd = open("/dev/tictactoe", O_RDWR);

    signal(SIGINT, inthand);

    while(!stop){
        printf(">");
        receivedLen = getline(&entries, &entriesLen, stdin);

        if(write(fd, entries, receivedLen) != receivedLen) {
            printf("Couldn't write command to /dev/tictactoe device: %s\n", strerror(errno));
            return -1;
        }

        /* Read the response. */
        if((rs = read(fd, respbuf, 256)) <= 0) {
            printf("Couldn't read response from /dev/tictactoe device: %s\n",
                strerror(errno));
            return -1;
        }

        printf("<");
        printf("%.*s", rs, respbuf);
        for (i = 0; i < receivedLen; i++){
            respbuf[i] = '\0';
        }
    }

    printf("exiting safely\n");
    close(fd);
    return 0;
}