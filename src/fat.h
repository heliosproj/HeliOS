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
    #if defined(BLOCK_CMD_SET_ADDRESS)
      #undef BLOCK_CMD_SET_ADDRESS
    #endif
    #define BLOCK_CMD_SET_ADDRESS 0x02u
    #if defined(BLOCK_IO_MODE_BLOCKING)
      #undef BLOCK_IO_MODE_BLOCKING
    #endif
    #define BLOCK_IO_MODE_BLOCKING 0x00u
    typedef struct __attribute__ ((packed)) FAT32BootSector_s {
      Byte_t jumpBoot[0x3];
      Byte_t oemName[0x8];
      Byte_t bytesPerSector[0x2];
      Byte_t sectorsPerCluster;
      Byte_t reservedSectors[0x2];
      Byte_t numFATs;
      Byte_t rootEntryCount[0x2];
      Byte_t totalSectors16[0x2];
      Byte_t mediaType;
      Byte_t FATSize16[0x2];
      Byte_t sectorsPerTrack[0x2];
      Byte_t numHeads[0x2];
      Byte_t hiddenSectors[0x4];
      Byte_t totalSectors32[0x4];
      Byte_t FATSize32[0x4];
      Byte_t extFlags[0x2];
      Byte_t fsVersion[0x2];
      Byte_t rootCluster[0x4];
      Byte_t fsInfo[0x2];
      Byte_t backupBootSector[0x2];
      Byte_t reserved[0xC];
      Byte_t driveNumber;
      Byte_t reserved1;
      Byte_t bootSignature;
      Byte_t volumeID[0x4];
      Byte_t volumeLabel[0xB];
      Byte_t fsType[0x8];
    } FAT32BootSector_t;
    typedef struct __attribute__ ((packed)) FAT32DirEntry_s {
      Byte_t name[0xB];
      Byte_t attr;
      Byte_t ntReserved;
      Byte_t createTimeTenth;
      Byte_t createTime[0x2];
      Byte_t createDate[0x2];
      Byte_t lastAccessDate[0x2];
      Byte_t firstClusterHigh[0x2];
      Byte_t writeTime[0x2];
      Byte_t writeDate[0x2];
      Byte_t firstClusterLow[0x2];
      Byte_t fileSize[0x4];
    } FAT32DirEntry_t;
    #if defined(FAT_ATTR_READ_ONLY)
      #undef FAT_ATTR_READ_ONLY
    #endif
    #define FAT_ATTR_READ_ONLY 0x01u
    #if defined(FAT_ATTR_HIDDEN)
      #undef FAT_ATTR_HIDDEN
    #endif
    #define FAT_ATTR_HIDDEN 0x02u
    #if defined(FAT_ATTR_SYSTEM)
      #undef FAT_ATTR_SYSTEM
    #endif
    #define FAT_ATTR_SYSTEM 0x04u
    #if defined(FAT_ATTR_VOLUME_ID)
      #undef FAT_ATTR_VOLUME_ID
    #endif
    #define FAT_ATTR_VOLUME_ID 0x08u
    #if defined(FAT_ATTR_DIRECTORY)
      #undef FAT_ATTR_DIRECTORY
    #endif
    #define FAT_ATTR_DIRECTORY 0x10u
    #if defined(FAT_ATTR_ARCHIVE)
      #undef FAT_ATTR_ARCHIVE
    #endif
    #define FAT_ATTR_ARCHIVE 0x20u
    #if defined(FAT_ATTR_LONG_NAME)
      #undef FAT_ATTR_LONG_NAME
    #endif
    #define FAT_ATTR_LONG_NAME 0x0Fu
    #if defined(FAT32_EOC_MIN)
      #undef FAT32_EOC_MIN
    #endif
    #define FAT32_EOC_MIN 0x0FFFFFF8u
    #if defined(FAT32_EOC_MAX)
      #undef FAT32_EOC_MAX
    #endif
    #define FAT32_EOC_MAX 0x0FFFFFFFu
    #if defined(FAT32_BAD_CLUSTER)
      #undef FAT32_BAD_CLUSTER
    #endif
    #define FAT32_BAD_CLUSTER 0x0FFFFFF7u
    #if defined(FAT32_FREE_CLUSTER)
      #undef FAT32_FREE_CLUSTER
    #endif
    #define FAT32_FREE_CLUSTER 0x00000000u
    #if defined(FAT32_ENTRY_MASK)
      #undef FAT32_ENTRY_MASK
    #endif
    #define FAT32_ENTRY_MASK 0x0FFFFFFFu
    #if defined(FAT32_RESERVED_BITS_MASK)
      #undef FAT32_RESERVED_BITS_MASK
    #endif
    #define FAT32_RESERVED_BITS_MASK 0xF0000000u
    #if defined(FAT32_MEDIA_DESCRIPTOR_ENTRY)
      #undef FAT32_MEDIA_DESCRIPTOR_ENTRY
    #endif
    #define FAT32_MEDIA_DESCRIPTOR_ENTRY 0x0FFFFFF8u
    #if defined(FAT32_ENTRY_SIZE_BYTES)
      #undef FAT32_ENTRY_SIZE_BYTES
    #endif
    #define FAT32_ENTRY_SIZE_BYTES 0x4u
    #if defined(FAT_83_NAME_LENGTH)
      #undef FAT_83_NAME_LENGTH
    #endif
    #define FAT_83_NAME_LENGTH 0xBu
    #if defined(FAT_83_BASENAME_LENGTH)
      #undef FAT_83_BASENAME_LENGTH
    #endif
    #define FAT_83_BASENAME_LENGTH 0x8u
    #if defined(FAT_83_EXTENSION_LENGTH)
      #undef FAT_83_EXTENSION_LENGTH
    #endif
    #define FAT_83_EXTENSION_LENGTH 0x3u
    #if defined(FAT_83_EXTENSION_OFFSET)
      #undef FAT_83_EXTENSION_OFFSET
    #endif
    #define FAT_83_EXTENSION_OFFSET 0x8u
    #if defined(FAT_ENTRY_DELETED)
      #undef FAT_ENTRY_DELETED
    #endif
    #define FAT_ENTRY_DELETED 0xE5u
    #if defined(FAT_BYTE_MASK)
      #undef FAT_BYTE_MASK
    #endif
    #define FAT_BYTE_MASK 0xFFu
    #if defined(FAT_BYTE_SHIFT_8)
      #undef FAT_BYTE_SHIFT_8
    #endif
    #define FAT_BYTE_SHIFT_8 0x8
    #if defined(FAT_BYTE_SHIFT_16)
      #undef FAT_BYTE_SHIFT_16
    #endif
    #define FAT_BYTE_SHIFT_16 0x10
    #if defined(FAT_BYTE_SHIFT_24)
      #undef FAT_BYTE_SHIFT_24
    #endif
    #define FAT_BYTE_SHIFT_24 0x18
    #if defined(FAT_CLUSTER_HIGH_SHIFT)
      #undef FAT_CLUSTER_HIGH_SHIFT
    #endif
    #define FAT_CLUSTER_HIGH_SHIFT 0x10u
    #if defined(FAT_CLUSTER_LOW_MASK)
      #undef FAT_CLUSTER_LOW_MASK
    #endif
    #define FAT_CLUSTER_LOW_MASK 0xFFFFu
    #if defined(FAT_MIN_VALID_CLUSTER)
      #undef FAT_MIN_VALID_CLUSTER
    #endif
    #define FAT_MIN_VALID_CLUSTER 0x2u
    #if defined(FAT_CLUSTER_DATA_OFFSET)
      #undef FAT_CLUSTER_DATA_OFFSET
    #endif
    #define FAT_CLUSTER_DATA_OFFSET 0x2u
    #if defined(FAT_CLUSTER_INCREMENT)
      #undef FAT_CLUSTER_INCREMENT
    #endif
    #define FAT_CLUSTER_INCREMENT 0x1u
    #if defined(FAT_MIN_SEARCH_CLUSTER)
      #undef FAT_MIN_SEARCH_CLUSTER
    #endif
    #define FAT_MIN_SEARCH_CLUSTER 0x3u
    #if defined(FAT_OEM_NAME_SIZE)
      #undef FAT_OEM_NAME_SIZE
    #endif
    #define FAT_OEM_NAME_SIZE 0x8u
    #if defined(FAT_VOLUME_LABEL_SIZE)
      #undef FAT_VOLUME_LABEL_SIZE
    #endif
    #define FAT_VOLUME_LABEL_SIZE 0xBu
    #if defined(FAT_FS_TYPE_SIZE)
      #undef FAT_FS_TYPE_SIZE
    #endif
    #define FAT_FS_TYPE_SIZE 0x8u
    #if defined(FAT_BOOT_JMP_EB)
      #undef FAT_BOOT_JMP_EB
    #endif
    #define FAT_BOOT_JMP_EB 0xEBu
    #if defined(FAT_BOOT_JMP_58)
      #undef FAT_BOOT_JMP_58
    #endif
    #define FAT_BOOT_JMP_58 0x58u
    #if defined(FAT_BOOT_JMP_90)
      #undef FAT_BOOT_JMP_90
    #endif
    #define FAT_BOOT_JMP_90 0x90u
    #if defined(FAT_BOOT_SIG_55)
      #undef FAT_BOOT_SIG_55
    #endif
    #define FAT_BOOT_SIG_55 0x55u
    #if defined(FAT_BOOT_SIG_AA)
      #undef FAT_BOOT_SIG_AA
    #endif
    #define FAT_BOOT_SIG_AA 0xAAu
    #if defined(FAT_MEDIA_TYPE_HARD_DISK)
      #undef FAT_MEDIA_TYPE_HARD_DISK
    #endif
    #define FAT_MEDIA_TYPE_HARD_DISK 0xF8u
    #if defined(FAT_DEFAULT_SECTORS_PER_TRACK)
      #undef FAT_DEFAULT_SECTORS_PER_TRACK
    #endif
    #define FAT_DEFAULT_SECTORS_PER_TRACK 0x3Fu
    #if defined(FAT_DEFAULT_NUM_HEADS)
      #undef FAT_DEFAULT_NUM_HEADS
    #endif
    #define FAT_DEFAULT_NUM_HEADS 0x10u
    #if defined(FAT_DRIVE_NUMBER_HDD)
      #undef FAT_DRIVE_NUMBER_HDD
    #endif
    #define FAT_DRIVE_NUMBER_HDD 0x80u
    #if defined(FAT_EXTENDED_BOOT_SIG)
      #undef FAT_EXTENDED_BOOT_SIG
    #endif
    #define FAT_EXTENDED_BOOT_SIG 0x29u
    #if defined(FAT_FSINFO_SECTOR)
      #undef FAT_FSINFO_SECTOR
    #endif
    #define FAT_FSINFO_SECTOR 0x1u
    #if defined(FAT_BACKUP_BOOT_SECTOR)
      #undef FAT_BACKUP_BOOT_SECTOR
    #endif
    #define FAT_BACKUP_BOOT_SECTOR 0x6u
    #if defined(FAT_LE16_HIGH_BYTE_OFFSET)
      #undef FAT_LE16_HIGH_BYTE_OFFSET
    #endif
    #define FAT_LE16_HIGH_BYTE_OFFSET 0x1
    #ifdef __cplusplus
      extern "C" {
    #endif
    HalfWord_t __ReadLE16__(const Byte_t *data_);
    Word_t __ReadLE32__(const Byte_t *data_);
    void __WriteLE16__(Byte_t *data_, HalfWord_t value_);
    void __WriteLE32__(Byte_t *data_, Word_t value_);
    Return_t __ReadSector__(const Volume_t *vol_, Word_t sector_, Byte_t **data_);
    Return_t __WriteSector__(const Volume_t *vol_, Word_t sector_, const Byte_t *data_);
    Return_t __ReadCluster__(const Volume_t *vol_, Word_t cluster_, Byte_t **data_);
    Return_t __GetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t *nextCluster_);
    Return_t __SetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t value_);
    Return_t __FindFreeCluster__(const Volume_t *vol_, Word_t startHint_, Word_t *freeCluster_);
    Return_t __FreeClusters__(const Volume_t *vol_, Word_t startCluster_);
    Word_t __ClusterToSector__(const Volume_t *vol_, Word_t cluster_);
    Base_t __IsDeviceMounted__(const HalfWord_t blockDeviceUID_);
    Return_t __AddMountedDevice__(const HalfWord_t blockDeviceUID_);
    Return_t __RemoveMountedDevice__(const HalfWord_t blockDeviceUID_);
    Base_t __ByteCompare__(const Byte_t *s1_, const Byte_t *s2_, Word_t len_);
    Return_t __ConvertToFAT83__(const Byte_t *path_, Byte_t *fat83_);
    Return_t __FindDirEntry__(const Volume_t *vol_, Word_t dirCluster_, const Byte_t *name83_, FAT32DirEntry_t *entry_, Word_t *entryCluster_, Word_t *
      entryOffset_);
    Return_t __FindFileByPath__(const Volume_t *vol_, const Byte_t *path_, FAT32DirEntry_t *entry_, Word_t *parentCluster_, Word_t *entryCluster_, Word_t *
      entryOffset_);
    Return_t __CreateDirEntry__(const Volume_t *vol_, Word_t parentCluster_, const Byte_t *name83_, Byte_t attr_, Word_t firstCluster_, Word_t size_);
    #if defined(POSIX_ARCH_OTHER)
      void __FSStateClear__(void);
    #endif
    #ifdef __cplusplus
      }
    #endif
  #endif
#endif