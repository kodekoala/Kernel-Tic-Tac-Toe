#include "tictactoe.h"

char unknownCmd[] = {'U', 'N', 'K', 'C', 'M', 'D', '\n'};
char invalidFmt[] = {'I', 'N', 'V', 'F', 'M', 'T', '\n'};
char board[] = {'*', '*', '*', '*', '*', '*', '*', '*', '*', '\n'};
char win[] = {'W', 'I', 'N', '\n'};
char tie[] = {'T', 'I', 'E', '\n'};
char illMove[] = {'I', 'L', 'L', 'M', 'O', 'V', 'E', '\n'};
char oot[] = {'O', 'O', 'T', '\n'};
char noGame[] = {'N', 'O', 'G', 'A', 'M', 'E', '\n'};
char ok[] = {'O', 'K', '\n'};

char* returnStr = NULL;

char xo[] = {'X', 'O'};
int turn = 0;
char* player = NULL; 
char* cpu = NULL; 
int gameOver = 1;
int boardFilled = 0;
int buffer_size = 0;

/* Declare mutex */
DEFINE_MUTEX(lock);

int tictactoe_open(struct inode *pinode, struct file *pfile){
    LOG_INFO("In function %s\n", __FUNCTION__);

    //In open we return 0 for a successful open, otherwise we should return an error
    return 0;
}

ssize_t tictactoe_read(struct file *pfile, char __user *buffer, size_t length,
                    loff_t *offset){
    int rv = 0;
    /* Lock */
    mutex_lock(&lock);
    
    LOG_INFO("In function %s\n", __FUNCTION__);
    LOG_INFO("In function %s the length is %zd\n", __FUNCTION__, length);
    
    if (buffer_size){
        if (copy_to_user(&buffer[0], &returnStr[0], buffer_size) != 0){
            LOG_INFO("Copy/Allocation Error\n");
            mutex_unlock(&lock);
            return -EFAULT;
        }
        rv = buffer_size;
        length = buffer_size;
    }
    buffer_size = 0;

    /* Unlock */
    mutex_unlock(&lock);

    //We return how many bytes someone reads from our file when they call read on it
    return rv;
}

ssize_t tictactoe_write(struct file *pfile, const char __user *buffer, size_t length,
                    loff_t *offset){
        
    char * kernelBuff;
    long colVal, rowVal;
    char * st;
    char *endptr;
    char ** tokenArr;
    char *dest;
    int numTokens = 0, illegal = 1;
    int i = 0;
    int c = 0;
    long temp = 0;
    int MAXLEN = 9;
    
    /* Lock */
    mutex_lock(&lock);
    LOG_INFO("In function %s\n", __FUNCTION__);
    LOG_INFO("In function %s the length is %zd\n", __FUNCTION__, length);

    /*
    Check that *buffer is a valid pointer, and also that you can read the length provided from it
    */

    if (buffer == NULL || length < 0){ //check passed in pointer
        LOG_ERROR("Invalid buffer or length passed\n");
        mutex_unlock(&lock);
        return -EFAULT;
    }

    if (!access_ok(buffer, length)){ 
        LOG_ERROR("Buffer inaccessible or not as large as specified\n");
        mutex_unlock(&lock);
        return -EFAULT;
    }

    if (length == 0){
        LOG_INFO("Empty command!");
        returnStr = unknownCmd;
        calcSize();
        return length;
    }

    /*
    If the command length is more than the maximum length for an accepted one...
    */
    if (length > MAXLEN){
        LOG_INFO("Length greater than 9\n");

        kernelBuff = kmalloc (2, GFP_KERNEL);

        if(copy_from_user( &kernelBuff[0], &buffer[0], sizeof(kernelBuff)) != 0){
            LOG_INFO("Copy/Allocation Error\n");
            kfree(kernelBuff);
            mutex_unlock(&lock);
            return -EFAULT;
        }

        returnStr = unknownCmd;
        if (kernelBuff[0] == '0'){
            if (kernelBuff[1] == '0' || kernelBuff[1] == '1' || kernelBuff[1] == '2' || kernelBuff[1] == '3'){
                returnStr = invalidFmt;
            }
        }
        calcSize();
        kfree(kernelBuff);
        mutex_unlock(&lock);
        return length;
    }

    kernelBuff = kmalloc (length, GFP_KERNEL);

    if(!kernelBuff){
        LOG_INFO("Allocation Error\n");
        mutex_unlock(&lock);
        return -EFAULT;
    }

    if(copy_from_user( &kernelBuff[0], &buffer[0], length) != 0){
        LOG_INFO("Copy/Allocation Error\n");
        kfree(kernelBuff);
        mutex_unlock(&lock);
        return -EFAULT;
    }

    if (*kernelBuff == 0 || kernelBuff[0] == '\n'){
        LOG_INFO("Empty command!");
        returnStr = unknownCmd;
        calcSize();
        kfree(kernelBuff);
        mutex_unlock(&lock);
        return length;
    }

    if (kernelBuff[length - 1] != '\n'){
        LOG_INFO("Invalid command, no newline char!");
        returnStr = unknownCmd;
        calcSize();
        kfree(kernelBuff);
        mutex_unlock(&lock);
        return length;
    }

    tokenArr = kmalloc(10 * sizeof(char*), GFP_KERNEL);

    if (tokenArr == NULL){
        LOG_INFO("Allocation Error\n");
        kfree(kernelBuff);
        mutex_unlock(&lock);
        return -ENOMEM;
    }

    for (i = 0; i < 10; i++){
        tokenArr[i] = NULL;    
    }

    kernelBuff[strcspn(kernelBuff, "\n")] = '\0';
    st = kernelBuff;

    i = -1;
    while((dest = strsep(&st, " ")) != NULL){
        i++;
        numTokens = i + 1;
        /*Give extra byte for null terminator \0 */
        tokenArr[i] = kmalloc (strlen(dest) + 1, GFP_KERNEL);
        if (tokenArr[i] == NULL){
            LOG_INFO("Allocation Error\n");
            for (c = 0; c < i; c++){
                kfree(tokenArr[c]);
            }
            kfree(tokenArr);
            kfree(kernelBuff);
            mutex_unlock(&lock);
            return -ENOMEM;
        }
        memcpy(tokenArr[i], dest, strlen(dest) + 1);
    }
    /*Options for valid commands:
    00 X/O
    01
    02 X Y
    03
    */

    LOG_INFO("Number of tokens is: %d\n", numTokens);

    if (numTokens == 1){
        if (strcmp(tokenArr[0], "01") == 0){
            /*The command is 01*/
            goto case1;
        }
        else if (strcmp(tokenArr[0], "03") == 0){
            /*The command is 03*/
            goto case3;
        }
        else if (strcmp(tokenArr[0], "00") == 0 || strcmp(tokenArr[0], "02") == 0)
        {
            illegal = 0;
        }
        
    }
    else if (numTokens == 2){
        if (strcmp(tokenArr[0], "00") == 0){
            illegal = 0;
            if(strcmp(tokenArr[1], "X") == 0 || strcmp(tokenArr[1], "O") == 0){
                LOG_INFO("The command is 00\n");
                /*The command is 00*/
                player = &xo[1];
                cpu = &xo[0];
                if (strcmp(tokenArr[1], "X") == 0){
                    player = &xo[0];
                    cpu = &xo[1];
                }
                goto case0;
            }
        }
        else if (strcmp(tokenArr[0], "01") == 0 || strcmp(tokenArr[0], "02") == 0 || strcmp(tokenArr[0], "03") == 0){
            illegal = 0;
        }
    }
    else if (numTokens == 3){
        if (strcmp(tokenArr[0], "02") == 0){
            goto case2;
        }
        else if (strcmp(tokenArr[0], "00") == 0 || strcmp(tokenArr[0], "01") == 0 || strcmp(tokenArr[0], "03") == 0){
            illegal = 0;
        }
    }
    
    returnStr = invalidFmt;
    if (illegal){
        returnStr = unknownCmd; 
    }
    goto clearMem;

    case0:
        for (i = 0; i < 9; i++){
            board[i] = '*';
        }
        returnStr = ok;
        gameOver = 0;
        boardFilled = 0;
        turn = 0;
        goto clearMem;

    case1:
        returnStr = board;
        goto clearMem;

    case2:
        if (gameOver){
            returnStr = noGame;
            goto clearMem;
        }

        if (*player != xo[turn]){
            returnStr = oot;
            goto clearMem;
        }

        for (i = 1; i < 3; i++){
            temp = strlen(tokenArr[i]);
            for (c=0;c<temp; c++){
                if (!isdigit(tokenArr[i][c]))
                {
                    LOG_INFO("Entered coordinate is not a number\n");
                    returnStr = invalidFmt;
                    goto clearMem;
                }
            }
        }
        
        i=0;
        c=0;

        colVal = simple_strtol(tokenArr[1], &endptr, 10);
        rowVal = simple_strtol(tokenArr[2], &endptr, 10);

        if (colVal < 0 || colVal > 2 || rowVal < 0 || rowVal > 2){
            LOG_INFO("Entered coordinate is invalid\n");
            returnStr = illMove;
            goto clearMem;
        }
        
        if (board[colVal + 3*rowVal] != '*'){
            returnStr = illMove;
            goto clearMem;
        }

        board[colVal + 3*rowVal] = *player;
    inputChoice:
        boardFilled++;

        if (rowCrossed() || columnCrossed() || diagonalCrossed()){
            returnStr = win;
            gameOver = 1;
            turn = 0;
            boardFilled = 0;
            goto clearMem;
        }

        if (boardFilled == 9){
            returnStr = tie;
            gameOver = 1;
            turn = 0;
            boardFilled = 0;
            goto clearMem;
        }

        /*Otherwise if it is not a win or tie*/
        turn = !turn;
        returnStr = ok;
        goto clearMem;


    case3:
        if (gameOver){
            returnStr = noGame;
            goto clearMem;
        }

        if (*player == xo[turn]){
            returnStr = oot;
            goto clearMem;
        }

        for (i = 0; i < 9; i++){
            if (board[i] == '*'){
                board[i] = *cpu;
                break;
            }
        }

        goto inputChoice;

    clearMem:
        for (i = 0; i < 10; i++){
            if (tokenArr[i] != NULL){
                kfree(tokenArr[i]);
            }
            else{
                break;
            }
        }
        kfree(tokenArr);
        kfree(kernelBuff);

        calcSize();

        /* Unlock */
        mutex_unlock(&lock);

        //Tell whoever called write function, we tell them how many bytes we wrote in our file
        return length;
}

int tictactoe_release(struct inode *pinode, struct file *pfile){
    LOG_INFO("In function %s\n", __FUNCTION__);
    
    //We return 0 because we don't have any error
    return 0;
}

int rowCrossed() 
{
    int i;
    for (i=0; i<3; i++) 
    { 
        if ((board[0+(i*3)] == board[1+(i*3)] && board[1+(i*3)] == board[2+(i*3)]) && board[0+(i*3)] != '*'){
            return 1;
        }
    } 
    return 0; 
} 
  
// A function that returns true if any of the column 
// is crossed with the same player's move 
int columnCrossed() 
{ 
    int i;
    for (i=0; i<3; i++) 
    { 
        if ((board[0+i] == board[3+i] && board[3+i] == board[6+i]) && board[0+i] != '*'){
            return 1;
        }
    } 
    return 0; 
} 

// A function that returns true if any of the diagonal 
// is crossed with the same player's move 
int diagonalCrossed() 
{ 
    if ((board[0] == board[4] && 
        board[4] == board[8]) && board[0] != '*'){ 
            return 1;
        }
          
    if ((board[2] == board[4] && 
        board[4] == board[6]) && board[2] != '*'){ 
            return 1;
        }
  
    return 0; 
} 

void calcSize(){
    buffer_size = 0;
    while (returnStr[buffer_size] != '\n'){
        buffer_size++;
    }
    buffer_size++;
}