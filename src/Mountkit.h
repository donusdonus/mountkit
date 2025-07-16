#ifndef __mountkit_H__
#define __mountkit_H__

// Build configuration
//#define EMBEDDED_BUILD  // Uncomment สำหรับ embedded build

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define mountkit_DEBUG 

// Forward declarations
typedef struct MyFile MyFile;
typedef struct MyFolder MyFolder;

/**
 * @brief โครงสร้างไฟล์ในระบบ - จัดเก็บข้อมูลไฟล์และ metadata
 */
typedef struct MyFile {
    size_t size;        // ขนาดข้อมูลปัจจุบันในไฟล์ (bytes)
    size_t capacity;    // ขนาดพื้นที่ที่จองไว้สำหรับไฟล์ (bytes)
    uint8_t *name;      // ชื่อไฟล์ (null-terminated string)
    uint8_t *data;      // ข้อมูลของไฟล์ (binary data)
    struct MyFile *next; // pointer ไปยังไฟล์ถัดไปใน directory เดียวกัน
} MyFile;

/**
 * @brief โครงสร้างโฟลเดอร์ในระบบ - จัดเก็บ directories และไฟล์
 */
typedef struct MyFolder {
    char *data;             // ชื่อของ directory
    MyFile *files;          // linked list ของไฟล์ทั้งหมดใน directory นี้
    struct MyFolder *subdir; // pointer ไปยัง subdirectory แรก
    struct MyFolder *dir;    // pointer ไปยัง sibling directory ถัดไป
} MyFolder;

/**
 * @brief คลาส mountkit - ระบบจัดการไฟล์และโฟลเดอร์ในหน่วยความจำ
 * 
 * ระบบไฟล์เสมือนที่ทำงานในหน่วยความจำ สามารถจำลองการทำงานของระบบไฟล์
 * แบบ Unix-like ได้ รองรับทั้ง desktop และ embedded systems
 */
class mountkit 
{

    
public:
    // =================================================================
    // CORE FUNCTIONS - ฟังก์ชันพื้นฐานสำหรับจัดการโฟลเดอร์และไฟล์
    // =================================================================
    
  
    /**
     * @brief สร้าง directory ตาม path ที่กำหนด (คล้าย mkdir -p ใน Linux)
     * @param root pointer ไปยัง root directory
     * @param path path ของ directory ที่ต้องการสร้าง (เช่น "home/user/documents")
     * @return pointer ไปยัง directory ที่สร้างขึ้น หรือ NULL ถ้าไม่สำเร็จ
     * 
     * ตัวอย่างการใช้งาน:
     * MyFolder *docs = mount.mkdir(&root, "home/user/Documents");
     * MyFolder *deep_dir = mount.mkdir(&root, "var/log/apache2");
     */
    MyFolder* mkdir(MyFolder **root, const char *path);
    
    /**
     * @brief สร้างไฟล์ใหม่ใน directory ที่กำหนด
     * @param folder pointer ไปยัง directory ที่จะสร้างไฟล์
     * @param filename ชื่อของไฟล์ที่ต้องการสร้าง
     * @return pointer ไปยังไฟล์ที่สร้างขึ้น หรือ NULL ถ้าไม่สำเร็จ
     * 
     * ตัวอย่างการใช้งาน:
     * MyFile *config = mount.mk(etc_dir, "config.txt");
     * MyFile *readme = mount.mk(docs_dir, "README.md");
     */
    MyFile* mk(MyFolder *folder, const char *filename);
    
    /**
     * @brief เปลี่ยน directory ปัจจุบัน (คล้าย cd ใน Linux)
     * @param root pointer ไปยัง root directory
     * @param path path ของ directory ที่ต้องการไป
     * @return pointer ไปยัง directory ปลายทาง หรือ NULL ถ้าไม่พบ
     * 
     * ตัวอย่างการใช้งาน:
     * MyFolder *current = mount.cd(root, "home/user");
     * MyFolder *parent = mount.cd(root, "../");
     * MyFolder *logs = mount.cd(root, "/var/log");
     */
    MyFolder* cd(MyFolder *root, const char *path);
    
    // =================================================================
    // FILE I/O FUNCTIONS - ฟังก์ชันสำหรับอ่านเขียนไฟล์
    // =================================================================
    
    /**
     * @brief เขียนข้อความลงไฟล์ (string version)
     * @param file pointer ไปยังไฟล์ที่ต้องการเขียน
     * @param str ข้อความที่ต้องการเขียน (null-terminated string)
     * @return 1 ถ้าสำเร็จ, 0 ถ้าไม่สำเร็จ
     * 
     * ตัวอย่างการใช้งาน:
     * mount.write(config_file, "server_port=8080\ndebug=true\n");
     * mount.write(log_file, "Application started successfully");
     */
    int write(MyFile *file, const char *str);
    
    /**
     * @brief เขียนข้อมูล binary ลงไฟล์
     * @param file pointer ไปยังไฟล์ที่ต้องการเขียน
     * @param data pointer ไปยังข้อมูลที่ต้องการเขียน
     * @param size ขนาดของข้อมูลเป็น bytes
     * @return 1 ถ้าสำเร็จ, 0 ถ้าไม่สำเร็จ
     * 
     * ตัวอย่างการใช้งาน:
     * uint8_t image_data[1024] = {...};
     * mount.write(image_file, image_data, sizeof(image_data));
     */
    int write(MyFile *file, uint8_t *data, size_t size);
    
    /**
     * @brief อ่านข้อมูลจากไฟล์
     * @param file pointer ไปยังไฟล์ที่ต้องการอ่าน
     * @param buffer buffer สำหรับเก็บข้อมูลที่อ่านได้
     * @param size จำนวน bytes ที่ต้องการอ่าน
     * @param offset ตำแหน่งเริ่มต้นในไฟล์ (default: 0)
     * @return จำนวน bytes ที่อ่านได้จริง
     * 
     * ตัวอย่างการใช้งาน:
     * uint8_t buffer[256];
     * int bytes_read = mount.read(config_file, buffer, sizeof(buffer));
     * int partial_read = mount.read(large_file, buffer, 100, 1000); // อ่าน 100 bytes เริ่มจากตำแหน่ง 1000
     */
    int read(MyFile *file, uint8_t *buffer, size_t size, size_t offset = 0);
    
    // =================================================================
    // UTILITY FUNCTIONS - ฟังก์ชันเสริมสำหรับจัดการระบบไฟล์
    // =================================================================
    
    /**
     * @brief แสดง path ของไฟล์และโฟลเดอร์ทั้งหมดใน directory tree
     * @param folder pointer ไปยัง directory ที่ต้องการแสดง
     * @param prefix prefix สำหรับ path (เช่น "/home")
     * 
     * ตัวอย่างการใช้งาน:
     * mount.PrintAllPath(root, "");        // แสดงทั้งระบบ
     * mount.PrintAllPath(home_dir, "/home"); // แสดงเฉพาะใน /home
     */
    void PrintAllPath(MyFolder *folder, char *prefix);
    
    /**
     * @brief ลบไฟล์ออกจาก directory
     * @param folder pointer ไปยัง directory ที่มีไฟล์อยู่
     * @param filename ชื่อไฟล์ที่ต้องการลบ
     * @return 1 ถ้าสำเร็จ, 0 ถ้าไม่พบไฟล์หรือไม่สำเร็จ
     * 
     * ตัวอย่างการใช้งาน:
     * mount.rm(temp_dir, "temp_file.tmp");
     * mount.rm(logs_dir, "old_log.txt");
     */
    int rm(MyFolder *folder, const char *filename);
    
    // =================================================================
    // BUILD-SPECIFIC FUNCTIONS - ฟังก์ชันที่แตกต่างกันตาม build type
    // =================================================================
    
    #ifdef EMBEDDED_BUILD
        // ===== EMBEDDED-SPECIFIC FUNCTIONS =====
        // ฟังก์ชันเฉพาะสำหรับ embedded systems (ใช้ทรัพยากรน้อย)
        
        /**
         * @brief ตรวจสอบว่ามี error เกิดขึ้นในระบบหรือไม่
         * @return true ถ้ามี error, false ถ้าปกติ
         * 
         * ตัวอย่างการใช้งาน:
         * if (mount.hasError()) {
         *     // จัดการ error
         *     mount.clearError();
         * }
         */
        bool hasError();
        
        /**
         * @brief เคลียร์ error flag ของระบบ
         * 
         * ตัวอย่างการใช้งาน:
         * mount.clearError(); // รีเซ็ต error state
         */
        void clearError();
        
        /**
         * @brief รัน test พื้นฐานสำหรับ embedded system (ใช้ทรัพยากรน้อย)
         * 
         * ตัวอย่างการใช้งาน:
         * mount.embedded_test(); // ทดสอบระบบแบบ lightweight
         */
        void embedded_test();
        
    #else
        // ===== DESKTOP-ONLY FUNCTIONS =====
        // ฟังก์ชันเฉพาะสำหรับ desktop (feature ครบถ้วน)
        
        /**
         * @brief ลบโฟลเดอร์และ subdirectories ทั้งหมดแบบ recursive
         * @param folder pointer ไปยังโฟลเดอร์ที่ต้องการลบ
         * 
         * ตัวอย่างการใช้งาน:
         * mount.removeFolderRecursive(temp_folder); // ลบทุกอย่างใน temp
         */
        void removeFolderRecursive(MyFolder *folder);
        
        /**
         * @brief ลบ directory ตาม path (คล้าย rmdir ใน Linux)
         * @param root pointer ไปยัง root directory
         * @param path path ของ directory ที่ต้องการลบ
         * 
         * ตัวอย่างการใช้งาน:
         * mount.rmdir(&root, "temp/old_data");
         * mount.rmdir(&root, "/var/tmp");
         */
        void rmdir(MyFolder **root, const char *path);
        
        /**
         * @brief แสดง path ปัจจุบัน (คล้าย pwd ใน Linux)
         * @param folder directory ปัจจุบัน
         * @param root root directory สำหรับอ้างอิง
         * 
         * ตัวอย่างการใช้งาน:
         * mount.pwd(current_dir, root); // แสดง "/home/user/Documents"
         */
        void pwd(MyFolder *folder, MyFolder *root);
        
        /**
         * @brief คัดลอกไฟล์จาก directory หนึ่งไปอีก directory หนึ่ง
         * @param src_folder directory ต้นทาง
         * @param filename ชื่อไฟล์ที่ต้องการคัดลอก
         * @param dst_folder directory ปลายทาง
         * @return 1 ถ้าสำเร็จ, 0 ถ้าไม่สำเร็จ
         * 
         * ตัวอย่างการใช้งาน:
         * mount.cp(home_dir, "document.txt", backup_dir);
         * mount.cp(src_dir, "config.ini", etc_dir);
         */
        int cp(MyFolder *src_folder, const char *filename, MyFolder *dst_folder);
        
        /**
         * @brief ย้ายไฟล์จาก directory หนึ่งไปอีก directory หนึ่ง
         * @param src_folder directory ต้นทาง
         * @param filename ชื่อไฟล์ที่ต้องการย้าย
         * @param dst_folder directory ปลายทาง
         * @return 1 ถ้าสำเร็จ, 0 ถ้าไม่สำเร็จ
         * 
         * ตัวอย่างการใช้งาน:
         * mount.mv(downloads_dir, "file.zip", documents_dir);
         * mount.mv(temp_dir, "processed.dat", archive_dir);
         */
        int mv(MyFolder *src_folder, const char *filename, MyFolder *dst_folder);
        
        /**
         * @brief แสดงรายการไฟล์ใน directory (คล้าย ls ใน Linux)
         * @param folder directory ที่ต้องการแสดง
         * @param show_details แสดงรายละเอียด (size, capacity) หรือไม่
         * 
         * ตัวอย่างการใช้งาน:
         * mount.dir(current_dir);           // แสดงแค่ชื่อไฟล์
         * mount.dir(current_dir, true);     // แสดงรายละเอียดด้วย
         */
        const char * dir(MyFolder *folder, bool show_details = false);
        
        /**
         * @brief เพิ่มข้อความต่อท้ายไฟล์ (string version)
         * @param file pointer ไปยังไฟล์
         * @param str ข้อความที่ต้องการเพิ่ม
         * @return 1 ถ้าสำเร็จ, 0 ถ้าไม่สำเร็จ
         * 
         * ตัวอย่างการใช้งาน:
         * mount.append(log_file, "\n[INFO] Operation completed");
         * mount.append(config_file, "new_setting=value\n");
         */
        int append(MyFile *file, const char *str);
        
        /**
         * @brief เพิ่มข้อมูล binary ต่อท้ายไฟล์
         * @param file pointer ไปยังไฟล์
         * @param data pointer ไปยังข้อมูล
         * @param size ขนาดข้อมูล
         * @return 1 ถ้าสำเร็จ, 0 ถ้าไม่สำเร็จ
         * 
         * ตัวอย่างการใช้งาน:
         * uint8_t extra_data[100] = {...};
         * mount.append(binary_file, extra_data, sizeof(extra_data));
         */
        int append(MyFile *file, uint8_t *data, size_t size);
        
        /**
         * @brief แสดงเนื้อหาของไฟล์ (คล้าย cat ใน Linux)
         * @param file pointer ไปยังไฟล์ที่ต้องการแสดง
         * 
         * ตัวอย่างการใช้งาน:
         * mount.cat(readme_file);    // แสดงเนื้อหา README
         * mount.cat(config_file);    // แสดงการตั้งค่า
         */
        void cat(MyFile *file);
        
        /**
         * @brief รัน unit tests พื้นฐาน
         * 
         * ตัวอย่างการใช้งาน:
         * mount.run_tests(); // ทดสอบฟังก์ชันพื้นฐาน
         */
        void run_tests(void);
        
        /**
         * @brief คำนวณขนาด memory ที่ใช้ทั้งหมดใน directory tree
         * @param folder directory ที่ต้องการคำนวณ
         * @param include_subdirs รวม subdirectories หรือไม่
         * @return ขนาด memory รวมเป็น bytes
         * 
         * ตัวอย่างการใช้งาน:
         * size_t total = mount.calculateFolderCapacity(root, true);
         * size_t this_dir = mount.calculateFolderCapacity(current_dir, false);
         * printf("Total memory: %zu bytes\n", total);
         */
        size_t calculateFolderCapacity(MyFolder *folder, bool include_subdirs = true);
        
    #endif
    
    // =================================================================
    // COMMON FUNCTIONS - ฟังก์ชันที่ใช้ได้ทั้ง desktop และ embedded
    // =================================================================
    
    /**
     * @brief รัน comprehensive test และแสดงรายงานผลการทดสอบ
     * 
     * Desktop: รัน stress test แบบเต็ม (หลายพัน tests)
     * Embedded: รัน basic test แบบ lightweight
     * 
     * ตัวอย่างการใช้งาน:
     * mount.auto_test_and_report(); // ทดสอบระบบอัตโนมัติ
     */
    void auto_test_and_report(void);

private:
   /**
     * @brief สร้างโฟลเดอร์ใหม่ในหน่วยความจำ
     * @param folder pointer ไปยัง pointer ของโฟลเดอร์ที่จะสร้าง
     * @param name ชื่อของโฟลเดอร์ที่ต้องการสร้าง
     * 
     * ตัวอย่างการใช้งาน:
     * MyFolder *new_folder = NULL;
     * mount.createFolder(&new_folder, "Documents");
     */
    void createFolder(MyFolder **folder, const char *name);
    
    /**
     * @brief ลบไฟล์ทั้งหมดใน linked list และคืนหน่วยความจำ
     * @param file pointer ไปยังไฟล์แรกใน linked list
     * 
     * ตัวอย่างการใช้งาน:
     * mount.freeFiles(folder->files);
     */
    void freeFiles(MyFile *file);
    
    /**
     * @brief ลบโฟลเดอร์และเนื้อหาทั้งหมดอย่างถาวร (recursive)
     * @param folder pointer ไปยังโฟลเดอร์ที่ต้องการลบ
     * 
     * ตัวอย่างการใช้งาน:
     * mount.removeFolder(old_folder);
     */
    void removeFolder(MyFolder *folder);
    
    // Private members สำหรับ internal implementation
        /**
     * @brief หา parent directory ของ folder ที่กำหนด
     * @param root root directory สำหรับ search
     * @param target folder ที่ต้องการหา parent
     * @return pointer ไปยัง parent folder หรือ NULL ถ้าไม่พบ
     */
    MyFolder* findParent(MyFolder *root, MyFolder *target);
};

#endif // __mountkit_H__