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

    #define BLOCK_CMD_SET_ADDRESS 0x02u

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

    #define FAT_ATTR_READ_ONLY 0x01u

    #define FAT_ATTR_HIDDEN 0x02u

    #define FAT_ATTR_SYSTEM 0x04u

    #define FAT_ATTR_VOLUME_ID 0x08u

    #define FAT_ATTR_DIRECTORY 0x10u

    #define FAT_ATTR_ARCHIVE 0x20u

    #define FAT_ATTR_LONG_NAME 0x0Fu

    #define FAT32_EOC_MIN 0x0FFFFFF8u 

    #define FAT32_EOC_MAX 0x0FFFFFFFu 

    #define FAT32_BAD_CLUSTER 0x0FFFFFF7u 

    #define FAT32_FREE_CLUSTER 0x00000000u 

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

