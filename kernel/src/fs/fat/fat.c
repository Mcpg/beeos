
#include "fat.h"
#include "fs/devfs/devfs.h"
#include <stdint.h>
#include <stdlib.h>

/* Simple debug macros */
#define DEBUG_FAT

#ifdef DEBUG_FAT
#include "kprintf.h"
#define DEBUG_FAT_PRINT(...) kprintf(__VA_ARGS__)
#else
#define DEBUG_FAT_PRINT(...) ;
#endif

/* BIOS Parameter Block */
struct fat_bpb {
    uint8_t  jmp_nop[3]; /* jmp and nop moving the code outside of the BPB  */
    char     oem_id[8]; /* OEM identifier */
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors; /* amount of reserved sectors (boot sector included) */
    uint8_t  fat_amount; /* amount of file allocation tables on the volume, usually 2 */
    uint16_t root_dir_entries; /* max amount of entries that can fit in root directory */
    uint16_t total_sectors; /* if it's 0, the value is stored in large_sectors field */
    uint8_t  media_desc; /* media descriptor type */
    uint16_t sectors_per_fat; /* used only in FAT12/16 */
    uint16_t sectors_per_track;
    uint32_t hidden_sectors;
    uint32_t large_sectors;
}__attribute__((packed));

/* Extended Boot Record for FAT 12 and FAT16 */
struct fat12_16_ebr {
    uint8_t  drive_number; /* drive number, identical to dl register in BIOS interrupt 0x13 */
    uint8_t  nt_flags; /* Windows NT flags (reserved) */
    uint8_t  signature; /* either 0x28 or 0x29 */
    uint32_t serial; /* volume ID */
    char     volume_label[11];
    char     system_id[8]; /* system identifier. it's a string representing FAT fs type. it shouldn't be trusted */
}__attribute__((packed));

/* Extended Boot Record for FAT32 */
struct fat32_ebr {
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_dir_cluster; /* cluster number the root directory (often it's 2) */
    uint16_t fsinfo_sector; /* sector number of FSInfo structure */
    uint16_t backup_boot_sector; /* sector number of backup boot sector */
    uint8_t  reserved[12]; /* should be all 0s */
    uint8_t  drive_number; /* just like in fat12_16_ebr */
    uint8_t  nt_flags; /* Windows NT flags (reserved) */
    uint8_t  signature; /* either 0x28 or 0x29 */
    uint32_t serial; /* volume ID */
    char     volume_label[11];
    char     system_id[8]; /* system identifier. in FAT32, should be "FAT32   ", but the value shouldn't be trusted */
}__attribute__((packed));

enum fat_type {
    fat12, fat16, fat32
};

struct fat_super_block {
    struct super_block base;
    enum fat_type      fat_type;
    uint16_t           bytes_per_sector;
    uint8_t            sectors_per_cluster;
    uint16_t           reserved_sectors;
    uint8_t            fat_amount;
    uint16_t           root_dir_entries;
    uint32_t           total_sectors;
    uint32_t           sectors_per_fat;
    uint32_t           hidden_sectors;
};

static const struct super_ops fat_sb_ops = {
    .inode_alloc = NULL,
    .inode_free  = NULL,
    .inode_read  = NULL,
};


struct super_block *fat_super_create(dev_t dev)
{
    DEBUG_FAT_PRINT("fat_super_create executed (dev == %lu)", dev);
    
    struct fat_super_block *sb;
    struct fat_bpb bpb;
    struct fat12_16_ebr fat16ebr;
    struct fat32_ebr fat32ebr;

    if (devfs_read(dev, &bpb, sizeof(bpb), 0) != sizeof(bpb))
    {
        DEBUG_FAT_PRINT("fat_bpb size != %d!", sizeof(bpb));
        return NULL;
    }

    sb = (struct fat_super_block *) kmalloc(sizeof(struct super_block));
    if (sb == NULL)
    {
        DEBUG_FAT_PRINT("sb == NULL after kmalloc!");
        return NULL;
    }

    return sb;
}