#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <Mountkit.h>

int main() {
    // สร้าง Mountkit object
    Mountkit mk("TestFS", 1024000);
    
    // รันการทดสอบอัตโนมัติ 100 กรณีต่อฟังก์ชัน
    mk.auto_test_and_report();
    
    return 0;
}