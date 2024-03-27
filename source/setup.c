#include <wafel/utils.h>
#include <wafel/ios/thread.h>
#include <wafel/ios/svc.h>

#include "fsa.h"
#include "setup.h"

void copy_sysprod(int fd){
    int res = copy_file(fd, "/vol/sdcard/sys_prod.xml", "/vol/system/sys/config/sys_prod.xml");
    debug_printf("copy_file: %x\n", res);
}

void mount_sd(int fd, char* path)
{
    // Mount sd to /vol/sdcard
    int ret = FSA_Mount(fd, "/dev/sdcard01", path, 0, NULL, 0);;
    int i = 1;
    while(ret < 0)
    {
        ret = FSA_Mount(fd, "/dev/sdcard01", path, 0, NULL, 0);
        debug_printf("Mount SD attempt %d, %X\n", i++, ret);
        usleep(1000);

        //if (ret == 0xFFFCFFEA || ret == 0xFFFCFFE6 || i >= 0x100) break;
    }
    debug_printf("Mounted SD...\n");
}

int flush_slc(int fsaHandle){
    int ret = FSA_FlushVolume(fsaHandle, "/vol/system");
    debug_printf("Flush SLC returned %X\n", ret);
    return ret;
}

u32 setup_main(void* arg){

    int fsaHandle = -1;
    int i = 1;
    while(fsaHandle < 0){
        usleep(1000);
        fsaHandle = FSA_Open();
        debug_printf("FSA open attempt %d %X\n", i++, fsaHandle);
    }

    mount_sd(fsaHandle, "/vol/sdcard");

    int ret = FSA_Mount(fsaHandle, "/dev/slc01", "/vol/system", 0, NULL, 0);;
    debug_printf("Mount SLC: %x\n", ret);

    // This is just to test if SLC is actually mounted
    int dir = 0;
    ret = FSA_OpenDir(fsaHandle, "/vol/system", &dir);
    debug_printf("Open /vol/system: %X\n", ret);

    copy_sysprod(fsaHandle);

    flush_slc(fsaHandle);
    ret = FSA_Remove(fsaHandle, "/vol/sdcard/wiiu/ios_plugins/wafel_sysprod_copy.ipx");
    debug_printf("FSA_Remove: %X\n", ret);

    ret = FSA_Unmount(fsaHandle, "/vol/sdcard", 1);
    debug_printf("Unmount SD Card: %X\n", ret);
}
