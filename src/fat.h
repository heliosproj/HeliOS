/*UNCRUSTIFY-OFF*/
/**
 * @file fat.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief FAT32 filesystem utilities API header
 * @details
 * Defines FAT32 boot sector structures, directory entry formats, and helper function prototypes for cluster and filename operations.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    #define BLOCK_CMD_SET_ADDRESS 0x02u

    #define BLOCK_IO_MODE_BLOCKING 0x00u

    /**
     * @brief FAT32 boot sector structure
     * @details Packed structure representing the FAT32 filesystem boot sector
     * with all metadata fields.
     */
    typedef struct __attribute__ ((packed)) FAT32BootSector_s {

      Byte_t jumpBoot[0x3]; /**< Jump instruction to boot code */

      Byte_t oemName[0x8]; /**< OEM name string */

      Byte_t bytesPerSector[0x2]; /**< Bytes per sector (little-endian) */

      Byte_t sectorsPerCluster; /**< Sectors per cluster */

      Byte_t reservedSectors[0x2]; /**< Number of reserved sectors
                                    * (little-endian) */

      Byte_t numFATs; /**< Number of FAT copies */

      Byte_t rootEntryCount[0x2]; /**< Root directory entry count
                                   * (little-endian, 0 for FAT32) */

      Byte_t totalSectors16[0x2]; /**< Total sectors if < 65536 (little-endian)
                                   */

      Byte_t mediaType; /**< Media descriptor */

      Byte_t FATSize16[0x2]; /**< Sectors per FAT for FAT12/16 (little-endian)
                              */

      Byte_t sectorsPerTrack[0x2]; /**< Sectors per track (little-endian) */

      Byte_t numHeads[0x2]; /**< Number of heads (little-endian) */

      Byte_t hiddenSectors[0x4]; /**< Hidden sectors (little-endian) */

      Byte_t totalSectors32[0x4]; /**< Total sectors (little-endian) */

      Byte_t FATSize32[0x4]; /**< Sectors per FAT for FAT32 (little-endian) */

      Byte_t extFlags[0x2]; /**< Extended flags (little-endian) */

      Byte_t fsVersion[0x2]; /**< Filesystem version (little-endian) */

      Byte_t rootCluster[0x4]; /**< Root directory cluster (little-endian) */

      Byte_t fsInfo[0x2]; /**< FSInfo sector number (little-endian) */

      Byte_t backupBootSector[0x2]; /**< Backup boot sector location
                                     * (little-endian) */

      Byte_t reserved[0xC]; /**< Reserved bytes */

      Byte_t driveNumber; /**< BIOS drive number */

      Byte_t reserved1; /**< Reserved byte */

      Byte_t bootSignature; /**< Extended boot signature (0x29) */

      Byte_t volumeID[0x4]; /**< Volume serial number */

      Byte_t volumeLabel[0xB]; /**< Volume label string */

      Byte_t fsType[0x8]; /**< Filesystem type string */

    } FAT32BootSector_t;

    /**
     * @brief FAT32 directory entry structure
     * @details Packed structure representing a 32-byte directory entry in FAT32
     * filesystem.
     */
    typedef struct __attribute__ ((packed)) FAT32DirEntry_s {

      Byte_t name[0xB]; /**< 8.3 filename (space-padded) */

      Byte_t attr; /**< File attributes */

      Byte_t ntReserved; /**< Reserved for Windows NT */

      Byte_t createTimeTenth; /**< Creation time fine resolution (10ms units) */

      Byte_t createTime[0x2]; /**< Creation time (little-endian) */

      Byte_t createDate[0x2]; /**< Creation date (little-endian) */

      Byte_t lastAccessDate[0x2]; /**< Last access date (little-endian) */

      Byte_t firstClusterHigh[0x2]; /**< High 16 bits of first cluster
                                     * (little-endian) */

      Byte_t writeTime[0x2]; /**< Last write time (little-endian) */

      Byte_t writeDate[0x2]; /**< Last write date (little-endian) */

      Byte_t firstClusterLow[0x2]; /**< Low 16 bits of first cluster
                                    * (little-endian) */

      Byte_t fileSize[0x4]; /**< File size in bytes (little-endian) */

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

    #endif /* ifdef __cplusplus */
    /**
     * @brief Reads a 16-bit little-endian value
     * @details Internal helper to convert little-endian bytes to 16-bit value.
     *
     * @param[in] data_ Pointer to 2-byte little-endian data
     *
     * @return          16-bit value in host byte order
     */
    HalfWord_t __ReadLE16__(const Byte_t *data_);
    /**
     * @brief Reads a 32-bit little-endian value
     * @details Internal helper to convert little-endian bytes to 32-bit value.
     *
     * @param[in] data_ Pointer to 4-byte little-endian data
     *
     * @return          32-bit value in host byte order
     */
    Word_t __ReadLE32__(const Byte_t *data_);
    /**
     * @brief Writes a 16-bit value as little-endian
     * @details Internal helper to convert 16-bit value to little-endian bytes.
     *
     * @param[out] data_  Pointer to 2-byte buffer
     * @param[in]  value_ 16-bit value to write
     */
    void __WriteLE16__(Byte_t *data_, HalfWord_t value_);
    /**
     * @brief Writes a 32-bit value as little-endian
     * @details Internal helper to convert 32-bit value to little-endian bytes.
     *
     * @param[out] data_  Pointer to 4-byte buffer
     * @param[in]  value_ 32-bit value to write
     */
    void __WriteLE32__(Byte_t *data_, Word_t value_);
    Return_t __ReadSector__(const Volume_t *vol_, Word_t sector_, Byte_t **data_);
    Return_t __WriteSector__(const Volume_t *vol_, Word_t sector_, const Byte_t *data_);
    Return_t __ReadCluster__(const Volume_t *vol_, Word_t cluster_, Byte_t **data_);
    Return_t __GetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t *nextCluster_);
    Return_t __SetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t value_);
    Return_t __FindFreeCluster__(const Volume_t *vol_, Word_t startHint_, Word_t *freeCluster_);
    Return_t __FreeClusters__(const Volume_t *vol_, Word_t startCluster_);
    /**
     * @brief Converts cluster number to sector number
     * @details Internal helper to calculate the first sector of a cluster.
     *
     * @param[in] vol_     Pointer to volume structure
     * @param[in] cluster_ Cluster number
     *
     * @return             Sector number of the first sector in the cluster
     */
    Word_t __ClusterToSector__(const Volume_t *vol_, Word_t cluster_);
    /**
     * @brief Checks if a device is mounted
     * @details Internal function to check mount status of a block device.
     *
     * @param[in] blockDeviceUID_ Block device UID to check
     *
     * @return                    Non-zero if device is mounted, 0 otherwise
     */
    Base_t __IsDeviceMounted__(const HalfWord_t blockDeviceUID_);
    Return_t __AddMountedDevice__(const HalfWord_t blockDeviceUID_);
    Return_t __RemoveMountedDevice__(const HalfWord_t blockDeviceUID_);
    /**
     * @brief Compares two byte sequences
     * @details Internal byte-wise comparison function.
     *
     * @param[in] s1_  First byte sequence
     * @param[in] s2_  Second byte sequence
     * @param[in] len_ Number of bytes to compare
     *
     * @return         Non-zero if sequences are equal, 0 otherwise
     */
    Base_t __ByteCompare__(const Byte_t *s1_, const Byte_t *s2_, Word_t len_);
    Return_t __ConvertToFAT83__(const Byte_t *path_, Byte_t *fat83_);
    /**
     * @brief Finds directory entry in a directory
     * @details Internal function to search for a file/directory entry by name.
     *
     * @param[in]  vol_          Pointer to volume structure
     * @param[in]  dirCluster_   Directory cluster to search
     * @param[in]  name83_       FAT 8.3 name to find
     * @param[out] entry_        Pointer to store found directory entry
     * @param[out] entryCluster_ Pointer to store cluster containing entry
     * @param[out] entryOffset_  Pointer to store offset of entry within cluster
     *
     * @return                   ReturnOK if entry was found
     * @return                   ReturnError if entry not found
     */
    Return_t __FindDirEntry__(const Volume_t *vol_, Word_t dirCluster_, const Byte_t *name83_, FAT32DirEntry_t *entry_, Word_t *entryCluster_, Word_t *

      entryOffset_);
    /**
     * @brief Finds file by full path
     * @details Internal function to locate a file by traversing directory path.
     *
     * @param[in]  vol_           Pointer to volume structure
     * @param[in]  path_          Full file path
     * @param[out] entry_         Pointer to store found directory entry
     * @param[out] parentCluster_ Pointer to store parent directory cluster
     * @param[out] entryCluster_  Pointer to store cluster containing entry
     * @param[out] entryOffset_   Pointer to store offset of entry within
     *                            cluster
     *
     * @return                    ReturnOK if file was found
     * @return                    ReturnError if file not found or path invalid
     */
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