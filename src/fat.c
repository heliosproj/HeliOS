#include "config.h"
#if defined(CONFIG_ENABLE_IO_SUBSYSTEM)
  #include "fat.h"
  static HalfWord_t mountedDevices[CONFIG_FAT_MAX_MOUNTED_VOLUMES];
  static Byte_t mountedDeviceCount = 0x0u;
  HalfWord_t __ReadLE16__(const Byte_t *data_) {
    return ((HalfWord_t) data_[0x0u] | ((HalfWord_t) data_[FAT_LE16_HIGH_BYTE_OFFSET] << FAT_BYTE_SHIFT_8));
  }
  Word_t __ReadLE32__(const Byte_t *data_) {
    return ((Word_t) data_[0x0u] | ((Word_t) data_[FAT_LE16_HIGH_BYTE_OFFSET] << FAT_BYTE_SHIFT_8) | ((Word_t) data_[0x2] << FAT_BYTE_SHIFT_16) | ((Word_t) data_[0x3] << FAT_BYTE_SHIFT_24));
  }
  void __WriteLE16__(Byte_t *data_, HalfWord_t value_) {
    data_[0x0u] = (Byte_t) (value_ & FAT_BYTE_MASK);
    data_[FAT_LE16_HIGH_BYTE_OFFSET] = (Byte_t) ((value_ >> FAT_BYTE_SHIFT_8) & FAT_BYTE_MASK);
  }
  void __WriteLE32__(Byte_t *data_, Word_t value_) {
    data_[0x0u] = (Byte_t) (value_ & FAT_BYTE_MASK);
    data_[FAT_LE16_HIGH_BYTE_OFFSET] = (Byte_t) ((value_ >> FAT_BYTE_SHIFT_8) & FAT_BYTE_MASK);
    data_[0x2] = (Byte_t) ((value_ >> FAT_BYTE_SHIFT_16) & FAT_BYTE_MASK);
    data_[0x3] = (Byte_t) ((value_ >> FAT_BYTE_SHIFT_24) & FAT_BYTE_MASK);
  }
  Base_t __ByteCompare__(const Byte_t *s1_, const Byte_t *s2_, Word_t len_) {
    Word_t i = 0x0u;
    for(i = 0x0u; i < len_; i++) {
      if(s1_[i] != s2_[i]) {
        return (false);
      }
    }
    return (true);
  }
  Return_t __ConvertToFAT83__(const Byte_t *path_, Byte_t *fat83_) {
    FUNCTION_ENTER;
    Word_t i = 0x0u;
    Word_t j = 0x0u;
    Word_t nameLen = 0x0u;
    Word_t extLen = 0x0u;
    const Byte_t *dotPos = null;
    if(__PointerIsNotNull__(path_) && __PointerIsNotNull__(fat83_)) {
      for(i = 0x0u; i < FAT_83_NAME_LENGTH; i++) {
        fat83_[i] = ' ';
      }
      for(i = 0x0u; path_[i] != '\0'; i++) {
        if(path_[i] == '.') {
          dotPos = &path_[i];
        }
      }
      nameLen = 0x0u;
      for(i = 0x0u; path_[i] != '\0' && path_[i] != '.' && nameLen < FAT_83_BASENAME_LENGTH; i++) {
        Byte_t c = path_[i];
        if((c >= 'a') && (c <= 'z')) {
          c = c - 'a' + 'A';
        }
        fat83_[nameLen++] = c;
      }
      if(__PointerIsNotNull__(dotPos)) {
        extLen = 0x0u;
        for(j = 0x1u; dotPos[j] != '\0' && extLen < FAT_83_EXTENSION_LENGTH; j++) {
          Byte_t c = dotPos[j];
          if((c >= 'a') && (c <= 'z')) {
            c = c - 'a' + 'A';
          }
          fat83_[FAT_83_BASENAME_LENGTH + extLen++] = c;
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
              if((fatEntry->name[0x0u] != FAT_ENTRY_DELETED) && ((fatEntry->attr & FAT_ATTR_LONG_NAME) != FAT_ATTR_LONG_NAME)) {
                if(__ByteCompare__(fatEntry->name, name83_, FAT_83_NAME_LENGTH)) {
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
    Byte_t name83[FAT_83_NAME_LENGTH] = {
      0x0u
    };
    Byte_t component[CONFIG_FAT_MAX_PATH_COMPONENT] = {
      0x0u
    };
    Word_t pathIdx = 0x0u;
    Word_t componentIdx = 0x0u;
    Word_t currentCluster = 0x0u;
    FAT32DirEntry_t dirEntry = {
      0x0u
    };
    Base_t continueProcessing = true;
    if(__PointerIsNotNull__(vol_) && __PointerIsNotNull__(path_)) {
      currentCluster = vol_->rootDirCluster;
      if(path_[0x0u] == '/') {
        pathIdx = 0x1u;
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
          while(path_[pathIdx] != '\0' && path_[pathIdx] != '/' && componentIdx < FAT_BYTE_MASK) {
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
                  currentCluster = ((Word_t) __ReadLE16__(dirEntry.firstClusterHigh) << FAT_BYTE_SHIFT_16) | __ReadLE16__(dirEntry.firstClusterLow);
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
  Return_t __FreeClusters__(const Volume_t *vol_, Word_t startCluster_) {
    Word_t currentCluster = startCluster_;
    Word_t nextCluster = 0x0u;
    if(__PointerIsNull__(vol_)) {
      return (ReturnError);
    }
    while(currentCluster >= FAT_MIN_VALID_CLUSTER && currentCluster < FAT32_EOC_MIN) {
      if(ERROR(__GetFATEntry__(vol_, currentCluster, &nextCluster))) {
        return (ReturnError);
      }
      if(ERROR(__SetFATEntry__(vol_, currentCluster, FAT32_FREE_CLUSTER))) {
        return (ReturnError);
      }
      currentCluster = nextCluster;
    }
    return (ReturnOK);
  }
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
      return (ReturnError);
    }
    entriesPerCluster = ((Word_t) vol_->bytesPerSector * vol_->sectorsPerCluster) / sizeof(FAT32DirEntry_t);
    while(!found && currentCluster < FAT32_EOC_MIN) {
      if(ERROR(__ReadCluster__(vol_, currentCluster, &clusterData))) {
        return (ReturnError);
      }
      for(entryIdx = 0x0u; entryIdx < entriesPerCluster; entryIdx++) {
        fatEntry = (FAT32DirEntry_t *) (clusterData + (entryIdx * sizeof(FAT32DirEntry_t)));
        if((fatEntry->name[0x0u] == 0x00u) || (fatEntry->name[0x0u] == FAT_ENTRY_DELETED)) {
          __memcpy__(fatEntry->name, name83_, 0xB);
          fatEntry->attr = attr_;
          fatEntry->ntReserved = 0x0u;
          fatEntry->createTimeTenth = 0x0u;
          __WriteLE16__(fatEntry->createTime, 0x0u);
          __WriteLE16__(fatEntry->createDate, 0x0u);
          __WriteLE16__(fatEntry->lastAccessDate, 0x0u);
          __WriteLE16__(fatEntry->firstClusterHigh, (HalfWord_t) (firstCluster_ >> 0x10u));
          __WriteLE16__(fatEntry->writeTime, 0x0u);
          __WriteLE16__(fatEntry->writeDate, 0x0u);
          __WriteLE16__(fatEntry->firstClusterLow, (HalfWord_t) (firstCluster_ & 0xFFFFu));
          __WriteLE32__(fatEntry->fileSize, size_);
          firstSector = __ClusterToSector__(vol_, currentCluster);
          for(i = 0x0u; i < vol_->sectorsPerCluster; i++) {
            if(ERROR(__WriteSector__(vol_, firstSector + i, clusterData + (i * vol_->bytesPerSector)))) {
              __KernelFreeMemory__(clusterData);
              return (ReturnError);
            }
          }
          __KernelFreeMemory__(clusterData);
          return (ReturnOK);
        }
      }
      __KernelFreeMemory__(clusterData);
      if(OK(__GetFATEntry__(vol_, currentCluster, &nextCluster))) {
        if(nextCluster >= FAT32_EOC_MIN) {
          Word_t newCluster = 0x0u;
          if(OK(__FindFreeCluster__(vol_, currentCluster + 0x1u, &newCluster))) {
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
            return (ReturnError);
          }
        }
        currentCluster = nextCluster;
      } else {
        return (ReturnError);
      }
    }
    return (ReturnError);
  }
  Return_t __ReadSector__(const Volume_t *vol_, Word_t sector_, Byte_t **data_) {
    FUNCTION_ENTER;
    Size_t blockSize = sizeof(BlockDeviceCommand_t);
    BlockDeviceCommand_t *cmd = null;
    Size_t readSize = 0x0u;
    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(data_)) {
      if(OK(__KernelAllocateMemory__((volatile Addr_t **) &cmd, blockSize))) {
        cmd->command = BLOCK_CMD_SET_ADDRESS;
        cmd->blockNumber = sector_;
        cmd->blockCount = 0x1u;
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
  Return_t __WriteSector__(const Volume_t *vol_, Word_t sector_, const Byte_t *data_) {
    FUNCTION_ENTER;
    Size_t blockSize = sizeof(BlockDeviceCommand_t);
    BlockDeviceCommand_t *cmd = null;
    Size_t writeSize = 0x0u;
    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(data_)) {
      if(OK(__KernelAllocateMemory__((volatile Addr_t **) &cmd, blockSize))) {
        cmd->command = BLOCK_CMD_SET_ADDRESS;
        cmd->blockNumber = sector_;
        cmd->blockCount = 0x1u;
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
    return (vol_->dataStartSector + ((cluster_ - FAT_MIN_VALID_CLUSTER) * vol_->sectorsPerCluster));
  }
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
  Return_t __GetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t *nextCluster_) {
    FUNCTION_ENTER;
    Word_t fatOffset = 0x0u;
    Word_t fatSector = 0x0u;
    Word_t entryOffset = 0x0u;
    Byte_t *sectorData = null;
    Word_t fatEntry = 0x0u;
    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(nextCluster_)) {
      fatOffset = cluster_ * FAT32_ENTRY_SIZE_BYTES;
      fatSector = vol_->fatStartSector + (fatOffset / vol_->bytesPerSector);
      entryOffset = fatOffset % vol_->bytesPerSector;
      if(OK(__ReadSector__(vol_, fatSector, &sectorData))) {
        fatEntry = __ReadLE32__(sectorData + entryOffset) & FAT32_ENTRY_MASK;
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
  Return_t __SetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t value_) {
    FUNCTION_ENTER;
    Word_t fatOffset = 0x0u;
    Word_t fatSector = 0x0u;
    Word_t entryOffset = 0x0u;
    Byte_t *sectorData = null;
    Byte_t i = 0x0u;
    if(__ObjectIsValid__(vol_)) {
      fatOffset = cluster_ * FAT32_ENTRY_SIZE_BYTES;
      fatSector = vol_->fatStartSector + (fatOffset / vol_->bytesPerSector);
      entryOffset = fatOffset % vol_->bytesPerSector;
      if(OK(__ReadSector__(vol_, fatSector, &sectorData))) {
        __WriteLE32__(sectorData + entryOffset, (value_ & FAT32_ENTRY_MASK) | (__ReadLE32__(sectorData + entryOffset) & FAT32_RESERVED_BITS_MASK));
        if(OK(__WriteSector__(vol_, fatSector, sectorData))) {
          for(i = 0x1u; i < vol_->numFATs; i++) {
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
  Return_t __FindFreeCluster__(const Volume_t *vol_, Word_t startHint_, Word_t *freeCluster_) {
    FUNCTION_ENTER;
    Word_t cluster = 0x0u;
    Word_t maxCluster = 0x0u;
    Word_t fatEntry = 0x0u;
    Word_t searchStart = 0x0u;
    Base_t found = false;
    Base_t continueSearch = true;
    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(freeCluster_)) {
      maxCluster = (vol_->sectorsPerFAT * vol_->bytesPerSector) / FAT32_ENTRY_SIZE_BYTES;
      if(maxCluster > CONFIG_FAT_MAX_SEARCHABLE_CLUSTERS) {
        maxCluster = CONFIG_FAT_MAX_SEARCHABLE_CLUSTERS;
      }
      searchStart = (startHint_ >= FAT_83_EXTENSION_LENGTH) ? startHint_ : FAT_83_EXTENSION_LENGTH;
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
      if(!found && continueSearch && (searchStart > FAT_83_EXTENSION_LENGTH)) {
        for(cluster = FAT_83_EXTENSION_LENGTH; cluster < searchStart && continueSearch && !found; cluster++) {
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
        return (true);
      }
    }
    return (false);
  }
  Return_t __AddMountedDevice__(const HalfWord_t blockDeviceUID_) {
    if(mountedDeviceCount < CONFIG_FAT_MAX_MOUNTED_VOLUMES) {
      mountedDevices[mountedDeviceCount] = blockDeviceUID_;
      mountedDeviceCount++;
      return (ReturnOK);
    }
    return (ReturnError);
  }
  Return_t __RemoveMountedDevice__(const HalfWord_t blockDeviceUID_) {
    Byte_t i = 0x0u;
    Byte_t j = 0x0u;
    for(i = 0x0u; i < mountedDeviceCount; i++) {
      if(mountedDevices[i] == blockDeviceUID_) {
        for(j = i; j < mountedDeviceCount - 0x1u; j++) {
          mountedDevices[j] = mountedDevices[j + 0x1u];
        }
        mountedDeviceCount--;
        return (ReturnOK);
      }
    }
    return (ReturnError);
  }
  #if defined(POSIX_ARCH_OTHER)
    void __FSStateClear__(void) {
      mountedDeviceCount = 0x0u;
      return;
    }
  #endif
#endif