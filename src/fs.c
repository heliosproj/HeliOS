/*UNCRUSTIFY-OFF*/
/**
 * @file fs.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Kernel source for FAT32 filesystem support
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "fs.h"


/* Forward declaration of block device command structure */
#define BLOCK_CMD_SET_ADDRESS 0x02u
#define BLOCK_IO_MODE_BLOCKING 0x00u

typedef struct BlockDeviceCommand_s {
  Byte_t command;
  Word_t blockNumber;
  HalfWord_t blockCount;
  Byte_t transferMode;
} BlockDeviceCommand_t;


/* FAT32 Boot Sector Structure (aligned for direct memory mapping) */
typedef struct __attribute__ ((packed)) FAT32BootSector_s {
  Byte_t jumpBoot[0x3];               /* 0x00: Jump instruction */
  Byte_t oemName[0x8]; /* 0x03: OEM name */
  Byte_t bytesPerSector[0x2]; /* 0x0B: Bytes per sector (little-endian) */
  Byte_t sectorsPerCluster; /* 0x0D: Sectors per cluster */
  Byte_t reservedSectors[0x2]; /* 0x0E: Reserved sectors */
  Byte_t numFATs; /* 0x10: Number of FATs */
  Byte_t rootEntryCount[0x2]; /* 0x11: Root entries (0x0 for FAT32) */
  Byte_t totalSectors16[0x2]; /* 0x13: Total sectors (0x0 for FAT32) */
  Byte_t mediaType; /* 0x15: Media descriptor */
  Byte_t FATSize16[0x2]; /* 0x16: FAT size (0x0 for FAT32) */
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
  Byte_t name[0xB];                  /* 0x00: 8.3 filename */
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


/* Helper function to read 16-bit little-endian value */
static HalfWord_t __ReadLE16__(const Byte_t *data_) {
  return((HalfWord_t) data_[0x0] | ((HalfWord_t) data_[0x1] << 0x8));
}


/* Helper function to read 32-bit little-endian value */
static Word_t __ReadLE32__(const Byte_t *data_) {
  return((Word_t) data_[0x0] | ((Word_t) data_[0x1] << 0x8) | ((Word_t) data_[0x2] << 0x10) | ((Word_t) data_[0x3] << 0x18));
}


/* Helper function to write 16-bit little-endian value */
static void __WriteLE16__(Byte_t *data_, HalfWord_t value_) {
  data_[0x0] = (Byte_t) (value_ & 0xFFu);
  data_[0x1] = (Byte_t) ((value_ >> 0x8) & 0xFFu);
}


/* Helper function to write 32-bit little-endian value */
static void __WriteLE32__(Byte_t *data_, Word_t value_) {
  data_[0x0] = (Byte_t) (value_ & 0xFFu);
  data_[0x1] = (Byte_t) ((value_ >> 0x8) & 0xFFu);
  data_[0x2] = (Byte_t) ((value_ >> 0x10) & 0xFFu);
  data_[0x3] = (Byte_t) ((value_ >> 0x18) & 0xFFu);
}


/* Mount tracking - tracks which block devices are currently mounted */
#define MAX_MOUNTED_VOLUMES 0x8u
static HalfWord_t mountedDevices[MAX_MOUNTED_VOLUMES];
static Byte_t mountedDeviceCount = 0x0u;
/* Forward declarations for helper functions */
static Return_t __ReadSector__(const Volume_t *vol_, Word_t sector_, Byte_t **data_);
static Return_t __WriteSector__(const Volume_t *vol_, Word_t sector_, const Byte_t *data_);
static Return_t __ReadCluster__(const Volume_t *vol_, Word_t cluster_, Byte_t **data_);
static Return_t __GetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t *nextCluster_);
static Return_t __SetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t value_);
static Word_t __ClusterToSector__(const Volume_t *vol_, Word_t cluster_);
static Return_t __FindFreeCluster__(const Volume_t *vol_, Word_t startHint_, Word_t *freeCluster_);
static Base_t __IsDeviceMounted__(const HalfWord_t blockDeviceUID_);
static Return_t __AddMountedDevice__(const HalfWord_t blockDeviceUID_);
static Return_t __RemoveMountedDevice__(const HalfWord_t blockDeviceUID_);
static Base_t __ByteCompare__(const Byte_t *s1_, const Byte_t *s2_, Word_t len_);
static Return_t __ConvertToFAT83__(const Byte_t *path_, Byte_t *fat83_);
static Return_t __FindDirEntry__(const Volume_t *vol_, Word_t dirCluster_, const Byte_t *name83_, FAT32DirEntry_t *entry_, Word_t *entryCluster_, Word_t *
  entryOffset_);
static Return_t __FindFileByPath__(const Volume_t *vol_, const Byte_t *path_, FAT32DirEntry_t *entry_, Word_t *parentCluster_, Word_t *entryCluster_, Word_t *
  entryOffset_);
static Return_t __FreeClusters__(const Volume_t *vol_, Word_t startCluster_);
static Return_t __CreateDirEntry__(const Volume_t *vol_, Word_t parentCluster_, const Byte_t *name83_, Byte_t attr_, Word_t firstCluster_, Word_t size_);


Return_t xFSMount(Volume_t **volume_) {
  FUNCTION_ENTER;


  Volume_t *vol = null;
  Byte_t *bootSectorData = null;
  FAT32BootSector_t *bs = null;


  if(__PointerIsNotNull__(volume_)) {
    /* Check if device is already mounted */
    if(__IsDeviceMounted__(CONFIG_FS_BLOCK_DEVICE_UID)) {
      __ReturnError__();
      FUNCTION_EXIT;
    }

    /* Allocate volume structure in kernel heap memory */
    if(OK(__KernelAllocateMemory__((volatile Addr_t **) &vol, sizeof(Volume_t)))) {
      /* Store block device UID for all I/O operations */
      vol->blockDeviceUID = CONFIG_FS_BLOCK_DEVICE_UID;
      vol->mounted = false;

      /* Read boot sector (sector 0x0) */
      if(OK(__ReadSector__(vol, 0x0u, &bootSectorData))) {
        bs = (FAT32BootSector_t *) bootSectorData;


        /* Parse boot sector parameters */
        vol->bytesPerSector = __ReadLE16__(bs->bytesPerSector);
        vol->sectorsPerCluster = bs->sectorsPerCluster;
        vol->reservedSectors = __ReadLE16__(bs->reservedSectors);
        vol->numFATs = bs->numFATs;
        vol->sectorsPerFAT = __ReadLE32__(bs->FATSize32);
        vol->rootDirCluster = __ReadLE32__(bs->rootCluster);


        /* Calculate FAT and data region start sectors */
        vol->fatStartSector = vol->reservedSectors;
        vol->dataStartSector = vol->reservedSectors + (vol->numFATs * vol->sectorsPerFAT);

        /* Validate FAT32 filesystem */
        if((vol->bytesPerSector >= 0x200u) && (vol->sectorsPerCluster > 0x0u) && (vol->rootDirCluster >= 0x2u)) {
          vol->mounted = true;

          /* Add device to mounted list */
          if(OK(__AddMountedDevice__(CONFIG_FS_BLOCK_DEVICE_UID))) {
            /* Free boot sector buffer */
            if(OK(__KernelFreeMemory__(bootSectorData))) {
              *volume_ = vol;
              __ReturnOk__();
            } else {
              __AssertOnElse__();
            }
          } else {
            /* Failed to track mount - cleanup */
            __KernelFreeMemory__(bootSectorData);
            __KernelFreeMemory__(vol);
            __AssertOnElse__();
          }
        } else {
          /* Invalid FAT32 parameters */
          __KernelFreeMemory__(bootSectorData);
          __KernelFreeMemory__(vol);
          __AssertOnElse__();
        }
      } else {
        /* Failed to read boot sector */
        __KernelFreeMemory__(vol);
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xFSUnmount(Volume_t *volume_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(volume_)) {
    /* Remove device from mounted list */
    __RemoveMountedDevice__(volume_->blockDeviceUID);
    volume_->mounted = false;

    /* Free volume structure from kernel heap */
    if(OK(__KernelFreeMemory__(volume_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xFSGetVolumeInfo(const Volume_t *volume_, VolumeInfo_t **info_) {
  FUNCTION_ENTER;


  VolumeInfo_t *info = null;
  Word_t cluster = 0;
  Word_t fatEntry = 0;
  Word_t totalClusters = 0;
  Word_t freeClusters = 0;
  Word_t maxCluster = 0;


  if(__PointerIsNotNull__(volume_) && __PointerIsNotNull__(info_)) {
    /* Check if volume is mounted */
    if(!volume_->mounted) {
      __ReturnError__();
      FUNCTION_EXIT;
    }

    /* Allocate info structure in user heap (returned to caller) */
    if(OK(xMemAlloc((volatile Addr_t **) &info, sizeof(VolumeInfo_t)))) {
      info->bytesPerSector = volume_->bytesPerSector;
      info->sectorsPerCluster = volume_->sectorsPerCluster;
      info->bytesPerCluster = (Word_t) volume_->bytesPerSector * volume_->sectorsPerCluster;

      /* Calculate total clusters based on FAT size */
      /* Each FAT entry is 4 bytes, so total clusters = (sectorsPerFAT * bytesPerSector) / 4 */
      maxCluster = (volume_->sectorsPerFAT * volume_->bytesPerSector) / 4u;

      /* Limit to reasonable maximum to avoid excessive scanning */
      if(maxCluster > 0x1000u) {
        maxCluster = 0x1000u; /* Limit scan to 4K clusters for performance */
      }

      /* Count free and total clusters */
      /* Start from cluster 2 (0 and 1 are reserved) */
      for(cluster = 2u; cluster < maxCluster; cluster++) {
        if(OK(__GetFATEntry__(volume_, cluster, &fatEntry))) {
          totalClusters++;
          if(fatEntry == FAT32_FREE_CLUSTER) {
            freeClusters++;
          }
        } else {
          /* Stop counting if we can't read FAT entries */
          break;
        }
      }

      info->totalClusters = totalClusters;
      info->freeClusters = freeClusters;
      info->totalBytes = totalClusters * info->bytesPerCluster;
      info->freeBytes = freeClusters * info->bytesPerCluster;
      *info_ = info;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


Return_t xFSFormat(const Byte_t *volumeLabel_) {
  FUNCTION_ENTER;


  Volume_t tempVol;
  Byte_t *bootSector = null;
  FAT32BootSector_t *bs = null;
  HalfWord_t bytesPerSector = 0x200u;
  Byte_t sectorsPerCluster = 0x8u;
  HalfWord_t reservedSectors = 0x20u;
  Byte_t numFATs = 0x2u;
  Word_t sectorsPerFAT = 0x100u;  /* 0x100 sectors for FAT (128KB per FAT) */
  Word_t rootDirCluster = 0x2u;
  Word_t fatStart = reservedSectors;
  Word_t dataStart = reservedSectors + (numFATs * sectorsPerFAT);


  /* Temporary volume structure for formatting */
  tempVol.blockDeviceUID = CONFIG_FS_BLOCK_DEVICE_UID;
  tempVol.bytesPerSector = bytesPerSector;
  tempVol.sectorsPerCluster = sectorsPerCluster;
  tempVol.reservedSectors = reservedSectors;
  tempVol.numFATs = numFATs;
  tempVol.sectorsPerFAT = sectorsPerFAT;
  tempVol.rootDirCluster = rootDirCluster;
  tempVol.fatStartSector = fatStart;
  tempVol.dataStartSector = dataStart;
  tempVol.mounted = false;

  /* Allocate and zero boot sector from user heap (required by xDeviceWrite) */
  if(OK(xMemAlloc((volatile Addr_t **) &bootSector, bytesPerSector))) {
    __memset__(bootSector, 0x00u, bytesPerSector);
    bs = (FAT32BootSector_t *) bootSector;


    /* Fill in boot sector */
    bs->jumpBoot[0] = 0xEBu; /* Jump instruction */
    bs->jumpBoot[1] = 0x58u;
    bs->jumpBoot[2] = 0x90u;
    __memcpy__(bs->oemName, "HELIOS  ", 0x8u);
    __WriteLE16__(bs->bytesPerSector, bytesPerSector);
    bs->sectorsPerCluster = sectorsPerCluster;
    __WriteLE16__(bs->reservedSectors, reservedSectors);
    bs->numFATs = numFATs;
    __WriteLE16__(bs->rootEntryCount, 0x0u);  /* 0x0 for FAT32 */
    __WriteLE16__(bs->totalSectors16, 0x0u); /* 0x0 for FAT32 */
    bs->mediaType = 0xF8u; /* Fixed disk */
    __WriteLE16__(bs->FATSize16, 0x0u); /* 0x0 for FAT32 */
    __WriteLE16__(bs->sectorsPerTrack, 0x3Fu);
    __WriteLE16__(bs->numHeads, 0x10u);
    __WriteLE32__(bs->hiddenSectors, 0x0u);
    __WriteLE32__(bs->totalSectors32, 0x800u);  /* 1MB / 512 bytes */
    __WriteLE32__(bs->FATSize32, sectorsPerFAT);
    __WriteLE16__(bs->extFlags, 0x0u);
    __WriteLE16__(bs->fsVersion, 0x0u);
    __WriteLE32__(bs->rootCluster, rootDirCluster);
    __WriteLE16__(bs->fsInfo, 0x1u);
    __WriteLE16__(bs->backupBootSector, 0x6u);
    bs->driveNumber = 0x80u;
    bs->bootSignature = 0x29u;
    __WriteLE32__(bs->volumeID, 0x12345678u);
    __memcpy__(bs->volumeLabel, volumeLabel_, 0xBu);
    __memcpy__(bs->fsType, "FAT32   ", 0x8u);


    /* Boot sector signature */
    bootSector[0x1FEu] = 0x55u;
    bootSector[0x1FFu] = 0xAAu;

    /* Write boot sector */
    if(OK(__WriteSector__(&tempVol, 0x0u, bootSector))) {
      Byte_t *fatSector = null;
      Word_t sector = 0x0u;
      Word_t fat = 0x0u;
      Base_t fatInitSuccess = true;


      /* Free boot sector as we're done with it */
      xMemFree((Addr_t *) bootSector);

      /* Initialize FAT tables - allocate a zero-filled sector buffer */
      if(OK(xMemAlloc((volatile Addr_t **) &fatSector, bytesPerSector))) {
        __memset__(fatSector, 0x00u, bytesPerSector);

        /* Write zeros to all sectors of all FAT copies */
        for(fat = 0x0u; fat < numFATs && fatInitSuccess; fat++) {
          Word_t fatStartSector = fatStart + (fat * sectorsPerFAT);


          for(sector = 0x0u; sector < sectorsPerFAT && fatInitSuccess; sector++) {
            if(ERROR(__WriteSector__(&tempVol, fatStartSector + sector, fatSector))) {
              fatInitSuccess = false;
            }
          }
        }

        if(fatInitSuccess) {
          /* Now set special FAT entries:
           * - Cluster 0x0: Media descriptor (0x0FFFFFF8)
           * - Cluster 0x1: Clean/dirty flag (0x0FFFFFFF)
           * - Cluster 0x2: Root directory (EOC marker 0x0FFFFFFF) */
          if(OK(__SetFATEntry__(&tempVol, 0x0u, 0x0FFFFFF8u)) && OK(__SetFATEntry__(&tempVol, 0x1u, 0x0FFFFFFFu)) && OK(__SetFATEntry__(&tempVol, 0x2u,
            FAT32_EOC_MAX))) {
            /* Initialize root directory cluster to zeros */
            Word_t rootFirstSector = __ClusterToSector__(&tempVol, rootDirCluster);


            for(sector = 0x0u; sector < sectorsPerCluster && fatInitSuccess; sector++) {
              if(ERROR(__WriteSector__(&tempVol, rootFirstSector + sector, fatSector))) {
                fatInitSuccess = false;
              }
            }

            xMemFree((Addr_t *) fatSector);

            if(fatInitSuccess) {
              __ReturnOk__();
            } else {
              __AssertOnElse__();
            }
          } else {
            xMemFree((Addr_t *) fatSector);
            __AssertOnElse__();
          }
        } else {
          xMemFree((Addr_t *) fatSector);
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      xMemFree((Addr_t *) bootSector);
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xFileOpen(File_t **file_, Volume_t *volume_, const Byte_t *path_, const Byte_t mode_) {
  FUNCTION_ENTER;


  File_t *file = null;
  FAT32DirEntry_t entry;
  Base_t fileExists = false;
  Word_t parentCluster = 0;
  Word_t i = 0;


  if(__PointerIsNotNull__(file_) && __PointerIsNotNull__(volume_) && __PointerIsNotNull__(path_)) {
    /* Check if volume is mounted */
    if(!volume_->mounted) {
      __ReturnError__();
      FUNCTION_EXIT;
    }

    /* Try to find existing file */
    if(OK(__FindFileByPath__(volume_, path_, &entry, &parentCluster, null, null))) {
      fileExists = true;
    } else {
      /* If file doesn't exist, try to determine parent directory */
      const Byte_t *lastSlash = null;
      Byte_t parentPath[256];
      Word_t parentPathLen = 0;
      FAT32DirEntry_t parentEntry;


      /* Find parent directory cluster */
      for(i = 0; path_[i] != '\0'; i++) {
        if(path_[i] == '/') {
          lastSlash = &path_[i];
        }
      }

      if(__PointerIsNotNull__(lastSlash)) {
        parentPathLen = lastSlash - path_;

        if(parentPathLen == 0x0) {
          /* Parent is root */
          parentCluster = volume_->rootDirCluster;
        } else {
          __memcpy__(parentPath, path_, parentPathLen);
          parentPath[parentPathLen] = '\0';

          /* Find the parent directory and get its cluster number */
          if(OK(__FindFileByPath__(volume_, parentPath, &parentEntry, null, null, null))) {
            /* Get the cluster number of the parent directory itself */
            parentCluster = ((Word_t) __ReadLE16__(parentEntry.firstClusterHigh) << 0x10) | __ReadLE16__(parentEntry.firstClusterLow);
          } else {
            parentCluster = volume_->rootDirCluster;
          }
        }
      } else {
        /* No slash - parent is root */
        parentCluster = volume_->rootDirCluster;
      }
    }

    /* Allocate file structure in kernel heap */
    if(OK(__KernelAllocateMemory__((volatile Addr_t **) &file, sizeof(File_t)))) {
      /* Store reference to parent volume */
      file->volume = volume_;
      file->mode = mode_;
      file->position = 0;
      file->isOpen = true;
      file->isDirty = false;
      file->parentDirCluster = parentCluster;

      /* Copy path to file structure */
      for(i = 0; path_[i] != '\0' && i < 255; i++) {
        file->path[i] = path_[i];
      }

      file->path[i] = '\0';

      if(fileExists) {
        /* Open existing file - get cluster and size from directory entry */
        file->firstCluster = ((Word_t) __ReadLE16__(entry.firstClusterHigh) << 0x10) | __ReadLE16__(entry.firstClusterLow);
        file->currentCluster = file->firstCluster;
        file->fileSize = __ReadLE32__(entry.fileSize);

        /* If append mode, seek to end */
        if((mode_ & FS_MODE_APPEND) != 0x0) {
          file->position = file->fileSize;

          /* Need to navigate to last cluster for append */
          if((file->firstCluster >= 0x2u) && (file->firstCluster < FAT32_EOC_MIN)) {
            Word_t nextCluster = 0;
            Word_t clusterSize = (Word_t) volume_->bytesPerSector * volume_->sectorsPerCluster;
            Word_t clustersToSkip = file->fileSize / clusterSize;


            file->currentCluster = file->firstCluster;

            for(i = 0; i < clustersToSkip; i++) {
              if(OK(__GetFATEntry__(volume_, file->currentCluster, &nextCluster))) {
                if(nextCluster >= FAT32_EOC_MIN) {
                  break;
                }

                file->currentCluster = nextCluster;
              } else {
                break;
              }
            }
          }
        }
      } else {
        /* File doesn't exist - check if CREATE mode is set */
        if((mode_ & FS_MODE_CREATE) == 0x0) {
          /* Cannot open non-existent file without CREATE mode */
          __KernelFreeMemory__(file);
          __ReturnError__();
          FUNCTION_EXIT;
        }

        /* New file - will be created on first write */
        file->firstCluster = 0;
        file->currentCluster = 0;
        file->fileSize = 0;
      }

      *file_ = file;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    /* NULL pointer passed - return error instead of asserting */
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


Return_t xFileClose(File_t *file_) {
  FUNCTION_ENTER;


  FAT32DirEntry_t entry;
  Word_t entryCluster = 0;
  Word_t entryOffset = 0;
  Byte_t *clusterData = null;
  Word_t firstSector = 0;
  Word_t i = 0;
  const Byte_t *lastSlash = null;
  const Byte_t *fileName = null;
  Byte_t name83[11];


  if(__PointerIsNotNull__(file_)) {
    /* If file is dirty, create or update directory entry */
    if(file_->isDirty && (file_->path[0] != '\0')) {
      /* Extract just the filename from path */
      fileName = file_->path;

      for(i = 0; file_->path[i] != '\0'; i++) {
        if(file_->path[i] == '/') {
          lastSlash = &file_->path[i];
        }
      }

      if(__PointerIsNotNull__(lastSlash)) {
        fileName = lastSlash + 1;
      }

      /* Convert filename to 8.3 format */
      if(OK(__ConvertToFAT83__(fileName, name83))) {
        /* Check if file entry already exists */
        if(OK(__FindFileByPath__(file_->volume, file_->path, &entry, null, &entryCluster, &entryOffset))) {
          /* Update existing directory entry */
          if(OK(__ReadCluster__(file_->volume, entryCluster, &clusterData))) {
            FAT32DirEntry_t *fatEntry = (FAT32DirEntry_t *) (clusterData + entryOffset);


            /* Update file size and first cluster */
            __WriteLE32__(fatEntry->fileSize, file_->fileSize);
            __WriteLE16__(fatEntry->firstClusterHigh, (HalfWord_t) (file_->firstCluster >> 16));
            __WriteLE16__(fatEntry->firstClusterLow, (HalfWord_t) (file_->firstCluster & 0xFFFFu));


            /* Write directory cluster back */
            firstSector = __ClusterToSector__(file_->volume, entryCluster);

            for(i = 0; i < file_->volume->sectorsPerCluster; i++) {
              __WriteSector__(file_->volume, firstSector + i, clusterData + (i * file_->volume->bytesPerSector));
            }

            __KernelFreeMemory__(clusterData);
          }
        } else {
          /* Create new directory entry */
          __CreateDirEntry__(file_->volume, file_->parentDirCluster, name83, FAT_ATTR_ARCHIVE, file_->firstCluster, file_->fileSize);
        }
      }
    }

    file_->isOpen = false;

    /* Free file structure from kernel heap */
    if(OK(__KernelFreeMemory__(file_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    /* NULL pointer passed - return error instead of asserting */
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


Return_t xFileRead(File_t *file_, const Size_t size_, Byte_t **data_) {
  FUNCTION_ENTER;


  Byte_t *buffer = null;
  Byte_t *clusterData = null;
  Size_t bytesToRead = size_;
  Size_t bytesRead = 0;
  Word_t clusterSize = 0;
  Word_t offsetInCluster = 0;
  Word_t bytesFromCluster = 0;
  Word_t nextCluster = 0;


  if(__PointerIsNotNull__(file_) && __PointerIsNotNull__(data_) && file_->isOpen && (nil < size_)) {
    /* Initialize output pointer to null for error cases */
    *data_ = null;
    clusterSize = (Word_t) file_->volume->bytesPerSector * file_->volume->sectorsPerCluster;

    /* Don't read past EOF */
    if((file_->position + bytesToRead) > file_->fileSize) {
      bytesToRead = file_->fileSize - file_->position;
    }

    if(nil == bytesToRead) {
      /* Already at EOF */
      __AssertOnElse__();
      FUNCTION_EXIT;
    }

    /* Allocate buffer for read data from user heap (returned to caller) */
    if(OK(xMemAlloc((volatile Addr_t **) &buffer, bytesToRead))) {
      /* If not at start of file, navigate to correct cluster */
      if(file_->currentCluster == 0x0) {
        file_->currentCluster = file_->firstCluster;
      }

      /* Read data cluster by cluster */
      while(bytesRead < bytesToRead) {
        /* Read current cluster */
        if(OK(__ReadCluster__(file_->volume, file_->currentCluster, &clusterData))) {
          /* Calculate offset within cluster */
          offsetInCluster = file_->position % clusterSize;


          /* Calculate how many bytes to copy from this cluster */
          bytesFromCluster = clusterSize - offsetInCluster;

          if(bytesFromCluster > (bytesToRead - bytesRead)) {
            bytesFromCluster = bytesToRead - bytesRead;
          }

          /* Copy data from cluster to buffer */
          __memcpy__(buffer + bytesRead, clusterData + offsetInCluster, bytesFromCluster);
          bytesRead += bytesFromCluster;
          file_->position += bytesFromCluster;
          __KernelFreeMemory__(clusterData);

          /* Move to next cluster if needed */
          if(bytesRead < bytesToRead) {
            if(OK(__GetFATEntry__(file_->volume, file_->currentCluster, &nextCluster))) {
              if(nextCluster >= FAT32_EOC_MIN) {
                /* Unexpected EOF */
                break;
              }

              file_->currentCluster = nextCluster;
            } else {
              xMemFree(buffer);
              __AssertOnElse__();
              FUNCTION_EXIT;
            }
          }
        } else {
          xMemFree(buffer);
          __AssertOnElse__();
          FUNCTION_EXIT;
        }
      }

      *data_ = buffer;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xFileWrite(File_t *file_, const Size_t size_, const Byte_t *data_) {
  FUNCTION_ENTER;


  Byte_t *clusterData = null;
  Size_t bytesToWrite = size_;
  Size_t bytesWritten = 0;
  Word_t clusterSize = 0;
  Word_t offsetInCluster = 0;
  Word_t bytesToCluster = 0;
  Word_t nextCluster = 0;
  Word_t i = 0;
  Word_t firstSector = 0;


  if(__PointerIsNotNull__(file_) && __PointerIsNotNull__(data_) && file_->isOpen && (nil < size_)) {
    clusterSize = (Word_t) file_->volume->bytesPerSector * file_->volume->sectorsPerCluster;

    /* Check write mode */
    if(((file_->mode & FS_MODE_WRITE) == 0x0) && ((file_->mode & FS_MODE_APPEND) == 0x0)) {
      __AssertOnElse__();
      FUNCTION_EXIT;
    }

    /* If append mode, seek to end */
    if((file_->mode & FS_MODE_APPEND) != 0x0) {
      file_->position = file_->fileSize;
    }

    /* If at start and no clusters allocated, allocate first cluster */
    if(file_->firstCluster == 0x0) {
      /* Find a free cluster starting from cluster 3 */
      Word_t freeCluster = 0;


      if(OK(__FindFreeCluster__(file_->volume, 3u, &freeCluster))) {
        file_->firstCluster = freeCluster;
        file_->currentCluster = freeCluster;
        __SetFATEntry__(file_->volume, freeCluster, FAT32_EOC_MAX);
        file_->isDirty = true;
      } else {
        /* No free clusters available */
        __AssertOnElse__();
        FUNCTION_EXIT;
      }
    }

    /* Navigate to correct cluster if needed */
    if(file_->currentCluster == 0x0) {
      file_->currentCluster = file_->firstCluster;


      /* Seek to correct cluster based on position */
      if(file_->position > 0) {
        Word_t targetCluster = file_->position / clusterSize;
        Word_t currentClusterIdx = 0;

        /* Follow the FAT chain to reach the target cluster */
        while(currentClusterIdx < targetCluster) {
          if(OK(__GetFATEntry__(file_->volume, file_->currentCluster, &nextCluster))) {
            if(nextCluster >= FAT32_EOC_MIN) {
              /* Reached end of chain before target - need to extend the file */
              break;
            }
            file_->currentCluster = nextCluster;
            currentClusterIdx++;
          } else {
            /* Error following FAT chain */
            __AssertOnElse__();
            FUNCTION_EXIT;
          }
        }
      }
    }

    /* Write data cluster by cluster */
    while(bytesWritten < bytesToWrite) {
      /* Read-modify-write current cluster */
      if(OK(__ReadCluster__(file_->volume, file_->currentCluster, &clusterData))) {
        offsetInCluster = file_->position % clusterSize;
        bytesToCluster = clusterSize - offsetInCluster;

        if(bytesToCluster > (bytesToWrite - bytesWritten)) {
          bytesToCluster = bytesToWrite - bytesWritten;
        }

        /* Modify cluster data */
        __memcpy__(clusterData + offsetInCluster, data_ + bytesWritten, bytesToCluster);


        /* Write cluster back */
        firstSector = __ClusterToSector__(file_->volume, file_->currentCluster);

        for(i = 0; i < file_->volume->sectorsPerCluster; i++) {
          __WriteSector__(file_->volume, firstSector + i, clusterData + (i * file_->volume->bytesPerSector));
        }

        bytesWritten += bytesToCluster;
        file_->position += bytesToCluster;

        if(file_->position > file_->fileSize) {
          file_->fileSize = file_->position;
          file_->isDirty = true;
        }

        __KernelFreeMemory__(clusterData);

        /* Allocate next cluster if needed */
        if(bytesWritten < bytesToWrite) {
          if(OK(__GetFATEntry__(file_->volume, file_->currentCluster, &nextCluster))) {
            if(nextCluster >= FAT32_EOC_MIN) {
              /* Need to allocate new cluster - find a free one */
              Word_t newCluster = 0;


              /* Start searching from current cluster + 1 for better locality */
              if(OK(__FindFreeCluster__(file_->volume, file_->currentCluster + 1u, &newCluster))) {
                /* Link current cluster to new cluster */
                __SetFATEntry__(file_->volume, file_->currentCluster, newCluster);


                /* Mark new cluster as end of chain */
                __SetFATEntry__(file_->volume, newCluster, FAT32_EOC_MAX);
                nextCluster = newCluster;
              } else {
                /* No free clusters available */
                __AssertOnElse__();
                FUNCTION_EXIT;
              }
            }

            file_->currentCluster = nextCluster;
          } else {
            __AssertOnElse__();
            FUNCTION_EXIT;
          }
        }
      } else {
        __AssertOnElse__();
        FUNCTION_EXIT;
      }
    }

    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xFileSeek(File_t *file_, const Word_t offset_, const Byte_t origin_) {
  FUNCTION_ENTER;


  Word_t newPosition = 0;
  Word_t clusterSize = 0;
  Word_t clustersToSkip = 0;
  Word_t i = 0;
  Word_t nextCluster = 0;


  if(__PointerIsNotNull__(file_) && file_->isOpen) {
    clusterSize = (Word_t) file_->volume->bytesPerSector * file_->volume->sectorsPerCluster;

    /* Calculate new position based on origin */
    switch(origin_) {
    case FS_SEEK_SET: newPosition = offset_;
      break;
    case FS_SEEK_CUR: newPosition = file_->position + offset_;
      break;
    case FS_SEEK_END: newPosition = file_->fileSize + offset_;
      break;
    default: __AssertOnElse__();
      FUNCTION_EXIT;
    }

    /* Don't seek past EOF for reads */
    if(newPosition > file_->fileSize) {
      newPosition = file_->fileSize;
    }

    file_->position = newPosition;


    /* Update current cluster */
    clustersToSkip = newPosition / clusterSize;
    file_->currentCluster = file_->firstCluster;

    for(i = 0; i < clustersToSkip; i++) {
      if(OK(__GetFATEntry__(file_->volume, file_->currentCluster, &nextCluster))) {
        if(nextCluster >= FAT32_EOC_MIN) {
          break;
        }

        file_->currentCluster = nextCluster;
      } else {
        __AssertOnElse__();
        FUNCTION_EXIT;
      }
    }

    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xFileTell(const File_t *file_, Word_t *position_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(file_) && __PointerIsNotNull__(position_)) {
    *position_ = file_->position;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xFileGetSize(const File_t *file_, Word_t *size_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(file_) && __PointerIsNotNull__(size_)) {
    *size_ = file_->fileSize;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xFileSync(File_t *file_) {
  FUNCTION_ENTER;


  FAT32DirEntry_t entry;
  Word_t entryCluster = 0;
  Word_t entryOffset = 0;
  Byte_t *clusterData = null;
  Word_t firstSector = 0;
  Word_t i = 0;
  const Byte_t *lastSlash = null;
  const Byte_t *fileName = null;
  Byte_t name83[11];


  if(__PointerIsNotNull__(file_)) {
    /* If file is dirty, update directory entry */
    if(file_->isDirty && (file_->path[0] != '\0')) {
      /* Extract just the filename from path */
      fileName = file_->path;

      for(i = 0; file_->path[i] != '\0'; i++) {
        if(file_->path[i] == '/') {
          lastSlash = &file_->path[i];
        }
      }

      if(__PointerIsNotNull__(lastSlash)) {
        fileName = lastSlash + 1;
      }

      /* Convert to 8.3 format */
      if(OK(__ConvertToFAT83__(fileName, name83))) {
        /* Find the existing directory entry */
        if(OK(__FindDirEntry__(file_->volume, file_->parentDirCluster, name83, &entry, &entryCluster, &entryOffset))) {
          /* Update the directory entry with new size and cluster info */
          __WriteLE32__(entry.fileSize, file_->fileSize);
          __WriteLE16__(entry.firstClusterLow, (HalfWord_t) (file_->firstCluster & 0xFFFFu));
          __WriteLE16__(entry.firstClusterHigh, (HalfWord_t) ((file_->firstCluster >> 16) & 0xFFFFu));

          /* Read the cluster containing the directory entry */
          if(OK(__ReadCluster__(file_->volume, entryCluster, &clusterData))) {
            /* Update the entry in the cluster */
            __memcpy__(clusterData + entryOffset, &entry, sizeof(FAT32DirEntry_t));

            /* Write the cluster back */
            firstSector = __ClusterToSector__(file_->volume, entryCluster);

            for(i = 0; i < file_->volume->sectorsPerCluster; i++) {
              if(ERROR(__WriteSector__(file_->volume, firstSector + i, clusterData + (i * file_->volume->bytesPerSector)))) {
                __KernelFreeMemory__(clusterData);
                __ReturnError__();
                FUNCTION_EXIT;
              }
            }

            __KernelFreeMemory__(clusterData);
            file_->isDirty = false;
            __ReturnOk__();
          } else {
            __AssertOnElse__();
          }
        } else {
          /* Entry not found - may be a new file that wasn't written yet */
          __ReturnOk__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      /* File not dirty or no path - nothing to sync */
      __ReturnOk__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xFileTruncate(File_t *file_, const Word_t size_) {
  FUNCTION_ENTER;


  Word_t clusterSize = 0;
  Word_t clustersNeeded = 0;
  Word_t currentCluster = 0;
  Word_t nextCluster = 0;
  Word_t i = 0;


  if(__PointerIsNotNull__(file_) && file_->isOpen) {
    clusterSize = (Word_t) file_->volume->bytesPerSector * file_->volume->sectorsPerCluster;

    /* If truncating to larger size, file will be extended on write */
    if(size_ >= file_->fileSize) {
      file_->fileSize = size_;
      file_->isDirty = true;
      __ReturnOk__();
      FUNCTION_EXIT;
    }

    /* Truncating to smaller size - free excess clusters */
    clustersNeeded = (size_ + clusterSize - 1) / clusterSize;
    currentCluster = file_->firstCluster;

    /* Navigate to last needed cluster */
    for(i = 1; i < clustersNeeded && currentCluster != 0x0; i++) {
      if(OK(__GetFATEntry__(file_->volume, currentCluster, &nextCluster))) {
        if(nextCluster >= FAT32_EOC_MIN) {
          break;
        }

        currentCluster = nextCluster;
      } else {
        __AssertOnElse__();
        FUNCTION_EXIT;
      }
    }

    /* Mark this cluster as end of chain and free remaining */
    if(OK(__GetFATEntry__(file_->volume, currentCluster, &nextCluster))) {
      __SetFATEntry__(file_->volume, currentCluster, FAT32_EOC_MAX);

      /* Free remaining clusters in chain */
      while(nextCluster < FAT32_EOC_MIN) {
        Word_t clusterToFree = nextCluster;


        __GetFATEntry__(file_->volume, nextCluster, &nextCluster);
        __SetFATEntry__(file_->volume, clusterToFree, FAT32_FREE_CLUSTER);
      }
    }

    file_->fileSize = size_;
    file_->isDirty = true;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xFileEOF(const File_t *file_, Base_t *eof_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(file_) && __PointerIsNotNull__(eof_)) {
    *eof_ = (file_->position >= file_->fileSize) ? true : false;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xDirOpen(Dir_t **dir_, Volume_t *volume_, const Byte_t *path_) {
  FUNCTION_ENTER;


  Dir_t *dir = null;
  FAT32DirEntry_t entry;
  Word_t dirCluster = 0;


  if(__PointerIsNotNull__(dir_) && __PointerIsNotNull__(volume_) && __PointerIsNotNull__(path_)) {
    /* Check if volume is mounted */
    if(!volume_->mounted) {
      __ReturnError__();
      FUNCTION_EXIT;
    }

    /* Check if path is root directory */
    if((path_[0] == '/') && (path_[1] == '\0')) {
      dirCluster = volume_->rootDirCluster;
    } else {
      /* Find the directory by path */
      if(OK(__FindFileByPath__(volume_, path_, &entry, null, null, null))) {
        /* Verify it's a directory */
        if((entry.attr & FAT_ATTR_DIRECTORY) == 0x0) {
          /* Not a directory */
          __ReturnError__();
          FUNCTION_EXIT;
        }

        /* Get directory's first cluster */
        dirCluster = ((Word_t) __ReadLE16__(entry.firstClusterHigh) << 0x10) | __ReadLE16__(entry.firstClusterLow);
      } else {
        /* Directory not found */
        __ReturnError__();
        FUNCTION_EXIT;
      }
    }

    /* Allocate directory handle in kernel heap */
    if(OK(__KernelAllocateMemory__((volatile Addr_t **) &dir, sizeof(Dir_t)))) {
      dir->volume = volume_;
      dir->entryIndex = 0;
      dir->isOpen = true;
      dir->currentCluster = dirCluster;
      *dir_ = dir;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    /* NULL pointer passed - return error instead of asserting */
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


Return_t xDirClose(Dir_t *dir_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(dir_)) {
    dir_->isOpen = false;

    /* Free directory handle from kernel heap */
    if(OK(__KernelFreeMemory__(dir_))) {
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    /* NULL pointer passed - return error instead of asserting */
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


Return_t xDirRead(Dir_t *dir_, DirEntry_t **entry_) {
  FUNCTION_ENTER;


  Byte_t *clusterData = null;
  FAT32DirEntry_t *fatEntry = null;
  DirEntry_t *dirEntry = null;
  Word_t entriesPerCluster = 0;
  Word_t entryOffsetInCluster = 0;
  Word_t nextCluster = 0;


  if(__PointerIsNotNull__(dir_) && __PointerIsNotNull__(entry_) && dir_->isOpen) {
    entriesPerCluster = ((Word_t) dir_->volume->bytesPerSector * dir_->volume->sectorsPerCluster) / sizeof(FAT32DirEntry_t);

    /* Read current cluster */
    if(OK(__ReadCluster__(dir_->volume, dir_->currentCluster, &clusterData))) {
      /* Calculate entry offset within cluster */
      entryOffsetInCluster = dir_->entryIndex % entriesPerCluster;
      fatEntry = (FAT32DirEntry_t *) (clusterData + (entryOffsetInCluster * sizeof(FAT32DirEntry_t)));

      /* Skip deleted entries (first byte = 0xE5) and end marker (first byte =
       * 0x00) */
      while(fatEntry->name[0x0] == 0xE5u || fatEntry->name[0x0] == 0x00u) {
        if(fatEntry->name[0x0] == 0x00u) {
          /* End of directory */
          __KernelFreeMemory__(clusterData);
          __AssertOnElse__();
          FUNCTION_EXIT;
        }

        dir_->entryIndex++;
        entryOffsetInCluster = dir_->entryIndex % entriesPerCluster;

        /* Check if we need to read next cluster */
        if(entryOffsetInCluster == 0x0) {
          if(OK(__GetFATEntry__(dir_->volume, dir_->currentCluster, &nextCluster))) {
            if(nextCluster >= FAT32_EOC_MIN) {
              /* End of directory chain */
              __KernelFreeMemory__(clusterData);
              __AssertOnElse__();
              FUNCTION_EXIT;
            }

            __KernelFreeMemory__(clusterData);
            dir_->currentCluster = nextCluster;

            if(ERROR(__ReadCluster__(dir_->volume, dir_->currentCluster, &clusterData))) {
              __AssertOnElse__();
              FUNCTION_EXIT;
            }
          } else {
            __KernelFreeMemory__(clusterData);
            __AssertOnElse__();
            FUNCTION_EXIT;
          }
        }

        fatEntry = (FAT32DirEntry_t *) (clusterData + (entryOffsetInCluster * sizeof(FAT32DirEntry_t)));
      }

      /* Skip long filename entries */
      if((fatEntry->attr & FAT_ATTR_LONG_NAME) == FAT_ATTR_LONG_NAME) {
        /* Skip LFN entry - just increment and try again */
        dir_->entryIndex++;
        __KernelFreeMemory__(clusterData);
        __AssertOnElse__();
        FUNCTION_EXIT;
      }

      /* Allocate and fill directory entry from user heap (returned to caller)
       */
      if(OK(xMemAlloc((volatile Addr_t **) &dirEntry, sizeof(DirEntry_t)))) {
        /* Convert 8.3 filename to null-terminated string */
        Word_t i = 0;
        Word_t j = 0;


        /* Copy name part (8 chars) */
        for(i = 0; i < 8 && fatEntry->name[i] != ' '; i++) {
          dirEntry->name[j++] = fatEntry->name[i];
        }

        /* Add extension if present */
        if(fatEntry->name[8] != ' ') {
          dirEntry->name[j++] = '.';

          for(i = 8; i < 11 && fatEntry->name[i] != ' '; i++) {
            dirEntry->name[j++] = fatEntry->name[i];
          }
        }

        dirEntry->name[j] = '\0';


        /* Fill in file attributes */
        dirEntry->size = __ReadLE32__(fatEntry->fileSize);
        dirEntry->firstCluster = ((Word_t) __ReadLE16__(fatEntry->firstClusterHigh) << 0x10) | __ReadLE16__(fatEntry->firstClusterLow);
        dirEntry->isDirectory = (fatEntry->attr & FAT_ATTR_DIRECTORY) ? true : false;
        dirEntry->isReadOnly = (fatEntry->attr & FAT_ATTR_READ_ONLY) ? true : false;
        dirEntry->isHidden = (fatEntry->attr & FAT_ATTR_HIDDEN) ? true : false;
        dirEntry->isSystem = (fatEntry->attr & FAT_ATTR_SYSTEM) ? true : false;
        dir_->entryIndex++;
        __KernelFreeMemory__(clusterData);
        *entry_ = dirEntry;
        __ReturnOk__();
      } else {
        __KernelFreeMemory__(clusterData);
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xDirRewind(Dir_t *dir_) {
  FUNCTION_ENTER;

  if(__PointerIsNotNull__(dir_)) {
    dir_->entryIndex = 0;
    __ReturnOk__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


Return_t xDirMake(Volume_t *volume_, const Byte_t *path_) {
  FUNCTION_ENTER;


  Word_t parentCluster = 0;
  Word_t newDirCluster = 0;
  Byte_t dirName83[11];
  Byte_t *clusterData = null;
  FAT32DirEntry_t *dotEntry = null;
  FAT32DirEntry_t *dotdotEntry = null;
  Word_t firstSector = 0;
  Word_t i = 0;
  const Byte_t *lastSlash = null;
  const Byte_t *dirName = null;
  Byte_t parentPath[256] = {
    0
  };
  Word_t parentPathLen = 0;


  if(__PointerIsNotNull__(volume_) && __PointerIsNotNull__(path_)) {
    /* Check if volume is mounted */
    if(!volume_->mounted) {
      __ReturnError__();
      FUNCTION_EXIT;
    }

    /* Extract directory name and parent path */
    dirName = path_;
    lastSlash = null;

    for(i = 0; path_[i] != '\0'; i++) {
      if(path_[i] == '/') {
        lastSlash = &path_[i];
      }
    }

    if(__PointerIsNotNull__(lastSlash)) {
      dirName = lastSlash + 1;


      /* Extract parent path */
      parentPathLen = lastSlash - path_;

      if(parentPathLen == 0x0) {
        /* Parent is root */
        parentPath[0] = '/';
        parentPath[1] = '\0';
      } else {
        __memcpy__(parentPath, path_, parentPathLen);
        parentPath[parentPathLen] = '\0';
      }
    } else {
      /* No slash, parent is root */
      parentPath[0] = '/';
      parentPath[1] = '\0';
    }

    /* Find parent directory */
    if(ERROR(__FindFileByPath__(volume_, parentPath, null, &parentCluster, null, null))) {
      __AssertOnElse__();
      FUNCTION_EXIT;
    }

    /* Convert directory name to 8.3 format */
    if(ERROR(__ConvertToFAT83__(dirName, dirName83))) {
      __AssertOnElse__();
      FUNCTION_EXIT;
    }

    /* Allocate a cluster for the new directory */
    if(OK(__FindFreeCluster__(volume_, 3u, &newDirCluster))) {
      /* Mark cluster as end of chain */
      if(OK(__SetFATEntry__(volume_, newDirCluster, FAT32_EOC_MAX))) {
        /* Initialize directory cluster with . and .. entries */
        if(OK(__ReadCluster__(volume_, newDirCluster, &clusterData))) {
          /* Zero out the cluster */
          __memset__(clusterData, 0x00u, (Word_t) volume_->bytesPerSector * volume_->sectorsPerCluster);


          /* Create . entry (self reference) */
          dotEntry = (FAT32DirEntry_t *) clusterData;
          __memcpy__(dotEntry->name, ".          ", 11);
          dotEntry->attr = FAT_ATTR_DIRECTORY;
          __WriteLE16__(dotEntry->firstClusterHigh, (HalfWord_t) (newDirCluster >> 16));
          __WriteLE16__(dotEntry->firstClusterLow, (HalfWord_t) (newDirCluster & 0xFFFFu));
          __WriteLE32__(dotEntry->fileSize, 0);


          /* Create .. entry (parent reference) */
          dotdotEntry = (FAT32DirEntry_t *) (clusterData + sizeof(FAT32DirEntry_t));
          __memcpy__(dotdotEntry->name, "..         ", 11);
          dotdotEntry->attr = FAT_ATTR_DIRECTORY;
          __WriteLE16__(dotdotEntry->firstClusterHigh, (HalfWord_t) (parentCluster >> 16));
          __WriteLE16__(dotdotEntry->firstClusterLow, (HalfWord_t) (parentCluster & 0xFFFFu));
          __WriteLE32__(dotdotEntry->fileSize, 0);


          /* Write directory cluster */
          firstSector = __ClusterToSector__(volume_, newDirCluster);

          for(i = 0; i < volume_->sectorsPerCluster; i++) {
            if(ERROR(__WriteSector__(volume_, firstSector + i, clusterData + (i * volume_->bytesPerSector)))) {
              __KernelFreeMemory__(clusterData);
              __AssertOnElse__();
              FUNCTION_EXIT;
            }
          }

          __KernelFreeMemory__(clusterData);

          /* Add entry to parent directory */
          if(OK(__CreateDirEntry__(volume_, parentCluster, dirName83, FAT_ATTR_DIRECTORY, newDirCluster, 0))) {
            __ReturnOk__();
          } else {
            /* Failed to add entry - free the cluster */
            __SetFATEntry__(volume_, newDirCluster, FAT32_FREE_CLUSTER);
            __AssertOnElse__();
          }
        } else {
          /* Failed to read cluster - free it */
          __SetFATEntry__(volume_, newDirCluster, FAT32_FREE_CLUSTER);
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      /* No free clusters */
      __AssertOnElse__();
    }
  } else {
    /* NULL pointer passed - return error instead of asserting */
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


Return_t xDirRemove(Volume_t *volume_, const Byte_t *path_) {
  FUNCTION_ENTER;


  FAT32DirEntry_t entry;
  Word_t entryCluster = 0;
  Word_t entryOffset = 0;
  Word_t dirCluster = 0;
  Byte_t *clusterData = null;
  FAT32DirEntry_t *fatEntry = null;
  Word_t entriesPerCluster = 0;
  Word_t entryIdx = 0;
  Word_t firstSector = 0;
  Word_t i = 0;
  Base_t isEmpty = true;
  Word_t currentCluster = 0;
  Word_t nextCluster = 0;


  if(__PointerIsNotNull__(volume_) && __PointerIsNotNull__(path_)) {
    /* Check if volume is mounted */
    if(!volume_->mounted) {
      __ReturnError__();
      FUNCTION_EXIT;
    }

    /* Find the directory entry */
    if(OK(__FindFileByPath__(volume_, path_, &entry, null, &entryCluster, &entryOffset))) {
      /* Must be a directory */
      if((entry.attr & FAT_ATTR_DIRECTORY) == 0x0) {
        __AssertOnElse__();
        FUNCTION_EXIT;
      }

      /* Get directory's first cluster */
      dirCluster = ((Word_t) __ReadLE16__(entry.firstClusterHigh) << 16) | __ReadLE16__(entry.firstClusterLow);


      /* Check if directory is empty (only . and .. entries) */
      /* Must check all clusters in the directory's cluster chain */
      entriesPerCluster = ((Word_t) volume_->bytesPerSector * volume_->sectorsPerCluster) / sizeof(FAT32DirEntry_t);
      currentCluster = dirCluster;
      nextCluster = 0;

      /* Iterate through all clusters in directory */
      while(isEmpty && currentCluster >= 2 && currentCluster < FAT32_EOC_MIN) {
        if(OK(__ReadCluster__(volume_, currentCluster, &clusterData))) {
          for(entryIdx = 0; entryIdx < entriesPerCluster; entryIdx++) {
            fatEntry = (FAT32DirEntry_t *) (clusterData + (entryIdx * sizeof(FAT32DirEntry_t)));

            /* End of directory? */
            if(fatEntry->name[0x0] == 0x00u) {
              __KernelFreeMemory__(clusterData);
              goto check_empty;
            }

            /* Skip deleted, . and .. entries */
            if((fatEntry->name[0x0] == 0xE5u) || __ByteCompare__(fatEntry->name, (const Byte_t *) ".          ", 0xB) || __ByteCompare__(fatEntry->name, (const
              Byte_t *) "..         ", 0xB)) {
              continue;
            }

            /* Found a real entry - directory is not empty */
            isEmpty = false;
            break;
          }

          __KernelFreeMemory__(clusterData);

          if(!isEmpty) {
            break;
          }

          /* Move to next cluster */
          if(OK(__GetFATEntry__(volume_, currentCluster, &nextCluster))) {
            currentCluster = nextCluster;
          } else {
            break;
          }
        } else {
          __AssertOnElse__();
          FUNCTION_EXIT;
        }
      }

check_empty:

      if(!isEmpty) {
        /* Directory not empty - cannot remove */
        __ReturnError__();
        FUNCTION_EXIT;
      }

      /* Directory is empty - mark entry as deleted */
      if(OK(__ReadCluster__(volume_, entryCluster, &clusterData))) {
        clusterData[entryOffset] = 0xE5u;


        /* Write directory cluster back */
        firstSector = __ClusterToSector__(volume_, entryCluster);

        for(i = 0; i < volume_->sectorsPerCluster; i++) {
          if(ERROR(__WriteSector__(volume_, firstSector + i, clusterData + (i * volume_->bytesPerSector)))) {
            __KernelFreeMemory__(clusterData);
            __AssertOnElse__();
            FUNCTION_EXIT;
          }
        }

        __KernelFreeMemory__(clusterData);

        /* Free directory's cluster chain */
        if((dirCluster >= 0x2u) && (dirCluster < FAT32_EOC_MIN)) {
          if(OK(__FreeClusters__(volume_, dirCluster))) {
            __ReturnOk__();
          } else {
            __AssertOnElse__();
          }
        } else {
          __ReturnOk__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      /* Directory not found */
      __ReturnError__();
    }
  } else {
    /* NULL pointer passed - return error instead of asserting */
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


Return_t xFileExists(Volume_t *volume_, const Byte_t *path_, Base_t *exists_) {
  FUNCTION_ENTER;


  FAT32DirEntry_t entry;


  if(__PointerIsNotNull__(volume_) && __PointerIsNotNull__(path_) && __PointerIsNotNull__(exists_)) {
    /* Check if volume is mounted */
    if(!volume_->mounted) {
      *exists_ = false;
      __ReturnError__();
      FUNCTION_EXIT;
    }

    /* Try to find the file using helper function */
    if(OK(__FindFileByPath__(volume_, path_, &entry, null, null, null))) {
      *exists_ = true;
      __ReturnOk__();
    } else {
      *exists_ = false;
      __ReturnOk__();
    }
  } else {
    /* NULL pointer passed - return error instead of asserting */
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


Return_t xFileUnlink(Volume_t *volume_, const Byte_t *path_) {
  FUNCTION_ENTER;


  FAT32DirEntry_t entry;
  Word_t entryCluster = 0;
  Word_t entryOffset = 0;
  Word_t firstCluster = 0;
  Byte_t *clusterData = null;
  Word_t firstSector = 0;
  Word_t i = 0;


  if(__PointerIsNotNull__(volume_) && __PointerIsNotNull__(path_)) {
    /* Check if volume is mounted */
    if(!volume_->mounted) {
      __ReturnError__();
      FUNCTION_EXIT;
    }

    /* Find the file and get its location */
    if(OK(__FindFileByPath__(volume_, path_, &entry, null, &entryCluster, &entryOffset))) {
      /* Don't allow deleting directories with this function */
      if((entry.attr & FAT_ATTR_DIRECTORY) != 0x0) {
        __AssertOnElse__();
        FUNCTION_EXIT;
      }

      /* Get first cluster to free */
      firstCluster = ((Word_t) __ReadLE16__(entry.firstClusterHigh) << 0x10) | __ReadLE16__(entry.firstClusterLow);

      /* Read directory cluster containing the entry */
      if(OK(__ReadCluster__(volume_, entryCluster, &clusterData))) {
        /* Mark entry as deleted (first byte = 0xE5) */
        clusterData[entryOffset] = 0xE5u;


        /* Write directory cluster back */
        firstSector = __ClusterToSector__(volume_, entryCluster);

        for(i = 0; i < volume_->sectorsPerCluster; i++) {
          if(ERROR(__WriteSector__(volume_, firstSector + i, clusterData + (i * volume_->bytesPerSector)))) {
            __KernelFreeMemory__(clusterData);
            __AssertOnElse__();
            FUNCTION_EXIT;
          }
        }

        __KernelFreeMemory__(clusterData);

        /* Free file's cluster chain if it has one */
        if((firstCluster >= 0x2u) && (firstCluster < FAT32_EOC_MIN)) {
          if(OK(__FreeClusters__(volume_, firstCluster))) {
            __ReturnOk__();
          } else {
            __AssertOnElse__();
          }
        } else {
          /* File had no clusters allocated (zero-length file) */
          __ReturnOk__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      /* File not found */
      __ReturnError__();
    }
  } else {
    /* NULL pointer passed - return error instead of asserting */
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


Return_t xFileRename(Volume_t *volume_, const Byte_t *oldPath_, const Byte_t *newPath_) {
  FUNCTION_ENTER;


  FAT32DirEntry_t oldEntry;
  Word_t oldEntryCluster = 0;
  Word_t oldEntryOffset = 0;
  Word_t newParentCluster = 0;
  Byte_t newName83[11];
  Byte_t *clusterData = null;
  Word_t firstSector = 0;
  Word_t i = 0;
  Word_t firstCluster = 0;
  Word_t fileSize = 0;
  Byte_t attr = 0;
  const Byte_t *lastSlash = null;
  const Byte_t *fileName = null;


  if(__PointerIsNotNull__(volume_) && __PointerIsNotNull__(oldPath_) && __PointerIsNotNull__(newPath_)) {
    /* Check if volume is mounted */
    if(!volume_->mounted) {
      __ReturnError__();
      FUNCTION_EXIT;
    }

    /* Find the old file entry */
    if(OK(__FindFileByPath__(volume_, oldPath_, &oldEntry, null, &oldEntryCluster, &oldEntryOffset))) {
      /* Extract just the filename from newPath */
      fileName = newPath_;
      lastSlash = null;

      for(i = 0; newPath_[i] != '\0'; i++) {
        if(newPath_[i] == '/') {
          lastSlash = &newPath_[i];
        }
      }

      if(__PointerIsNotNull__(lastSlash)) {
        fileName = lastSlash + 1;
      }

      /* Convert new filename to 8.3 format */
      if(ERROR(__ConvertToFAT83__(fileName, newName83))) {
        __AssertOnElse__();
        FUNCTION_EXIT;
      }

      /* For simplicity, assume rename is in same directory */
      /* Get parent directory from old path */
      if(ERROR(__FindFileByPath__(volume_, oldPath_, null, &newParentCluster, null, null))) {
        __AssertOnElse__();
        FUNCTION_EXIT;
      }

      /* Get file attributes and cluster info from old entry */
      firstCluster = ((Word_t) __ReadLE16__(oldEntry.firstClusterHigh) << 0x10) | __ReadLE16__(oldEntry.firstClusterLow);
      fileSize = __ReadLE32__(oldEntry.fileSize);
      attr = oldEntry.attr;

      /* Create new directory entry */
      if(OK(__CreateDirEntry__(volume_, newParentCluster, newName83, attr, firstCluster, fileSize))) {
        /* Mark old entry as deleted */
        if(OK(__ReadCluster__(volume_, oldEntryCluster, &clusterData))) {
          clusterData[oldEntryOffset] = 0xE5u;


          /* Write directory cluster back */
          firstSector = __ClusterToSector__(volume_, oldEntryCluster);

          for(i = 0; i < volume_->sectorsPerCluster; i++) {
            if(ERROR(__WriteSector__(volume_, firstSector + i, clusterData + (i * volume_->bytesPerSector)))) {
              __KernelFreeMemory__(clusterData);
              __AssertOnElse__();
              FUNCTION_EXIT;
            }
          }

          __KernelFreeMemory__(clusterData);
          __ReturnOk__();
        } else {
          __AssertOnElse__();
        }
      } else {
        __AssertOnElse__();
      }
    } else {
      /* Old file not found */
      __ReturnError__();
    }
  } else {
    /* NULL pointer passed - return error instead of asserting */
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


Return_t xFileGetInfo(Volume_t *volume_, const Byte_t *path_, DirEntry_t **entry_) {
  FUNCTION_ENTER;


  FAT32DirEntry_t fatEntry;
  DirEntry_t *dirEntry = null;


  if(__PointerIsNotNull__(volume_) && __PointerIsNotNull__(path_) && __PointerIsNotNull__(entry_)) {
    /* Check if volume is mounted */
    if(!volume_->mounted) {
      __ReturnError__();
      FUNCTION_EXIT;
    }

    /* Find the file */
    if(OK(__FindFileByPath__(volume_, path_, &fatEntry, null, null, null))) {
      /* Allocate and fill directory entry from user heap (returned to caller)
       */
      if(OK(xMemAlloc((volatile Addr_t **) &dirEntry, sizeof(DirEntry_t)))) {
        /* Convert 8.3 filename to null-terminated string */
        Word_t i = 0;
        Word_t j = 0;


        /* Copy name part (8 chars) */
        for(i = 0; i < 8 && fatEntry.name[i] != ' '; i++) {
          dirEntry->name[j++] = fatEntry.name[i];
        }

        /* Add extension if present */
        if(fatEntry.name[8] != ' ') {
          dirEntry->name[j++] = '.';

          for(i = 8; i < 11 && fatEntry.name[i] != ' '; i++) {
            dirEntry->name[j++] = fatEntry.name[i];
          }
        }

        dirEntry->name[j] = '\0';


        /* Fill in file attributes */
        dirEntry->size = __ReadLE32__(fatEntry.fileSize);
        dirEntry->firstCluster = ((Word_t) __ReadLE16__(fatEntry.firstClusterHigh) << 0x10) | __ReadLE16__(fatEntry.firstClusterLow);
        dirEntry->isDirectory = (fatEntry.attr & FAT_ATTR_DIRECTORY) ? true : false;
        dirEntry->isReadOnly = (fatEntry.attr & FAT_ATTR_READ_ONLY) ? true : false;
        dirEntry->isHidden = (fatEntry.attr & FAT_ATTR_HIDDEN) ? true : false;
        dirEntry->isSystem = (fatEntry.attr & FAT_ATTR_SYSTEM) ? true : false;
        *entry_ = dirEntry;
        __ReturnOk__();
      } else {
        __AssertOnElse__();
      }
    } else {
      /* File not found */
      __AssertOnElse__();
    }
  } else {
    /* NULL pointer passed - return error instead of asserting */
    __ReturnError__();
  }

  FUNCTION_EXIT;
}


/* ============================================================================
* Internal Helper Functions
* ========================================================================== */


/**
 * @brief Simple byte-by-byte comparison for FAT names
 * @param  s1_  First buffer
 * @param  s2_  Second buffer
 * @param  len_ Number of bytes to compare
 * @return      true if equal, false if not equal
 */
static Base_t __ByteCompare__(const Byte_t *s1_, const Byte_t *s2_, Word_t len_) {
  Word_t i = 0;


  for(i = 0; i < len_; i++) {
    if(s1_[i] != s2_[i]) {
      return(false);
    }
  }

  return(true);
}


/**
 * @brief Convert a filename to 8.3 FAT format
 * @param  path_  Input filename (e.g., "FILE.TXT" or "file.txt")
 * @param  fat83_ Output buffer (11 bytes) for FAT 8.3 name
 * @return        ReturnOK on success, ReturnError on invalid name
 */
static Return_t __ConvertToFAT83__(const Byte_t *path_, Byte_t *fat83_) {
  Word_t i = 0;
  Word_t j = 0;
  Word_t nameLen = 0;
  Word_t extLen = 0;
  const Byte_t *dotPos = null;


  if(__PointerIsNull__(path_) || __PointerIsNull__(fat83_)) {
    return(ReturnError);
  }

  /* Initialize output to spaces */
  for(i = 0; i < 11; i++) {
    fat83_[i] = ' ';
  }

  /* Find dot position for extension */
  for(i = 0; path_[i] != '\0'; i++) {
    if(path_[i] == '.') {
      dotPos = &path_[i];
    }
  }

  /* Copy name part (up to 8 chars, before dot or end) */
  nameLen = 0;

  for(i = 0; path_[i] != '\0' && path_[i] != '.' && nameLen < 8; i++) {
    Byte_t c = path_[i];


    /* Convert to uppercase */
    if((c >= 'a') && (c <= 'z')) {
      c = c - 'a' + 'A';
    }

    fat83_[nameLen++] = c;
  }

  /* Copy extension part (up to 3 chars, after dot) */
  if(__PointerIsNotNull__(dotPos)) {
    extLen = 0;

    for(j = 1; dotPos[j] != '\0' && extLen < 3; j++) {
      Byte_t c = dotPos[j];


      /* Convert to uppercase */
      if((c >= 'a') && (c <= 'z')) {
        c = c - 'a' + 'A';
      }

      fat83_[8 + extLen++] = c;
    }
  }

  return(ReturnOK);
}


/**
 * @brief Search a directory cluster chain for a file/directory entry
 * @param  vol_          Pointer to mounted volume
 * @param  dirCluster_   Starting cluster of directory to search
 * @param  name83_       8.3 FAT format name to search for (11 bytes)
 * @param  entry_        Pointer to receive directory entry if found (can be
 *                       NULL)
 * @param  entryCluster_ Pointer to receive cluster containing the entry (can be
 *                       NULL)
 * @param  entryOffset_  Pointer to receive byte offset within cluster (can be
 *                       NULL)
 * @return               ReturnOK if found, ReturnError if not found
 */
static Return_t __FindDirEntry__(const Volume_t *vol_, Word_t dirCluster_, const Byte_t *name83_, FAT32DirEntry_t *entry_, Word_t *entryCluster_, Word_t *
  entryOffset_) {
  Byte_t *clusterData = null;
  FAT32DirEntry_t *fatEntry = null;
  Word_t entriesPerCluster = 0;
  Word_t entryIdx = 0;
  Word_t currentCluster = dirCluster_;
  Word_t nextCluster = 0;
  Base_t found = false;


  if(__PointerIsNull__(vol_) || __PointerIsNull__(name83_)) {
    return(ReturnError);
  }

  entriesPerCluster = ((Word_t) vol_->bytesPerSector * vol_->sectorsPerCluster) / sizeof(FAT32DirEntry_t);

  /* Search through directory cluster chain */
  while(!found && currentCluster < FAT32_EOC_MIN) {
    /* Read directory cluster */
    if(ERROR(__ReadCluster__(vol_, currentCluster, &clusterData))) {
      return(ReturnError);
    }

    /* Search entries in this cluster */
    for(entryIdx = 0; entryIdx < entriesPerCluster; entryIdx++) {
      fatEntry = (FAT32DirEntry_t *) (clusterData + (entryIdx * sizeof(FAT32DirEntry_t)));

      /* Check for end of directory */
      if(fatEntry->name[0x0] == 0x00u) {
        __KernelFreeMemory__(clusterData);

        return(ReturnError); /* Not found */
      }

      /* Skip deleted entries and long filename entries */
      if((fatEntry->name[0x0] == 0xE5u) || ((fatEntry->attr & FAT_ATTR_LONG_NAME) == FAT_ATTR_LONG_NAME)) {
        continue;
      }

      /* Compare filenames */
      if(__ByteCompare__(fatEntry->name, name83_, 11)) {
        /* Found it! */
        if(__PointerIsNotNull__(entry_)) {
          __memcpy__(entry_, fatEntry, sizeof(FAT32DirEntry_t));
        }

        if(__PointerIsNotNull__(entryCluster_)) {
          *entryCluster_ = currentCluster;
        }

        if(__PointerIsNotNull__(entryOffset_)) {
          *entryOffset_ = entryIdx * sizeof(FAT32DirEntry_t);
        }

        __KernelFreeMemory__(clusterData);

        return(ReturnOK);
      }
    }

    __KernelFreeMemory__(clusterData);

    /* Move to next cluster in chain */
    if(OK(__GetFATEntry__(vol_, currentCluster, &nextCluster))) {
      currentCluster = nextCluster;
    } else {
      return(ReturnError);
    }
  }

  return(ReturnError); /* Not found */
}


/**
 * @brief Parse a path and find the corresponding file/directory entry
 * @param  vol_           Pointer to mounted volume
 * @param  path_          Path to find (e.g., "/dir/file.txt" or "file.txt")
 * @param  entry_         Pointer to receive directory entry if found (can be
 *                        NULL)
 * @param  parentCluster_ Pointer to receive parent directory cluster (can be
 *                        NULL)
 * @param  entryCluster_  Pointer to receive cluster containing the entry (can
 *                        be NULL)
 * @param  entryOffset_   Pointer to receive byte offset within cluster (can be
 *                        NULL)
 * @return                ReturnOK if found, ReturnError if not found
 */
static Return_t __FindFileByPath__(const Volume_t *vol_, const Byte_t *path_, FAT32DirEntry_t *entry_, Word_t *parentCluster_, Word_t *entryCluster_, Word_t *
  entryOffset_) {
  Byte_t name83[11];
  Byte_t component[256];
  Word_t pathIdx = 0;
  Word_t componentIdx = 0;
  Word_t currentCluster = 0;
  FAT32DirEntry_t dirEntry;
  Return_t result = ReturnError;


  if(__PointerIsNull__(vol_) || __PointerIsNull__(path_)) {
    return(ReturnError);
  }

  /* Start at root directory */
  currentCluster = vol_->rootDirCluster;

  /* Skip leading slash if present */
  if(path_[0] == '/') {
    pathIdx = 1;
  }

  /* If path is just "/" or empty, return root directory info */
  if(path_[pathIdx] == '\0') {
    if(__PointerIsNotNull__(entry_)) {
      __memset__(entry_, 0x00u, sizeof(FAT32DirEntry_t));
      entry_->attr = FAT_ATTR_DIRECTORY;
    }

    if(__PointerIsNotNull__(parentCluster_)) {
      *parentCluster_ = currentCluster;
    }

    if(__PointerIsNotNull__(entryCluster_)) {
      *entryCluster_ = currentCluster;
    }

    if(__PointerIsNotNull__(entryOffset_)) {
      *entryOffset_ = 0;
    }

    return(ReturnOK);
  }

  /* Parse path components and traverse directories */
  while(path_[pathIdx] != '\0') {
    /* Extract next component */
    componentIdx = 0;

    while(path_[pathIdx] != '\0' && path_[pathIdx] != '/' && componentIdx < 255) {
      component[componentIdx++] = path_[pathIdx++];
    }

    component[componentIdx] = '\0';

    /* Skip slash */
    if(path_[pathIdx] == '/') {
      pathIdx++;
    }

    /* Convert component to 8.3 format */
    if(ERROR(__ConvertToFAT83__(component, name83))) {
      return(ReturnError);
    }

    /* Search for this component in current directory */
    result = __FindDirEntry__(vol_, currentCluster, name83, &dirEntry, entryCluster_, entryOffset_);

    if(ERROR(result)) {
      return(ReturnError); /* Component not found */
    }

    /* If this is the last component, we're done */
    if(path_[pathIdx] == '\0') {
      if(__PointerIsNotNull__(entry_)) {
        __memcpy__(entry_, &dirEntry, sizeof(FAT32DirEntry_t));
      }

      if(__PointerIsNotNull__(parentCluster_)) {
        *parentCluster_ = currentCluster;
      }

      return(ReturnOK);
    }

    /* Otherwise, move into this directory (must be a directory) */
    if((dirEntry.attr & FAT_ATTR_DIRECTORY) == 0x0) {
      return(ReturnError); /* Not a directory, can't traverse further */
    }

    /* Update parent and current cluster for next iteration */
    currentCluster = ((Word_t) __ReadLE16__(dirEntry.firstClusterHigh) << 0x10) | __ReadLE16__(dirEntry.firstClusterLow);
  }

  return(result);
}


/**
 * @brief Free all clusters in a cluster chain
 * @param  vol_          Pointer to mounted volume
 * @param  startCluster_ First cluster in chain to free
 * @return               ReturnOK on success, ReturnError on failure
 */
static Return_t __FreeClusters__(const Volume_t *vol_, Word_t startCluster_) {
  Word_t currentCluster = startCluster_;
  Word_t nextCluster = 0;


  if(__PointerIsNull__(vol_)) {
    return(ReturnError);
  }

  /* Follow cluster chain and free each cluster */
  while(currentCluster >= 2 && currentCluster < FAT32_EOC_MIN) {
    /* Get next cluster before we free this one */
    if(ERROR(__GetFATEntry__(vol_, currentCluster, &nextCluster))) {
      return(ReturnError);
    }

    /* Free this cluster */
    if(ERROR(__SetFATEntry__(vol_, currentCluster, FAT32_FREE_CLUSTER))) {
      return(ReturnError);
    }

    currentCluster = nextCluster;
  }

  return(ReturnOK);
}


/**
 * @brief Create a new directory entry in a parent directory
 * @param  vol_           Pointer to mounted volume
 * @param  parentCluster_ Parent directory cluster
 * @param  name83_        Filename in 8.3 FAT format (11 bytes)
 * @param  attr_          File attributes
 * @param  firstCluster_  First cluster of file/directory
 * @param  size_          File size (0 for directories)
 * @return                ReturnOK on success, ReturnError on failure
 */
static Return_t __CreateDirEntry__(const Volume_t *vol_, Word_t parentCluster_, const Byte_t *name83_, Byte_t attr_, Word_t firstCluster_, Word_t size_) {
  Byte_t *clusterData = null;
  FAT32DirEntry_t *fatEntry = null;
  Word_t entriesPerCluster = 0;
  Word_t entryIdx = 0;
  Word_t currentCluster = parentCluster_;
  Word_t nextCluster = 0;
  Word_t firstSector = 0;
  Word_t i = 0;
  Base_t found = false;


  if(__PointerIsNull__(vol_) || __PointerIsNull__(name83_)) {
    return(ReturnError);
  }

  entriesPerCluster = ((Word_t) vol_->bytesPerSector * vol_->sectorsPerCluster) / sizeof(FAT32DirEntry_t);

  /* Search through directory cluster chain for free entry */
  while(!found && currentCluster < FAT32_EOC_MIN) {
    /* Read directory cluster */
    if(ERROR(__ReadCluster__(vol_, currentCluster, &clusterData))) {
      return(ReturnError);
    }

    /* Search for free entry in this cluster */
    for(entryIdx = 0; entryIdx < entriesPerCluster; entryIdx++) {
      fatEntry = (FAT32DirEntry_t *) (clusterData + (entryIdx * sizeof(FAT32DirEntry_t)));

      /* Check for free entry (deleted or end marker) */
      if((fatEntry->name[0x0] == 0x00u) || (fatEntry->name[0x0] == 0xE5u)) {
        /* Found free entry! Fill it in */
        __memcpy__(fatEntry->name, name83_, 0xB);
        fatEntry->attr = attr_;
        fatEntry->ntReserved = 0x0;
        fatEntry->createTimeTenth = 0;
        __WriteLE16__(fatEntry->createTime, 0);
        __WriteLE16__(fatEntry->createDate, 0);
        __WriteLE16__(fatEntry->lastAccessDate, 0);
        __WriteLE16__(fatEntry->firstClusterHigh, (HalfWord_t) (firstCluster_ >> 16));
        __WriteLE16__(fatEntry->writeTime, 0);
        __WriteLE16__(fatEntry->writeDate, 0);
        __WriteLE16__(fatEntry->firstClusterLow, (HalfWord_t) (firstCluster_ & 0xFFFFu));
        __WriteLE32__(fatEntry->fileSize, size_);


        /* Write cluster back */
        firstSector = __ClusterToSector__(vol_, currentCluster);

        for(i = 0; i < vol_->sectorsPerCluster; i++) {
          if(ERROR(__WriteSector__(vol_, firstSector + i, clusterData + (i * vol_->bytesPerSector)))) {
            __KernelFreeMemory__(clusterData);

            return(ReturnError);
          }
        }

        __KernelFreeMemory__(clusterData);

        return(ReturnOK);
      }
    }

    __KernelFreeMemory__(clusterData);

    /* Move to next cluster in chain */
    if(OK(__GetFATEntry__(vol_, currentCluster, &nextCluster))) {
      if(nextCluster >= FAT32_EOC_MIN) {
        /* Need to allocate new cluster for directory */
        Word_t newCluster = 0;


        if(OK(__FindFreeCluster__(vol_, currentCluster + 1u, &newCluster))) {
          /* Link current cluster to new cluster */
          __SetFATEntry__(vol_, currentCluster, newCluster);


          /* Mark new cluster as end of chain */
          __SetFATEntry__(vol_, newCluster, FAT32_EOC_MAX);

          /* Zero out new cluster */
          if(OK(__ReadCluster__(vol_, newCluster, &clusterData))) {
            __memset__(clusterData, 0x00u, (Word_t) vol_->bytesPerSector * vol_->sectorsPerCluster);
            firstSector = __ClusterToSector__(vol_, newCluster);

            for(i = 0; i < vol_->sectorsPerCluster; i++) {
              __WriteSector__(vol_, firstSector + i, clusterData + (i * vol_->bytesPerSector));
            }

            __KernelFreeMemory__(clusterData);
          }

          currentCluster = newCluster;
          continue;
        } else {
          return(ReturnError); /* No free clusters */
        }
      }

      currentCluster = nextCluster;
    } else {
      return(ReturnError);
    }
  }

  return(ReturnError); /* Shouldn't reach here */
}


/**
 * @brief Read a single sector from the block device
 * @param  vol_    Pointer to mounted volume
 * @param  sector_ Sector number to read
 * @param  data_   Pointer to receive allocated buffer with sector data
 * @return         ReturnOK on success, ReturnError on failure
 */
static Return_t __ReadSector__(const Volume_t *vol_, Word_t sector_, Byte_t **data_) {
  FUNCTION_ENTER;


  Size_t blockSize = sizeof(BlockDeviceCommand_t);
  BlockDeviceCommand_t *cmd = null;
  Size_t readSize = 0;


  if(__PointerIsNotNull__(vol_) && __PointerIsNotNull__(data_)) {
    /* Allocate command structure from kernel heap (required by kernel-level
     * device API) */
    if(OK(__KernelAllocateMemory__((volatile Addr_t **) &cmd, blockSize))) {
      /* Set up block device command to read single sector */
      cmd->command = BLOCK_CMD_SET_ADDRESS;
      cmd->blockNumber = sector_;
      cmd->blockCount = 1;
      cmd->transferMode = BLOCK_IO_MODE_BLOCKING;

      /* Configure block device to address this sector */
      if(OK(__DeviceConfigDevice__(vol_->blockDeviceUID, &blockSize, (Addr_t *) cmd))) {
        /* Read the sector data */
        readSize = (Size_t) vol_->bytesPerSector;

        if(OK(__DeviceRead__(vol_->blockDeviceUID, &readSize, (Addr_t **) data_))) {
          __KernelFreeMemory__(cmd);
          __ReturnOk__();
        } else {
          __KernelFreeMemory__(cmd);
          __AssertOnElse__();
        }
      } else {
        __KernelFreeMemory__(cmd);
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Write a single sector to the block device
 * @param  vol_    Pointer to mounted volume
 * @param  sector_ Sector number to write
 * @param  data_   Buffer containing sector data to write
 * @return         ReturnOK on success, ReturnError on failure
 */
static Return_t __WriteSector__(const Volume_t *vol_, Word_t sector_, const Byte_t *data_) {
  FUNCTION_ENTER;


  Size_t blockSize = sizeof(BlockDeviceCommand_t);
  BlockDeviceCommand_t *cmd = null;
  Size_t writeSize = 0;


  if(__PointerIsNotNull__(vol_) && __PointerIsNotNull__(data_)) {
    /* Allocate command structure from kernel heap (required by kernel-level
     * device API) */
    if(OK(__KernelAllocateMemory__((volatile Addr_t **) &cmd, blockSize))) {
      /* Set up block device command to write single sector */
      cmd->command = BLOCK_CMD_SET_ADDRESS;
      cmd->blockNumber = sector_;
      cmd->blockCount = 1;
      cmd->transferMode = BLOCK_IO_MODE_BLOCKING;

      /* Configure block device to address this sector */
      if(OK(__DeviceConfigDevice__(vol_->blockDeviceUID, &blockSize, (Addr_t *) cmd))) {
        /* Write the sector data */
        writeSize = (Size_t) vol_->bytesPerSector;

        if(OK(__DeviceWrite__(vol_->blockDeviceUID, &writeSize, (Addr_t *) data_))) {
          __KernelFreeMemory__(cmd);
          __ReturnOk__();
        } else {
          __KernelFreeMemory__(cmd);
          __AssertOnElse__();
        }
      } else {
        __KernelFreeMemory__(cmd);
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Convert cluster number to first sector number
 * @param  vol_     Pointer to mounted volume
 * @param  cluster_ Cluster number
 * @return          First sector number of the cluster
 */
static Word_t __ClusterToSector__(const Volume_t *vol_, Word_t cluster_) {
  /* First cluster is cluster 2 in FAT32 */
  return(vol_->dataStartSector + ((cluster_ - 2u) * vol_->sectorsPerCluster));
}


/**
 * @brief Read entire cluster from volume
 * @param  vol_     Pointer to mounted volume
 * @param  cluster_ Cluster number to read
 * @param  data_    Pointer to receive allocated buffer with cluster data
 * @return          ReturnOK on success, ReturnError on failure
 */
static Return_t __ReadCluster__(const Volume_t *vol_, Word_t cluster_, Byte_t **data_) {
  FUNCTION_ENTER;


  Word_t firstSector = 0;
  Word_t clusterSize = 0;
  Byte_t *buffer = null;
  Byte_t *sectorData = null;
  Word_t i = 0;


  if(__PointerIsNotNull__(vol_) && __PointerIsNotNull__(data_)) {
    /* Calculate cluster size and first sector */
    clusterSize = (Word_t) vol_->bytesPerSector * vol_->sectorsPerCluster;
    firstSector = __ClusterToSector__(vol_, cluster_);

    /* Allocate buffer for entire cluster */
    if(OK(__KernelAllocateMemory__((volatile Addr_t **) &buffer, clusterSize))) {
      /* Read all sectors in cluster */
      for(i = 0; i < vol_->sectorsPerCluster; i++) {
        if(OK(__ReadSector__(vol_, firstSector + i, &sectorData))) {
          /* Copy sector data to cluster buffer */
          __memcpy__(buffer + (i * vol_->bytesPerSector), sectorData, vol_->bytesPerSector);


          /* Free sector buffer */
          __KernelFreeMemory__(sectorData);
        } else {
          /* Failed to read sector - clean up and fail */
          __KernelFreeMemory__(buffer);
          __AssertOnElse__();
          FUNCTION_EXIT;
        }
      }

      *data_ = buffer;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Get FAT entry for a cluster (find next cluster in chain)
 * @param  vol_         Pointer to mounted volume
 * @param  cluster_     Cluster number to look up
 * @param  nextCluster_ Pointer to receive next cluster number
 * @return              ReturnOK on success, ReturnError on failure
 */
static Return_t __GetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t *nextCluster_) {
  FUNCTION_ENTER;


  Word_t fatOffset = 0;
  Word_t fatSector = 0;
  Word_t entryOffset = 0;
  Byte_t *sectorData = null;
  Word_t fatEntry = 0;


  if(__PointerIsNotNull__(vol_) && __PointerIsNotNull__(nextCluster_)) {
    /* Calculate FAT offset (each entry is 4 bytes in FAT32) */
    fatOffset = cluster_ * 4u;
    fatSector = vol_->fatStartSector + (fatOffset / vol_->bytesPerSector);
    entryOffset = fatOffset % vol_->bytesPerSector;

    /* Read FAT sector */
    if(OK(__ReadSector__(vol_, fatSector, &sectorData))) {
      /* Read 32-bit FAT entry (mask upper 4 bits per FAT32 spec) */
      fatEntry = __ReadLE32__(sectorData + entryOffset) & 0x0FFFFFFFu;


      /* Free sector buffer */
      __KernelFreeMemory__(sectorData);
      *nextCluster_ = fatEntry;
      __ReturnOk__();
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Set FAT entry for a cluster (update cluster chain)
 * @param  vol_     Pointer to mounted volume
 * @param  cluster_ Cluster number to update
 * @param  value_   Value to write (next cluster or marker)
 * @return          ReturnOK on success, ReturnError on failure
 */
static Return_t __SetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t value_) {
  FUNCTION_ENTER;


  Word_t fatOffset = 0;
  Word_t fatSector = 0;
  Word_t entryOffset = 0;
  Byte_t *sectorData = null;
  Byte_t i = 0;


  if(__PointerIsNotNull__(vol_)) {
    /* Calculate FAT offset (each entry is 4 bytes in FAT32) */
    fatOffset = cluster_ * 4u;
    fatSector = vol_->fatStartSector + (fatOffset / vol_->bytesPerSector);
    entryOffset = fatOffset % vol_->bytesPerSector;

    /* Read FAT sector */
    if(OK(__ReadSector__(vol_, fatSector, &sectorData))) {
      /* Write 32-bit FAT entry (preserve upper 4 bits per FAT32 spec) */
      __WriteLE32__(sectorData + entryOffset, (value_ & 0x0FFFFFFFu) | (__ReadLE32__(sectorData + entryOffset) & 0xF0000000u));

      /* Write sector back */
      if(OK(__WriteSector__(vol_, fatSector, sectorData))) {
        /* Update all FAT copies */
        for(i = 1; i < vol_->numFATs; i++) {
          __WriteSector__(vol_, fatSector + (i * vol_->sectorsPerFAT), sectorData);
        }

        __KernelFreeMemory__(sectorData);
        __ReturnOk__();
      } else {
        __KernelFreeMemory__(sectorData);
        __AssertOnElse__();
      }
    } else {
      __AssertOnElse__();
    }
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Find a free cluster in the FAT
 * @param  vol_         Pointer to mounted volume
 * @param  startHint_   Cluster number to start searching from (hint for
 *                      efficiency)
 * @param  freeCluster_ Pointer to receive free cluster number
 * @return              ReturnOK on success, ReturnError if no free cluster
 *                      found
 */
static Return_t __FindFreeCluster__(const Volume_t *vol_, Word_t startHint_, Word_t *freeCluster_) {
  FUNCTION_ENTER;


  Word_t cluster = 0;
  Word_t maxCluster = 0;
  Word_t fatEntry = 0;
  Word_t searchStart = 0;


  if(__PointerIsNotNull__(vol_) && __PointerIsNotNull__(freeCluster_)) {
    /* Calculate maximum cluster number based on FAT size */


    /* Each FAT entry is 4 bytes, so total clusters = (sectorsPerFAT *
     * bytesPerSector) / 4 */
    maxCluster = (vol_->sectorsPerFAT * vol_->bytesPerSector) / 4u;

    /* Limit to reasonable maximum to avoid excessive searching */
    if(maxCluster > 0x10000u) {
      maxCluster = 0x10000u; /* Limit to 64K clusters for now */
    }

    /* Start from hint, but ensure we start from at least cluster 3 */
    /* Clusters 0 and 1 are reserved, cluster 2 is root directory */
    searchStart = (startHint_ >= 3u) ? startHint_ : 3u;

    /* Search for free cluster starting from hint */
    for(cluster = searchStart; cluster < maxCluster; cluster++) {
      if(OK(__GetFATEntry__(vol_, cluster, &fatEntry))) {
        if(fatEntry == FAT32_FREE_CLUSTER) {
          /* Found a free cluster */
          *freeCluster_ = cluster;
          __ReturnOk__();
          FUNCTION_EXIT;
        }
      } else {
        /* Error reading FAT entry */
        __AssertOnElse__();
        FUNCTION_EXIT;
      }
    }

    /* If we didn't find anything from the hint to end, search from cluster 3 to
     * hint */
    if(searchStart > 3u) {
      for(cluster = 3u; cluster < searchStart; cluster++) {
        if(OK(__GetFATEntry__(vol_, cluster, &fatEntry))) {
          if(fatEntry == FAT32_FREE_CLUSTER) {
            /* Found a free cluster */
            *freeCluster_ = cluster;
            __ReturnOk__();
            FUNCTION_EXIT;
          }
        } else {
          /* Error reading FAT entry */
          __AssertOnElse__();
          FUNCTION_EXIT;
        }
      }
    }

    /* No free cluster found */
    __AssertOnElse__();
  } else {
    __AssertOnElse__();
  }

  FUNCTION_EXIT;
}


/**
 * @brief Check if a block device is already mounted
 * @param  blockDeviceUID_ Block device UID to check
 * @return                 true if mounted, false if not
 */
static Base_t __IsDeviceMounted__(const HalfWord_t blockDeviceUID_) {
  Byte_t i = 0;


  for(i = 0; i < mountedDeviceCount; i++) {
    if(mountedDevices[i] == blockDeviceUID_) {
      return(true);
    }
  }

  return(false);
}


/**
 * @brief Add a block device to the mounted devices list
 * @param  blockDeviceUID_ Block device UID to add
 * @return                 ReturnOK on success, ReturnError if list is full
 */
static Return_t __AddMountedDevice__(const HalfWord_t blockDeviceUID_) {
  if(mountedDeviceCount < MAX_MOUNTED_VOLUMES) {
    mountedDevices[mountedDeviceCount] = blockDeviceUID_;
    mountedDeviceCount++;

    return(ReturnOK);
  }

  return(ReturnError);
}


/**
 * @brief Remove a block device from the mounted devices list
 * @param  blockDeviceUID_ Block device UID to remove
 * @return                 ReturnOK on success, ReturnError if not found
 */
static Return_t __RemoveMountedDevice__(const HalfWord_t blockDeviceUID_) {
  Byte_t i = 0;
  Byte_t j = 0;


  for(i = 0; i < mountedDeviceCount; i++) {
    if(mountedDevices[i] == blockDeviceUID_) {
      /* Shift remaining elements down */
      for(j = i; j < mountedDeviceCount - 1; j++) {
        mountedDevices[j] = mountedDevices[j + 1];
      }

      mountedDeviceCount--;

      return(ReturnOK);
    }
  }

  return(ReturnError);
}


#if defined(POSIX_ARCH_OTHER)


/* For unit testing only! */
  void __FSStateClear__(void) {
    /* Clear mount tracking state */
    mountedDeviceCount = 0;

    return;
  }


#endif /* if defined(POSIX_ARCH_OTHER) */