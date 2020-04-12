#ifndef TICTACTOE_H
#define TICTACTOE_H

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/ctype.h>

#define MODULE_NAME "tictactoe"

#define _MY_PRINTK(once, level, fmt, ...)                       \
    do {                                                          \
        printk##once(KERN_##level "[" MODULE_NAME "]" fmt,    \
                ##__VA_ARGS__);                         \
    } while (0)                                 

#define LOG_INFO(format, ...) _MY_PRINTK(, INFO, format, ##__VA_ARGS__)       

#define LOG_WARN(format, ...) _MY_PRINTK(, WARN, format, ##__VA_ARGS__)       

#define LOG_ERROR(format, ...) _MY_PRINTK(, ERR, format, ##__VA_ARGS__)       

int tictactoe_open(struct inode *pinode, struct file *pfile);
ssize_t tictactoe_read(struct file *pfile, char __user *buffer, size_t length,
                    loff_t *offset);
ssize_t tictactoe_write(struct file *pfile, const char __user *buffer, size_t length,
                    loff_t *offset);
int tictactoe_release(struct inode *pinode, struct file *pfile);
int rowCrossed(void);
int columnCrossed(void);
int diagonalCrossed(void);
void clearMem(char * kernelBuff, char** tokenArr);
void calcSize(void);

#endif