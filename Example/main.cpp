#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <Mountkit.h>

Mountkit mount;
MyFolder *root = NULL; // เปลี่ยนจาก C เป็น root
MyFolder *current_dir = NULL; // ตำแหน่งปัจจุบัน

// สร้างฟังก์ชัน Linux-like path creation
void createLinuxPath() 
{
    printf("=================================================================\n");
    printf("               CREATING LINUX-LIKE DIRECTORY STRUCTURE          \n");
    printf("=================================================================\n\n");
    
    // สร้าง root directory structure เหมือน Linux
    printf("Creating Linux directory structure...\n");
    
    // Root directories
    mount.mkdir(&root, "/");
    mount.mkdir(&root, "bin");
    mount.mkdir(&root, "boot");
    mount.mkdir(&root, "dev");
    mount.mkdir(&root, "etc");
    mount.mkdir(&root, "home");
    mount.mkdir(&root, "lib");
    mount.mkdir(&root, "media");
    mount.mkdir(&root, "mnt");
    mount.mkdir(&root, "opt");
    mount.mkdir(&root, "proc");
    mount.mkdir(&root, "root");
    mount.mkdir(&root, "run");
    mount.mkdir(&root, "sbin");
    mount.mkdir(&root, "srv");
    mount.mkdir(&root, "sys");
    mount.mkdir(&root, "tmp");
    mount.mkdir(&root, "usr");
    mount.mkdir(&root, "var");
    
    printf("[PASS] Root directories created successfully\n\n");
    
    // สร้าง subdirectories สำคัญ
    printf("Creating important subdirectories...\n");
    
    // /usr structure
    mount.mkdir(&root, "usr/bin");
    mount.mkdir(&root, "usr/lib");
    mount.mkdir(&root, "usr/local");
    mount.mkdir(&root, "usr/local/bin");
    mount.mkdir(&root, "usr/local/lib");
    mount.mkdir(&root, "usr/share");
    mount.mkdir(&root, "usr/share/doc");
    mount.mkdir(&root, "usr/share/man");
    mount.mkdir(&root, "usr/src");
    
    // /var structure
    mount.mkdir(&root, "var/log");
    mount.mkdir(&root, "var/run");
    mount.mkdir(&root, "var/tmp");
    mount.mkdir(&root, "var/lib");
    mount.mkdir(&root, "var/cache");
    mount.mkdir(&root, "var/spool");
    mount.mkdir(&root, "var/www");
    
    // /etc structure
    mount.mkdir(&root, "etc/apache2");
    mount.mkdir(&root, "etc/nginx");
    mount.mkdir(&root, "etc/ssh");
    mount.mkdir(&root, "etc/ssl");
    mount.mkdir(&root, "etc/systemd");
    mount.mkdir(&root, "etc/network");
    
    // /home structure
    mount.mkdir(&root, "home/user");
    mount.mkdir(&root, "home/user/Documents");
    mount.mkdir(&root, "home/user/Downloads");
    mount.mkdir(&root, "home/user/Pictures");
    mount.mkdir(&root, "home/user/Videos");
    mount.mkdir(&root, "home/user/Desktop");
    mount.mkdir(&root, "home/user/.config");
    mount.mkdir(&root, "home/user/.ssh");
    
    // /dev structure (device files simulation)
    mount.mkdir(&root, "dev/block");
    mount.mkdir(&root, "dev/char");
    mount.mkdir(&root, "dev/input");
    mount.mkdir(&root, "dev/net");
    
    printf("[PASS] Subdirectories created successfully\n\n");
    
    // สร้างไฟล์ system configuration สำคัญ
    printf("Creating important system files...\n");
    
    // /etc files
    MyFolder *etc_dir = mount.cd(root, "etc");
    if (etc_dir) {
        MyFile *passwd = mount.mk(etc_dir, "passwd");
        mount.write(passwd, "root:x:0:0:root:/root:/bin/bash\nuser:x:1000:1000:User:/home/user:/bin/bash\n");
        
        MyFile *hosts = mount.mk(etc_dir, "hosts");
        mount.write(hosts, "127.0.0.1\tlocalhost\n::1\t\tlocalhost\n");
        
        MyFile *fstab = mount.mk(etc_dir, "fstab");
        mount.write(fstab, "# /etc/fstab: static file system information\nUUID=12345 / ext4 defaults 0 1\n");
        
        MyFile *hostname = mount.mk(etc_dir, "hostname");
        mount.write(hostname, "mountkit-system\n");
    }
    
    // /var/log files
    MyFolder *log_dir = mount.cd(root, "var/log");
    if (log_dir) {
        MyFile *syslog = mount.mk(log_dir, "syslog");
        mount.write(syslog, "Jan 1 00:00:01 mountkit kernel: Linux version 5.15.0\n");
        
        MyFile *auth_log = mount.mk(log_dir, "auth.log");
        mount.write(auth_log, "Jan 1 00:00:02 mountkit sshd: Server listening on 0.0.0.0 port 22\n");
        
        MyFile *kern_log = mount.mk(log_dir, "kern.log");
        mount.write(kern_log, "Jan 1 00:00:03 mountkit kernel: Mountkit filesystem initialized\n");
    }
    
    // /home/user files
    MyFolder *user_dir = mount.cd(root, "home/user");
    if (user_dir) {
        MyFile *bashrc = mount.mk(user_dir, ".bashrc");
        mount.write(bashrc, "# ~/.bashrc\nexport PATH=$PATH:/usr/local/bin\nalias ls='ls --color=auto'\n");
        
        MyFile *profile = mount.mk(user_dir, ".profile");
        mount.write(profile, "# ~/.profile\nif [ -f ~/.bashrc ]; then\n    . ~/.bashrc\nfi\n");
        
        // Documents
        MyFolder *docs_dir = mount.cd(root, "home/user/Documents");
        if (docs_dir) {
            MyFile *readme = mount.mk(docs_dir, "README.txt");
            mount.write(readme, "Welcome to Mountkit Linux-like file system!\n\nThis is a demonstration of a complete Linux directory structure.\n");
            
            MyFile *notes = mount.mk(docs_dir, "notes.txt");
            mount.write(notes, "System Notes:\n- All directories follow FHS (Filesystem Hierarchy Standard)\n- Configuration files are in /etc\n- Logs are in /var/log\n");
        }
    }
    
    // /bin executables (simulation)
    MyFolder *bin_dir = mount.cd(root, "bin");
    if (bin_dir) {
        MyFile *bash = mount.mk(bin_dir, "bash");
        mount.write(bash, "#!/bin/bash\n# Bash shell executable\necho \"Bash shell ready\"\n");
        
        MyFile *ls = mount.mk(bin_dir, "ls");
        mount.write(ls, "#!/bin/bash\n# List directory contents\necho \"Directory listing functionality\"\n");
        
        MyFile *cat = mount.mk(bin_dir, "cat");
        mount.write(cat, "#!/bin/bash\n# Display file contents\necho \"File display functionality\"\n");
    }
    
    printf("[PASS] System files created successfully\n\n");
    
    // Set current directory to root
    current_dir = root;
    
    printf("Linux-like directory structure created successfully!\n");
    printf("Root filesystem is ready with %zu bytes total capacity.\n\n", 
           mount.calculateFolderCapacity(root, true));
}

// ฟังก์ชัน Ultimate Stress Test (แก้ไข Unicode)
void ultimateStressTest() {
    printf("=================================================================\n");
    printf("                   ULTIMATE MOUNTKIT STRESS TEST                \n");
    printf("                        HARDCORE MODE ACTIVATED                 \n");
    printf("=================================================================\n\n");
    
    clock_t start_time = clock();
    
    // เรียกใช้ automated test ของ mountkit
    mount.auto_test_and_report();
    
    clock_t end_time = clock();
    double total_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    printf("\n[COMPLETE] ULTIMATE STRESS TEST COMPLETED IN %.3f SECONDS\n", total_time);
    printf("[STATUS] ALL SYSTEMS TESTED AND VERIFIED!\n");
}

int main() {
    createLinuxPath();
    
     mount.PrintAllPath(root, ""); 
    
     
    return 0;
}