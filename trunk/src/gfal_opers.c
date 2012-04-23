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
 * @file gfal_opers.c
 * @brief file for the mapping of the gfal operators
 * @author Devresse Adrien
 * @version 0.1
 * @date 2/07/2011
 * */
 
#define _GNU_SOURCE

#include <gfal_api.h>
#include <errno.h>

#include <string.h>
#include "gfal_opers.h"
#include "gfal_ext.h"

char mount_point[2048]; 
size_t s_mount_point=0;

char local_mount_point[2048];
size_t s_local_mount_point=0;

gboolean guid_mode=FALSE;

void gfalfs_set_local_mount_point(const char* local_mp){
	g_strlcpy(local_mount_point, local_mp, 2048);
	s_local_mount_point= strlen(local_mount_point);
}

void gfalfs_set_remote_mount_point(const char* remote_mp){
	g_strlcpy(mount_point, remote_mp, 2048);
	s_mount_point= strlen(remote_mp);
}

void gfalfs_construct_path(const char* path, char* buff, size_t s_buff){
	if(guid_mode){
		g_strlcpy(buff, path+1, s_buff);
	}else{
		 char* p = (char*) mempcpy(buff, mount_point, MIN(s_buff-1, s_mount_point));
		 p = mempcpy(p, path, MIN(s_buff-1-(p-buff), strlen(path)));
		 *p ='\0' ;	
	}
}

void gfalfs_construct_path_from_abs_local(const char* path, char* buff, size_t s_buff){
	char tmp_buff[2048];
	if(strstr(path, local_mount_point)== (char*)path){
		g_strlcpy(tmp_buff, path + s_local_mount_point, 2048);
	}
	gfalfs_construct_path(tmp_buff, buff, s_buff);
}

// convert a remote path of result in a local path
static void convert_external_readlink_to_local_readlink(char* external_buff, size_t s_ext, char* local_buff, ssize_t s_local){ 
	if( s_local > 0){	
			char internal_buff[2048];
			if(s_ext > s_mount_point){
				size_t s_input = MIN(s_ext, 2048-1);
				*((char*)mempcpy(internal_buff, external_buff+s_mount_point , s_input-s_mount_point )) = '\0';
		
				g_strlcpy(local_buff,local_mount_point, s_local );
				if(s_local > s_local_mount_point)
					g_strlcat(local_buff, internal_buff , s_local);
			}else{
				*((char*)mempcpy(local_buff, external_buff, MIN(s_ext, s_local-1) )) = '\0';
			}
		
	}
}


static int gfalfs_getattr(const char *path, struct stat *stbuf)
{
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE, "gfalfs_getattr path %s ", (char*) path);
	char buff[2048];
	char err_buff[1024];
	int ret=-1;
	gfalfs_construct_path(path, buff, 2048);
	if(fuse_interrupted())
		return -(ECANCELED);
    int a= gfal_lstat(buff, stbuf);
    if( (ret = -(gfal_posix_code_error()))){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_getattr error %d for path %s: %s ", (int) gfal_posix_code_error(), (char*)buff, (char*)gfal_posix_strerror_r(err_buff, 1024));
		gfal_posix_clear_error();
		return ret;
	}
	if(fuse_interrupted())
		return -(ECANCELED);
    return a;
}

static int gfalfs_readlink(const char *path, char* link_buff, size_t buffsiz)
{
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE, "gfalfs_readlink path %s ", (char*) path);
	char buff[2048];
	char err_buff[1024];
	char tmp_link_buff[2048];
	int ret=-1;
	gfalfs_construct_path(path, buff, 2048);
    ssize_t a= gfal_readlink(buff, tmp_link_buff, 2048-1);
	convert_external_readlink_to_local_readlink(tmp_link_buff, a, link_buff, buffsiz );
    if( (ret = -(gfal_posix_code_error()))){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_readlink error %d for path %s: %s ", (int) gfal_posix_code_error(), (char*)buff, (char*)gfal_posix_strerror_r(err_buff, 1024));
		gfal_posix_clear_error();
		return ret;
	}
	if(fuse_interrupted())
		return -(ECANCELED);
    return 0;
}

static int gfalfs_opendir(const char * path, struct fuse_file_info * f){
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_opendir path %s ", (char*) path);
	char buff[2048];
	char err_buff[1024];
	int ret;
	gfalfs_construct_path(path, buff, 2048);
	DIR* i = gfal_opendir(buff);
    if( (ret= -(gfal_posix_code_error()))){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_opendir err %d for path %s: %s", (int)gfal_posix_code_error(), (char*) buff, (char*) gfal_posix_strerror_r(err_buff, 1024));
		gfal_posix_clear_error();
		return ret;	
	}
	f->fh= gfalFS_dir_handle_new((void*)i, buff);
	if(fuse_interrupted())
		return -(ECANCELED);
	return (i!= NULL)?0:-(gfal_posix_code_error());
}

static int gfalfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_readdir path %s ",(char*) path);
	
	return gfalFS_dir_handle_readdir((gfalFS_dir_handle)fi->fh, offset, buf, filler);
}

static int gfalfs_open(const char *path, struct fuse_file_info *fi)
{
	char buff[2048];
	char err_buff[1024];
	int ret =-1;
	gfalfs_construct_path(path, buff, 2048);
	int i = gfal_open(buff,fi->flags,755);
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_open path %s %d", (char*) path, (int) i);
    if( (ret = -(gfal_posix_code_error())) || i==0){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_open err %d for path %s: %s ", (int) gfal_posix_code_error(), (char*)buff, (char*)gfal_posix_strerror_r(err_buff, 1024));
		gfal_posix_clear_error();
		return ret;	
	}
	
	fi->fh= i;
	if(fuse_interrupted())
		return -(ECANCELED);
	return 0;
}

static int gfalfs_creat (const char * path, mode_t mode , struct fuse_file_info * fi){
	char buff[2048];
	char err_buff[1024];
	int ret =-1;
	gfalfs_construct_path(path, buff, 2048);
	int i = gfal_creat(buff,mode);
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_open path %s %d", (char*) path, (int) i);
    if((ret = -(gfal_posix_code_error())) || i==0){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_open err %d for path %s: %s ", (int) gfal_posix_code_error(), (char*)buff, (char*)gfal_posix_strerror_r(err_buff, 1024));
		gfal_posix_clear_error();
		return ret;	
	}	
	fi->fh= i;
	if(fuse_interrupted())
		return -(ECANCELED);
	return 0;	
	
}

int gfalfs_chown (const char * path, uid_t uid, gid_t guid){
	// do nothing, change prop not authorized
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_chown path : %s ", (char*) path);
	gfal_posix_clear_error();
	return 0;
}

int gfalfs_utimens (const char * path, const struct timespec tv[2]){
	// do nothing, not implemented yet
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_utimens path : %s ", (char*) path);
	gfal_posix_clear_error();
	return 0;
}

int gfalfs_truncate (const char * path, off_t size){
	// do nothing, not implemented yet
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_truncate path : %s ", (char*) path);
	gfal_posix_clear_error();
	return 0;	
}


static int gfalfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
	char err_buff[1024];
	int ret = 0;
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_read path : %s fd : %d", (char*) path, (int) fi->fh);
	
	ret = gfal_pread(GPOINTER_TO_INT(fi->fh),(void*)buf, size, offset);
	if(ret <0 ){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_pread err %d for path %s: %s ", (int) gfal_posix_code_error(), (char*) path, (char*) gfal_posix_strerror_r(err_buff, 1024));
		ret = -(gfal_posix_code_error());
		gfal_posix_clear_error();
	}
	
	if(fuse_interrupted())
		return -(ECANCELED);
    return ret;
}


static int gfalfs_write(const char *path, const char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
	char err_buff[1024];
	int ret = 0;
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_write path : %s fd : %d", (char*) path, (int) fi->fh);
	
	ret = gfal_pwrite(GPOINTER_TO_INT(fi->fh),(void*)buf, size, offset);
	if(ret <0 ){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_pwrite err %d for path %s: %s ", (int) gfal_posix_code_error(), (char*) path, (char*) gfal_posix_strerror_r(err_buff, 1024));
		ret = -(gfal_posix_code_error());
		gfal_posix_clear_error();
	}
	
	if(fuse_interrupted())
		return -(ECANCELED);
    return ret;
}



static int gfalfs_access(const char * path, int flag){
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_access path : %s ", (char*) path);
	char buff[2048];
	char err_buff[1024];
	int ret;
	
	gfalfs_construct_path(path, buff, 2048);	
	int i = gfal_access(buff, flag);
	if( i < 0){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_access err %d for path %s: %s ", (int)gfal_posix_code_error(), (char*) buff, (char*) gfal_posix_strerror_r(err_buff, 1024));
		ret = -(gfal_posix_code_error());
		gfal_posix_clear_error();	
		return ret;
	}
	if(fuse_interrupted())
		return -(ECANCELED);
	return i;
}


static int gfalfs_unlink(const char * path){
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_access path : %s ", (char*) path);
	char buff[2048];
	char err_buff[1024];
	int ret;
	
	gfalfs_construct_path(path, buff, 2048);	
	int i = gfal_unlink(buff);
	if( i < 0){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_access err %d for path %s: %s ", (int)gfal_posix_code_error(), (char*) buff, (char*) gfal_posix_strerror_r(err_buff, 1024));
		ret = -(gfal_posix_code_error());
		gfal_posix_clear_error();	
		return ret;
	}
	if(fuse_interrupted())
		return -(ECANCELED);
	return i;
}


static int gfalfs_mkdir(const char * path, mode_t mode){
	gfal_posix_clear_error();
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_mkdir path : %s ", (char*) path);	
	char buff_path[2048];
	char err_buff[1024];
	
	gfalfs_construct_path(path, buff_path, 2048);
	int ret;	
	int i = gfal_mkdir(buff_path, mode);
	if( i < 0){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_mkdir err %d for path %s: %s ", (int) gfal_posix_code_error(), (char*) buff_path, (char*) gfal_posix_strerror_r(err_buff, 1024));
		ret = -(gfal_posix_code_error());
		gfal_posix_clear_error();	
		return ret;	
	}
	if(fuse_interrupted())
		return -(ECANCELED);
	return i;	
}

static int gfalfs_getxattr (const char * path, const char *name , char *buff, size_t s_buff){
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_getxattr path : %s, name : %s, size %d", (char*) path, (char*) name, (int) s_buff);	
	char buff_path[2048];
	char err_buff[1024];
	
	gfalfs_construct_path(path, buff_path, 2048);
	int ret;	
	int i = gfal_getxattr(buff_path, name, buff, s_buff);
	if( i < 0 ){
		const int errcode = gfal_posix_code_error();
		if(errcode != ENOATTR) // suppress verbose error for ENOATTR for perfs reasons
			gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_getxattr err %d for path %s: %s ", (int) errcode, (char*) buff_path, (char*) gfal_posix_strerror_r(err_buff, 1024));
		ret = -(errcode);
		gfal_posix_clear_error();	
		return ret;	
	}
	if(fuse_interrupted())
		return -(ECANCELED);
	return i;			
}


static int gfalfs_setxattr (const char * path, const char *name , const char *buff, size_t s_buff, int flag){
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_setxattr path : %s, name : %s", (char*) path, (char*) name);	
	char buff_path[2048];
	char err_buff[1024];
	gfalfs_construct_path(path, buff_path, 2048);
	
	
	int ret;	
	int i = gfal_setxattr(buff_path, name, buff, s_buff, flag);
	if( i < 0 ){
		const int errcode = gfal_posix_code_error();
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_setxattr err %d for path %s: %s ", (int) errcode, (char*) buff_path, (char*) gfal_posix_strerror_r(err_buff, 1024));
		ret = -(errcode);
		gfal_posix_clear_error();	
		return ret;	
	}
	if(fuse_interrupted())
		return -(ECANCELED);
	return i;			
}


static int gfalfs_listxattr (const char * path, char *list, size_t s_list){
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_listxattr path : %s, size %d", (char*) path, (int) s_list);	
	char buff_path[2048];
	char err_buff[1024];
	gfalfs_construct_path(path, buff_path, 2048);
	
	
	int ret;	
	int i = gfal_listxattr(buff_path, list, s_list);
	if( i < 0){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_listxattr err %d for path %s: %s ", (int) gfal_posix_code_error(), (char*) buff_path, (char*) gfal_posix_strerror_r(err_buff, 1024));
		ret = -(gfal_posix_code_error());
		gfal_posix_clear_error();	
		return ret;	
	}
	if(fuse_interrupted())
		return -(ECANCELED);
	return i;			
}

static int gfalfs_rename(const char*oldpath, const char* newpath){
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_rename oldpath : %s, newpath : %s ", (char*) oldpath, (char*) newpath);	
	char buff_oldpath[2048];
	char buff_newpath[2048];
	char err_buff[1024];
	
	int ret;
	gfalfs_construct_path(oldpath, buff_oldpath, 2048);	
	gfalfs_construct_path(newpath, buff_newpath, 2048);	
	int i = gfal_rename(buff_oldpath, buff_newpath);
	if( i < 0){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_rename err %d for oldpath %s: %s ", (int) gfal_posix_code_error(), (char*) buff_oldpath, (char*) gfal_posix_strerror_r(err_buff, 1024));
		ret = -(gfal_posix_code_error());
		gfal_posix_clear_error();
		return ret;		
	}
	if(fuse_interrupted())
		return -(ECANCELED);
	return i;	
	
}

static int gfalfs_symlink(const char*oldpath, const char* newpath){
	char buff_oldpath[2048];
	char buff_newpath[2048];
	char err_buff[1024];
	int ret;
	
	gfalfs_construct_path_from_abs_local(oldpath, buff_oldpath, 2048);	
	gfalfs_construct_path(newpath, buff_newpath, 2048);	
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_symlink oldpath : %s, newpath : %s ", (char*) buff_oldpath, (char*) buff_newpath);	
	int i = gfal_symlink(buff_oldpath, buff_newpath);
	if( i < 0){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_symlink err %d for oldpath %s: %s ", (int) gfal_posix_code_error(), (char*) buff_oldpath, (char*) gfal_posix_strerror_r(err_buff, 1024));
		ret = -(gfal_posix_code_error());
		gfal_posix_clear_error();
		return ret;		
	}
	if(fuse_interrupted())
		return -(ECANCELED);
	return i;	
}

static int gfalfs_release(const char* path, struct fuse_file_info *fi){
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_close fd : %d", (int) fi->fh);
	char err_buff[1024];
	const int fd = GPOINTER_TO_INT(fi->fh);
	
    int i = gfal_close(fd);
	if(i <0 ){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_close err %d for fd %d: %s ", (int) gfal_posix_code_error(), (int) fi->fh, (char*) gfal_posix_strerror_r(err_buff, 1024));
		i = -(gfal_posix_code_error());
		gfal_posix_clear_error();
	}
    return i;	
}

static int gfalfs_releasedir(const char* path, struct fuse_file_info *fi){
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_closedir fd : %d", (int) fi->fh);
	char err_buff[1024];
	
	DIR* d = gfalFS_dir_handle_get_fd((void*)fi->fh);
    int i = gfal_closedir(d);
    int ret;
	if(i <0 ){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_closedir err %d for fd %d: %s ", (int) gfal_posix_code_error(), (int) fi->fh, (char*) gfal_posix_strerror_r(err_buff, 1024));
		ret = -(gfal_posix_code_error());
		gfal_posix_clear_error();
		return ret;
	}
    return i;	
}

static int gfalfs_chmod(const char* path, mode_t mode){
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_chmod path : %s ", (char*) path);	
	char buff_path[2048];
	char err_buff[1024];
	int ret;
	
	gfalfs_construct_path(path, buff_path, 2048);	
	int i = gfal_chmod(buff_path, mode);
	if( i < 0){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_chmod err %d for path %s: %s ", (int) gfal_posix_code_error(), (char*) buff_path, (char*) gfal_posix_strerror_r(err_buff, 1024));
		ret = -(gfal_posix_code_error());
		gfal_posix_clear_error();
		return ret;			
	}
	if(fuse_interrupted())
		return -(ECANCELED);
	return i;		
	
}

static int gfalfs_rmdir(const char* path){
	gfalfs_log(NULL, G_LOG_LEVEL_MESSAGE,"gfalfs_rmdir path : %s ", (char*) path);	
	char buff_path[2048];
	char err_buff[1024];
	int ret;
	
	gfalfs_construct_path(path, buff_path, 2048);	
	int i = gfal_rmdir(buff_path);
	if( i < 0){
		gfalfs_log(NULL, G_LOG_LEVEL_WARNING , "gfalfs_rmdir err %d for path %s: %s ", (int) gfal_posix_code_error(),(char*) buff_path, (char*)gfal_posix_strerror_r(err_buff, 1024));
		ret = -(gfal_posix_code_error());
		gfal_posix_clear_error();
		return ret;		
	}
	if(fuse_interrupted())
		return -(ECANCELED);
	return i;		
}

struct fuse_operations gfal_oper = {
    .getattr	= gfalfs_getattr,
    .readdir	= gfalfs_readdir,
    .opendir	= gfalfs_opendir,
    .open	= gfalfs_open,
    .read	= gfalfs_read,
    .release = gfalfs_release,
    .releasedir = gfalfs_releasedir,
    .access = gfalfs_access,
    .create = gfalfs_creat,
    .mkdir = gfalfs_mkdir,
    .rmdir = gfalfs_rmdir,
    .chmod = gfalfs_chmod,
    .rename = gfalfs_rename,
    .write = gfalfs_write,
    .chown = gfalfs_chown,
    .utimens = gfalfs_utimens,
    .truncate = gfalfs_truncate,
    .symlink= gfalfs_symlink,
    .setxattr = gfalfs_setxattr,
    .getxattr= gfalfs_getxattr,
    .listxattr= gfalfs_listxattr,
    .readlink = gfalfs_readlink,
    .unlink = gfalfs_unlink
};



