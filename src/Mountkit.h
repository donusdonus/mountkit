#ifndef __MOUNTKIT_H__
#define __MOUNTKIT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


typedef struct {
    uint8_t* value;
    size_t size;
    size_t capacity;
} DataChunk;

typedef struct FileEntry {
    DataChunk name;
    DataChunk content;
    struct FileEntry* next;
} FileEntry;

typedef struct Directory {
    DataChunk name;
    FileEntry* files;
    struct Directory* subdirs;
    struct Directory* next;
} Directory;

class MiniMountFS {
public:
    MiniMountFS(size_t poolSize);
    ~MiniMountFS();

    void createFile(const char* name, const uint8_t* data, size_t size);
    void listFiles();

private:
    uint8_t* pool;
    size_t capacity;
    size_t used;
    Directory* root;
};


#endif