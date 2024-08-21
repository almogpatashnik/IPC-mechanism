//took this skeleton from recitation 6 code

// Declare what kind of code we want
// from the header files. Defining __KERNEL__
// and MODULE allows us to access kernel-level
// code not usually available to userspace programs.
#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE


#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/
#include <linux/slab.h>     /* for kmalloc and kfree */


MODULE_LICENSE("GPL");

//Our custom definitions of IOCTL operations
#include "message_slot.h"

// Declare an array of pointers to MessageChannelNode with size 256 - for each device file in the range 0-255
static  MessageChannelNode* messageChannelArray[MAX_MINOR + 1];



//==================Useful methods==========================
//this function looks in a specific minor for the channel with the number of channel_number - (Search in a nodelist)
static MessageChannelNode *get_channel(unsigned long minor, unsigned long channel_number){
  MessageChannelNode *head = messageChannelArray[minor];
  while (head != NULL)
  {
    /* code */
    if(head ->num_channel == channel_number){
      return head;
    }
    head = head-> next;
  }
  return NULL; //in a case where there isnt a channel channel_number in the minor
  
}

static MessageChannelNode *add_node(unsigned long minor, unsigned long channel_number){
  MessageChannelNode *head = messageChannelArray[minor]; 


  MessageChannelNode *to_add = (MessageChannelNode *)kmalloc(sizeof(MessageChannelNode), GFP_KERNEL);
  if (!to_add) {
        return NULL; // Memory allocation failure
  } 

  to_add -> num_channel = channel_number;
  to_add -> message = NULL;
  to_add -> len  = 0;

  if(head == NULL){// if it is the first node
    to_add -> next = NULL;
    messageChannelArray[minor] = to_add;
  }
  else{
    MessageChannelNode *second = head-> next;
    to_add -> next = second;
    head-> next = to_add;
  }
  return to_add;

}

//================== DEVICE FUNCTIONS ===========================
static int device_open( struct inode* inode,
                        struct file*  file )
{
  file -> private_data = 0 ; // we will define it inorder to recignize that the channel for the fille hadnt been initiallized yet.
  return SUCCESS;
}

//---------------------------------------------------------------
static int device_release( struct inode* inode,
                           struct file*  file)
{
  return SUCCESS; // =====Nothing to do for release in this implementation======
}

//---------------------------------------------------------------
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read( struct file* file,
                            char __user* buffer,
                            size_t       length,
                            loff_t*      offset )
{
  unsigned long curr_channel;
  int minor_num;
  size_t size_message;
  size_t i;

  MessageChannelNode *message_channel_node;

  curr_channel = (unsigned long)file -> private_data;
  if( curr_channel== 0){ // Which means no channel has been set on the file descriptor, returns -1 and errno is set to EINVAL
      return -EINVAL;
  }

  minor_num = iminor(file-> f_inode);
  if(minor_num < 0 || minor_num > MAX_MINOR){
    return -EINVAL;
  }

  message_channel_node = get_channel(minor_num, curr_channel); // to get the channel node of the file


  //If no message exists on the channel, returns -1 and errno is set to EWOULDBLOCK.
  if(message_channel_node == NULL || message_channel_node -> message == NULL)
  {
    return -EWOULDBLOCK;
  }

  size_message = message_channel_node -> len;


  // if (size_message < 0) {
  //   // This shouldn't happen since len field is size_t which means unegative
  //   return -EINVAL; // or another appropriate error code
  // }

  //If the provided buffer length is too small to hold the last message written on the channel, returns
  //-1 and errno is set to ENOSPC.
  if (size_message > length) {
      return -ENOSPC; // Buffer is too small to hold the message
  }

  //check the validity of user space buffers
  if (buffer == NULL){
        return -EINVAL;
  }

  for (i = 0; i < size_message; i++)
  {
    if(put_user(message_channel_node -> message[i], buffer+i)!= 0){
      return -EFAULT; 
    }
  }

  return size_message;
}

//---------------------------------------------------------------
// a processs which has already opened
// the device file attempts to write to it
static ssize_t device_write( struct file*       file,
                             const char __user* buffer,
                             size_t             length,
                             loff_t*            offset)
{

  unsigned long curr_channel;
  int minor_num;
  MessageChannelNode *message_channel_node;
  char *new_message;
  size_t i;
  
  if(length <= 0 || length > MAX_MESSAGE_LEN){// invalid length of message
    return -EMSGSIZE;
  }


  curr_channel = (unsigned long)file->private_data;
  if( curr_channel== 0){ // Which means no channel has been set on the file descriptor, returns -1 and errno is set to EINVAL
        return -EINVAL;
  }

  minor_num = iminor(file-> f_inode);
  if(minor_num < 0 || minor_num > MAX_MINOR){
    return -EINVAL;
  }

  message_channel_node= get_channel(minor_num, curr_channel); // to get the channel node of the file

  if(message_channel_node == NULL) // if nothing had been written before we need to add the proper node
  {
    message_channel_node = add_node(minor_num, curr_channel);
    if(message_channel_node == NULL)// which means there was allocation problem in add_node
    {
      return -ENOMEM;
    }
  }

  //check the validity of user space buffers
  if (buffer == NULL){
        return -EINVAL;
  }

  new_message = kmalloc(sizeof(char) * length ,GFP_KERNEL);
  if (!new_message) {
        return -ENOMEM;
  }

  for (i = 0; i < length; i++)
  {
    if(get_user(new_message[i], &buffer[i])!= 0){
      kfree(new_message);
      return -EFAULT; // what should we return here?
    }
  }

  //here we got to the end. of the buffer so we copy it to the message
  if((message_channel_node-> message) != NULL) /// for the first time of the channel it doesnt  exists
  {
    kfree(message_channel_node -> message);
  }
  message_channel_node -> len = length;
  message_channel_node -> message = new_message;
  //the old will be erased
  
  return length;
}

//----------------------------------------------------------------
static long device_ioctl( struct   file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long  ioctl_param )
{
  // Switch according to the ioctl called
  if( MSG_SLOT_CHANNEL != ioctl_command_id) {
    //printk( " didnt get MSG_SLOT_CHANNEL command"); - i didnt know if we are allowed to print, i think here it make sense to print but for the safe side i put it here as print
    return -EINVAL;
  }
  //due to disscusion in the forum: https://moodle.tau.ac.il/mod/forum/discuss.php?d=89274 i didnt check ioctl_param > MAX_CHANNEL_NUM
  if(ioctl_param <= 0) {
    //printk( "invalid ioctl_param"); i didnt know if we are allowed to print, i think here it make sense to print but for the safe side i put it here as print
    return -EINVAL;
  }

  file -> private_data = (void *)ioctl_param;
  return SUCCESS;
}

//==================== DEVICE SETUP =============================

// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
  .release        = device_release,
};

//---------------------------------------------------------------
// Initialize the module - Register the character device
static int __init simple_init(void)
{
  int rc, i;
  for (i = 0; i < MAX_MINOR + 1; i++) {
        messageChannelArray[i] = NULL;
  }

  rc = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops);
  if( rc < 0 ) {
    printk( KERN_ERR "%s registraion failed for  %d\n",
                       DEVICE_FILE_NAME, MAJOR_NUM );
    return rc;
  }

  //printk(KERN_INFO "Registered character device with major number %d\n", MAJOR_NUM);
  return SUCCESS;
}

//---------------------------------------------------------------
static void __exit simple_cleanup(void)
{
  // Unregister the device
  // Should always succeed
  int i;
  MessageChannelNode *curr, *temp;
    
    for (i = 0; i < MAX_MINOR + 1; i++) {
        curr = messageChannelArray[i];
        // Free curr message channels associated with this minor number
        while (curr != NULL) {
            temp = curr;
            curr = curr->next;
            if (temp->message != NULL) {
                kfree(temp->message);
            }
            kfree(temp);
        }
        messageChannelArray[i] = NULL;
    }
    
  unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
  //printk(KERN_INFO "message_slot: Unregistered character device\n");
}

//---------------------------------------------------------------
module_init(simple_init);
module_exit(simple_cleanup);

//========================= END OF FILE =========================
