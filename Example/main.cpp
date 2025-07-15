#include <stdio.h>
#include <stdint.h>
#include "Mountkit.h"

Mountkit fs("station",10000);

int main()
{
    fs.fopen("station/config/erp.txt","HELLO WORLD",wcmd::APPEND);
    printf("Hello World\r\n");   
    
    while(true);
    return 0 ;
}

