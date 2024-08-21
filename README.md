IPC-mechanism
Inter-process Communication (IPC) provides a mechanism to exchange data and information across multiple processes.
The project implement a kernel module that provides a new IPC mechanism, called a message slot. A message slot is a character device file through which processes communicate.
A message slot device has multiple message channels active concurrently, which can be used by multiple processes. After opening a message slot device file, a process uses ioctl() to specify the id of the message channel it wants to use. It subsequently uses read()/write() to receive/send messages on the channel. In contrast to pipes, a message channel preserves a message until itis overwritten, so the same message can be read multiple times. In additiod to the kernel module the project contain an interface (message_reader.c and message_sender.c) for the user to receive/send messages from the message slot.

This project was done as a part of Operation Systems course held at Tel-Aviv-University, Spring 2024.
Grade of the project: 100
