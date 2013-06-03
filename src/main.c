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



#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <glib.h>
#include "gfal_opers.h"
#include "params.h"

static const char* str_version = _GFALFS_VERSION;


     
static void path_to_abspath(const char* path, char* abs_buff, size_t s_buff){
	char cdir[2048];
	if(path == NULL || *path=='\0'){
		memset(abs_buff,'\0', s_buff);
		return;
	}
	if(*path!='/'){
		getcwd(cdir, 2048);
		g_strlcpy(abs_buff, cdir, s_buff);
		g_strlcat(abs_buff, "/",s_buff);
		g_strlcat(abs_buff, path,s_buff);		
	}else
		g_strlcpy(abs_buff, path, s_buff);
	
}

static void print_help(char* progname){
	g_printerr("Usage %s [-d] [-s] [-v] [mount_point] [remote_url]\n", progname);
	g_printerr("      %s [-g]           [mount_point]             \n", progname);
	g_printerr("\t [-d] : Debug mode 					          \n");	
	g_printerr("\t [-s] : Single thread mode			          \n");	
    g_printerr("\t [-o] : pass fuse specific option  			  \n");
	g_printerr("\t [-g] : Guid mode, without grid url		      \n");
	g_printerr("\t [-v] : Verbose mode, log all events with syslog, can cause major slowdown \n");
	g_printerr("\t [-V] : Print version number \n");
}

static void print_version(){
	printf("gfalFS_version : %s \n", str_version);
}

static void parse_args(int argc, char** argv, int* targc, char** targv){
	int c;
	char abs_path[2048];
    while( (c = getopt(argc, argv, "dshgvVo:"))  != -1){
		switch(c){
			case 'd':
				gfalfs_set_debug_mode(TRUE);
				targv[*targc] ="-d";
				*targc+=1;
				break;
			case 's':
				targv[*targc] ="-s";
				*targc+=1;
				break;
			case 'h':
				print_help(argv[0]);
				exit(1);
			case 'g':
				guid_mode = TRUE;
				break;
			case 'v':
				gfalfs_set_verbose_mode(TRUE);
				printf("verbose mode....\n");
				break;
			case 'V':
				print_version();
				exit(1);
        case 'o':
                // fuse options
                break;
			case '?':
				g_printerr("Unknow option -%c \n", optopt);
				print_help(argv[0]);
				exit(1);
		}		
	}
	int index = optind;
#if FUSE_MINOR_VERSION >= 8
	targv[(*targc)++] = "-obig_writes";
#endif
	//targv[(*targc)++] = "-odirect_io";
	if(guid_mode){
		if(index +1 != argc){
			g_printerr("Bad number of arguments \n");
			print_help(argv[0]);
			exit(1);
		}
		gfalfs_set_remote_mount_point("");	
		path_to_abspath(argv[index++], abs_path, 2048);
		gfalfs_set_local_mount_point(abs_path);
		targv[(*targc)++] = abs_path;
	}else{
		if(index +2 != argc){
			g_printerr("Bad number of arguments \n");
			print_help(argv[0]);
			exit(1);
		}
		path_to_abspath(argv[index++], abs_path, 2048);
		gfalfs_set_local_mount_point(abs_path);
		targv[(*targc)++] = abs_path;
		gfalfs_set_remote_mount_point(argv[index]);
	}
	return;	
}




int main(int argc, char *argv[])
{   
    if (!g_thread_supported())
        g_thread_init(NULL);
	int targc = 1;
	char* targv[20];
	targv[0] = argv[0]; 
	parse_args(argc, argv, &targc, targv);
	return fuse_main(targc, targv, &gfal_oper,NULL);
}
