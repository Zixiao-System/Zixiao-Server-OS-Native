#ifndef ZIXIAO_INITRD_H
#define ZIXIAO_INITRD_H

#include <kernel/vfs.h>

/* Create and initialize initrd filesystem */
vfs_node_t* initrd_create(void);

#endif // ZIXIAO_INITRD_H
