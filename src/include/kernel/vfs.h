#ifndef ZIXIAO_VFS_H
#define ZIXIAO_VFS_H

#include <kernel/types.h>

#define MAX_FILENAME 256
#define MAX_OPEN_FILES 128

// File types
#define VFS_FILE      0x01
#define VFS_DIRECTORY 0x02
#define VFS_CHARDEV   0x03
#define VFS_BLOCKDEV  0x04
#define VFS_PIPE      0x05
#define VFS_SYMLINK   0x06

// File operations
#define O_RDONLY 0x01
#define O_WRONLY 0x02
#define O_RDWR   0x03
#define O_CREAT  0x04
#define O_TRUNC  0x08

typedef struct vfs_node {
    char name[MAX_FILENAME];
    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t inode;
    uint64_t size;
    uint64_t impl;  // Implementation-specific data

    // File operations
    ssize_t (*read)(struct vfs_node* node, uint64_t offset, size_t size, uint8_t* buffer);
    ssize_t (*write)(struct vfs_node* node, uint64_t offset, size_t size, uint8_t* buffer);
    void (*open)(struct vfs_node* node, uint32_t flags);
    void (*close)(struct vfs_node* node);
    struct vfs_node* (*readdir)(struct vfs_node* node, uint64_t index);
    struct vfs_node* (*finddir)(struct vfs_node* node, const char* name);
} vfs_node_t;

// VFS API
void vfs_init(void);
void vfs_mount(const char* path, vfs_node_t* node);
vfs_node_t* vfs_open(const char* path, uint32_t flags);
void vfs_close(vfs_node_t* node);
ssize_t vfs_read(vfs_node_t* node, uint64_t offset, size_t size, uint8_t* buffer);
ssize_t vfs_write(vfs_node_t* node, uint64_t offset, size_t size, uint8_t* buffer);

#endif // ZIXIAO_VFS_H