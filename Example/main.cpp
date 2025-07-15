#include <stdio.h>
#include <stdint.h>
#include "Mountkit.h"

Mountkit fs("C",10000);

int main()
{
    fs.mkdir(&fs.drive.folder,"Folder1");
    fs.mkdir(&fs.drive.folder->folder,"Folder2");
    fs.mkdir(&fs.drive.folder->folder->subfolder,"Folder3");
    fs.mkdir(&fs.drive.folder->folder->folder,"Folder7");
    int a = 0;
    while(true);
    return 0 ;
}

