#include "Mountkit.h"

// Conditional includes และ debug control
#ifdef EMBEDDED_BUILD
    // Minimal includes สำหรับ embedded
    #include <stdlib.h>
    #include <string.h>
    #include <stdint.h>
    
    // Simple error handling แทน exit()
    static volatile int system_error_flag = 0;
    #define SET_ERROR_FLAG() (system_error_flag = 1)
    #define CHECK_ERROR_FLAG() (system_error_flag)
    #define CLEAR_ERROR_FLAG() (system_error_flag = 0)
    
#else
    // Desktop includes
    #include <cstring>
    #include <cstdlib>
    #include <cassert>
    #include <time.h>
    
    // Desktop error handling
    #define SET_ERROR_FLAG() 
    #define CHECK_ERROR_FLAG() (0)
    #define CLEAR_ERROR_FLAG()
#endif

// Debug control macros - ใช้ LIB_DEBUG เท่านั้น
#ifdef LIB_DEBUG
    #define DEBUG_PRINTF(...) printf(__VA_ARGS__)
    #define DEBUG_FPRINTF(stream, ...) fprintf(stream, __VA_ARGS__)
#else
    #define DEBUG_PRINTF(...) // Disable debug printf
    #define DEBUG_FPRINTF(stream, ...) // Disable debug fprintf
#endif

// แก้ไขฟังก์ชัน write ให้ใช้ debug control ที่สอดคล้องกัน
int Mountkit::write(MyFile *file, const char *str) {
    if (!file || !str) {
        #ifdef LIB_DEBUG
            printf("Error: Invalid file or string\n");
        #endif
        return 0;
    }
    
    size_t str_len = strlen(str);
    return write(file, (uint8_t*)str, str_len);
}

int Mountkit::write(MyFile *file, uint8_t *data, size_t size) {
    if (!file || !data || size == 0) {
        #ifdef LIB_DEBUG
            printf("Error: Invalid parameters\n");
        #endif
        return 0;
    }
    
    #ifdef LIB_DEBUG
        printf("Writing %zu bytes to file '%s'\n", size, (char*)file->name);
    #endif
    
    // ตรวจสอบว่าขนาดที่จะเขียนเกิน capacity หรือไม่
    if (size > file->capacity) {
        #ifdef EMBEDDED_BUILD
            // สำหรับ embedded: ไม่ขยาย buffer ถ้าเกิน capacity
            SET_ERROR_FLAG();
            return 0;
        #else
            // สำหรับ desktop: ขยาย buffer ได้
            size_t new_capacity = file->capacity;
            while (new_capacity < size) {
                new_capacity *= 2;
            }
            
            uint8_t *new_data = (uint8_t*)realloc(file->data, new_capacity);
            if (!new_data) {
                SET_ERROR_FLAG();
                return 0;
            }
            
            file->data = new_data;
            file->capacity = new_capacity;
            memset(file->data + file->size, 0, file->capacity - file->size);
        #endif
    }
    
    // เขียนข้อมูลลงไฟล์
    memcpy(file->data, data, size);
    file->size = size;
    
    #ifdef LIB_DEBUG
        printf("Write successful: %zu bytes written\n", size);
    #endif
    
    return 1;
}

// แก้ไขฟังก์ชัน append
int Mountkit::append(MyFile *file, const char *str) {
    if (!file || !str) {
        #ifdef LIB_DEBUG
            printf("Error: Invalid file or string\n");
        #endif
        return 0;
    }
    
    size_t str_len = strlen(str);
    return append(file, (uint8_t*)str, str_len);
}

int Mountkit::append(MyFile *file, uint8_t *data, size_t size) {
    if (!file || !data || size == 0) {
        #ifdef LIB_DEBUG
            printf("Error: Invalid parameters\n");
        #endif
        return 0;
    }
    
    size_t new_size = file->size + size;
    
    #ifdef LIB_DEBUG
        printf("Appending %zu bytes to file '%s'\n", size, (char*)file->name);
        printf("   Current size: %zu, New size will be: %zu\n", file->size, new_size);
    #endif
    
    // ตรวจสอบว่าต้องขยาย capacity หรือไม่
    if (new_size > file->capacity) {
        #ifdef LIB_DEBUG
            printf("New size (%zu) exceeds capacity (%zu), reallocating...\n", new_size, file->capacity);
        #endif
        
        // คำนวณ capacity ใหม่
        size_t new_capacity = file->capacity;
        while (new_capacity < new_size) {
            new_capacity *= 2;
        }
        
        #ifdef LIB_DEBUG
            printf("Expanding capacity from %zu to %zu bytes\n", file->capacity, new_capacity);
        #endif
        
        // ขยาย buffer
        uint8_t *new_data = (uint8_t*)realloc(file->data, new_capacity);
        if (!new_data) {
            #ifdef LIB_DEBUG
                printf("Error: Failed to reallocate memory for %zu bytes\n", new_capacity);
            #endif
            return 0;
        }
        
        file->data = new_data;
        file->capacity = new_capacity;
        
        #ifdef LIB_DEBUG
            printf("Memory reallocation successful\n");
        #endif
    }
    
    // เขียนข้อมูลต่อท้าย
    memcpy(file->data + file->size, data, size);
    file->size = new_size;
    
    #ifdef LIB_DEBUG
        printf("Append successful: %zu bytes added\n", size);
        printf("   New file size: %zu/%zu bytes (%.1f%% used)\n", 
               file->size, file->capacity, (file->size * 100.0) / file->capacity);
    #endif
    
    return 1;
}

// แก้ไขฟังก์ชัน read
int Mountkit::read(MyFile *file, uint8_t *buffer, size_t size, size_t offset) {
    if (!file || !buffer) {
        #ifdef LIB_DEBUG
            printf("Error: Invalid file or buffer\n");
        #endif
        return 0;
    }
    
    if (offset >= file->size) {
        #ifdef LIB_DEBUG
            printf("Error: Offset (%zu) exceeds file size (%zu)\n", offset, file->size);
        #endif
        return 0;
    }
    
    size_t available = file->size - offset;
    size_t to_read = (size > available) ? available : size;
    
    memcpy(buffer, file->data + offset, to_read);
    
    #ifdef LIB_DEBUG
        printf("Read %zu bytes from file '%s' at offset %zu\n", to_read, (char*)file->name, offset);
    #endif
    
    return (int)to_read;
}

// แก้ไขฟังก์ชัน cat - User output ไม่ควรใช้ debug control
void Mountkit::cat(MyFile *file) {
    if (!file) {
        printf("Error: Invalid file\n");
        return;
    }
    
    // cat output should always be visible (not debug-controlled)
    printf("Content of file '%s' (%zu bytes):\n", (char*)file->name, file->size);
    printf("──────────────────────────────────────\n");
    
    if (file->size == 0) {
        printf("(empty file)\n");
        return;
    }
    
    // แสดงเนื้อหาเป็น text
    bool is_text = true;
    for (size_t i = 0; i < file->size; ++i) {
        if (file->data[i] < 32 && file->data[i] != 9 && file->data[i] != 10 && file->data[i] != 13) {
            is_text = false;
            break;
        }
    }
    
    if (is_text) {
        for (size_t i = 0; i < file->size; ++i) {
            putchar(file->data[i]);
        }
        if (file->data[file->size - 1] != '\n') {
            putchar('\n');
        }
    } else {
        // hex dump
        for (size_t i = 0; i < file->size; ++i) {
            if (i % 16 == 0) {
                printf("%04zx: ", i);
            }
            printf("%02x ", file->data[i]);
            if (i % 16 == 15 || i == file->size - 1) {
                printf("\n");
            }
        }
    }
    
    printf("──────────────────────────────────────\n");
    printf("File info: %zu/%zu bytes (%.1f%% used)\n", 
           file->size, file->capacity, (file->size * 100.0) / file->capacity);
}

// แก้ไขฟังก์ชัน createFolder ให้ใช้ debug control
void Mountkit::createFolder(MyFolder **folder, const char *name) {
    MyFolder *newFolder = (MyFolder*)malloc(sizeof(MyFolder));
    if (!newFolder) {
        #ifdef LIB_DEBUG
            fprintf(stderr, "malloc failed for MyFolder\n");
        #endif
        *folder = NULL;
        return;
    }
    newFolder->data = strdup(name);
    if (!newFolder->data) {
        #ifdef LIB_DEBUG
            fprintf(stderr, "malloc failed for MyFolder->data\n");
        #endif
        free(newFolder);
        *folder = NULL;
        return;
    }
    newFolder->files = NULL;
    newFolder->subdir = NULL;
    newFolder->dir = NULL;
    *folder = newFolder;
}

// free memory ของไฟล์ในโฟลเดอร์
void Mountkit::freeFiles(MyFile *file) {
    while (file) {
        MyFile *next = file->next;
        free(file->name);
        free(file->data);
        free(file);
        file = next;
    }
}

// ลบโฟลเดอร์และลูกทั้งหมด
void Mountkit::removeFolder(MyFolder *folder) {
    if (!folder) return;
    freeFiles(folder->files);
    removeFolder(folder->subdir);
    removeFolder(folder->dir);
    free(folder->data);
    free(folder);
}

// ลบโฟลเดอร์และลูกทั้งหมด (เวอร์ชันที่ใช้ recursive)
void Mountkit::removeFolderRecursive(MyFolder *folder) {
    if (!folder) return;
    // ลบไฟล์ในโฟลเดอร์
    freeFiles(folder->files);
    folder->files = NULL;
    // ลบโฟลเดอร์ย่อย
    if (folder->subdir) {
        removeFolderRecursive(folder->subdir);
        folder->subdir = NULL;
    }
    // ลบโฟลเดอร์ปัจจุบัน
    free(folder->data);
    free(folder);
}

// ลบโฟลเดอร์ตาม path
void Mountkit::rmdir(MyFolder **root, const char *path) {
    char buf[256];
    strncpy(buf, path, sizeof(buf)); buf[sizeof(buf)-1] = '\0';
    char *token = strtok(buf, "/");
    MyFolder **current = root, **prev = NULL;
    while (token && *current) {
        MyFolder *iter = *current;
        prev = current;
        while (iter && strcmp(iter->data, token) != 0) {
            prev = &iter->dir;
            iter = iter->dir;
        }
        if (!iter) return;
        current = &iter->subdir;
        token = strtok(NULL, "/");
        if (!token) {
            *prev = iter->dir;
            iter->dir = NULL;
            removeFolder(iter);
            return;
        }
    }
}

// mkdir: สร้าง path และ return pointer ไปยัง Folder สุดท้าย
MyFolder* Mountkit::mkdir(MyFolder **root, const char *path) {
    if (!root || !path) {
        SET_ERROR_FLAG();
        return NULL;
    }
    
    char buf[256];
    strncpy(buf, path, sizeof(buf)); buf[sizeof(buf)-1] = '\0';
    char *token = strtok(buf, "/");
    MyFolder **current = root, *last = NULL;
    while (token) {
        MyFolder *iter = *current;
        MyFolder **prev = current;
        while (iter && strcmp(iter->data, token) != 0) {
            prev = &iter->dir;
            iter = iter->dir;
        }
        if (iter) {
            last = iter;
            current = &iter->subdir;
        } else {
            MyFolder *new_folder = NULL;
            createFolder(&new_folder, token);
            
            // ตรวจสอบว่า createFolder สำเร็จหรือไม่
            if (!new_folder) {
                SET_ERROR_FLAG();
                return NULL; // แทน crash
            }
            
            *prev = new_folder;
            last = new_folder;
            current = &(new_folder->subdir);
        }
        token = strtok(NULL, "/");
    }
    return last;
}

// mk: สร้างไฟล์ใหม่ในโฟลเดอร์ (ไม่ซ้ำชื่อ) พร้อมกำหนด capacity
MyFile* Mountkit::mk(MyFolder *folder, const char *filename) {
    if (!folder || !filename) {
        SET_ERROR_FLAG();
        return NULL;
    }
    
    // ตรวจสอบว่ามีไฟล์ชื่อเดียวกันอยู่แล้วหรือไม่
    for (MyFile *cur = folder->files; cur; cur = cur->next) {
        if (strcmp((char*)cur->name, filename) == 0) {
            // ถ้ามีไฟล์อยู่แล้ว ให้ return pointer เดิม
            return cur;
        }
    }
    
    // สร้างไฟล์ใหม่
    MyFile *file = (MyFile*)malloc(sizeof(MyFile));
    if (!file) {
        SET_ERROR_FLAG();
        return NULL; // แทน exit(1)
    }
    
    file->name = (uint8_t*)strdup(filename);
    if (!file->name) {
        free(file);
        SET_ERROR_FLAG();
        return NULL; // แทน exit(1)
    }
    
    // กำหนด capacity เริ่มต้น (ลดขนาดสำหรับ embedded)
    #ifdef EMBEDDED_BUILD
        file->capacity = 512; // 512 bytes สำหรับ embedded
    #else
        file->capacity = 4096; // 4KB สำหรับ desktop
    #endif
    
    file->data = (uint8_t*)malloc(file->capacity);
    if (!file->data) {
        free(file->name);
        free(file);
        SET_ERROR_FLAG();
        return NULL; // แทน exit(1)
    }
    
    file->size = 0;
    memset(file->data, 0, file->capacity);
    file->next = folder->files;
    folder->files = file;
    
    return file;
}

// cd: เดิน path และคืน pointer ของตำแหน่งนั้น (รองรับ .. และ .)
MyFolder* Mountkit::cd(MyFolder *root, const char *path) {
    if (!root || !path) return NULL;
    
    char buf[256];
    strncpy(buf, path, sizeof(buf)); 
    buf[sizeof(buf)-1] = '\0';
    
    char *token = strtok(buf, "/");
    MyFolder *current = root;
    
    // ถ้า token แรกตรงกับ root name ให้ข้ามไป
    if (token && strcmp(current->data, token) == 0) {
        token = strtok(NULL, "/");
    }
    
    while (token && current) {
        #ifdef LIB_DEBUG
            printf("Processing token: '%s'\n", token);
        #endif
        
        // จัดการ special cases
        if (strcmp(token, ".") == 0) {
            // Current directory - ไม่ต้องทำอะไร
            #ifdef LIB_DEBUG
                printf("   [DEBUG] Staying in current directory: %s\n", current->data);
            #endif
        } 
        else if (strcmp(token, "..") == 0) {
            // Parent directory - ต้องหา parent ของ current
            #ifdef LIB_DEBUG
                printf("   [DEBUG] Going to parent directory from: %s\n", current->data);
            #endif
            
            MyFolder *parent = findParent(root, current);
            if (parent) {
                current = parent;
                #ifdef LIB_DEBUG
                    printf("   [DEBUG] Found parent: %s\n", current->data);
                #endif
            } else {
                // ถ้าไม่พบ parent (อยู่ที่ root แล้ว) ให้อยู่ที่เดิม
                #ifdef LIB_DEBUG
                    printf("   [DEBUG] Already at root, staying at: %s\n", current->data);
                #endif
            }
        } 
        else {
            // Normal directory name
            MyFolder *iter = current->subdir;
            while (iter && strcmp(iter->data, token) != 0) {
                iter = iter->dir;
            }
            
            if (!iter) {
                #ifdef LIB_DEBUG
                    printf("   [DEBUG] Directory '%s' not found in %s\n", token, current->data);
                #endif
                return NULL; // Directory not found
            }
            
            current = iter;
            #ifdef LIB_DEBUG
                printf("   [DEBUG] Moved to directory: %s\n", current->data);
            #endif
        }
        
        token = strtok(NULL, "/");
    }
    
    return current;
}

// Helper function: หา parent directory ของ target
MyFolder* Mountkit::findParent(MyFolder *root, MyFolder *target) {
    if (!root || !target || root == target) {
        return NULL; // ไม่พบหรือ target คือ root
    }
    
    // ตรวจสอบว่า target อยู่ใน subdirectory ของ root หรือไม่
    MyFolder *child = root->subdir;
    while (child) {
        if (child == target) {
            return root; // พบ! root คือ parent ของ target
        }
        child = child->dir;
    }
    
    // ถ้าไม่พบใน level นี้ ให้ search ใน subdirectories แบบ recursive
    child = root->subdir;
    while (child) {
        MyFolder *found = findParent(child, target);
        if (found) {
            return found;
        }
        child = child->dir;
    }
    
    return NULL; // ไม่พบ parent
}

// pwd: แสดง path ปัจจุบัน
void Mountkit::pwd(MyFolder *folder, MyFolder *root) {
    if (!folder) return;
    const MyFolder *stack[128];
    int top = 0;
    MyFolder *cur = folder;
    while (cur && cur != root) {
        stack[top++] = cur;
        MyFolder *parent = root, *found = NULL;
        while (parent) {
            MyFolder *child = parent->subdir;
            while (child) {
                if (child == cur) { found = parent; break; }
                child = child->dir;
            }
            if (found) break;
            parent = parent->dir;
        }
        cur = found;
    }
    if (root) stack[top++] = root;
    for (int i = top - 1; i >= 0; --i)
        printf("/%s", stack[i]->data);
    printf("\n");
}

// PrintAllPath: แสดง path ของทุกโฟลเดอร์
void Mountkit::PrintAllPath(MyFolder *folder, char *prefix) {
    if (!folder) return;
    char path[256];
    if (prefix[0] != '\0')
        snprintf(path, sizeof(path), "%s/%s", prefix, folder->data);
    else
        snprintf(path, sizeof(path), "%s", folder->data);
    printf("%s\n", path);
    PrintAllPath(folder->subdir, path);
    PrintAllPath(folder->dir, prefix);
}

// rm: ลบไฟล์ในโฟลเดอร์ตามชื่อไฟล์
int Mountkit::rm(MyFolder *folder, const char *filename) {
    if (!folder || !filename) return 0;
    MyFile **cur = &folder->files;
    while (*cur) {
        if (strcmp((char*)(*cur)->name, filename) == 0) {
            MyFile *to_delete = *cur;
            *cur = to_delete->next;
            free(to_delete->name);
            free(to_delete->data);
            free(to_delete);
            return 1; // success
        }
        cur = &((*cur)->next);
    }
    return 0; // not found
}

// cp: คัดลอกไฟล์ในโฟลเดอร์ src ไปยังโฟลเดอร์ dst (ชื่อไฟล์เดียวกัน)
int Mountkit::cp(MyFolder *src_folder, const char *filename, MyFolder *dst_folder) {
    if (!src_folder || !dst_folder || !filename) return 0;
    // หาไฟล์ต้นทาง
    MyFile *src = src_folder->files;
    while (src && strcmp((char*)src->name, filename) != 0)
        src = src->next;
    if (!src) return 0; // ไม่พบไฟล์ต้นทาง

    // ตรวจสอบว่าปลายทางมีไฟล์ชื่อเดียวกันอยู่แล้วหรือไม่
    MyFile *dst = dst_folder->files;
    while (dst) {
        if (strcmp((char*)dst->name, filename) == 0)
            return 0; // ไม่คัดลอกซ้ำ
        dst = dst->next;
    }

    // สร้างไฟล์ใหม่ในปลายทาง
    MyFile *newfile = mk(dst_folder, filename);
    if (!newfile) return 0;

    // คัดลอกข้อมูล
    if (src->size > newfile->capacity) {
        // ขยาย buffer ถ้าจำเป็น
        uint8_t *newdata = (uint8_t*)realloc(newfile->data, src->size);
        if (!newdata) {
            // ถ้า realloc fail ให้ลบไฟล์ที่สร้างใหม่
            rm(dst_folder, filename);
            return 0;
        }
        newfile->data = newdata;
        newfile->capacity = src->size;
    }
    memcpy(newfile->data, src->data, src->size);
    newfile->size = src->size;
    return 1; // success
}

// mv: ย้ายไฟล์จากโฟลเดอร์ src ไปยังโฟลเดอร์ dst (ชื่อไฟล์เดียวกัน)
int Mountkit::mv(MyFolder *src_folder, const char *filename, MyFolder *dst_folder) {
    if (!src_folder || !dst_folder || !filename) return 0;

    // หาไฟล์ต้นทาง (และ pointer ไปยัง pointer ของมัน)
    MyFile **cur = &src_folder->files;
    while (*cur && strcmp((char*)(*cur)->name, filename) != 0) {
        cur = &((*cur)->next);
    }
    if (!*cur) return 0; // ไม่พบไฟล์ต้นทาง

    // ตรวจสอบว่าปลายทางมีไฟล์ชื่อเดียวกันอยู่แล้วหรือไม่
    for (MyFile *dst = dst_folder->files; dst; dst = dst->next) {
        if (strcmp((char*)dst->name, filename) == 0)
            return 0; // ไม่ย้ายซ้ำ
    }

    // ถอดไฟล์ออกจาก src_folder
    MyFile *moving = *cur;
    *cur = moving->next;

    // ใส่ไฟล์เข้า dst_folder
    moving->next = dst_folder->files;
    dst_folder->files = moving;

    return 1; // success
}

// dir: แสดงรายการไฟล์และโฟลเดอร์ในไดเรกทอรีปัจจุบัน
const char* Mountkit::dir(MyFolder *folder, bool show_details) {
    // Static buffer สำหรับเก็บผลลัพธ์
    static char result_buffer[8192]; // 8KB buffer
    result_buffer[0] = '\0'; // Clear buffer
    
    if (!folder) {
        snprintf(result_buffer, sizeof(result_buffer), "Error: Invalid folder\n");
        return result_buffer;
    }
    
    char temp_buffer[512];
    int folder_count = 0;
    int file_count = 0;
    size_t total_size = 0;
    
    // Header
    snprintf(temp_buffer, sizeof(temp_buffer), 
             "\nDirectory listing for: %s\n"
             "=====================================\n", 
             folder->data);
    strncat(result_buffer, temp_buffer, sizeof(result_buffer) - strlen(result_buffer) - 1);
    
    // แสดงรายการโฟลเดอร์ย่อย
    if (folder->subdir) {
        strncat(result_buffer, "\nFOLDERS:\n--------\n", 
                sizeof(result_buffer) - strlen(result_buffer) - 1);
        
        MyFolder *current_folder = folder->subdir;
        while (current_folder) {
            if (show_details) {
                snprintf(temp_buffer, sizeof(temp_buffer), 
                         "  [DIR]  %-20s  <FOLDER>\n", current_folder->data);
            } else {
                snprintf(temp_buffer, sizeof(temp_buffer), 
                         "  [DIR]  %s\n", current_folder->data);
            }
            strncat(result_buffer, temp_buffer, 
                    sizeof(result_buffer) - strlen(result_buffer) - 1);
            folder_count++;
            current_folder = current_folder->dir;
        }
    }
    
    // แสดงรายการไฟล์
    if (folder->files) {
        strncat(result_buffer, "\nFILES:\n------\n", 
                sizeof(result_buffer) - strlen(result_buffer) - 1);
        
        MyFile *current_file = folder->files;
        while (current_file) {
            if (show_details) {
                // แสดงรายละเอียด: ชื่อไฟล์, ขนาด, capacity
                snprintf(temp_buffer, sizeof(temp_buffer), 
                         "  [FILE] %-20s  %6zu bytes  (capacity: %zu)\n", 
                         (char*)current_file->name, 
                         current_file->size,
                         current_file->capacity);
            } else {
                // แสดงแบบง่าย
                snprintf(temp_buffer, sizeof(temp_buffer), 
                         "  [FILE] %s\n", (char*)current_file->name);
            }
            strncat(result_buffer, temp_buffer, 
                    sizeof(result_buffer) - strlen(result_buffer) - 1);
            
            file_count++;
            total_size += current_file->size;
            current_file = current_file->next;
        }
    }
    
    // แสดงสรุป
    strncat(result_buffer, "\n=====================================\n", 
            sizeof(result_buffer) - strlen(result_buffer) - 1);
    
    snprintf(temp_buffer, sizeof(temp_buffer), 
             "Summary:\n"
             "  Folders: %d\n"
             "  Files:   %d\n", 
             folder_count, file_count);
    strncat(result_buffer, temp_buffer, 
            sizeof(result_buffer) - strlen(result_buffer) - 1);
    
    if (show_details) {
        snprintf(temp_buffer, sizeof(temp_buffer), 
                 "  Total file size: %zu bytes\n", total_size);
        strncat(result_buffer, temp_buffer, 
                sizeof(result_buffer) - strlen(result_buffer) - 1);
        
        // แสดงขนาดในรูปแบบที่อ่านง่าย
        if (total_size >= 1024 * 1024) {
            snprintf(temp_buffer, sizeof(temp_buffer), 
                     "  Total file size: %.2f MB\n", total_size / (1024.0 * 1024.0));
            strncat(result_buffer, temp_buffer, 
                    sizeof(result_buffer) - strlen(result_buffer) - 1);
        } else if (total_size >= 1024) {
            snprintf(temp_buffer, sizeof(temp_buffer), 
                     "  Total file size: %.2f KB\n", total_size / 1024.0);
            strncat(result_buffer, temp_buffer, 
                    sizeof(result_buffer) - strlen(result_buffer) - 1);
        }
    }
    
    snprintf(temp_buffer, sizeof(temp_buffer), 
             "  Total items: %d\n\n", folder_count + file_count);
    strncat(result_buffer, temp_buffer, 
            sizeof(result_buffer) - strlen(result_buffer) - 1);
    
    return result_buffer;
}


// ฟังก์ชันสำหรับทดสอบอัตโนมัติ
void Mountkit::run_tests() {
    MyFolder *root = NULL;

    // Test 1: mkdir และ cd
    MyFolder *cfg = mkdir(&root, "root/cfg");
    assert(cfg && strcmp(cfg->data, "cfg") == 0);
    MyFolder *usr = mkdir(&root, "root/usr");
    assert(usr && strcmp(usr->data, "usr") == 0);

    // Test 2: mkdir ซ้อนหลายชั้น
    MyFolder *toon = mkdir(&root, "root/usr/toon");
    assert(toon && strcmp(toon->data, "toon") == 0);

    // Test 3: cd ไปยัง path ที่มีอยู่
    MyFolder *cd_toon = cd(root, "usr/toon");
    assert(cd_toon == toon);

    // Test 4: mk สร้างไฟล์ใหม่
    MyFile *f1 = mk(toon, "fileA.txt");
    assert(f1 && strcmp((char*)f1->name, "fileA.txt") == 0);

    // Test 5: mk ซ้ำชื่อเดิม ต้อง return pointer เดิม
    MyFile *f2 = mk(toon, "fileA.txt");
    assert(f2 == f1);

    // Test 6: cd ไป path ที่ไม่มี ต้องได้ NULL
    MyFolder *notfound = cd(root, "usr/none");
    assert(notfound == NULL);

    // Test 7: rmdir แล้ว cd ต้องได้ NULL
    rmdir(&root, "root/usr/toon");
    MyFolder *cd_after_rm = cd(root, "usr/toon");
    assert(cd_after_rm == NULL);

    // Test 8: mkdir หลัง rmdir ต้องสร้างใหม่ได้
    MyFolder *toon2 = mkdir(&root, "root/usr/toon");
    assert(toon2 && strcmp(toon2->data, "toon") == 0);

    // Test 9: mk หลัง rmdir
    MyFile *f3 = mk(toon2, "fileB.txt");
    assert(f3 && strcmp((char*)f3->name, "fileB.txt") == 0);

    // Test 10: removeFolder ทั้งหมด
    removeFolder(root);

    printf("All tests passed!\n");
}

// Ultimate Automated Test และ Report System - HARDCORE VERSION
void Mountkit::auto_test_and_report() { 
    #ifdef EMBEDDED_BUILD
        // ใช้ embedded test แทน
        embedded_test();
        return;
    #endif
    
    printf("=================================================================\n");
    printf("                MOUNTKIT COMPREHENSIVE STRESS TEST               \n");
    printf("                      HARDCORE MODE ACTIVATED                    \n");
    printf("=================================================================\n\n");
    
    clock_t global_start = clock();
    
    int total_tests = 0;
    int total_pass = 0;
    int total_fail = 0;
    int critical_failures = 0;
    
    MyFolder *test_root = NULL;
    
    // =================================================================
    // TEST CATEGORY 1: FOLDER OPERATIONS STRESS TEST
    // =================================================================
    printf("CATEGORY 1: FOLDER OPERATIONS STRESS TEST (200 tests)\n");
    printf("--------------------------------------------------------------\n");
    
    clock_t cat1_start = clock();
    int cat1_pass = 0, cat1_fail = 0;
    
    // Test 1.1: Massive folder creation
    printf("Test 1.1: Creating 100 nested directories...\n");
    for (int i = 0; i < 100; ++i) {
        char path[128];
        snprintf(path, sizeof(path), "stress/level_%03d/subdir_%03d", i/10, i%10);
        
        MyFolder *result = mkdir(&test_root, path);
        total_tests++;
        
        if (result) {
            cat1_pass++;
            if (i % 10 == 0) printf("   [PASS] Level %d created successfully\n", i);
        } else {
            cat1_fail++;
            critical_failures++;
            printf("   [FAIL] CRITICAL: Failed at level %d\n", i);
        }
        
        // ตรวจสอบ error flag
        if (CHECK_ERROR_FLAG()) {
            printf("[ERROR] Error flag detected - stopping test\n");
            break;
        }
    }
    
    // Test 1.2: Path navigation test
    printf("Test 1.2: Path navigation test (50 tests)...\n");
    for (int i = 0; i < 50; ++i) {
        char search_path[128];
        snprintf(search_path, sizeof(search_path), "level_%03d/subdir_%03d", i/5, i%10);
        
        MyFolder *found = cd(test_root, search_path);
        total_tests++;
        
        if (found) {
            cat1_pass++;
            if (i % 5 == 0) printf("   [PASS] Navigation to %s successful\n", search_path);
        } else {
            cat1_fail++;
            printf("   [WARN] Failed to navigate to %s\n", search_path);
        }
    }
    
    // Test 1.3: Directory deletion test
    printf("Test 1.3: Directory deletion test (50 tests)...\n");
    for (int i = 0; i < 50; ++i) {
        char path[128];
        snprintf(path, sizeof(path), "stress/level_%03d", i);
        rmdir(&test_root, path);
        total_tests++;
        
        // Verify deletion
        MyFolder *should_be_null = cd(test_root, path);
        if (should_be_null == NULL) {
            cat1_pass++;
            if (i % 10 == 0) printf("   [PASS] Directory %s deleted successfully\n", path);
        } else {
            cat1_fail++;
            printf("   [FAIL] Directory %s still exists after deletion!\n", path);
        }
    }
    
    clock_t cat1_end = clock();
    double cat1_time = ((double)(cat1_end - cat1_start)) / CLOCKS_PER_SEC;
    
    printf("\nCATEGORY 1 RESULTS:\n");
    printf("   [PASS] PASSED: %d/%d (%.1f%%)\n", cat1_pass, cat1_pass + cat1_fail, 
           cat1_pass + cat1_fail > 0 ? (cat1_pass * 100.0) / (cat1_pass + cat1_fail) : 0);
    printf("   [FAIL] FAILED: %d\n", cat1_fail);
    printf("   [TIME] EXECUTION TIME: %.3f seconds\n", cat1_time);
    printf("   [RATE] THROUGHPUT: %.0f operations/second\n\n", (cat1_pass + cat1_fail) / cat1_time);
    
    total_pass += cat1_pass;
    total_fail += cat1_fail;
    
    // =================================================================
    // TEST CATEGORY 2: FILE OPERATIONS STRESS TEST
    // =================================================================
    printf("CATEGORY 2: FILE OPERATIONS STRESS TEST (300 tests)\n");
    printf("--------------------------------------------------------------\n");
    
    clock_t cat2_start = clock();
    int cat2_pass = 0, cat2_fail = 0;
    
    MyFolder *file_test_dir = mkdir(&test_root, "file_test");
    
    // Test 2.1: Mass file creation
    printf("Test 2.1: Creating 100 files with various sizes...\n");
    MyFile *created_files[100];
    
    for (int i = 0; i < 100; ++i) {
        char filename[64];
        snprintf(filename, sizeof(filename), "test_file_%04d.dat", i);
        
        MyFile *file = mk(file_test_dir, filename);
        created_files[i] = file;
        total_tests++;
        
        if (file) {
            cat2_pass++;
            
            // Write test data
            char test_data[256];
            int data_size = 10 + (i % 246); // 10-255 bytes
            for (int j = 0; j < data_size; ++j) {
                test_data[j] = 'A' + (i + j) % 26;
            }
            test_data[data_size] = '\0';
            
            if (write(file, (uint8_t*)test_data, data_size)) {
                if (i % 10 == 0) printf("   [PASS] Created file %s (%d bytes)\n", filename, data_size);
            } else {
                printf("   [WARN] File created but write failed: %s\n", filename);
            }
        } else {
            cat2_fail++;
            critical_failures++;
            printf("   [FAIL] CRITICAL: Failed to create file %s\n", filename);
        }
    }
    
    // Test 2.2: Read/Write verification test
    printf("Test 2.2: Read/Write verification test (100 tests)...\n");
    for (int i = 0; i < 100; ++i) {
        MyFile *target_file = created_files[i];
        if (!target_file) continue;
        
        // Write operation
        char test_data[128];
        int data_size = 20 + (i % 108);
        for (int j = 0; j < data_size; ++j) {
            test_data[j] = '0' + (j % 10);
        }
        
        int write_result = write(target_file, (uint8_t*)test_data, data_size);
        total_tests++;
        
        if (write_result) {
            // Verify with read
            uint8_t read_buffer[128];
            int read_result = read(target_file, read_buffer, data_size, 0);
            
            if (read_result == data_size && memcmp(test_data, read_buffer, data_size) == 0) {
                cat2_pass++;
                if (i % 10 == 0) printf("   [PASS] Read/Write verification %d passed (%d bytes)\n", i, data_size);
            } else {
                cat2_fail++;
                printf("   [FAIL] Read/Write verification %d FAILED\n", i);
            }
        } else {
            cat2_fail++;
            printf("   [FAIL] Write operation %d FAILED\n", i);
        }
    }
    
    // Test 2.3: File operations (copy, move, delete)
    printf("Test 2.3: File operations test (100 tests)...\n");
    MyFolder *copy_dest = mkdir(&test_root, "copy_destination");
    MyFolder *move_dest = mkdir(&test_root, "move_destination");
    
    for (int i = 0; i < 100; ++i) {
        char source_filename[64];
        snprintf(source_filename, sizeof(source_filename), "test_file_%04d.dat", i);
        
        // Test copy
        int copy_result = cp(file_test_dir, source_filename, copy_dest);
        total_tests++;
        
        if (copy_result) {
            cat2_pass++;
            
            // Test move (every 3rd file)
            if (i % 3 == 0) {
                int move_result = mv(copy_dest, source_filename, move_dest);
                if (move_result) {
                    if (i % 15 == 0) printf("   [PASS] File operations batch %d completed\n", i);
                } else {
                    printf("   [WARN] Move operation failed for %s\n", source_filename);
                }
            }
        } else {
            cat2_fail++;
            printf("   [FAIL] Copy operation failed for %s\n", source_filename);
        }
    }
    
    clock_t cat2_end = clock();
    double cat2_time = ((double)(cat2_end - cat2_start)) / CLOCKS_PER_SEC;
    
    printf("\nCATEGORY 2 RESULTS:\n");
    printf("   [PASS] PASSED: %d/%d (%.1f%%)\n", cat2_pass, cat2_pass + cat2_fail, 
           cat2_pass + cat2_fail > 0 ? (cat2_pass * 100.0) / (cat2_pass + cat2_fail) : 0);
    printf("   [FAIL] FAILED: %d\n", cat2_fail);
    printf("   [TIME] EXECUTION TIME: %.3f seconds\n", cat2_time);
    printf("   [RATE] THROUGHPUT: %.0f operations/second\n\n", (cat2_pass + cat2_fail) / cat2_time);
    
    total_pass += cat2_pass;
    total_fail += cat2_fail;
    
    // =================================================================
    // TEST CATEGORY 3: MEMORY STRESS TEST
    // =================================================================
    printf("CATEGORY 3: MEMORY STRESS TEST (200 tests)\n");
    printf("--------------------------------------------------------------\n");
    
    clock_t cat3_start = clock();
    int cat3_pass = 0, cat3_fail = 0;
    
    // Test 3.1: Memory allocation/deallocation cycles
    printf("Test 3.1: Memory allocation cycles (100 tests)...\n");
    for (int cycle = 0; cycle < 100; ++cycle) {
        char temp_path[64];
        snprintf(temp_path, sizeof(temp_path), "memory_test/cycle_%03d", cycle);
        
        MyFolder *temp_folder = mkdir(&test_root, temp_path);
        total_tests++;
        
        if (temp_folder) {
            // Create files with varying sizes
            for (int j = 0; j < 3; ++j) {
                char temp_filename[32];
                snprintf(temp_filename, sizeof(temp_filename), "temp_%d.dat", j);
                MyFile *temp_file = mk(temp_folder, temp_filename);
                
                if (temp_file) {
                    char test_data[512];
                    int data_size = 100 + (j * 100);
                    memset(test_data, 'X', data_size);
                    write(temp_file, (uint8_t*)test_data, data_size);
                }
            }
            
            // Delete the structure
            rmdir(&test_root, temp_path);
            
            cat3_pass++;
            if (cycle % 10 == 0) printf("   [PASS] Memory cycle %d completed\n", cycle);
        } else {
            cat3_fail++;
            critical_failures++;
            printf("   [FAIL] CRITICAL: Memory allocation failed at cycle %d\n", cycle);
        }
    }
    
    // Test 3.2: Large data handling test
    printf("Test 3.2: Large data handling test (100 tests)...\n");
    MyFolder *large_test_dir = mkdir(&test_root, "large_data_test");
    
    for (int i = 0; i < 100; ++i) {
        char large_filename[32];
        snprintf(large_filename, sizeof(large_filename), "large_%04d.dat", i);
        
        MyFile *large_file = mk(large_test_dir, large_filename);
        total_tests++;
        
        if (large_file) {
            char large_data[1024];
            memset(large_data, 'L', sizeof(large_data));
            
            int write_result = write(large_file, (uint8_t*)large_data, sizeof(large_data));
            if (write_result) {
                cat3_pass++;
                if (i % 10 == 0) printf("   [PASS] Large data test %d passed (1KB)\n", i);
            } else {
                cat3_fail++;
                printf("   [FAIL] Large data write failed for %s\n", large_filename);
            }
        } else {
            cat3_fail++;
            printf("   [FAIL] Large file creation failed: %s\n", large_filename);
        }
    }
    
    clock_t cat3_end = clock();
    double cat3_time = ((double)(cat3_end - cat3_start)) / CLOCKS_PER_SEC;
    
    printf("\nCATEGORY 3 RESULTS:\n");
    printf("   [PASS] PASSED: %d/%d (%.1f%%)\n", cat3_pass, cat3_pass + cat3_fail, 
           cat3_pass + cat3_fail > 0 ? (cat3_pass * 100.0) / (cat3_pass + cat3_fail) : 0);
    printf("   [FAIL] FAILED: %d\n", cat3_fail);
    printf("   [TIME] EXECUTION TIME: %.3f seconds\n", cat3_time);
    printf("   [RATE] THROUGHPUT: %.0f operations/second\n\n", (cat3_pass + cat3_fail) / cat3_time);
    
    total_pass += cat3_pass;
    total_fail += cat3_fail;
    
    // =================================================================
    // FINAL COMPREHENSIVE REPORT
    // =================================================================
    clock_t global_end = clock();
    double total_time = ((double)(global_end - global_start)) / CLOCKS_PER_SEC;
    
    printf("=================================================================\n");
    printf("                    COMPREHENSIVE FINAL REPORT                  \n");
    printf("=================================================================\n");
    
    printf("\nULTIMATE STATISTICS:\n");
    printf("   [TOTAL] Total Tests Executed:    %d\n", total_tests);
    printf("   [PASS]  Total Passed:           %d\n", total_pass);
    printf("   [FAIL]  Total Failed:           %d\n", total_fail);
    printf("   [CRIT]  Critical Failures:      %d\n", critical_failures);
    
    if (total_tests > 0) {
        printf("   [RATE]  Success Rate:           %.2f%%\n", (total_pass * 100.0) / total_tests);
    }
    
    printf("   [TIME]  Total Execution Time:   %.3f seconds\n", total_time);
    if (total_time > 0) {
        printf("   [SPEED] Average Throughput:     %.0f tests/second\n", total_tests / total_time);
    }
    
    // Calculate memory usage
    size_t total_memory = calculateFolderCapacity(test_root, true);
    printf("   [MEM]   Total Memory Used:      %zu bytes (%.2f KB)\n", 
           total_memory, total_memory / 1024.0);
    
    printf("\nPERFORMANCE EVALUATION:\n");
    double success_rate = total_tests > 0 ? (total_pass * 100.0) / total_tests : 0;
    
    if (success_rate >= 99.0) {
        printf("   [GRADE] GRADE: A+ (EXCELLENT) - Mountkit is highly reliable!\n");
    } else if (success_rate >= 95.0) {
        printf("   [GRADE] GRADE: A (VERY GOOD) - Mountkit is well tested!\n");
    } else if (success_rate >= 90.0) {
        printf("   [GRADE] GRADE: B+ (GOOD) - Mountkit is solid!\n");
    } else if (success_rate >= 80.0) {
        printf("   [GRADE] GRADE: B (ACCEPTABLE) - Some improvements needed\n");
    } else {
        printf("   [GRADE] GRADE: C (NEEDS WORK) - Significant issues detected!\n");
    }
    
    if (critical_failures == 0) {
        printf("   [STAB]  STABILITY: ROCK SOLID - No critical failures detected!\n");
    } else {
        printf("   [STAB]  STABILITY: UNSTABLE - %d critical failures need attention!\n", critical_failures);
    }
    
    double throughput = total_time > 0 ? total_tests / total_time : 0;
    if (throughput > 1000) {
        printf("   [PERF]  PERFORMANCE: EXCELLENT (%.0f tests/sec)\n", throughput);
    } else if (throughput > 500) {
        printf("   [PERF]  PERFORMANCE: GOOD (%.0f tests/sec)\n", throughput);
    } else if (throughput > 100) {
        printf("   [PERF]  PERFORMANCE: ACCEPTABLE (%.0f tests/sec)\n", throughput);
    } else {
        printf("   [PERF]  PERFORMANCE: SLOW (%.0f tests/sec)\n", throughput);
    }
    
    printf("\n");
    if (total_fail == 0) {
        printf("[SUCCESS] PERFECT SCORE! ALL %d TESTS PASSED!\n", total_tests);
        printf("[STATUS]  MOUNTKIT IS CERTIFIED PRODUCTION READY!\n");
    } else if (critical_failures == 0) {
        printf("[GOOD]    EXCELLENT PERFORMANCE! Minor issues detected but no critical failures!\n");
        printf("[STATUS]  %d tests need attention out of %d total tests\n", total_fail, total_tests);
    } else {
        printf("[WARNING] ATTENTION REQUIRED! Critical stability issues detected!\n");
        printf("[ACTION]  %d critical failures need immediate fixing!\n", critical_failures);
    }
    
    printf("\n[COMPLETE] COMPREHENSIVE STRESS TEST COMPLETED!\n");
    printf("=================================================================\n\n");
    
    // Cleanup
    if (test_root) {
        removeFolder(test_root);
        printf("[CLEANUP] Memory cleanup completed. System ready.\n");
    }
}

// เพิ่มฟังก์ชัน embedded test ด้วย ASCII characters
#ifdef EMBEDDED_BUILD
void Mountkit::embedded_test() {
    CLEAR_ERROR_FLAG();
    
    printf("=== EMBEDDED SYSTEM TEST START ===\n");
    
    MyFolder *root = NULL;
    int test_count = 0;
    int pass_count = 0;
    
    // Test 1: Basic folder creation
    printf("Test 1: Basic folder creation\n");
    MyFolder *test_dir = mkdir(&root, "test");
    test_count++;
    if (test_dir && !hasError()) {
        pass_count++;
        printf("   [PASS] Folder created\n");
    } else {
        printf("   [FAIL] Folder creation failed\n");
        return;
    }
    
    // Test 2: Basic file creation
    printf("Test 2: Basic file creation\n");
    MyFile *test_file = mk(test_dir, "test.txt");
    test_count++;
    if (test_file && !hasError()) {
        pass_count++;
        printf("   [PASS] File created\n");
    } else {
        printf("   [FAIL] File creation failed\n");
        removeFolder(root);
        return;
    }
    
    // Test 3: Basic write operation
    printf("Test 3: Basic write operation\n");
    const char *test_data = "Hello Embedded System!";
    int write_result = write(test_file, test_data);
    test_count++;
    if (write_result && !hasError()) {
        pass_count++;
        printf("   [PASS] Write operation successful\n");
    } else {
        printf("   [FAIL] Write operation failed\n");
        removeFolder(root);
        return;
    }
    
    // Test 4: Basic read operation
    printf("Test 4: Basic read operation\n");
    uint8_t read_buffer[64];
    int read_result = read(test_file, read_buffer, 22, 0);
    test_count++;
    if (read_result == 22 && !hasError()) {
        pass_count++;
        printf("   [PASS] Read operation successful\n");
    } else {
        printf("   [FAIL] Read operation failed\n");
        removeFolder(root);
        return;
    }
    
    // Cleanup
    removeFolder(root);
    
    printf("\n=== EMBEDDED TEST RESULTS ===\n");
    printf("Total Tests: %d\n", test_count);
    printf("Passed: %d\n", pass_count);
    printf("Failed: %d\n", test_count - pass_count);
    
    if (pass_count == test_count) {
        printf("[STATUS] ALL TESTS PASSED - SYSTEM READY\n");
    } else {
        printf("[STATUS] SOME TESTS FAILED - CHECK SYSTEM\n");
    }
    
    printf("=== EMBEDDED TEST COMPLETE ===\n");
}

bool Mountkit::hasError() {
    return CHECK_ERROR_FLAG();
}

void Mountkit::clearError() {
    CLEAR_ERROR_FLAG();
}
#endif

// เพิ่มฟังก์ชัน calculateFolderCapacity ที่ขาดหาย
size_t Mountkit::calculateFolderCapacity(MyFolder *folder, bool include_subdirs) {
    if (!folder) return 0;
    
    size_t total_size = 0;
    
    // 1. Base size = ความยาวของชื่อ folder
    size_t base_size = 0;
    if (folder->data) {
        base_size = strlen(folder->data);
    }
    
    // 2. Path overhead = sizeof(MyFolder) structure ต่อ folder
    size_t path_overhead = sizeof(MyFolder);
    
    // 3. File metadata = รวม File.capacity ของไฟล์ทั้งหมด
    size_t file_capacity_total = 0;
    size_t file_metadata_total = 0;
    
    MyFile *current_file = folder->files;
    while (current_file) {
        // ใช้ capacity ของไฟล์ตามที่กำหนดในโจทย์
        file_capacity_total += current_file->capacity;
        
        // metadata ของ file structure เอง
        file_metadata_total += sizeof(MyFile);
        if (current_file->name) {
            file_metadata_total += strlen((char*)current_file->name) + 1;
        }
        
        current_file = current_file->next;
    }
    
    // รวมขนาดของ folder ปัจจุบัน
    total_size = base_size + path_overhead + file_capacity_total + file_metadata_total;
    
    // 4. ถ้ารวม subdirectories
    if (include_subdirs) {
        MyFolder *current_subdir = folder->subdir;
        while (current_subdir) {
            total_size += calculateFolderCapacity(current_subdir, true);
            current_subdir = current_subdir->dir;
        }
    }
    
    return total_size;
}