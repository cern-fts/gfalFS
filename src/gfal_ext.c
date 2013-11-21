// Copyright @ Members of the EMI Collaboration, 2010.
// See www.eu-emi.eu for details on the copyright holders.
// 
// Licensed under the Apache License, Version 2.0 (the "License"); 
// you may not use this file except in compliance with the License. 
// You may obtain a copy of the License at 
// 
//     http://www.apache.org/licenses/LICENSE-2.0 
// 
// Unless required by applicable law or agreed to in writing, software 
// distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and 
// limitations under the License.

/*
 * gfal_ext.c
 * some extended functionalities for gfalFS
 * author Devresse Adrien
 * */
 
#include <errno.h>
#include <math.h>
#include <string.h>

#include <gfal_api.h>

#include "gfal_ext.h"


gfalFS_dir_handle gfalFS_dir_handle_new(void* fh, const char* dirpath){
	gfalFS_dir_handle ret = g_new0(struct _gfalFS_dir_handle, 1) ;
	g_strlcpy(ret->path, dirpath, GFALFS_URL_MAX_LEN);
	ret->fh = fh;
	ret->offset = 0;
	ret->mut = g_mutex_new();
	return ret;
}


void gfalFS_dir_handle_delete(gfalFS_dir_handle handle){
	if(handle){
		g_mutex_free (handle->mut);
		free(handle);
	}
}

int gfalFS_dir_handle_readdir(gfalFS_dir_handle handle, off_t offset, void* buf, fuse_fill_dir_t filler){
	char buff[2048];
	char err_buff[1024];
	int ret;
	
	if(offset != handle->offset){ // corrupted seq
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_readdir err : Dir descriptor corruption, not in order %ld %ld", (long) offset, (long) handle->offset);
		return -(EFAULT);
	}
	if(handle->dir != NULL){ // try to recover from  previous saved status
	
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = handle->dir->d_ino;
		st.st_mode = handle->dir->d_type << 12;	
        gfalfs_tune_stat(&st);
	
		if( filler(buf, handle->dir->d_name, &st, handle->offset+1) ==1){ 
			return 0; // filler buffer full 
		} else{
			handle->offset += 1;
			handle->dir = NULL;
			return 0;
		}
	}
	
	while( (handle->dir = gfal_readdir(handle->fh)) != NULL){
		if(fuse_interrupted())
			return -(ECANCELED);	
			
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = handle->dir->d_ino;
		st.st_mode = handle->dir->d_type << 12;	
	
		ret = filler(buf, handle->dir->d_name, &st, handle->offset+1);	
		if(ret == 1) // buffer full
			return 0;
		handle->offset += 1;
		
	}
    if( (ret = -(gfal_posix_code_error()))){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_readdir err %d for path %s: %s ", (int) gfal_posix_code_error(), (char*)buff, (char*)gfal_posix_strerror_r(err_buff, 1024));
		gfal_posix_clear_error();
		return ret;
	}
	return 0;		
		
}

void* gfalFS_dir_handle_get_fd(gfalFS_dir_handle handle){
	return handle->fh;
}


void gfalfs_tune_stat(struct stat * st){
    // tune block size to 16Mega for cp optimization with big files on network file system
    st->st_blksize = (1 <<24);
    // workaround for utilities like du that use st_blocks (LCGUTIL-289)
    st->st_blocks = ceil(st->st_size / 512.0);
}
