/*UNCRUSTIFY-OFF*/


/**
 * @file fs.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Filesystem abstraction layer implementation
 * @details
 * Implements FAT32 filesystem operations including volume mounting, file and directory management, and sector-level block device access.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */



/*UNCRUSTIFY-ON*/


#include "config.h"


#if defined(CONFIG_ENABLE_IO_SUBSYSTEM)


  #include "fs.h"


  #include "fat.h"
  /**
   * @brief Mounts a FAT32 filesystem volume
   * @details Allocates and initializes a volume structure by reading the boot
   * sector and FAT.
   *
   * @param[out] volume_ Pointer to store the created volume handle
   *
   * @return             ReturnOK if volume was mounted successfully
   * @return             ReturnError if mount failed, allocation failed, or
   *                     invalid filesystem
   *
   * @warning Caller is responsible for unmounting with xFSUnmount()
   */
  Return_t xFSMount(Volume_t **volume_) {

    FUNCTION_ENTER;

    Volume_t *vol = null;

    Byte_t *bootSectorData = null;

    FAT32BootSector_t *bs = null;

    if(__PointerIsNotNull__(volume_)) {

      if(!__IsDeviceMounted__(CONFIG_FS_BLOCK_DEVICE_UID)) {

        if(OK(__KernelAllocateMemory__((volatile Addr_t **) &vol, sizeof(Volume_t)))) {

          vol->blockDeviceUID = CONFIG_FS_BLOCK_DEVICE_UID;

          vol->mounted = false;

          vol->bytesPerSector = 0x200u;

          vol->valid = VALID;

          if(OK(__ReadSector__(vol, 0x0u, &bootSectorData))) {

            bs = (FAT32BootSector_t *) bootSectorData;

            vol->bytesPerSector = __ReadLE16__(bs->bytesPerSector);

            vol->sectorsPerCluster = bs->sectorsPerCluster;

            vol->reservedSectors = __ReadLE16__(bs->reservedSectors);

            vol->numFATs = bs->numFATs;

            vol->sectorsPerFAT = __ReadLE32__(bs->FATSize32);

            vol->rootDirCluster = __ReadLE32__(bs->rootCluster);

            vol->fatStartSector = vol->reservedSectors;

            vol->dataStartSector = vol->reservedSectors + (vol->numFATs * vol->sectorsPerFAT);

            if((vol->bytesPerSector >= 0x200u) && (vol->sectorsPerCluster > 0x0u) && (vol->rootDirCluster >= 0x2u)) {

              vol->mounted = true;

              if(OK(__AddMountedDevice__(CONFIG_FS_BLOCK_DEVICE_UID))) {

                if(OK(__KernelFreeMemory__(bootSectorData))) {

                  *volume_ = vol;

                  __ReturnOk__();

                } else {

                  __AssertOnElse__();

                }

              } else {

                vol->valid = INVALID;

                __KernelFreeMemory__(bootSectorData);

                __KernelFreeMemory__(vol);

                __AssertOnElse__();

              }

            } else {

              vol->valid = INVALID;

              __KernelFreeMemory__(bootSectorData);

              __KernelFreeMemory__(vol);

              __AssertOnElse__();

            }

          } else {

            vol->valid = INVALID;

            __KernelFreeMemory__(vol);

            __AssertOnElse__();

          }

        } else {

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
   * @brief Unmounts a FAT32 filesystem volume
   * @details Flushes any pending writes and frees the volume structure.
   *
   * @param[in,out] volume_ Pointer to the volume to unmount
   *
   * @return                ReturnOK if volume was unmounted successfully
   * @return                ReturnError if volume is invalid or deallocation
   *                        failed
   *
   * @warning All open files and directories must be closed before unmounting
   */
  Return_t xFSUnmount(Volume_t *volume_) {

    FUNCTION_ENTER;

    if(__ObjectIsValid__(volume_)) {

      __RemoveMountedDevice__(volume_->blockDeviceUID);

      volume_->mounted = false;

      volume_->valid = INVALID;

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


  /**
   * @brief Gets volume information and statistics
   * @details Allocates and returns detailed information about the volume
   * including capacity and free space.
   *
   * @param[in]  volume_ Pointer to the volume to query
   * @param[out] info_   Pointer to store allocated volume information structure
   *
   * @return             ReturnOK if information was retrieved successfully
   * @return             ReturnError if volume is invalid, allocation failed, or
   *                     invalid parameter
   *
   * @warning Caller is responsible for freeing the allocated structure
   */
  Return_t xFSGetVolumeInfo(const Volume_t *volume_, VolumeInfo_t **info_) {

    FUNCTION_ENTER;

    VolumeInfo_t *info = null;

    Word_t cluster = 0x0u;

    Word_t fatEntry = 0x0u;

    Word_t totalClusters = 0x0u;

    Word_t freeClusters = 0x0u;

    Word_t maxCluster = 0x0u;

    if(__ObjectIsValid__(volume_) && __PointerIsNotNull__(info_)) {

      if(volume_->mounted) {

        if(OK(xMemAlloc((volatile Addr_t **) &info, sizeof(VolumeInfo_t)))) {

          info->valid = VALID;

          info->bytesPerSector = volume_->bytesPerSector;

          info->sectorsPerCluster = volume_->sectorsPerCluster;

          info->bytesPerCluster = (Word_t) volume_->bytesPerSector * volume_->sectorsPerCluster;

          maxCluster = (volume_->sectorsPerFAT * volume_->bytesPerSector) / 4u;

          if(maxCluster > 0x1000u) {

            maxCluster = 0x1000u;

          }

          for(cluster = 2u; cluster < maxCluster; cluster++) {

            if(OK(__GetFATEntry__(volume_, cluster, &fatEntry))) {

              totalClusters++;

              if(fatEntry == FAT32_FREE_CLUSTER) {

                freeClusters++;

              }

            } else {

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

        __AssertOnElse__();

      }

    } else {

      __AssertOnElse__();

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Formats a volume with FAT32 filesystem
   * @details Creates a new FAT32 filesystem on the default block device.
   *
   * @param[in] volumeLabel_ Volume label string (11 characters max)
   *
   * @return                 ReturnOK if format was successful
   * @return                 ReturnError if format failed or device unavailable
   *
   * @warning This operation destroys all existing data on the volume
   */
  Return_t xFSFormat(const Byte_t *volumeLabel_) {

    FUNCTION_ENTER;

    Volume_t tempVol;

    Byte_t *bootSector = null;

    FAT32BootSector_t *bs = null;

    HalfWord_t bytesPerSector = 0x200u;

    Byte_t sectorsPerCluster = 0x8u;

    HalfWord_t reservedSectors = 0x20u;

    Byte_t numFATs = 0x2u;

    Word_t sectorsPerFAT = 0x100u;

    Word_t rootDirCluster = 0x2u;

    Word_t fatStart = reservedSectors;

    Word_t dataStart = reservedSectors + (numFATs * sectorsPerFAT);

    if(__PointerIsNotNull__(volumeLabel_)) {

      tempVol.valid = VALID;

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

      if(OK(xMemAlloc((volatile Addr_t **) &bootSector, bytesPerSector))) {

        __memset__(bootSector, 0x00u, bytesPerSector);

        bs = (FAT32BootSector_t *) bootSector;

        bs->jumpBoot[0] = 0xEBu;

        bs->jumpBoot[1] = 0x58u;

        bs->jumpBoot[2] = 0x90u;

        __memcpy__(bs->oemName, "HELIOS  ", 0x8u);

        __WriteLE16__(bs->bytesPerSector, bytesPerSector);

        bs->sectorsPerCluster = sectorsPerCluster;

        __WriteLE16__(bs->reservedSectors, reservedSectors);

        bs->numFATs = numFATs;

        __WriteLE16__(bs->rootEntryCount, 0x0u);

        __WriteLE16__(bs->totalSectors16, 0x0u);

        bs->mediaType = 0xF8u;

        __WriteLE16__(bs->FATSize16, 0x0u);

        __WriteLE16__(bs->sectorsPerTrack, 0x3Fu);

        __WriteLE16__(bs->numHeads, 0x10u);

        __WriteLE32__(bs->hiddenSectors, 0x0u);

        __WriteLE32__(bs->totalSectors32, 0x800u);

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

        bootSector[0x1FEu] = 0x55u;

        bootSector[0x1FFu] = 0xAAu;

        if(OK(__WriteSector__(&tempVol, 0x0u, bootSector))) {

          Byte_t *fatSector = null;

          Word_t sector = 0x0u;

          Word_t fat = 0x0u;

          Base_t fatInitSuccess = true;

          xMemFree((Addr_t *) bootSector);

          if(OK(xMemAlloc((volatile Addr_t **) &fatSector, bytesPerSector))) {

            __memset__(fatSector, 0x00u, bytesPerSector);

            for(fat = 0x0u; fat < numFATs && fatInitSuccess; fat++) {

              Word_t fatStartSector = fatStart + (fat * sectorsPerFAT);

              for(sector = 0x0u; sector < sectorsPerFAT && fatInitSuccess; sector++) {

                if(ERROR(__WriteSector__(&tempVol, fatStartSector + sector, fatSector))) {

                  fatInitSuccess = false;

                }

              }

            }

            if(fatInitSuccess) {

              if(OK(__SetFATEntry__(&tempVol, 0x0u, 0x0FFFFFF8u)) && OK(__SetFATEntry__(&tempVol, 0x1u, 0x0FFFFFFFu)) && OK(__SetFATEntry__(&tempVol, 0x2u,

                FAT32_EOC_MAX))) {

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

    } else {

      __AssertOnElse__();

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Opens a file
   * @details Allocates and initializes a file handle for the specified path
   * with the given mode.
   *
   * @param[out] file_   Pointer to store the created file handle
   * @param[in]  volume_ Pointer to the mounted volume
   * @param[in]  path_   File path (null-terminated string)
   * @param[in]  mode_   Access mode flags (FS_MODE_READ, FS_MODE_WRITE,
   *                     FS_MODE_APPEND, FS_MODE_CREATE)
   *
   * @return             ReturnOK if file was opened successfully
   * @return             ReturnError if file not found, allocation failed, or
   *                     invalid parameters
   *
   * @warning Caller is responsible for closing the file with xFileClose()
   */
  Return_t xFileOpen(File_t **file_, Volume_t *volume_, const Byte_t *path_, const Byte_t mode_) {

    FUNCTION_ENTER;

    File_t *file = null;

    FAT32DirEntry_t entry;

    Base_t fileExists = false;

    Word_t parentCluster = 0x0u;

    Word_t i = 0x0u;

    if(__PointerIsNotNull__(file_) && __ObjectIsValid__(volume_) && __PointerIsNotNull__(path_)) {

      if(volume_->mounted) {

        if(OK(__FindFileByPath__(volume_, path_, &entry, &parentCluster, null, null))) {

          fileExists = true;

        } else {

          const Byte_t *lastSlash = null;

          Byte_t parentPath[256];

          Word_t parentPathLen = 0x0u;

          FAT32DirEntry_t parentEntry;

          for(i = 0x0u; path_[i] != '\0'; i++) {

            if(path_[i] == '/') {

              lastSlash = &path_[i];

            }

          }

          if(__PointerIsNotNull__(lastSlash)) {

            parentPathLen = lastSlash - path_;

            if(parentPathLen == 0x0u) {

              parentCluster = volume_->rootDirCluster;

            } else {

              __memcpy__(parentPath, path_, parentPathLen);

              parentPath[parentPathLen] = '\0';

              if(OK(__FindFileByPath__(volume_, parentPath, &parentEntry, null, null, null))) {

                parentCluster = ((Word_t) __ReadLE16__(parentEntry.firstClusterHigh) << 0x10) | __ReadLE16__(parentEntry.firstClusterLow);

              } else {

                parentCluster = volume_->rootDirCluster;

              }

            }

          } else {

            parentCluster = volume_->rootDirCluster;

          }

        }

        if(OK(__KernelAllocateMemory__((volatile Addr_t **) &file, sizeof(File_t)))) {

          file->valid = VALID;

          file->volume = volume_;

          file->mode = mode_;

          file->position = 0x0u;

          file->isOpen = true;

          file->isDirty = false;

          file->parentDirCluster = parentCluster;

          for(i = 0x0u; path_[i] != '\0' && i < 255; i++) {

            file->path[i] = path_[i];

          }

          file->path[i] = '\0';

          if(fileExists) {

            file->firstCluster = ((Word_t) __ReadLE16__(entry.firstClusterHigh) << 0x10) | __ReadLE16__(entry.firstClusterLow);

            file->currentCluster = file->firstCluster;

            file->fileSize = __ReadLE32__(entry.fileSize);

            if((mode_ & FS_MODE_APPEND) != 0x0u) {

              file->position = file->fileSize;

              if((file->firstCluster >= 0x2u) && (file->firstCluster < FAT32_EOC_MIN)) {

                Word_t nextCluster = 0x0u;

                Word_t clusterSize = (Word_t) volume_->bytesPerSector * volume_->sectorsPerCluster;

                Word_t clustersToSkip = file->fileSize / clusterSize;

                file->currentCluster = file->firstCluster;

                for(i = 0x0u; i < clustersToSkip; i++) {

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

            *file_ = file;

            __ReturnOk__();

          } else {

            if((mode_ & FS_MODE_CREATE) != 0x0u) {

              file->firstCluster = 0x0u;

              file->currentCluster = 0x0u;

              file->fileSize = 0x0u;

              *file_ = file;

              __ReturnOk__();

            } else {

              __KernelFreeMemory__(file);

              __AssertOnElse__();

            }

          }

        } else {

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
   * @brief Closes a file
   * @details Flushes any pending writes and frees the file handle.
   *
   * @param[in,out] file_ Pointer to the file to close
   *
   * @return              ReturnOK if file was closed successfully
   * @return              ReturnError if file is invalid or deallocation failed
   */
  Return_t xFileClose(File_t *file_) {

    FUNCTION_ENTER;

    FAT32DirEntry_t entry;

    Word_t entryCluster = 0x0u;

    Word_t entryOffset = 0x0u;

    Byte_t *clusterData = null;

    Word_t firstSector = 0x0u;

    Word_t i = 0x0u;

    const Byte_t *lastSlash = null;

    const Byte_t *fileName = null;

    Byte_t name83[11];

    if(__ObjectIsValid__(file_)) {

      if(file_->isDirty && (file_->path[0] != '\0')) {

        fileName = file_->path;

        for(i = 0x0u; file_->path[i] != '\0'; i++) {

          if(file_->path[i] == '/') {

            lastSlash = &file_->path[i];

          }

        }

        if(__PointerIsNotNull__(lastSlash)) {

          fileName = lastSlash + 1;

        }

        if(OK(__ConvertToFAT83__(fileName, name83))) {

          if(OK(__FindFileByPath__(file_->volume, file_->path, &entry, null, &entryCluster, &entryOffset))) {

            if(OK(__ReadCluster__(file_->volume, entryCluster, &clusterData))) {

              FAT32DirEntry_t *fatEntry = (FAT32DirEntry_t *) (clusterData + entryOffset);

              __WriteLE32__(fatEntry->fileSize, file_->fileSize);

              __WriteLE16__(fatEntry->firstClusterHigh, (HalfWord_t) (file_->firstCluster >> 16));

              __WriteLE16__(fatEntry->firstClusterLow, (HalfWord_t) (file_->firstCluster & 0xFFFFu));

              firstSector = __ClusterToSector__(file_->volume, entryCluster);

              for(i = 0x0u; i < file_->volume->sectorsPerCluster; i++) {

                __WriteSector__(file_->volume, firstSector + i, clusterData + (i * file_->volume->bytesPerSector));

              }

              __KernelFreeMemory__(clusterData);

            }

          } else {

            __CreateDirEntry__(file_->volume, file_->parentDirCluster, name83, FAT_ATTR_ARCHIVE, file_->firstCluster, file_->fileSize);

          }

        }

      }

      file_->isOpen = false;

      file_->valid = INVALID;

      if(OK(__KernelFreeMemory__(file_))) {

        __ReturnOk__();

      } else {

        __AssertOnElse__();

      }

    } else {

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Reads data from a file
   * @details Allocates and returns data read from the current file position.
   *
   * @param[in,out] file_ Pointer to the file to read from
   * @param[in]     size_ Number of bytes to read
   * @param[out]    data_ Pointer to store allocated data buffer
   *
   * @return              ReturnOK if read was successful
   * @return              ReturnError if read failed, allocation failed, or
   *                      invalid parameters
   *
   * @warning Caller is responsible for freeing the allocated buffer
   */
  Return_t xFileRead(File_t *file_, const Size_t size_, Byte_t **data_) {

    FUNCTION_ENTER;

    Byte_t *buffer = null;

    Byte_t *clusterData = null;

    Size_t bytesToRead = size_;

    Size_t bytesRead = 0x0u;

    Word_t clusterSize = 0x0u;

    Word_t offsetInCluster = 0x0u;

    Word_t bytesFromCluster = 0x0u;

    Word_t nextCluster = 0x0u;

    Base_t success = false;

    if(__ObjectIsValid__(file_) && __PointerIsNotNull__(data_) && file_->isOpen && (0x0u < size_)) {

      *data_ = null;

      clusterSize = (Word_t) file_->volume->bytesPerSector * file_->volume->sectorsPerCluster;

      if((file_->position + bytesToRead) > file_->fileSize) {

        bytesToRead = file_->fileSize - file_->position;

      }

      if(0x0u != bytesToRead) {

        if(OK(xMemAlloc((volatile Addr_t **) &buffer, bytesToRead))) {

          if(file_->currentCluster == 0x0u) {

            file_->currentCluster = file_->firstCluster;

          }

          success = true;

          while(bytesRead < bytesToRead && success) {

            if(OK(__ReadCluster__(file_->volume, file_->currentCluster, &clusterData))) {

              offsetInCluster = file_->position % clusterSize;

              bytesFromCluster = clusterSize - offsetInCluster;

              if(bytesFromCluster > (bytesToRead - bytesRead)) {

                bytesFromCluster = bytesToRead - bytesRead;

              }

              __memcpy__(buffer + bytesRead, clusterData + offsetInCluster, bytesFromCluster);

              bytesRead += bytesFromCluster;

              file_->position += bytesFromCluster;

              __KernelFreeMemory__(clusterData);

              if(bytesRead < bytesToRead) {

                if(OK(__GetFATEntry__(file_->volume, file_->currentCluster, &nextCluster))) {

                  if(nextCluster >= FAT32_EOC_MIN) {

                    break;

                  }

                  file_->currentCluster = nextCluster;

                } else {

                  xMemFree(buffer);

                  success = false;

                  __AssertOnElse__();

                }

              }

            } else {

              xMemFree(buffer);

              success = false;

              __AssertOnElse__();

            }

          }

          if(success) {

            *data_ = buffer;

            __ReturnOk__();

          } else {

            __AssertOnElse__();

          }

        } else {

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
   * @brief Writes data to a file
   * @details Writes data to the file at the current position.
   *
   * @param[in,out] file_ Pointer to the file to write to
   * @param[in]     size_ Number of bytes to write
   * @param[in]     data_ Pointer to data buffer
   *
   * @return              ReturnOK if write was successful
   * @return              ReturnError if write failed, disk full, or invalid
   *                      parameters
   */
  Return_t xFileWrite(File_t *file_, const Size_t size_, const Byte_t *data_) {

    FUNCTION_ENTER;

    Byte_t *clusterData = null;

    Size_t bytesToWrite = size_;

    Size_t bytesWritten = 0x0u;

    Word_t clusterSize = 0x0u;

    Word_t offsetInCluster = 0x0u;

    Word_t bytesToCluster = 0x0u;

    Word_t nextCluster = 0x0u;

    Word_t i = 0x0u;

    Word_t firstSector = 0x0u;

    Base_t success = false;

    if(__ObjectIsValid__(file_) && __PointerIsNotNull__(data_) && file_->isOpen && (0x0u < size_)) {

      clusterSize = (Word_t) file_->volume->bytesPerSector * file_->volume->sectorsPerCluster;

      if(((file_->mode & FS_MODE_WRITE) != 0x0u) || ((file_->mode & FS_MODE_APPEND) != 0x0u)) {

        if((file_->mode & FS_MODE_APPEND) != 0x0u) {

          file_->position = file_->fileSize;

        }

        success = true;

        if(file_->firstCluster == 0x0u) {

          Word_t freeCluster = 0x0u;

          if(OK(__FindFreeCluster__(file_->volume, 3u, &freeCluster))) {

            file_->firstCluster = freeCluster;

            file_->currentCluster = freeCluster;

            __SetFATEntry__(file_->volume, freeCluster, FAT32_EOC_MAX);

            file_->isDirty = true;

          } else {

            success = false;

            __AssertOnElse__();

          }

        }

        if(success && (file_->currentCluster == 0x0u)) {

          file_->currentCluster = file_->firstCluster;

          if(file_->position > 0) {

            Word_t targetCluster = file_->position / clusterSize;

            Word_t currentClusterIdx = 0x0u;

            while(currentClusterIdx < targetCluster && success) {

              if(OK(__GetFATEntry__(file_->volume, file_->currentCluster, &nextCluster))) {

                if(nextCluster >= FAT32_EOC_MIN) {

                  break;

                }

                file_->currentCluster = nextCluster;

                currentClusterIdx++;

              } else {

                success = false;

                __AssertOnElse__();

              }

            }

          }

        }

        while(bytesWritten < bytesToWrite && success) {

          if(OK(__ReadCluster__(file_->volume, file_->currentCluster, &clusterData))) {

            offsetInCluster = file_->position % clusterSize;

            bytesToCluster = clusterSize - offsetInCluster;

            if(bytesToCluster > (bytesToWrite - bytesWritten)) {

              bytesToCluster = bytesToWrite - bytesWritten;

            }

            __memcpy__(clusterData + offsetInCluster, data_ + bytesWritten, bytesToCluster);

            firstSector = __ClusterToSector__(file_->volume, file_->currentCluster);

            for(i = 0x0u; i < file_->volume->sectorsPerCluster; i++) {

              __WriteSector__(file_->volume, firstSector + i, clusterData + (i * file_->volume->bytesPerSector));

            }

            bytesWritten += bytesToCluster;

            file_->position += bytesToCluster;

            if(file_->position > file_->fileSize) {

              file_->fileSize = file_->position;

              file_->isDirty = true;

            }

            __KernelFreeMemory__(clusterData);

            if(bytesWritten < bytesToWrite) {

              if(OK(__GetFATEntry__(file_->volume, file_->currentCluster, &nextCluster))) {

                if(nextCluster >= FAT32_EOC_MIN) {

                  Word_t newCluster = 0x0u;

                  if(OK(__FindFreeCluster__(file_->volume, file_->currentCluster + 1u, &newCluster))) {

                    __SetFATEntry__(file_->volume, file_->currentCluster, newCluster);

                    __SetFATEntry__(file_->volume, newCluster, FAT32_EOC_MAX);

                    nextCluster = newCluster;

                  } else {

                    success = false;

                    __AssertOnElse__();

                  }

                }

                file_->currentCluster = nextCluster;

              } else {

                success = false;

                __AssertOnElse__();

              }

            }

          } else {

            success = false;

            __AssertOnElse__();

          }

        }

        if(success) {

          __ReturnOk__();

        } else {

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
   * @brief Seeks to a position in a file
   * @details Changes the file position for subsequent read/write operations.
   *
   * @param[in,out] file_   Pointer to the file
   * @param[in]     offset_ Byte offset for the seek operation
   * @param[in]     origin_ Seek origin (FS_SEEK_SET, FS_SEEK_CUR, FS_SEEK_END)
   *
   * @return                ReturnOK if seek was successful
   * @return                ReturnError if seek failed or invalid parameters
   */
  Return_t xFileSeek(File_t *file_, const Word_t offset_, const Byte_t origin_) {

    FUNCTION_ENTER;

    Word_t newPosition = 0x0u;

    Word_t clusterSize = 0x0u;

    Word_t clustersToSkip = 0x0u;

    Word_t i = 0x0u;

    Word_t nextCluster = 0x0u;

    Base_t validOrigin = true;

    Base_t success = true;

    if(__ObjectIsValid__(file_) && file_->isOpen) {

      clusterSize = (Word_t) file_->volume->bytesPerSector * file_->volume->sectorsPerCluster;

      switch(origin_) {

      case FS_SEEK_SET: newPosition = offset_;

        break;

      case FS_SEEK_CUR: newPosition = file_->position + offset_;

        break;

      case FS_SEEK_END: newPosition = file_->fileSize + offset_;

        break;

      default:

        validOrigin = false;

        __AssertOnElse__();

        break;

      }

      if(validOrigin) {

        if((newPosition > file_->fileSize) && ((file_->mode & FS_MODE_WRITE) == 0x0u) && ((file_->mode & FS_MODE_APPEND) == 0x0u)) {

          newPosition = file_->fileSize;

        }

        file_->position = newPosition;

        clustersToSkip = newPosition / clusterSize;

        file_->currentCluster = file_->firstCluster;

        for(i = 0x0u; i < clustersToSkip && success; i++) {

          if(OK(__GetFATEntry__(file_->volume, file_->currentCluster, &nextCluster))) {

            if(nextCluster >= FAT32_EOC_MIN) {

              break;

            }

            file_->currentCluster = nextCluster;

          } else {

            success = false;

            __AssertOnElse__();

          }

        }

        if(success) {

          __ReturnOk__();

        } else {

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
   * @brief Gets the current file position
   * @details Returns the byte offset of the current read/write position.
   *
   * @param[in]  file_     Pointer to the file
   * @param[out] position_ Pointer to store the current position
   *
   * @return               ReturnOK if position was retrieved successfully
   * @return               ReturnError if file is invalid or invalid parameter
   */
  Return_t xFileTell(const File_t *file_, Word_t *position_) {

    FUNCTION_ENTER;

    if(__ObjectIsValid__(file_) && __PointerIsNotNull__(position_)) {

      *position_ = file_->position;

      __ReturnOk__();

    } else {

      __AssertOnElse__();

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Gets the size of a file
   * @details Returns the total size of the file in bytes.
   *
   * @param[in]  file_ Pointer to the file
   * @param[out] size_ Pointer to store the file size
   *
   * @return           ReturnOK if size was retrieved successfully
   * @return           ReturnError if file is invalid or invalid parameter
   */
  Return_t xFileGetSize(const File_t *file_, Word_t *size_) {

    FUNCTION_ENTER;

    if(__ObjectIsValid__(file_) && __PointerIsNotNull__(size_)) {

      *size_ = file_->fileSize;

      __ReturnOk__();

    } else {

      __AssertOnElse__();

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Synchronizes file data to disk
   * @details Flushes all pending writes to ensure data is written to the
   * storage device.
   *
   * @param[in,out] file_ Pointer to the file to sync
   *
   * @return              ReturnOK if sync was successful
   * @return              ReturnError if sync failed or file is invalid
   */
  Return_t xFileSync(File_t *file_) {

    FUNCTION_ENTER;

    FAT32DirEntry_t entry;

    Word_t entryCluster = 0x0u;

    Word_t entryOffset = 0x0u;

    Byte_t *clusterData = null;

    Word_t firstSector = 0x0u;

    Word_t i = 0x0u;

    const Byte_t *lastSlash = null;

    const Byte_t *fileName = null;

    Byte_t name83[11];

    Base_t writeSuccess = true;

    if(__ObjectIsValid__(file_)) {

      if(file_->isDirty && (file_->path[0] != '\0')) {

        fileName = file_->path;

        for(i = 0x0u; file_->path[i] != '\0'; i++) {

          if(file_->path[i] == '/') {

            lastSlash = &file_->path[i];

          }

        }

        if(__PointerIsNotNull__(lastSlash)) {

          fileName = lastSlash + 1;

        }

        if(OK(__ConvertToFAT83__(fileName, name83))) {

          if(OK(__FindDirEntry__(file_->volume, file_->parentDirCluster, name83, &entry, &entryCluster, &entryOffset))) {

            __WriteLE32__(entry.fileSize, file_->fileSize);

            __WriteLE16__(entry.firstClusterLow, (HalfWord_t) (file_->firstCluster & 0xFFFFu));

            __WriteLE16__(entry.firstClusterHigh, (HalfWord_t) ((file_->firstCluster >> 16) & 0xFFFFu));

            if(OK(__ReadCluster__(file_->volume, entryCluster, &clusterData))) {

              __memcpy__(clusterData + entryOffset, &entry, sizeof(FAT32DirEntry_t));

              firstSector = __ClusterToSector__(file_->volume, entryCluster);

              writeSuccess = true;

              for(i = 0x0u; i < file_->volume->sectorsPerCluster && writeSuccess; i++) {

                if(ERROR(__WriteSector__(file_->volume, firstSector + i, clusterData + (i * file_->volume->bytesPerSector)))) {

                  writeSuccess = false;

                }

              }

              __KernelFreeMemory__(clusterData);

              if(writeSuccess) {

                file_->isDirty = false;

                __ReturnOk__();

              } else {

                __AssertOnElse__();

              }

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

        __ReturnOk__();

      }

    } else {

      __AssertOnElse__();

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Truncates a file to specified size
   * @details Resizes the file, either expanding or shrinking it.
   *
   * @param[in,out] file_ Pointer to the file to truncate
   * @param[in]     size_ New file size in bytes
   *
   * @return              ReturnOK if truncate was successful
   * @return              ReturnError if truncate failed or file is invalid
   */
  Return_t xFileTruncate(File_t *file_, const Word_t size_) {

    FUNCTION_ENTER;

    Word_t clusterSize = 0x0u;

    Word_t clustersNeeded = 0x0u;

    Word_t currentCluster = 0x0u;

    Word_t nextCluster = 0x0u;

    Word_t i = 0x0u;

    Base_t success = true;

    if(__ObjectIsValid__(file_) && file_->isOpen) {

      clusterSize = (Word_t) file_->volume->bytesPerSector * file_->volume->sectorsPerCluster;

      if(size_ >= file_->fileSize) {

        file_->fileSize = size_;

        file_->isDirty = true;

        __ReturnOk__();

      } else {

        clustersNeeded = (size_ + clusterSize - 1) / clusterSize;

        currentCluster = file_->firstCluster;

        for(i = 1; i < clustersNeeded && currentCluster != 0x0u && success; i++) {

          if(OK(__GetFATEntry__(file_->volume, currentCluster, &nextCluster))) {

            if(nextCluster >= FAT32_EOC_MIN) {

              break;

            }

            currentCluster = nextCluster;

          } else {

            success = false;

            __AssertOnElse__();

          }

        }

        if(success) {

          if(OK(__GetFATEntry__(file_->volume, currentCluster, &nextCluster))) {

            __SetFATEntry__(file_->volume, currentCluster, FAT32_EOC_MAX);

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

      }

    } else {

      __AssertOnElse__();

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Checks if file position is at end-of-file
   * @details Returns true if the current position is at or beyond the end of
   * the file.
   *
   * @param[in]  file_ Pointer to the file to check
   * @param[out] eof_  Pointer to store the result (true if at EOF, false
   *                   otherwise)
   *
   * @return           ReturnOK if check was successful
   * @return           ReturnError if file is invalid or invalid parameter
   */
  Return_t xFileEOF(const File_t *file_, Base_t *eof_) {

    FUNCTION_ENTER;

    if(__ObjectIsValid__(file_) && __PointerIsNotNull__(eof_)) {

      *eof_ = (file_->position >= file_->fileSize) ? true : false;

      __ReturnOk__();

    } else {

      __AssertOnElse__();

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Opens a directory for reading
   * @details Allocates and initializes a directory handle for iteration.
   *
   * @param[out] dir_    Pointer to store the created directory handle
   * @param[in]  volume_ Pointer to the mounted volume
   * @param[in]  path_   Directory path (null-terminated string)
   *
   * @return             ReturnOK if directory was opened successfully
   * @return             ReturnError if directory not found, allocation failed,
   *                     or invalid parameters
   *
   * @warning Caller is responsible for closing with xDirClose()
   */
  Return_t xDirOpen(Dir_t **dir_, Volume_t *volume_, const Byte_t *path_) {

    FUNCTION_ENTER;

    Dir_t *dir = null;

    FAT32DirEntry_t entry;

    Word_t dirCluster = 0x0u;

    Base_t validPath = false;

    if(__PointerIsNotNull__(dir_) && __ObjectIsValid__(volume_) && __PointerIsNotNull__(path_)) {

      if(volume_->mounted) {

        if((path_[0] == '/') && (path_[1] == '\0')) {

          dirCluster = volume_->rootDirCluster;

          validPath = true;

        } else {

          if(OK(__FindFileByPath__(volume_, path_, &entry, null, null, null))) {

            if((entry.attr & FAT_ATTR_DIRECTORY) != 0x0u) {

              dirCluster = ((Word_t) __ReadLE16__(entry.firstClusterHigh) << 0x10) | __ReadLE16__(entry.firstClusterLow);

              validPath = true;

            } else {

              __AssertOnElse__();

            }

          } else {

            __AssertOnElse__();

          }

        }

        if(validPath) {

          if(OK(__KernelAllocateMemory__((volatile Addr_t **) &dir, sizeof(Dir_t)))) {

            dir->valid = VALID;

            dir->volume = volume_;

            dir->entryIndex = 0x0u;

            dir->isOpen = true;

            dir->currentCluster = dirCluster;

            *dir_ = dir;

            __ReturnOk__();

          } else {

            __AssertOnElse__();

          }

        } else {

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
   * @brief Closes a directory
   * @details Frees the directory handle.
   *
   * @param[in,out] dir_ Pointer to the directory to close
   *
   * @return             ReturnOK if directory was closed successfully
   * @return             ReturnError if directory is invalid or deallocation
   *                     failed
   */
  Return_t xDirClose(Dir_t *dir_) {

    FUNCTION_ENTER;

    if(__ObjectIsValid__(dir_)) {

      dir_->isOpen = false;

      dir_->valid = INVALID;

      if(OK(__KernelFreeMemory__(dir_))) {

        __ReturnOk__();

      } else {

        __AssertOnElse__();

      }

    } else {

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Reads the next directory entry
   * @details Allocates and returns the next entry in the directory.
   *
   * @param[in,out] dir_   Pointer to the directory
   * @param[out]    entry_ Pointer to store allocated directory entry
   *
   * @return               ReturnOK if entry was read successfully
   * @return               ReturnError if end of directory, allocation failed,
   *                       or invalid parameters
   *
   * @warning Caller is responsible for freeing the allocated entry
   */
  Return_t xDirRead(Dir_t *dir_, DirEntry_t **entry_) {

    FUNCTION_ENTER;

    Byte_t *clusterData = null;

    FAT32DirEntry_t *fatEntry = null;

    DirEntry_t *dirEntry = null;

    Word_t entriesPerCluster = 0x0u;

    Word_t entryOffsetInCluster = 0x0u;

    Word_t nextCluster = 0x0u;

    Base_t endOfDirectory = false;

    Base_t success = false;

    if(__ObjectIsValid__(dir_) && __PointerIsNotNull__(entry_) && dir_->isOpen) {

      entriesPerCluster = ((Word_t) dir_->volume->bytesPerSector * dir_->volume->sectorsPerCluster) / sizeof(FAT32DirEntry_t);

      if(OK(__ReadCluster__(dir_->volume, dir_->currentCluster, &clusterData))) {

        entryOffsetInCluster = dir_->entryIndex % entriesPerCluster;

        fatEntry = (FAT32DirEntry_t *) (clusterData + (entryOffsetInCluster * sizeof(FAT32DirEntry_t)));

        while((fatEntry->name[0x0u] == 0xE5u || fatEntry->name[0x0u] == 0x00u) && !endOfDirectory) {

          if(fatEntry->name[0x0u] == 0x00u) {

            endOfDirectory = true;

          } else {

            dir_->entryIndex++;

            entryOffsetInCluster = dir_->entryIndex % entriesPerCluster;

            if(entryOffsetInCluster == 0x0u) {

              if(OK(__GetFATEntry__(dir_->volume, dir_->currentCluster, &nextCluster))) {

                if(nextCluster >= FAT32_EOC_MIN) {

                  endOfDirectory = true;

                } else {

                  __KernelFreeMemory__(clusterData);

                  dir_->currentCluster = nextCluster;

                  if(OK(__ReadCluster__(dir_->volume, dir_->currentCluster, &clusterData))) {

                    fatEntry = (FAT32DirEntry_t *) (clusterData + (entryOffsetInCluster * sizeof(FAT32DirEntry_t)));

                  } else {

                    endOfDirectory = true;

                  }

                }

              } else {

                endOfDirectory = true;

              }

            } else {

              fatEntry = (FAT32DirEntry_t *) (clusterData + (entryOffsetInCluster * sizeof(FAT32DirEntry_t)));

            }

          }

        }

        if(!endOfDirectory) {

          if((fatEntry->attr & FAT_ATTR_LONG_NAME) == FAT_ATTR_LONG_NAME) {

            dir_->entryIndex++;

          } else {

            if(OK(xMemAlloc((volatile Addr_t **) &dirEntry, sizeof(DirEntry_t)))) {

              Word_t i = 0x0u;

              Word_t j = 0x0u;

              dirEntry->valid = VALID;

              for(i = 0x0u; i < 8 && fatEntry->name[i] != ' '; i++) {

                dirEntry->name[j++] = fatEntry->name[i];

              }

              if(fatEntry->name[8] != ' ') {

                dirEntry->name[j++] = '.';

                for(i = 8; i < 11 && fatEntry->name[i] != ' '; i++) {

                  dirEntry->name[j++] = fatEntry->name[i];

                }

              }

              dirEntry->name[j] = '\0';

              dirEntry->size = __ReadLE32__(fatEntry->fileSize);

              dirEntry->firstCluster = ((Word_t) __ReadLE16__(fatEntry->firstClusterHigh) << 0x10) | __ReadLE16__(fatEntry->firstClusterLow);

              dirEntry->isDirectory = (fatEntry->attr & FAT_ATTR_DIRECTORY) ? true : false;

              dirEntry->isReadOnly = (fatEntry->attr & FAT_ATTR_READ_ONLY) ? true : false;

              dirEntry->isHidden = (fatEntry->attr & FAT_ATTR_HIDDEN) ? true : false;

              dirEntry->isSystem = (fatEntry->attr & FAT_ATTR_SYSTEM) ? true : false;

              dir_->entryIndex++;

              *entry_ = dirEntry;

              success = true;

            }

          }

        }

        __KernelFreeMemory__(clusterData);

        if(success) {

          __ReturnOk__();

        } else {

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
   * @brief Rewinds directory to first entry
   * @details Resets the directory position to the beginning.
   *
   * @param[in,out] dir_ Pointer to the directory to rewind
   *
   * @return             ReturnOK if rewind was successful
   * @return             ReturnError if directory is invalid
   */
  Return_t xDirRewind(Dir_t *dir_) {

    FUNCTION_ENTER;

    if(__ObjectIsValid__(dir_)) {

      dir_->entryIndex = 0x0u;

      __ReturnOk__();

    } else {

      __AssertOnElse__();

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Creates a new directory
   * @details Creates a directory at the specified path.
   *
   * @param[in] volume_ Pointer to the mounted volume
   * @param[in] path_   Directory path to create (null-terminated string)
   *
   * @return            ReturnOK if directory was created successfully
   * @return            ReturnError if creation failed, path exists, or invalid
   *                    parameters
   */
  Return_t xDirMake(Volume_t *volume_, const Byte_t *path_) {

    FUNCTION_ENTER;

    Word_t parentCluster = 0x0u;

    Word_t newDirCluster = 0x0u;

    Byte_t dirName83[11];

    Byte_t *clusterData = null;

    FAT32DirEntry_t *dotEntry = null;

    FAT32DirEntry_t *dotdotEntry = null;

    Word_t firstSector = 0x0u;

    Word_t i = 0x0u;

    const Byte_t *lastSlash = null;

    const Byte_t *dirName = null;

    Byte_t parentPath[256] = {

      0

    };

    Word_t parentPathLen = 0x0u;

    Base_t writeSuccess = true;

    if(__ObjectIsValid__(volume_) && __PointerIsNotNull__(path_)) {

      if(volume_->mounted) {

        dirName = path_;

        lastSlash = null;

        for(i = 0x0u; path_[i] != '\0'; i++) {

          if(path_[i] == '/') {

            lastSlash = &path_[i];

          }

        }

        if(__PointerIsNotNull__(lastSlash)) {

          dirName = lastSlash + 1;

          parentPathLen = lastSlash - path_;

          if(parentPathLen == 0x0u) {

            parentPath[0] = '/';

            parentPath[1] = '\0';

          } else {

            __memcpy__(parentPath, path_, parentPathLen);

            parentPath[parentPathLen] = '\0';

          }

        } else {

          parentPath[0] = '/';

          parentPath[1] = '\0';

        }

        if(OK(__FindFileByPath__(volume_, parentPath, null, &parentCluster, null, null))) {

          if(OK(__ConvertToFAT83__(dirName, dirName83))) {

            if(OK(__FindFreeCluster__(volume_, 3u, &newDirCluster))) {

              if(OK(__SetFATEntry__(volume_, newDirCluster, FAT32_EOC_MAX))) {

                if(OK(__ReadCluster__(volume_, newDirCluster, &clusterData))) {

                  __memset__(clusterData, 0x00u, (Word_t) volume_->bytesPerSector * volume_->sectorsPerCluster);

                  dotEntry = (FAT32DirEntry_t *) clusterData;

                  __memcpy__(dotEntry->name, ".          ", 11);

                  dotEntry->attr = FAT_ATTR_DIRECTORY;

                  __WriteLE16__(dotEntry->firstClusterHigh, (HalfWord_t) (newDirCluster >> 16));

                  __WriteLE16__(dotEntry->firstClusterLow, (HalfWord_t) (newDirCluster & 0xFFFFu));

                  __WriteLE32__(dotEntry->fileSize, 0);

                  dotdotEntry = (FAT32DirEntry_t *) (clusterData + sizeof(FAT32DirEntry_t));

                  __memcpy__(dotdotEntry->name, "..         ", 11);

                  dotdotEntry->attr = FAT_ATTR_DIRECTORY;

                  __WriteLE16__(dotdotEntry->firstClusterHigh, (HalfWord_t) (parentCluster >> 16));

                  __WriteLE16__(dotdotEntry->firstClusterLow, (HalfWord_t) (parentCluster & 0xFFFFu));

                  __WriteLE32__(dotdotEntry->fileSize, 0);

                  firstSector = __ClusterToSector__(volume_, newDirCluster);

                  for(i = 0x0u; i < volume_->sectorsPerCluster && writeSuccess; i++) {

                    if(ERROR(__WriteSector__(volume_, firstSector + i, clusterData + (i * volume_->bytesPerSector)))) {

                      writeSuccess = false;

                    }

                  }

                  __KernelFreeMemory__(clusterData);

                  if(writeSuccess) {

                    if(OK(__CreateDirEntry__(volume_, parentCluster, dirName83, FAT_ATTR_DIRECTORY, newDirCluster, 0))) {

                      __ReturnOk__();

                    } else {

                      __SetFATEntry__(volume_, newDirCluster, FAT32_FREE_CLUSTER);

                      __AssertOnElse__();

                    }

                  } else {

                    __AssertOnElse__();

                  }

                } else {

                  __SetFATEntry__(volume_, newDirCluster, FAT32_FREE_CLUSTER);

                  __AssertOnElse__();

                }

              } else {

                __AssertOnElse__();

              }

            } else {

              __AssertOnElse__();

            }

          } else {

            __AssertOnElse__();

          }

        } else {

          __AssertOnElse__();

        }

      } else {

        __AssertOnElse__();

      }

    } else {

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Removes a directory
   * @details Deletes an empty directory at the specified path.
   *
   * @param[in] volume_ Pointer to the mounted volume
   * @param[in] path_   Directory path to remove (null-terminated string)
   *
   * @return            ReturnOK if directory was removed successfully
   * @return            ReturnError if removal failed, directory not empty, or
   *                    invalid parameters
   */
  Return_t xDirRemove(Volume_t *volume_, const Byte_t *path_) {

    FUNCTION_ENTER;

    FAT32DirEntry_t entry;

    Word_t entryCluster = 0x0u;

    Word_t entryOffset = 0x0u;

    Word_t dirCluster = 0x0u;

    Byte_t *clusterData = null;

    FAT32DirEntry_t *fatEntry = null;

    Word_t entriesPerCluster = 0x0u;

    Word_t entryIdx = 0x0u;

    Word_t firstSector = 0x0u;

    Word_t i = 0x0u;

    Base_t isEmpty = true;

    Word_t currentCluster = 0x0u;

    Word_t nextCluster = 0x0u;

    Base_t writeSuccess = true;

    if(__ObjectIsValid__(volume_) && __PointerIsNotNull__(path_)) {

      if(volume_->mounted) {

        if(OK(__FindFileByPath__(volume_, path_, &entry, null, &entryCluster, &entryOffset))) {

          if((entry.attr & FAT_ATTR_DIRECTORY) != 0x0u) {

            dirCluster = ((Word_t) __ReadLE16__(entry.firstClusterHigh) << 16) | __ReadLE16__(entry.firstClusterLow);

            entriesPerCluster = ((Word_t) volume_->bytesPerSector * volume_->sectorsPerCluster) / sizeof(FAT32DirEntry_t);

            currentCluster = dirCluster;

            nextCluster = 0x0u;

            while(isEmpty && currentCluster >= 2 && currentCluster < FAT32_EOC_MIN) {

              if(OK(__ReadCluster__(volume_, currentCluster, &clusterData))) {

                for(entryIdx = 0x0u; entryIdx < entriesPerCluster && isEmpty; entryIdx++) {

                  fatEntry = (FAT32DirEntry_t *) (clusterData + (entryIdx * sizeof(FAT32DirEntry_t)));

                  if(fatEntry->name[0x0u] == 0x00u) {

                    break;

                  }

                  if((fatEntry->name[0x0u] == 0xE5u) || __ByteCompare__(fatEntry->name, (const Byte_t *) ".          ", 0xB) || __ByteCompare__(fatEntry->name,

                    (

                    const Byte_t *) "..         ", 0xB)) {

                    continue;

                  }

                  isEmpty = false;

                }

                __KernelFreeMemory__(clusterData);

                if(!isEmpty) {

                  break;

                }

                if(OK(__GetFATEntry__(volume_, currentCluster, &nextCluster))) {

                  currentCluster = nextCluster;

                } else {

                  break;

                }

              } else {

                break;

              }

            }

            if(isEmpty) {

              if(OK(__ReadCluster__(volume_, entryCluster, &clusterData))) {

                clusterData[entryOffset] = 0xE5u;

                firstSector = __ClusterToSector__(volume_, entryCluster);

                for(i = 0x0u; i < volume_->sectorsPerCluster && writeSuccess; i++) {

                  if(ERROR(__WriteSector__(volume_, firstSector + i, clusterData + (i * volume_->bytesPerSector)))) {

                    writeSuccess = false;

                  }

                }

                __KernelFreeMemory__(clusterData);

                if(writeSuccess) {

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

                __AssertOnElse__();

              }

            } else {

              __AssertOnElse__();

            }

          } else {

            __AssertOnElse__();

          }

        } else {

        }

      } else {

        __AssertOnElse__();

      }

    } else {

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Checks if a file exists
   * @details Returns true if a file or directory exists at the specified path.
   *
   * @param[in]  volume_ Pointer to the mounted volume
   * @param[in]  path_   File path to check (null-terminated string)
   * @param[out] exists_ Pointer to store the result (true if exists, false
   *                     otherwise)
   *
   * @return             ReturnOK if check was successful
   * @return             ReturnError if volume is invalid or invalid parameters
   */
  Return_t xFileExists(Volume_t *volume_, const Byte_t *path_, Base_t *exists_) {

    FUNCTION_ENTER;

    FAT32DirEntry_t entry;

    if(__ObjectIsValid__(volume_) && __PointerIsNotNull__(path_) && __PointerIsNotNull__(exists_)) {

      if(volume_->mounted) {

        if(OK(__FindFileByPath__(volume_, path_, &entry, null, null, null))) {

          *exists_ = true;

          __ReturnOk__();

        } else {

          *exists_ = false;

          __ReturnOk__();

        }

      } else {

        *exists_ = false;

        __AssertOnElse__();

      }

    } else {

      __AssertOnElse__();

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Deletes a file
   * @details Removes the file at the specified path.
   *
   * @param[in] volume_ Pointer to the mounted volume
   * @param[in] path_   File path to delete (null-terminated string)
   *
   * @return            ReturnOK if file was deleted successfully
   * @return            ReturnError if deletion failed, file not found, or
   *                    invalid parameters
   */
  Return_t xFileUnlink(Volume_t *volume_, const Byte_t *path_) {

    FUNCTION_ENTER;

    FAT32DirEntry_t entry;

    Word_t entryCluster = 0x0u;

    Word_t entryOffset = 0x0u;

    Word_t firstCluster = 0x0u;

    Byte_t *clusterData = null;

    Word_t firstSector = 0x0u;

    Word_t i = 0x0u;

    Base_t writeSuccess = true;

    if(__ObjectIsValid__(volume_) && __PointerIsNotNull__(path_)) {

      if(volume_->mounted) {

        if(OK(__FindFileByPath__(volume_, path_, &entry, null, &entryCluster, &entryOffset))) {

          if((entry.attr & FAT_ATTR_DIRECTORY) == 0x0u) {

            firstCluster = ((Word_t) __ReadLE16__(entry.firstClusterHigh) << 0x10) | __ReadLE16__(entry.firstClusterLow);

            if(OK(__ReadCluster__(volume_, entryCluster, &clusterData))) {

              clusterData[entryOffset] = 0xE5u;

              firstSector = __ClusterToSector__(volume_, entryCluster);

              for(i = 0x0u; i < volume_->sectorsPerCluster && writeSuccess; i++) {

                if(ERROR(__WriteSector__(volume_, firstSector + i, clusterData + (i * volume_->bytesPerSector)))) {

                  writeSuccess = false;

                }

              }

              __KernelFreeMemory__(clusterData);

              if(writeSuccess) {

                if((firstCluster >= 0x2u) && (firstCluster < FAT32_EOC_MIN)) {

                  if(OK(__FreeClusters__(volume_, firstCluster))) {

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

              __AssertOnElse__();

            }

          } else {

            __AssertOnElse__();

          }

        } else {

        }

      } else {

        __AssertOnElse__();

      }

    } else {

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Renames or moves a file
   * @details Changes the path of a file or directory.
   *
   * @param[in] volume_  Pointer to the mounted volume
   * @param[in] oldPath_ Current file path (null-terminated string)
   * @param[in] newPath_ New file path (null-terminated string)
   *
   * @return             ReturnOK if rename was successful
   * @return             ReturnError if rename failed, file not found, or
   *                     invalid parameters
   */
  Return_t xFileRename(Volume_t *volume_, const Byte_t *oldPath_, const Byte_t *newPath_) {

    FUNCTION_ENTER;

    FAT32DirEntry_t oldEntry;

    Word_t oldEntryCluster = 0x0u;

    Word_t oldEntryOffset = 0x0u;

    Word_t newParentCluster = 0x0u;

    Byte_t newName83[11];

    Byte_t *clusterData = null;

    Word_t firstSector = 0x0u;

    Word_t i = 0x0u;

    Word_t firstCluster = 0x0u;

    Word_t fileSize = 0x0u;

    Byte_t attr = 0x0u;

    const Byte_t *lastSlash = null;

    const Byte_t *fileName = null;

    Base_t writeSuccess = true;

    if(__ObjectIsValid__(volume_) && __PointerIsNotNull__(oldPath_) && __PointerIsNotNull__(newPath_)) {

      if(volume_->mounted) {

        if(OK(__FindFileByPath__(volume_, oldPath_, &oldEntry, null, &oldEntryCluster, &oldEntryOffset))) {

          fileName = newPath_;

          lastSlash = null;

          for(i = 0x0u; newPath_[i] != '\0'; i++) {

            if(newPath_[i] == '/') {

              lastSlash = &newPath_[i];

            }

          }

          if(__PointerIsNotNull__(lastSlash)) {

            fileName = lastSlash + 1;

          }

          if(OK(__ConvertToFAT83__(fileName, newName83))) {

            if(OK(__FindFileByPath__(volume_, oldPath_, null, &newParentCluster, null, null))) {

              firstCluster = ((Word_t) __ReadLE16__(oldEntry.firstClusterHigh) << 0x10) | __ReadLE16__(oldEntry.firstClusterLow);

              fileSize = __ReadLE32__(oldEntry.fileSize);

              attr = oldEntry.attr;

              if(OK(__CreateDirEntry__(volume_, newParentCluster, newName83, attr, firstCluster, fileSize))) {

                if(OK(__ReadCluster__(volume_, oldEntryCluster, &clusterData))) {

                  clusterData[oldEntryOffset] = 0xE5u;

                  firstSector = __ClusterToSector__(volume_, oldEntryCluster);

                  for(i = 0x0u; i < volume_->sectorsPerCluster && writeSuccess; i++) {

                    if(ERROR(__WriteSector__(volume_, firstSector + i, clusterData + (i * volume_->bytesPerSector)))) {

                      writeSuccess = false;

                    }

                  }

                  __KernelFreeMemory__(clusterData);

                  if(writeSuccess) {

                    __ReturnOk__();

                  } else {

                    __AssertOnElse__();

                  }

                } else {

                  __AssertOnElse__();

                }

              } else {

                __AssertOnElse__();

              }

            } else {

              __AssertOnElse__();

            }

          } else {

            __AssertOnElse__();

          }

        } else {

        }

      } else {

        __AssertOnElse__();

      }

    } else {

    }

    FUNCTION_EXIT;

  }


  /**
   * @brief Gets file or directory information
   * @details Allocates and returns metadata about a file or directory.
   *
   * @param[in]  volume_ Pointer to the mounted volume
   * @param[in]  path_   File or directory path (null-terminated string)
   * @param[out] entry_  Pointer to store allocated directory entry
   *
   * @return             ReturnOK if information was retrieved successfully
   * @return             ReturnError if file not found, allocation failed, or
   *                     invalid parameters
   *
   * @warning Caller is responsible for freeing the allocated entry
   */
  Return_t xFileGetInfo(Volume_t *volume_, const Byte_t *path_, DirEntry_t **entry_) {

    FUNCTION_ENTER;

    FAT32DirEntry_t fatEntry;

    DirEntry_t *dirEntry = null;

    if(__ObjectIsValid__(volume_) && __PointerIsNotNull__(path_) && __PointerIsNotNull__(entry_)) {

      if(volume_->mounted) {

        if(OK(__FindFileByPath__(volume_, path_, &fatEntry, null, null, null))) {

          if(OK(xMemAlloc((volatile Addr_t **) &dirEntry, sizeof(DirEntry_t)))) {

            Word_t i = 0x0u;

            Word_t j = 0x0u;

            dirEntry->valid = VALID;

            for(i = 0x0u; i < 8 && fatEntry.name[i] != ' '; i++) {

              dirEntry->name[j++] = fatEntry.name[i];

            }

            if(fatEntry.name[8] != ' ') {

              dirEntry->name[j++] = '.';

              for(i = 8; i < 11 && fatEntry.name[i] != ' '; i++) {

                dirEntry->name[j++] = fatEntry.name[i];

              }

            }

            dirEntry->name[j] = '\0';

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

          __AssertOnElse__();

        }

      } else {

        __AssertOnElse__();

      }

    } else {

    }

    FUNCTION_EXIT;

  }


#endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */