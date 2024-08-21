//took this skeleton from recitation 6 code
#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H


#include <linux/ioctl.h>
#ifdef __KERNEL__
#include <linux/types.h> // for size_t in kernel space
#else
#include <sys/types.h> // for size_t in user space
#endif

// The major device number.
// We don't rely on dynamic registration
// any more. We want ioctls to know this
// number at compile time.
#define MAJOR_NUM 235

// Set the message of the device driver
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long)

#define DEVICE_RANGE_NAME "Message_Slot"
#define MAX_MESSAGE_LEN 128
#define MAX_MINOR 255
#define MAX_CHANNEL_NUM 1048576 //2**20
#define DEVICE_FILE_NAME "message_slot"
#define SUCCESS 0

//its a struct for holding a meesage of a channel.
typedef struct MessageChannelNode {
    unsigned long num_channel;
    char *message; // the actual message
    size_t len; // the actual size of the message
    struct MessageChannelNode* next;
} MessageChannelNode;


#endif
