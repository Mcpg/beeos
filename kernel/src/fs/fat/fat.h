
#ifndef BEEOS_FS_FAT_H
#define BEEOS_FS_FAT_H

#include "fs/vfs.h"

struct super_block *fat_super_create(dev_t dev);

#endif /* BEEOS_FS_FAT_H */