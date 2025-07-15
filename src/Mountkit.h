#ifndef __MOUNTKIT_H__
#define __MOUNTKIT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>



enum wcmd {READ,WRITE,APPEND};

typedef struct {
    uint8_t* value;
    size_t size;
    size_t capacity;
} MyFile;

typedef struct MyFolder {
    MyFile name;
    MyFile* files;
    MyFolder* dir;
} MyFolder;

class Mountkit {
public:
    Mountkit(const char *name,size_t size);
   // ~Mountkit();

    MyFile* fopen(const char* name, const uint8_t* data, size_t size,wcmd mode);
    MyFile* fopen(const char* name, const char *text,wcmd mode);
    MyFile* find(const char* name);
    void listFiles();

private:

    MyFolder* cd(const char* name);
    MyFolder* mkdir(MyFolder *src,const char* name);
    MyFolder* rmdir(MyFolder *src,const char* name);
    MyFile* dir(MyFolder *src);
    MyFile* mk(MyFile *src,const char* name);
    MyFile* rm(MyFile *src,const char* name);

    size_t capacity;
    size_t used;
    MyFolder* root;


    /* temporary variable */
    size_t temp_int;
};


#endif