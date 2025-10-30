/*UNCRUSTIFY-OFF*/
/**
 * @file fat.c
 * @author Manny Peterson <manny@heliosproject.org>
 * @brief Low-level FAT32 filesystem implementation
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * (C) 2020-2026 Manny Peterson <manny@heliosproject.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "config.h"
#if defined(CONFIG_ENABLE_IO_SUBSYSTEM)
  #include "fat.h"


/* Mount tracking - tracks which block devices are currently mounted */
  #define MAX_MOUNTED_VOLUMES 0x8u
  static HalfWord_t mountedDevices[MAX_MOUNTED_VOLUMES];
  static Byte_t mountedDeviceCount = 0x0u;


/* Helper function to read 16-bit little-endian value */
  HalfWord_t __ReadLE16__(const Byte_t *data_) {
    return((HalfWord_t) data_[0x0u] | ((HalfWord_t) data_[0x1] << 0x8));
  }


/* Helper function to read 32-bit little-endian value */
  Word_t __ReadLE32__(const Byte_t *data_) {
    return((Word_t) data_[0x0u] | ((Word_t) data_[0x1] << 0x8) | ((Word_t) data_[0x2] << 0x10) | ((Word_t) data_[0x3] << 0x18));
  }


/* Helper function to write 16-bit little-endian value */
  void __WriteLE16__(Byte_t *data_, HalfWord_t value_) {
    data_[0x0u] = (Byte_t) (value_ & 0xFFu);
    data_[0x1] = (Byte_t) ((value_ >> 0x8) & 0xFFu);
  }


/* Helper function to write 32-bit little-endian value */
  void __WriteLE32__(Byte_t *data_, Word_t value_) {
    data_[0x0u] = (Byte_t) (value_ & 0xFFu);
    data_[0x1] = (Byte_t) ((value_ >> 0x8) & 0xFFu);
    data_[0x2] = (Byte_t) ((value_ >> 0x10) & 0xFFu);
    data_[0x3] = (Byte_t) ((value_ >> 0x18) & 0xFFu);
  }


/**
 * @brief Simple byte-by-byte comparison for FAT names
 * @param  s1_  First buffer
 * @param  s2_  Second buffer
 * @param  len_ Number of bytes to compare
 * @return      true if equal, false if not equal
 */
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
 * @brief Convert a filename to 8.3 FAT format
 * @param  path_  Input filename (e.g., "FILE.TXT" or "file.txt")
 * @param  fat83_ Output buffer (11 bytes) for FAT 8.3 name
 * @return        ReturnOK on success, ReturnError on invalid name
 */
  Return_t __ConvertToFAT83__(const Byte_t *path_, Byte_t *fat83_) {
    FUNCTION_ENTER;


    Word_t i = 0x0u;
    Word_t j = 0x0u;
    Word_t nameLen = 0x0u;
    Word_t extLen = 0x0u;
    const Byte_t *dotPos = null;


    if(__PointerIsNull__(path_) || __PointerIsNull__(fat83_)) {
      FUNCTION_EXIT;
    }

    /* Initialize output to spaces */
    for(i = 0x0u; i < 11; i++) {
      fat83_[i] = ' ';
    }

    /* Find dot position for extension */
    for(i = 0x0u; path_[i] != '\0'; i++) {
      if(path_[i] == '.') {
        dotPos = &path_[i];
      }
    }

    /* Copy name part (up to 8 chars, before dot or end) */
    nameLen = 0x0u;

    for(i = 0x0u; path_[i] != '\0' && path_[i] != '.' && nameLen < 8; i++) {
      Byte_t c = path_[i];


      /* Convert to uppercase */
      if((c >= 'a') && (c <= 'z')) {
        c = c - 'a' + 'A';
      }

      fat83_[nameLen++] = c;
    }

    /* Copy extension part (up to 3 chars, after dot) */
    if(__PointerIsNotNull__(dotPos)) {
      extLen = 0x0u;

      for(j = 1; dotPos[j] != '\0' && extLen < 3; j++) {
        Byte_t c = dotPos[j];


        /* Convert to uppercase */
        if((c >= 'a') && (c <= 'z')) {
          c = c - 'a' + 'A';
        }

        fat83_[8 + extLen++] = c;
      }
    }

    ret = ReturnOK;
    FUNCTION_EXIT;
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


    if(__PointerIsNull__(vol_) || __PointerIsNull__(name83_)) {
      FUNCTION_EXIT;
    }

    entriesPerCluster = ((Word_t) vol_->bytesPerSector * vol_->sectorsPerCluster) / sizeof(FAT32DirEntry_t);

    /* Search through directory cluster chain */
    while(!found && currentCluster < FAT32_EOC_MIN) {
      /* Read directory cluster */
      if(ERROR(__ReadCluster__(vol_, currentCluster, &clusterData))) {
        FUNCTION_EXIT;
      }

      /* Search entries in this cluster */
      for(entryIdx = 0x0u; entryIdx < entriesPerCluster; entryIdx++) {
        fatEntry = (FAT32DirEntry_t *) (clusterData + (entryIdx * sizeof(FAT32DirEntry_t)));

        /* Check for end of directory */
        if(fatEntry->name[0x0u] == 0x00u) {
          __KernelFreeMemory__(clusterData);
          FUNCTION_EXIT; /* Not found */
        }

        /* Skip deleted entries and long filename entries */
        if((fatEntry->name[0x0u] == 0xE5u) || ((fatEntry->attr & FAT_ATTR_LONG_NAME) == FAT_ATTR_LONG_NAME)) {
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
          ret = ReturnOK;
          FUNCTION_EXIT;
        }
      }

      __KernelFreeMemory__(clusterData);

      /* Move to next cluster in chain */
      if(OK(__GetFATEntry__(vol_, currentCluster, &nextCluster))) {
        currentCluster = nextCluster;
      } else {
        FUNCTION_EXIT;
      }
    }

    FUNCTION_EXIT; /* Not found */
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
  Return_t __FindFileByPath__(const Volume_t *vol_, const Byte_t *path_, FAT32DirEntry_t *entry_, Word_t *parentCluster_, Word_t *entryCluster_, Word_t *
    entryOffset_) {
    FUNCTION_ENTER;


    Byte_t name83[11];
    Byte_t component[256];
    Word_t pathIdx = 0x0u;
    Word_t componentIdx = 0x0u;
    Word_t currentCluster = 0x0u;
    FAT32DirEntry_t dirEntry;
    Return_t result = ReturnError;


    if(__PointerIsNull__(vol_) || __PointerIsNull__(path_)) {
      FUNCTION_EXIT;
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
        *entryOffset_ = 0x0u;
      }

      ret = ReturnOK;
      FUNCTION_EXIT;
    }

    /* Parse path components and traverse directories */
    while(path_[pathIdx] != '\0') {
      /* Extract next component */
      componentIdx = 0x0u;

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
        FUNCTION_EXIT;
      }

      /* Search for this component in current directory */
      result = __FindDirEntry__(vol_, currentCluster, name83, &dirEntry, entryCluster_, entryOffset_);

      if(ERROR(result)) {
        FUNCTION_EXIT; /* Component not found */
      }

      /* If this is the last component, we're done */
      if(path_[pathIdx] == '\0') {
        if(__PointerIsNotNull__(entry_)) {
          __memcpy__(entry_, &dirEntry, sizeof(FAT32DirEntry_t));
        }

        if(__PointerIsNotNull__(parentCluster_)) {
          *parentCluster_ = currentCluster;
        }

        ret = ReturnOK;
        FUNCTION_EXIT;
      }

      /* Otherwise, move into this directory (must be a directory) */
      if((dirEntry.attr & FAT_ATTR_DIRECTORY) == 0x0u) {
        FUNCTION_EXIT; /* Not a directory, can't traverse further */
      }

      /* Update parent and current cluster for next iteration */
      currentCluster = ((Word_t) __ReadLE16__(dirEntry.firstClusterHigh) << 0x10) | __ReadLE16__(dirEntry.firstClusterLow);
    }

    ret = result;
    FUNCTION_EXIT;
  }


/**
 * @brief Free all clusters in a cluster chain
 * @param  vol_          Pointer to mounted volume
 * @param  startCluster_ First cluster in chain to free
 * @return               ReturnOK on success, ReturnError on failure
 */
  Return_t __FreeClusters__(const Volume_t *vol_, Word_t startCluster_) {
    Word_t currentCluster = startCluster_;
    Word_t nextCluster = 0x0u;


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

    /* Search through directory cluster chain for free entry */
    while(!found && currentCluster < FAT32_EOC_MIN) {
      /* Read directory cluster */
      if(ERROR(__ReadCluster__(vol_, currentCluster, &clusterData))) {
        return(ReturnError);
      }

      /* Search for free entry in this cluster */
      for(entryIdx = 0x0u; entryIdx < entriesPerCluster; entryIdx++) {
        fatEntry = (FAT32DirEntry_t *) (clusterData + (entryIdx * sizeof(FAT32DirEntry_t)));

        /* Check for free entry (deleted or end marker) */
        if((fatEntry->name[0x0u] == 0x00u) || (fatEntry->name[0x0u] == 0xE5u)) {
          /* Found free entry! Fill it in */
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


          /* Write cluster back */
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

      /* Move to next cluster in chain */
      if(OK(__GetFATEntry__(vol_, currentCluster, &nextCluster))) {
        if(nextCluster >= FAT32_EOC_MIN) {
          /* Need to allocate new cluster for directory */
          Word_t newCluster = 0x0u;


          if(OK(__FindFreeCluster__(vol_, currentCluster + 1u, &newCluster))) {
            /* Link current cluster to new cluster */
            __SetFATEntry__(vol_, currentCluster, newCluster);


            /* Mark new cluster as end of chain */
            __SetFATEntry__(vol_, newCluster, FAT32_EOC_MAX);

            /* Zero out new cluster */
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
  Return_t __ReadSector__(const Volume_t *vol_, Word_t sector_, Byte_t **data_) {
    FUNCTION_ENTER;


    Size_t blockSize = sizeof(BlockDeviceCommand_t);
    BlockDeviceCommand_t *cmd = null;
    Size_t readSize = 0x0u;


    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(data_)) {
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
  Return_t __WriteSector__(const Volume_t *vol_, Word_t sector_, const Byte_t *data_) {
    FUNCTION_ENTER;


    Size_t blockSize = sizeof(BlockDeviceCommand_t);
    BlockDeviceCommand_t *cmd = null;
    Size_t writeSize = 0x0u;


    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(data_)) {
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
  Word_t __ClusterToSector__(const Volume_t *vol_, Word_t cluster_) {
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
  Return_t __ReadCluster__(const Volume_t *vol_, Word_t cluster_, Byte_t **data_) {
    FUNCTION_ENTER;


    Word_t firstSector = 0x0u;
    Word_t clusterSize = 0x0u;
    Byte_t *buffer = null;
    Byte_t *sectorData = null;
    Word_t i = 0x0u;


    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(data_)) {
      /* Calculate cluster size and first sector */
      clusterSize = (Word_t) vol_->bytesPerSector * vol_->sectorsPerCluster;
      firstSector = __ClusterToSector__(vol_, cluster_);

      /* Allocate buffer for entire cluster */
      if(OK(__KernelAllocateMemory__((volatile Addr_t **) &buffer, clusterSize))) {
        /* Read all sectors in cluster */
        for(i = 0x0u; i < vol_->sectorsPerCluster; i++) {
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
  Return_t __GetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t *nextCluster_) {
    FUNCTION_ENTER;


    Word_t fatOffset = 0x0u;
    Word_t fatSector = 0x0u;
    Word_t entryOffset = 0x0u;
    Byte_t *sectorData = null;
    Word_t fatEntry = 0x0u;


    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(nextCluster_)) {
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
  Return_t __SetFATEntry__(const Volume_t *vol_, Word_t cluster_, Word_t value_) {
    FUNCTION_ENTER;


    Word_t fatOffset = 0x0u;
    Word_t fatSector = 0x0u;
    Word_t entryOffset = 0x0u;
    Byte_t *sectorData = null;
    Byte_t i = 0x0u;


    if(__ObjectIsValid__(vol_)) {
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
  Return_t __FindFreeCluster__(const Volume_t *vol_, Word_t startHint_, Word_t *freeCluster_) {
    FUNCTION_ENTER;


    Word_t cluster = 0x0u;
    Word_t maxCluster = 0x0u;
    Word_t fatEntry = 0x0u;
    Word_t searchStart = 0x0u;


    if(__ObjectIsValid__(vol_) && __PointerIsNotNull__(freeCluster_)) {
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

      /* If we didn't find anything from the hint to end, search from cluster 3
       * to hint */
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
 * @brief Add a block device to the mounted devices list
 * @param  blockDeviceUID_ Block device UID to add
 * @return                 ReturnOK on success, ReturnError if list is full
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
 * @brief Remove a block device from the mounted devices list
 * @param  blockDeviceUID_ Block device UID to remove
 * @return                 ReturnOK on success, ReturnError if not found
 */
  Return_t __RemoveMountedDevice__(const HalfWord_t blockDeviceUID_) {
    Byte_t i = 0x0u;
    Byte_t j = 0x0u;


    for(i = 0x0u; i < mountedDeviceCount; i++) {
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
      mountedDeviceCount = 0x0u;

      return;
    }


  #endif /* if defined(POSIX_ARCH_OTHER) */
#endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */