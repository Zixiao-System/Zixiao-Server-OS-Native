#include <kernel/vfs.h>
#include <kernel/string.h>
#include <kernel/console.h>

static vfs_node_t* vfs_root = NULL;

void vfs_init(void) {
    vfs_root = NULL;
    console_printf("VFS initialized\n");
}

void vfs_mount(const char* path, vfs_node_t* node) {
    if (strcmp(path, "/") == 0) {
        vfs_root = node;
        console_printf("Mounted root filesystem\n");
    }
}

static vfs_node_t* vfs_traverse_path(const char* path) {
    if (!vfs_root) {
        return NULL;
    }

    if (strcmp(path, "/") == 0) {
        return vfs_root;
    }

    /* Simple path traversal (only supports root-level files for now) */
    if (path[0] == '/') {
        path++;
    }

    if (vfs_root->finddir) {
        return vfs_root->finddir(vfs_root, path);
    }

    return NULL;
}

vfs_node_t* vfs_open(const char* path, uint32_t flags) {
    vfs_node_t* node = vfs_traverse_path(path);

    if (node && node->open) {
        node->open(node, flags);
    }

    return node;
}

void vfs_close(vfs_node_t* node) {
    if (node && node->close) {
        node->close(node);
    }
}

ssize_t vfs_read(vfs_node_t* node, uint64_t offset, size_t size, uint8_t* buffer) {
    if (node && node->read) {
        return node->read(node, offset, size, buffer);
    }
    return -1;
}

ssize_t vfs_write(vfs_node_t* node, uint64_t offset, size_t size, uint8_t* buffer) {
    if (node && node->write) {
        return node->write(node, offset, size, buffer);
    }
    return -1;
}
