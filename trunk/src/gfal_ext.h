#pragma once
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file gfal_ext.c
 * @brief header for some extended functionalities for gfalFS
 * @author Devresse Adrien
 * @date 19/09/2011
 * */

#include <stdlib.h>
#include <glib.h>
#include "gfal_opers.h"
#include "params.h"

typedef struct _gfalFS_file_handle{
	char path[GFALFS_URL_MAX_LEN];
	void* fh;
	off_t offset;
	GMutex* mut;
	
} *gfalFS_file_handle;


typedef struct _gfalFS_dir_handle{
	char path[GFALFS_URL_MAX_LEN];
	void* fh;
	off_t offset; // current offset
	struct dirent* dir; // last dir, NULL if no state 
	GMutex* mut;
	
} *gfalFS_dir_handle;


gfalFS_dir_handle gfalFS_dir_handle_new(void* fh, const char* dirpath);
int gfalFS_dir_handle_readdir(gfalFS_dir_handle handle, off_t offset, void* buff, fuse_fill_dir_t filler);
void* gfalFS_dir_handle_get_fd(gfalFS_dir_handle handle);
void gfalFS_dir_handle_delete(gfalFS_dir_handle handle);


gfalFS_file_handle gfalFS_file_handle_new(void* fh, const char* path);


void gfalFS_file_handle_delete(gfalFS_file_handle handle);

void* gfalFS_file_handle_get_fd(gfalFS_file_handle handle);

int gfalFS_file_handle_write(gfalFS_file_handle handle, const char *buf, size_t size, off_t offset);

int gfalFS_file_handle_read(gfalFS_file_handle handle, const char *buf, size_t size, off_t offset);

