#include "message_slot.h"  

#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */

#include <stdio.h>
#include <stdlib.h>


#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        perror("There should be only 2 inputs");
        exit(1);
    }

    const char *file_path = argv[1];
    unsigned int channel_id = atoi(argv[2]);
    char message[MAX_MESSAGE_LEN];

    int fd = open(file_path,O_RDONLY ) ;// open the file for read 
    if(fd < 0 ){
        perror("Failed to open device file");
        exit(1);
    }
    if(ioctl(fd, MSG_SLOT_CHANNEL, channel_id) < 0)
    {
        perror("Failed to determine the channel");
        exit(1);
    }
    ssize_t length = read(fd, message, MAX_MESSAGE_LEN);            
    if(length < 0)
    {
        perror("Failed to read the message");
        close(fd);
        exit(1);
    }

    close(fd);
    if(write(1, message, length) != length) // 1 = stdout
    {
        perror("Failed to write the message to stdout");
        close(fd);
        exit(1);
    }

    exit (0);

}