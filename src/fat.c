/*UNCRUSTIFY-OFF*/
/**
 * @file fat.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief FAT32 filesystem utilities implementation
 * @details
 * Implements low-level FAT32 filesystem helper functions for cluster chain traversal, directory entry parsing, and filename handling.
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

  #include "fat.h"

  #define MAX_MOUNTED_VOLUMES 0x8u

  static HalfWord_t mountedDevices[MAX_MOUNTED_VOLUMES];

  static Byte_t mountedDeviceCount = 0x0u;
  HalfWord_t __ReadLE16__(const Byte_t *data_) {

    return((HalfWord_t) data_[0x0u] | ((HalfWord_t) data_[0x1] << 0x8));

  }


  Word_t __ReadLE32__(const Byte_t *data_) {

    return((Word_t) data_[0x0u] | ((Word_t) data_[0x1] << 0x8) | ((Word_t) data_[0x2] << 0x10) | ((Word_t) data_[0x3] << 0x18));

  }


  void __WriteLE16__(Byte_t *data_, HalfWord_t value_) {

    data_[0x0u] = (Byte_t) (value_ & 0xFFu);

    data_[0x1] = (Byte_t) ((value_ >> 0x8) & 0xFFu);

  }


  void __WriteLE32__(Byte_t *data_, Word_t value_) {

    data_[0x0u] = (Byte_t) (value_ & 0xFFu);

    data_[0x1] = (Byte_t) ((value_ >> 0x8) & 0xFFu);

    data_[0x2] = (Byte_t) ((value_ >> 0x10) & 0xFFu);

    data_[0x3] = (Byte_t) ((value_ >> 0x18) & 0xFFu);

  }


  Base_t __ByteCompare__(const Byte_t *s1_, const Byte_t *s2_, Word_t len_) {

    Word_t i = 0x0u;

    for(i = 0x0u; i < len_; i++) {

      if(s1_[i] != s2_[i]) {

        return(false);

      }

    }

    return(true);

  }


  /**
  * @brief Converts filename to FAT 8.3 format
  * @details Internal function to convert a filename to FAT 8.3 format
  * (space-padded).
  *
  * @param[in]  path_  Input filename
  * @param[out] fat83_ Output buffer for 11-byte FAT 8.3 name
  *
  * @return            ReturnOK if conversion was successful
  * @return            ReturnError if filename is invalid
  */
  Return_t __ConvertToFAT83__(const Byte_t *path_, Byte_t *fat83_) {

    FUNCTION_ENTER;

    Word_t i = 0x0u;

    Word_t j = 0x0u;

    Word_t nameLen = 0x0u;

    Word_t extLen = 0x0u;

    const Byte_t *dotPos = null;

    if(__PointerIsNotNull__(path_) && __PointerIsNotNull__(fat83_)) {

      for(i = 0x0u; i < 11; i++) {

        fat83_[i] = ' ';

      }

      for(i = 0x0u; path_[i] != '\0'; i++) {

        if(path_[i] == '.') {

          dotPos = &path_[i];

        }

      }

      nameLen = 0x0u;

      for(i = 0x0u; path_[i] != '\0' && path_[i] != '.' && nameLen < 8; i++) {

        Byte_t c = path_[i];

        if((c >= 'a') && (c <= 'z')) {

          c = c - 'a' + 'A';

        }

        fat83_[nameLen++] = c;

      }

      if(__PointerIsNotNull__(dotPos)) {

        extLen = 0x0u;

        for(j = 1; dotPos[j] != '\0' && extLen < 3; j++) {

          Byte_t c = dotPos[j];

          if((c >= 'a') && (c <= 'z')) {

            c = c - 'a' + 'A';

          }

          fat83_[8 + extLen++] = c;

        }

      }

      __ReturnOk__();

    } else {

      __AssertOnElse__();

    }

    FUNCTION_EXIT;

  }


  Return_t __FindDirEntry__(const Volume_t *vol_, Word_t dirCluster_, const Byte_t *name83_, FAT32DirEntry_t *entry_, Word_t *entryCluster_, Word_t *

    entryOffset_) {

    FUNCTION_ENTER;

    Byte_t *clusterData = null;

    FAT32DirEntry_t *fatEntry = null;

    Word_t entriesPerCluster = 0x0u;

    Word_t entryIdx = 0x0u;

    Word_t currentCluster = dirCluster_;

    Word_t nextCluster = 0x0u;

    Base_t found = false;

    if(__PointerIsNotNull__(vol_) && __PointerIsNotNull__(name83_)) {

      entriesPerCluster = ((Word_t) vol_->bytesPerSector * vol_->sectorsPerCluster) / sizeof(FAT32DirEntry_t);

      while(!found && currentCluster < FAT32_EOC_MIN) {

        if(OK(__ReadCluster__(vol_, currentCluster, &clusterData))) {

          for(entryIdx = 0x0u; entryIdx < entriesPerCluster; entryIdx++) {

            fatEntry = (FAT32DirEntry_t *) (clusterData + (entryIdx * sizeof(FAT32DirEntry_t)));

            if(fatEntry->name[0x0u] == 0x00u) {

              __KernelFreeMemory__(clusterData);

              found = true;

              break;

            } else {

              if((fatEntry->name[0x0u] != 0xE5u) && ((fatEntry->attr & FAT_ATTR_LONG_NAME) != FAT_ATTR_LONG_NAME)) {

                if(__ByteCompare__(fatEntry->name, name83_, 11)) {

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

                  __ReturnOk__();

                  found = true;

                  break;

                }

              }

            }

          }

          if(!found) {

            __KernelFreeMemory__(clusterData);

            if(OK(__GetFATEntry__(vol_, currentCluster, &nextCluster))) {

              currentCluster = nextCluster;

            } else {

              found = true;

            }

          }

        } else {

          found = true;

        }

      }

    } else {

      __AssertOnElse__();

    }

    FUNCTION_EXIT;

  }


  Return_t __FindFileByPath__(const Volume_t *vol_, const Byte_t *path_, FAT32DirEntry_t *entry_, Word_t *parentCluster_, Word_t *entryCluster_, Word_t *

    entryOffset_) {

    FUNCTION_ENTER;

    Byte_t name83[11];

    Byte_t component[256];

    Word_t pathIdx = 0x0u;

    Word_t componentIdx = 0x0u;

    Word_t currentCluster = 0x0u;

    FAT32DirEntry_t dirEntry;

    Base_t continueProcessing = true;

    if(__PointerIsNotNull__(vol_) && __PointerIsNotNull__(path_)) {

      currentCluster = vol_->rootDirCluster;

      if(path_[0] == '/') {

        pathIdx = 1;

      }

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

          *entryOffset_ = 0x0u;

        }

        __ReturnOk__();

      } else {

        while(continueProcessing && path_[pathIdx] != '\0') {

          componentIdx = 0x0u;

          while(path_[pathIdx] != '\0' && path_[pathIdx] != '/' && componentIdx < 255) {

            component[componentIdx++] = path_[pathIdx++];

          }

          component[componentIdx] = '\0';

          if(path_[pathIdx] == '/') {

            pathIdx++;

          }

          if(OK(__ConvertToFAT83__(component, name83))) {

            if(OK(__FindDirEntry__(vol_, currentCluster, name83, &dirEntry, entryCluster_, entryOffset_))) {

              if(path_[pathIdx] == '\0') {

                if(__PointerIsNotNull__(entry_)) {

                  __memcpy__(entry_, &dirEntry, sizeof(FAT32DirEntry_t));

                }

                if(__PointerIsNotNull__(parentCluster_)) {

                  *parentCluster_ = currentCluster;

                }

                __ReturnOk__();

                continueProcessing = false;

              } else {

                if((dirEntry.attr & FAT_ATTR_DIRECTORY) != 0x0u) {

                  currentCluster = ((Word_t) __ReadLE16__(dirEntry.firstClusterHigh) << 0x10) | __ReadLE16__(dirEntry.firstClusterLow);

                } else {

                  continueProcessing = false;

                  __AssertOnElse__();

                }

              }

            } else {

              continueProcessing = false;

              __AssertOnElse__();

            }

          } else {

            continueProcessing = false;

            __AssertOnElse__();

          }

        }

      }

    } else {

      __AssertOnElse__();

    }

    FUNCTION_EXIT;

  }


  /**
  * @brief Frees a cluster chain
  * @details Internal function to mark clusters as free in the FAT.
  *
  * @param[in] vol_          Pointer to volume structure
  * @param[in] startCluster_ First cluster in chain to free
  *
  * @return                  ReturnOK if clusters were freed successfully
  * @return                  ReturnError if operation failed
  */
  Return_t __FreeClusters__(const Volume_t *vol_, Word_t startCluster_) {

    Word_t currentCluster = startCluster_;

    Word_t nextCluster = 0x0u;

    if(__PointerIsNull__(vol_)) {

      return(ReturnError);

    }

    while(currentCluster >= 2 && currentCluster < FAT32_EOC_MIN) {

      if(ERROR(__GetFATEntry__(vol_, currentCluster, &nextCluster))) {

        return(ReturnError);

      }

      if(ERROR(__SetFATEntry__(vol_, currentCluster, FAT32_FREE_CLUSTER))) {

        return(ReturnError);

      }

      currentCluster = nextCluster;

    }

    return(ReturnOK);

  }


  /**
  * @brief Creates a directory entry
  * @details Internal function to add a new file/directory entry to a
  * directory.
  *
  * @param[in] vol_           Pointer to volume structure
  * @param[in] parentCluster_ Parent directory cluster
  * @param[in] name83_        FAT 8.3 name for new entry
  * @param[in] attr_          File attributes
  * @param[in] firstCluster_  First cluster of file/directory
  * @param[in] size_          File size in bytes
  *
  * @return                   ReturnOK if entry was created successfully
  * @return                   ReturnError if creation failed
  */
  Return_t __CreateDirEntry__(const Volume_t *vol_, Word_t parentCluster_, const Byte_t *name83_, Byte_t attr_, Word_t firstCluster_, Word_t size_) {

    Byte_t *clusterData = null;

    FAT32DirEntry_t *fatEntry = null;

    Word_t entriesPerCluster = 0x0u;

    Word_t entryIdx = 0x0u;

    Word_t currentCluster = parentCluster_;

    Word_t nextCluster = 0x0u;

    Word_t firstSector = 0x0u;

    Word_t i = 0x0u;

    Base_t found = false;

    if(__PointerIsNull__(vol_) || __PointerIsNull__(name83_)) {

      return(ReturnError);

    }

    entriesPerCluster = ((Word_t) vol_->bytesPerSector * vol_->sectorsPerCluster) / sizeof(FAT32DirEntry_t);

    while(!found && currentCluster < FAT32_EOC_MIN) {

      if(ERROR(__ReadCluster__(vol_, currentCluster, &clusterData))) {

        return(ReturnError);

      }

      for(entryIdx = 0x0u; entryIdx < entriesPerCluster; entryIdx++) {

        fatEntry = (FAT32DirEntry_t *) (clusterData + (entryIdx * sizeof(FAT32DirEntry_t)));

        if((fatEntry->name[0x0u] == 0x00u) || (fatEntry->name[0x0u] == 0xE5u)) {

          __memcpy__(fatEntry->name, name83_, 0xB);

          fatEntry->attr = attr_;

          fatEntry->ntReserved = 0x0u;

          fatEntry->createTimeTenth = 0x0u;

          __WriteLE16__(fatEntry->createTime, 0);

          __WriteLE16__(fatEntry->createDate, 0);

          __WriteLE16__(fatEntry->lastAccessDate, 0);

          __WriteLE16__(fatEntry->firstClusterHigh, (HalfWord_t) (firstCluster_ >> 16));

          __WriteLE16__(fatEntry->writeTime, 0);

          __WriteLE16__(fatEntry->writeDate, 0);

          __WriteLE16__(fatEntry->firstClusterLow, (HalfWord_t) (firstCluster_ & 0xFFFFu));

          __WriteLE32__(fatEntry->fileSize, size_);

          firstSector = __ClusterToSector__(vol_, currentCluster);

          for(i = 0x0u; i < vol_->sectorsPerCluster; i++) {

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

      if(OK(__GetFATEntry__(vol_, currentCluster, &nextCluster))) {

        if(nextCluster >= FAT32_EOC_MIN) {

          Word_t newCluster = 0x0u;

          if(OK(__FindFreeCluster__(vol_, currentCluster + 1u, &newCluster))) {

            __SetFATEntry__(vol_, currentCluster, newCluster);

            __SetFATEntry__(vol_, newCluster, FAT32_EOC_MAX);

            if(OK(__ReadCluster__(vol_, newCluster, &clusterData))) {

              __memset__(clusterData, 0x00u, (Word_t) vol_->bytesPerSector * vol_->sectorsPerCluster);

              firstSector = __ClusterToSector__(vol_, newCluster);

              for(i = 0x0u; i < vol_->sectorsPerCluster; i++) {

                __WriteSector__(vol_, firstSector + i, clusterData + (i * vol_->bytesPerSector));

              }

              __KernelFreeMemory__(clusterData);

            }

            currentCluster = newCluster;

            continue;

          } else {

            return(ReturnError);

          }

        }

        currentCluster = nextCluster;

      } else {

        return(ReturnError);

      }

    }

    return(ReturnError);

  }


  /**
  * @brief Reads a sector from block device
  * @details Internal function to read one sector from the volume's block
  * device.
  *
  * @param[in]  vol_    Pointer to volume structure
  * @param[in]  sector_ Sector number to read
  * @param[out] data_   Pointer to store allocated sector data
  *
  * @return             ReturnOK if read was successful
  * @return             ReturnError if read failed or allocation failed
  */
  Return_t __ReadSector__(const Volume_t *vol_, Word_t sector_, Byte_t **data_) {

    FUNCTION_ENTER;

    Size_t blockSize = sizeof(BlockDeviceCommand_t);

    BlockDeviceCommand_t *cmd = null;

    Size_t readSize = 0x0u;

    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(data_)) {

      if(OK(__KernelAllocateMemory__((volatile Addr_t **) &cmd, blockSize))) {

        cmd->command = BLOCK_CMD_SET_ADDRESS;

        cmd->blockNumber = sector_;

        cmd->blockCount = 1;

        cmd->transferMode = BLOCK_IO_MODE_BLOCKING;

        if(OK(__DeviceConfigDevice__(vol_->blockDeviceUID, &blockSize, (Addr_t *) cmd))) {

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
  * @brief Writes a sector to block device
  * @details Internal function to write one sector to the volume's block
  * device.
  *
  * @param[in] vol_    Pointer to volume structure
  * @param[in] sector_ Sector number to write
  * @param[in] data_   Pointer to sector data
  *
  * @return            ReturnOK if write was successful
  * @return            ReturnError if write failed
  */
  Return_t __WriteSector__(const Volume_t *vol_, Word_t sector_, const Byte_t *data_) {

    FUNCTION_ENTER;

    Size_t blockSize = sizeof(BlockDeviceCommand_t);

    BlockDeviceCommand_t *cmd = null;

    Size_t writeSize = 0x0u;

    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(data_)) {

      if(OK(__KernelAllocateMemory__((volatile Addr_t **) &cmd, blockSize))) {

        cmd->command = BLOCK_CMD_SET_ADDRESS;

        cmd->blockNumber = sector_;

        cmd->blockCount = 1;

        cmd->transferMode = BLOCK_IO_MODE_BLOCKING;

        if(OK(__DeviceConfigDevice__(vol_->blockDeviceUID, &blockSize, (Addr_t *) cmd))) {

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


  Word_t __ClusterToSector__(const Volume_t *vol_, Word_t cluster_) {

    return(vol_->dataStartSector + ((cluster_ - 2u) * vol_->sectorsPerCluster));

  }


  /**
  * @brief Reads a cluster from filesystem
  * @details Internal function to read all sectors of a cluster.
  *
  * @param[in]  vol_     Pointer to volume structure
  * @param[in]  cluster_ Cluster number to read
  * @param[out] data_    Pointer to store allocated cluster data
  *
  * @return              ReturnOK if read was successful
  * @return              ReturnError if read failed or allocation failed
  */
  Return_t __ReadCluster__(const Volume_t *vol_, Word_t cluster_, Byte_t **data_) {

    FUNCTION_ENTER;

    Word_t firstSector = 0x0u;

    Word_t clusterSize = 0x0u;

    Byte_t *buffer = null;

    Byte_t *sectorData = null;

    Word_t i = 0x0u;

    Base_t allSectorsRead = true;

    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(data_)) {

      clusterSize = (Word_t) vol_->bytesPerSector * vol_->sectorsPerCluster;

      firstSector = __ClusterToSector__(vol_, cluster_);

      if(OK(__KernelAllocateMemory__((volatile Addr_t **) &buffer, clusterSize))) {

        for(i = 0x0u; i < vol_->sectorsPerCluster && allSectorsRead; i++) {

          if(OK(__ReadSector__(vol_, firstSector + i, &sectorData))) {

            __memcpy__(buffer + (i * vol_->bytesPerSector), sectorData, vol_->bytesPerSector);

            __KernelFreeMemory__(sectorData);

          } else {

            __KernelFreeMemory__(buffer);

            __AssertOnElse__();

            allSectorsRead = false;

          }

        }

        if(allSectorsRead) {

          *data_ = buffer;

          __ReturnOk__();

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
  * @brief Gets FAT entry for a cluster
  * @details Internal function to read the FAT table entry for a given
  * cluster.
  *
  * @param[in]  vol_         Pointer to volume structure
  * @param[in]  cluster_     Cluster number to look up
  * @param[out] nextCluster_ Pointer to store next cluster number
  *
  * @return                  ReturnOK if entry was read successfully
  * @return                  ReturnError if read failed
  */
  Return_t __GetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t *nextCluster_) {

    FUNCTION_ENTER;

    Word_t fatOffset = 0x0u;

    Word_t fatSector = 0x0u;

    Word_t entryOffset = 0x0u;

    Byte_t *sectorData = null;

    Word_t fatEntry = 0x0u;

    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(nextCluster_)) {

      fatOffset = cluster_ * 4u;

      fatSector = vol_->fatStartSector + (fatOffset / vol_->bytesPerSector);

      entryOffset = fatOffset % vol_->bytesPerSector;

      if(OK(__ReadSector__(vol_, fatSector, &sectorData))) {

        fatEntry = __ReadLE32__(sectorData + entryOffset) & 0x0FFFFFFFu;

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
  * @brief Sets FAT entry for a cluster
  * @details Internal function to write a FAT table entry for a given
  * cluster.
  *
  * @param[in] vol_     Pointer to volume structure
  * @param[in] cluster_ Cluster number to update
  * @param[in] value_   Value to write (next cluster or special marker)
  *
  * @return             ReturnOK if entry was written successfully
  * @return             ReturnError if write failed
  */
  Return_t __SetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t value_) {

    FUNCTION_ENTER;

    Word_t fatOffset = 0x0u;

    Word_t fatSector = 0x0u;

    Word_t entryOffset = 0x0u;

    Byte_t *sectorData = null;

    Byte_t i = 0x0u;

    if(__ObjectIsValid__(vol_)) {

      fatOffset = cluster_ * 4u;

      fatSector = vol_->fatStartSector + (fatOffset / vol_->bytesPerSector);

      entryOffset = fatOffset % vol_->bytesPerSector;

      if(OK(__ReadSector__(vol_, fatSector, &sectorData))) {

        __WriteLE32__(sectorData + entryOffset, (value_ & 0x0FFFFFFFu) | (__ReadLE32__(sectorData + entryOffset) & 0xF0000000u));

        if(OK(__WriteSector__(vol_, fatSector, sectorData))) {

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
  * @brief Finds a free cluster in FAT
  * @details Internal function to search FAT for an available cluster.
  *
  * @param[in]  vol_         Pointer to volume structure
  * @param[in]  startHint_   Cluster number to start searching from
  * @param[out] freeCluster_ Pointer to store found free cluster number
  *
  * @return                  ReturnOK if free cluster was found
  * @return                  ReturnError if no free clusters available
  */
  Return_t __FindFreeCluster__(const Volume_t *vol_, Word_t startHint_, Word_t *freeCluster_) {

    FUNCTION_ENTER;

    Word_t cluster = 0x0u;

    Word_t maxCluster = 0x0u;

    Word_t fatEntry = 0x0u;

    Word_t searchStart = 0x0u;

    Base_t found = false;

    Base_t continueSearch = true;

    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(freeCluster_)) {

      maxCluster = (vol_->sectorsPerFAT * vol_->bytesPerSector) / 4u;

      if(maxCluster > 0x10000u) {

        maxCluster = 0x10000u;

      }

      searchStart = (startHint_ >= 3u) ? startHint_ : 3u;

      for(cluster = searchStart; cluster < maxCluster && continueSearch && !found; cluster++) {

        if(OK(__GetFATEntry__(vol_, cluster, &fatEntry))) {

          if(fatEntry == FAT32_FREE_CLUSTER) {

            *freeCluster_ = cluster;

            __ReturnOk__();

            found = true;

          }

        } else {

          __AssertOnElse__();

          continueSearch = false;

        }

      }

      if(!found && continueSearch && (searchStart > 3u)) {

        for(cluster = 3u; cluster < searchStart && continueSearch && !found; cluster++) {

          if(OK(__GetFATEntry__(vol_, cluster, &fatEntry))) {

            if(fatEntry == FAT32_FREE_CLUSTER) {

              *freeCluster_ = cluster;

              __ReturnOk__();

              found = true;

            }

          } else {

            __AssertOnElse__();

            continueSearch = false;

          }

        }

      }

      if(!found && continueSearch) {

        __AssertOnElse__();

      }

    } else {

      __AssertOnElse__();

    }

    FUNCTION_EXIT;

  }


  Base_t __IsDeviceMounted__(const HalfWord_t blockDeviceUID_) {

    Byte_t i = 0x0u;

    for(i = 0x0u; i < mountedDeviceCount; i++) {

      if(mountedDevices[i] == blockDeviceUID_) {

        return(true);

      }

    }

    return(false);

  }


  /**
  * @brief Adds device to mounted list
  * @details Internal function to track mounted devices.
  *
  * @param[in] blockDeviceUID_ Block device UID to add
  *
  * @return                    ReturnOK if device was added successfully
  * @return                    ReturnError if operation failed
  */
  Return_t __AddMountedDevice__(const HalfWord_t blockDeviceUID_) {

    if(mountedDeviceCount < MAX_MOUNTED_VOLUMES) {

      mountedDevices[mountedDeviceCount] = blockDeviceUID_;

      mountedDeviceCount++;

      return(ReturnOK);

    }

    return(ReturnError);

  }


  /**
  * @brief Removes device from mounted list
  * @details Internal function to untrack mounted devices.
  *
  * @param[in] blockDeviceUID_ Block device UID to remove
  *
  * @return                    ReturnOK if device was removed successfully
  * @return                    ReturnError if operation failed
  */
  Return_t __RemoveMountedDevice__(const HalfWord_t blockDeviceUID_) {

    Byte_t i = 0x0u;

    Byte_t j = 0x0u;

    for(i = 0x0u; i < mountedDeviceCount; i++) {

      if(mountedDevices[i] == blockDeviceUID_) {

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
    void __FSStateClear__(void) {

      mountedDeviceCount = 0x0u;

      return;

    }


  #endif /* if defined(POSIX_ARCH_OTHER) */

#endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */