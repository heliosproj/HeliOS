/*UNCRUSTIFY-OFF*/
/**
 * @file fs_harness.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Unit test harness header for filesystem
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
#ifndef FS_HARNESS_H_
  #define FS_HARNESS_H_


  #include "../unit/unit.h"
  void fs_harness(void);
  void test_mount_unmount(void);
  void test_file_operations(void);
  void test_directory_operations(void);
  void test_file_management(void);
  void test_fs_edge_cases(void);
  void test_large_file_operations(void);
  void test_cluster_boundary_operations(void);
  void test_partial_io_operations(void);
  void test_volume_info_validation(void);
  void test_file_mode_validation(void);
  void test_closed_file_operations(void);
  void test_multiple_file_operations(void);


#endif /* ifndef FS_HARNESS_H_ */