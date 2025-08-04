#include <stdio.h>
#include <string.h>
#include <hardware/clocks.h>
#include <hardware/flash.h>
#include "blockdevice/flash.h"
#include "filesystem/littlefs.h"
#include "filesystem/vfs.h"


bool fs_init(void)
{
    printf("Initialize custom file system\n");

    blockdevice_t *flash = blockdevice_flash_create(PICO_FLASH_SIZE_BYTES - PICO_FS_DEFAULT_SIZE, 0);
    filesystem_t *lfs = filesystem_littlefs_create(500, 16);

    // Mount littlefs on-board flash to `/`
    int err = fs_mount("/", lfs, flash);
    if (err == -1)
    {
        printf("format / with littlefs\n");
        err = fs_format(lfs, flash);
        if (err == -1)
        {
            printf("fs_format error: %s\n", strerror(errno));
            return false;
        }
        err = fs_mount("/", lfs, flash);
        if (err == -1)
        {
            printf("fs_mount error: %s\n", strerror(errno));
            return false;
        }
    }

    return err == 0;
}