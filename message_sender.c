#include "message_slot.h"  

#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        perror("There should be only 3 inputs");
        exit(1);
    }

    const char *file_path = argv[1];
    unsigned int channel_id = atoi(argv[2]);
    const char *message = argv[3];

    int fd = open(file_path,O_RDWR ); // open the file for read and write
    if(fd < 0 ){
        perror("Failed to open device file");
        exit(1);
    }
    if(ioctl(fd, MSG_SLOT_CHANNEL, channel_id) < 0)
    {
        perror("Failed to determine the channel");
        exit(1);
    }

    if(write(fd, message, strlen(message)) != strlen(message))
    {
        perror("Failed to write the message");
        exit(1);
    }
    close(fd);
    exit(0);

}