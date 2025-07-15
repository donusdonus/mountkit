#include "Mountkit.h"



Mountkit::Mountkit(const char *name,size_t size)
{ 
    mkdir(root,name);

    capacity = size; 
}

MyFile* Mountkit::fopen(const char* name, const uint8_t* data, size_t size,wcmd mode)
{
    return NULL;
}

MyFile* Mountkit::fopen(const char* name, const char *text,wcmd mode)
{
    
    return NULL;
}

MyFolder* Mountkit::mkdir(MyFolder *src,const char* name)
{
    
    return NULL;
}


MyFile* Mountkit::find(const char* name)
{
    
    return NULL;
}