#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>

typedef struct File {
    size_t size, capacity;
    uint8_t *name, *data;
    struct File *next;
} File;

typedef struct Folder {
    char *data;
    File *files;
    struct Folder *subdir, *dir;
} Folder;

// สร้างโฟลเดอร์ใหม่
void createFolder(Folder **folder, const char *name) {
    Folder *newFolder = (Folder*)malloc(sizeof(Folder));
    if (!newFolder) {
        fprintf(stderr, "malloc failed for Folder\n");
        exit(1);
    }
    newFolder->data = strdup(name);
    if (!newFolder->data) {
        fprintf(stderr, "malloc failed for Folder->data\n");
        free(newFolder);
        exit(1);
    }
    newFolder->files = NULL;
    newFolder->subdir = NULL;
    newFolder->dir = NULL;
    *folder = newFolder;
}

// free memory ของไฟล์ในโฟลเดอร์
void freeFiles(File *file) {
    while (file) {
        File *next = file->next;
        free(file->name);
        free(file->data);
        free(file);
        file = next;
    }
}

// ลบโฟลเดอร์และลูกทั้งหมด
void removeFolder(Folder *folder) {
    if (!folder) return;
    freeFiles(folder->files);
    removeFolder(folder->subdir);
    removeFolder(folder->dir);
    free(folder->data);
    free(folder);
}

// ลบโฟลเดอร์ตาม path
void rmdir(Folder **root, const char *path) {
    char buf[256];
    strncpy(buf, path, sizeof(buf)); buf[sizeof(buf)-1] = '\0';
    char *token = strtok(buf, "/");
    Folder **current = root, **prev = NULL;
    while (token && *current) {
        Folder *iter = *current;
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
Folder* mkdir(Folder **root, const char *path) {
    char buf[256];
    strncpy(buf, path, sizeof(buf)); buf[sizeof(buf)-1] = '\0';
    char *token = strtok(buf, "/");
    Folder **current = root, *last = NULL;
    while (token) {
        Folder *iter = *current;
        Folder **prev = current;
        while (iter && strcmp(iter->data, token) != 0) {
            prev = &iter->dir;
            iter = iter->dir;
        }
        if (iter) {
            last = iter;
            current = &iter->subdir;
        } else {
            createFolder(prev, token);
            last = *prev;
            current = &((*prev)->subdir);
        }
        token = strtok(NULL, "/");
    }
    return last;
}

// mk: สร้างไฟล์ใหม่ในโฟลเดอร์ (ไม่ซ้ำชื่อ)
File* mk(Folder *folder, const char *filename) {
    if (!folder || !filename) return NULL;
    for (File *cur = folder->files; cur; cur = cur->next)
        if (strcmp((char*)cur->name, filename) == 0) return cur;
    File *file = (File*)malloc(sizeof(File));
    if (!file) {
        fprintf(stderr, "malloc failed for File\n");
        return NULL;
    }
    file->name = (uint8_t*)strdup(filename);
    if (!file->name) {
        fprintf(stderr, "malloc failed for File->name\n");
        free(file);
        return NULL;
    }
    file->capacity = 4096;
    file->data = (uint8_t*)malloc(file->capacity);
    if (!file->data) {
        fprintf(stderr, "malloc failed for File->data\n");
        free(file->name);
        free(file);
        return NULL;
    }
    file->size = 0;
    file->next = folder->files;
    folder->files = file;
    return file;
}

// cd: เดิน path และคืน pointer ของตำแหน่งนั้น
Folder* cd(Folder *root, const char *path) {
    if (!root || !path) return NULL;
    char buf[256];
    strncpy(buf, path, sizeof(buf)); buf[sizeof(buf)-1] = '\0';
    char *token = strtok(buf, "/");
    Folder *current = root;
    if (token && strcmp(current->data, token) == 0) token = strtok(NULL, "/");
    while (token && current) {
        Folder *iter = current->subdir;
        while (iter && strcmp(iter->data, token) != 0)
            iter = iter->dir;
        if (!iter) return NULL;
        current = iter;
        token = strtok(NULL, "/");
    }
    return current;
}

// pwd: แสดง path ปัจจุบัน
void pwd(Folder *folder, Folder *root) {
    if (!folder) return;
    const Folder *stack[128];
    int top = 0;
    Folder *cur = folder;
    while (cur && cur != root) {
        stack[top++] = cur;
        Folder *parent = root, *found = NULL;
        while (parent) {
            Folder *child = parent->subdir;
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
void PrintAllPath(Folder *folder, char *prefix) {
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
int rm(Folder *folder, const char *filename) {
    if (!folder || !filename) return 0;
    File **cur = &folder->files;
    while (*cur) {
        if (strcmp((char*)(*cur)->name, filename) == 0) {
            File *to_delete = *cur;
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
int cp(Folder *src_folder, const char *filename, Folder *dst_folder) {
    if (!src_folder || !dst_folder || !filename) return 0;
    // หาไฟล์ต้นทาง
    File *src = src_folder->files;
    while (src && strcmp((char*)src->name, filename) != 0)
        src = src->next;
    if (!src) return 0; // ไม่พบไฟล์ต้นทาง

    // ตรวจสอบว่าปลายทางมีไฟล์ชื่อเดียวกันอยู่แล้วหรือไม่
    File *dst = dst_folder->files;
    while (dst) {
        if (strcmp((char*)dst->name, filename) == 0)
            return 0; // ไม่คัดลอกซ้ำ
        dst = dst->next;
    }

    // สร้างไฟล์ใหม่ในปลายทาง
    File *newfile = mk(dst_folder, filename);
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
int mv(Folder *src_folder, const char *filename, Folder *dst_folder) {
    if (!src_folder || !dst_folder || !filename) return 0;

    // หาไฟล์ต้นทาง (และ pointer ไปยัง pointer ของมัน)
    File **cur = &src_folder->files;
    while (*cur && strcmp((char*)(*cur)->name, filename) != 0) {
        cur = &((*cur)->next);
    }
    if (!*cur) return 0; // ไม่พบไฟล์ต้นทาง

    // ตรวจสอบว่าปลายทางมีไฟล์ชื่อเดียวกันอยู่แล้วหรือไม่
    for (File *dst = dst_folder->files; dst; dst = dst->next) {
        if (strcmp((char*)dst->name, filename) == 0)
            return 0; // ไม่ย้ายซ้ำ
    }

    // ถอดไฟล์ออกจาก src_folder
    File *moving = *cur;
    *cur = moving->next;

    // ใส่ไฟล์เข้า dst_folder
    moving->next = dst_folder->files;
    dst_folder->files = moving;

    return 1; // success
}

// ฟังก์ชันสำหรับทดสอบอัตโนมัติ
void run_tests() {
    Folder *root = NULL;

    // Test 1: mkdir และ cd
    Folder *cfg = mkdir(&root, "root/cfg");
    assert(cfg && strcmp(cfg->data, "cfg") == 0);
    Folder *usr = mkdir(&root, "root/usr");
    assert(usr && strcmp(usr->data, "usr") == 0);

    // Test 2: mkdir ซ้อนหลายชั้น
    Folder *toon = mkdir(&root, "root/usr/toon");
    assert(toon && strcmp(toon->data, "toon") == 0);

    // Test 3: cd ไปยัง path ที่มีอยู่
    Folder *cd_toon = cd(root, "usr/toon");
    assert(cd_toon == toon);

    // Test 4: mk สร้างไฟล์ใหม่
    File *f1 = mk(toon, "fileA.txt");
    assert(f1 && strcmp((char*)f1->name, "fileA.txt") == 0);

    // Test 5: mk ซ้ำชื่อเดิม ต้อง return pointer เดิม
    File *f2 = mk(toon, "fileA.txt");
    assert(f2 == f1);

    // Test 6: cd ไป path ที่ไม่มี ต้องได้ NULL
    Folder *notfound = cd(root, "usr/none");
    assert(notfound == NULL);

    // Test 7: rmdir แล้ว cd ต้องได้ NULL
    rmdir(&root, "root/usr/toon");
    Folder *cd_after_rm = cd(root, "usr/toon");
    assert(cd_after_rm == NULL);

    // Test 8: mkdir หลัง rmdir ต้องสร้างใหม่ได้
    Folder *toon2 = mkdir(&root, "root/usr/toon");
    assert(toon2 && strcmp(toon2->data, "toon") == 0);

    // Test 9: mk หลัง rmdir
    File *f3 = mk(toon2, "fileB.txt");
    assert(f3 && strcmp((char*)f3->name, "fileB.txt") == 0);

    // Test 10: removeFolder ทั้งหมด
    removeFolder(root);

    printf("All tests passed!\n");
}

void auto_test_and_report() {
    int pass, fail, total = 100;
    char msg[128];

    printf("===== Automated Test & Report =====\n");

    // Test mkdir & cd
    pass = fail = 0;
    Folder *root = NULL;
    for (int i = 0; i < total; ++i) {
        snprintf(msg, sizeof(msg), "mkdir/cd: create and access root/folder%d", i);
        char foldername[32];
        snprintf(foldername, sizeof(foldername), "root/folder%d", i);
        Folder *f = mkdir(&root, foldername);
        Folder *fcd = cd(root, foldername + 5); // +5 ข้าม "root/"
        if (f && fcd == f) {
            printf("[PASS] %s\n", msg);
            ++pass;
        } else {
            printf("[FAIL] %s\n", msg);
            ++fail;
        }
    }
    printf("mkdir & cd summary: pass=%d, fail=%d\n\n", pass, fail);

    // Test mk
    pass = fail = 0;
    Folder *testf = mkdir(&root, "root/testmk");
    for (int i = 0; i < total; ++i) {
        snprintf(msg, sizeof(msg), "mk: create file%d.txt in testmk", i);
        char filename[32];
        snprintf(filename, sizeof(filename), "file%d.txt", i);
        File *f = mk(testf, filename);
        if (f && strcmp((char*)f->name, filename) == 0) {
            printf("[PASS] %s\n", msg);
            ++pass;
        } else {
            printf("[FAIL] %s\n", msg);
            ++fail;
        }
    }
    printf("mk summary: pass=%d, fail=%d\n\n", pass, fail);

    // Test rm
    pass = fail = 0;
    for (int i = 0; i < total; ++i) {
        snprintf(msg, sizeof(msg), "rm: remove file%d.txt from testmk", i);
        char filename[32];
        snprintf(filename, sizeof(filename), "file%d.txt", i);
        if (rm(testf, filename)) {
            printf("[PASS] %s\n", msg);
            ++pass;
        } else {
            printf("[FAIL] %s\n", msg);
            ++fail;
        }
    }
    printf("rm summary: pass=%d, fail=%d\n\n", pass, fail);

    // Test cp
    pass = fail = 0;
    Folder *src = mkdir(&root, "root/cpsrc");
    Folder *dst = mkdir(&root, "root/cpdst");
    for (int i = 0; i < total; ++i) {
        snprintf(msg, sizeof(msg), "cp: copy cpfile%d.txt from cpsrc to cpdst", i);
        char filename[32];
        snprintf(filename, sizeof(filename), "cpfile%d.txt", i);
        File *f = mk(src, filename);
        if (f) {
            memset(f->data, i, 10);
            f->size = 10;
        }
        if (cp(src, filename, dst)) {
            printf("[PASS] %s\n", msg);
            ++pass;
        } else {
            printf("[FAIL] %s\n", msg);
            ++fail;
        }
    }
    printf("cp summary: pass=%d, fail=%d\n\n", pass, fail);

    // Test mv
    pass = fail = 0;
    Folder *mvsrc = mkdir(&root, "root/mvsrc");
    Folder *mvdst = mkdir(&root, "root/mvdst");
    for (int i = 0; i < total; ++i) {
        snprintf(msg, sizeof(msg), "mv: move mvfile%d.txt from mvsrc to mvdst", i);
        char filename[32];
        snprintf(filename, sizeof(filename), "mvfile%d.txt", i);
        File *f = mk(mvsrc, filename);
        if (f) {
            memset(f->data, i, 10);
            f->size = 10;
        }
        if (mv(mvsrc, filename, mvdst)) {
            printf("[PASS] %s\n", msg);
            ++pass;
        } else {
            printf("[FAIL] %s\n", msg);
            ++fail;
        }
    }
    printf("mv summary: pass=%d, fail=%d\n\n", pass, fail);

    // Test rmdir
    pass = fail = 0;
    for (int i = 0; i < total; ++i) {
        snprintf(msg, sizeof(msg), "rmdir: remove root/rmdir%d", i);
        char foldername[32];
        snprintf(foldername, sizeof(foldername), "root/rmdir%d", i);
        Folder *f = mkdir(&root, foldername);
        if (f) {
            rmdir(&root, foldername);
            if (!cd(root, foldername + 5)) {
                printf("[PASS] %s\n", msg);
                ++pass;
            } else {
                printf("[FAIL] %s\n", msg);
                ++fail;
            }
        } else {
            printf("[FAIL] %s (mkdir fail)\n", msg);
            ++fail;
        }
    }
    printf("rmdir summary: pass=%d, fail=%d\n\n", pass, fail);

    // Cleanup
    removeFolder(root);
}

int main() {
    auto_test_and_report();
    return 0;
}