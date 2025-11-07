#include <kernel/vfs.h>
#include <kernel/string.h>
#include <kernel/console.h>
#include <kernel/types.h>

#define INITRD_MAX_FILES 32

typedef struct {
    char name[MAX_FILENAME];
    uint64_t offset;
    uint64_t size;
} initrd_file_header_t;

typedef struct {
    uint32_t num_files;
    initrd_file_header_t files[INITRD_MAX_FILES];
    uint8_t* data_start;
} initrd_fs_t;

static initrd_fs_t initrd_fs;
static vfs_node_t initrd_nodes[INITRD_MAX_FILES];
static vfs_node_t initrd_root;

/* File operations */
static ssize_t initrd_read(vfs_node_t* node, uint64_t offset, size_t size, uint8_t* buffer) {
    if (!node || node->inode >= initrd_fs.num_files) {
        return -1;
    }

    initrd_file_header_t* file = &initrd_fs.files[node->inode];

    if (offset >= file->size) {
        return 0;
    }

    size_t bytes_to_read = size;
    if (offset + size > file->size) {
        bytes_to_read = file->size - offset;
    }

    uint8_t* file_data = initrd_fs.data_start + file->offset;
    memcpy(buffer, file_data + offset, bytes_to_read);

    return bytes_to_read;
}

static vfs_node_t* initrd_readdir(vfs_node_t* node, uint64_t index) {
    if (index >= initrd_fs.num_files) {
        return NULL;
    }
    return &initrd_nodes[index];
}

static vfs_node_t* initrd_finddir(vfs_node_t* node, const char* name) {
    for (uint32_t i = 0; i < initrd_fs.num_files; i++) {
        if (strcmp(initrd_fs.files[i].name, name) == 0) {
            return &initrd_nodes[i];
        }
    }
    return NULL;
}

static void initrd_open(vfs_node_t* node, uint32_t flags) {
    /* Nothing to do for initrd */
    (void)node;
    (void)flags;
}

static void initrd_close(vfs_node_t* node) {
    /* Nothing to do for initrd */
    (void)node;
}

/* Create a simple in-memory initrd with test files */
vfs_node_t* initrd_create(void) {
    /* Initialize initrd filesystem structure */
    memset(&initrd_fs, 0, sizeof(initrd_fs_t));

    /* Create some test files in memory */
    static uint8_t test_data[1024];
    initrd_fs.data_start = test_data;

    /* File 1: welcome.txt */
    initrd_fs.num_files = 0;
    strcpy(initrd_fs.files[0].name, "welcome.txt");
    initrd_fs.files[0].offset = 0;
    const char* welcome_msg = "Welcome to Zixiao OS!\nThis is a test file in the initrd.\n";
    size_t welcome_len = strlen(welcome_msg);
    memcpy(test_data, welcome_msg, welcome_len);
    initrd_fs.files[0].size = welcome_len;
    initrd_fs.num_files++;

    /* File 2: readme.txt */
    strcpy(initrd_fs.files[1].name, "readme.txt");
    initrd_fs.files[1].offset = welcome_len;
    const char* readme_msg = "Zixiao Server OS - Dual Architecture Kernel\n"
                             "Supports: x86_64 and ARM64\n"
                             "Features: VFS, InitRD, Interrupts, Drivers\n";
    size_t readme_len = strlen(readme_msg);
    memcpy(test_data + welcome_len, readme_msg, readme_len);
    initrd_fs.files[1].size = readme_len;
    initrd_fs.num_files++;

    /* Create VFS nodes for each file */
    for (uint32_t i = 0; i < initrd_fs.num_files; i++) {
        memset(&initrd_nodes[i], 0, sizeof(vfs_node_t));
        strcpy(initrd_nodes[i].name, initrd_fs.files[i].name);
        initrd_nodes[i].flags = VFS_FILE;
        initrd_nodes[i].inode = i;
        initrd_nodes[i].size = initrd_fs.files[i].size;
        initrd_nodes[i].read = initrd_read;
        initrd_nodes[i].open = initrd_open;
        initrd_nodes[i].close = initrd_close;
    }

    /* Create root directory node */
    memset(&initrd_root, 0, sizeof(vfs_node_t));
    strcpy(initrd_root.name, "initrd");
    initrd_root.flags = VFS_DIRECTORY;
    initrd_root.readdir = initrd_readdir;
    initrd_root.finddir = initrd_finddir;

    console_printf("InitRD created with %d files\n", initrd_fs.num_files);

    return &initrd_root;
}
