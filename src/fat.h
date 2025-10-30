/*UNCRUSTIFY-OFF*/
/**
 * @file fat.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Low-level FAT32 filesystem implementation
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef FAT_H_
  #define FAT_H_

  #include "config.h"
  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)
    #include "defines.h"
    #include "types.h"
    #include "console.h"
    #include "device.h"
    #include "fs.h"
    #include "mem.h"
    #include "port.h"
    #include "posix.h"
    #include "queue.h"
    #include "streams.h"
    #include "sys.h"
    #include "task.h"
    #include "timer.h"


/* Forward declaration of block device command structure */
    #define BLOCK_CMD_SET_ADDRESS 0x02u
    #define BLOCK_IO_MODE_BLOCKING 0x00u


/* FAT32 Boot Sector Structure (aligned for direct memory mapping) */
    typedef struct __attribute__ ((packed)) FAT32BootSector_s {
      Byte_t jumpBoot[0x3];             /* 0x00: Jump instruction */
      Byte_t oemName[0x8]; /* 0x03: OEM name */
      Byte_t bytesPerSector[0x2]; /* 0x0B: Bytes per sector (little-endian) */
      Byte_t sectorsPerCluster; /* 0x0D: Sectors per cluster */
      Byte_t reservedSectors[0x2]; /* 0x0E: Reserved sectors */
      Byte_t numFATs; /* 0x10: Number of FATs */
      Byte_t rootEntryCount[0x2]; /* 0x11: Root entries (0x0u for FAT32) */
      Byte_t totalSectors16[0x2]; /* 0x13: Total sectors (0x0u for FAT32) */
      Byte_t mediaType; /* 0x15: Media descriptor */
      Byte_t FATSize16[0x2]; /* 0x16: FAT size (0x0u for FAT32) */
      Byte_t sectorsPerTrack[0x2]; /* 0x18: Sectors per track */
      Byte_t numHeads[0x2]; /* 0x1A: Number of heads */
      Byte_t hiddenSectors[0x4]; /* 0x1C: Hidden sectors */
      Byte_t totalSectors32[0x4]; /* 0x20: Total sectors */
      Byte_t FATSize32[0x4]; /* 0x24: FAT size */
      Byte_t extFlags[0x2]; /* 0x28: Extended flags */
      Byte_t fsVersion[0x2]; /* 0x2A: Filesystem version */
      Byte_t rootCluster[0x4]; /* 0x2C: Root directory cluster */
      Byte_t fsInfo[0x2]; /* 0x30: FSInfo sector */
      Byte_t backupBootSector[0x2]; /* 0x32: Backup boot sector */
      Byte_t reserved[0xC]; /* 0x34: Reserved */
      Byte_t driveNumber; /* 0x40: Drive number */
      Byte_t reserved1; /* 0x41: Reserved */
      Byte_t bootSignature; /* 0x42: Boot signature (0x29) */
      Byte_t volumeID[0x4]; /* 0x43: Volume ID */
      Byte_t volumeLabel[0xB]; /* 0x47: Volume label */
      Byte_t fsType[0x8]; /* 0x52: Filesystem type */
    } FAT32BootSector_t;


/* FAT32 Directory Entry Structure (0x20 bytes) */
    typedef struct __attribute__ ((packed)) FAT32DirEntry_s {
      Byte_t name[0xB];                /* 0x00: 8.3 filename */
      Byte_t attr; /* 0x0B: File attributes */
      Byte_t ntReserved; /* 0x0C: Reserved for Windows NT */
      Byte_t createTimeTenth; /* 0x0D: Creation time (tenths of second) */
      Byte_t createTime[0x2]; /* 0x0E: Creation time */
      Byte_t createDate[0x2]; /* 0x10: Creation date */
      Byte_t lastAccessDate[0x2]; /* 0x12: Last access date */
      Byte_t firstClusterHigh[0x2]; /* 0x14: High word of first cluster */
      Byte_t writeTime[0x2]; /* 0x16: Last write time */
      Byte_t writeDate[0x2]; /* 0x18: Last write date */
      Byte_t firstClusterLow[0x2]; /* 0x1A: Low word of first cluster */
      Byte_t fileSize[0x4]; /* 0x1C: File size */
    } FAT32DirEntry_t;


/* FAT32 File Attributes */
    #define FAT_ATTR_READ_ONLY 0x01u
    #define FAT_ATTR_HIDDEN 0x02u
    #define FAT_ATTR_SYSTEM 0x04u
    #define FAT_ATTR_VOLUME_ID 0x08u
    #define FAT_ATTR_DIRECTORY 0x10u
    #define FAT_ATTR_ARCHIVE 0x20u
    #define FAT_ATTR_LONG_NAME 0x0Fu


/* FAT32 Cluster Markers */
    #define FAT32_EOC_MIN 0x0FFFFFF8u /* End of cluster chain (minimum) */
    #define FAT32_EOC_MAX 0x0FFFFFFFu /* End of cluster chain (maximum) */
    #define FAT32_BAD_CLUSTER 0x0FFFFFF7u /* Bad cluster marker */
    #define FAT32_FREE_CLUSTER 0x00000000u /* Free cluster */


    #ifdef __cplusplus
      extern "C" {
    #endif /* ifdef __cplusplus */
/* Endian conversion functions */
    HalfWord_t __ReadLE16__(const Byte_t *data_);
    Word_t __ReadLE32__(const Byte_t *data_);
    void __WriteLE16__(Byte_t *data_, HalfWord_t value_);
    void __WriteLE32__(Byte_t *data_, Word_t value_);
/* Block I/O operations */
    Return_t __ReadSector__(const Volume_t *vol_, Word_t sector_, Byte_t **data_);
    Return_t __WriteSector__(const Volume_t *vol_, Word_t sector_, const Byte_t *data_);
    Return_t __ReadCluster__(const Volume_t *vol_, Word_t cluster_, Byte_t **data_);
/* FAT table operations */
    Return_t __GetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t *nextCluster_);
    Return_t __SetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t value_);
    Return_t __FindFreeCluster__(const Volume_t *vol_, Word_t startHint_, Word_t *freeCluster_);
    Return_t __FreeClusters__(const Volume_t *vol_, Word_t startCluster_);
/* Cluster/sector calculations */
    Word_t __ClusterToSector__(const Volume_t *vol_, Word_t cluster_);
/* Mount tracking */
    Base_t __IsDeviceMounted__(const HalfWord_t blockDeviceUID_);
    Return_t __AddMountedDevice__(const HalfWord_t blockDeviceUID_);
    Return_t __RemoveMountedDevice__(const HalfWord_t blockDeviceUID_);
/* Path/name utilities */
    Base_t __ByteCompare__(const Byte_t *s1_, const Byte_t *s2_, Word_t len_);
    Return_t __ConvertToFAT83__(const Byte_t *path_, Byte_t *fat83_);
/* Directory entry helpers */
    Return_t __FindDirEntry__(const Volume_t *vol_, Word_t dirCluster_, const Byte_t *name83_, FAT32DirEntry_t *entry_, Word_t *entryCluster_, Word_t *
      entryOffset_);
    Return_t __FindFileByPath__(const Volume_t *vol_, const Byte_t *path_, FAT32DirEntry_t *entry_, Word_t *parentCluster_, Word_t *entryCluster_, Word_t *
      entryOffset_);
    Return_t __CreateDirEntry__(const Volume_t *vol_, Word_t parentCluster_, const Byte_t *name83_, Byte_t attr_, Word_t firstCluster_, Word_t size_);

    #if defined(POSIX_ARCH_OTHER)
      void __FSStateClear__(void);
    #endif /* if defined(POSIX_ARCH_OTHER) */

    #ifdef __cplusplus
      }
    #endif /* ifdef __cplusplus */
  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */
#endif /* ifndef FAT_H_ */