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


#include <gfal_api.h>
#include <errno.h>
#include <glib.h>
#include <stdarg.h>
#include <syslog.h>

#include "params.h"

static gboolean verbose_mode = FALSE;
static gboolean debug_mode = FALSE;

/**
 * define verbose mode for gfalFS and GFAL 2.0
 * */
void gfalfs_set_verbose_mode(gboolean status){
	verbose_mode= status;
	if(status)
		gfal_set_verbose(GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_TRACE);
}

inline gboolean gfalfs_get_verbose_mode(){
	return verbose_mode;
}


/**
 * define verbose mode for gfalFS and GFAL 2.0
 * */
void gfalfs_set_debug_mode(gboolean status){
	debug_mode= status;
}

inline gboolean gfalfs_get_debug_mode(){
	return debug_mode;
}



static inline void gfalfs_log_debug(const char* prefix, const char* format, va_list va){
	if(gfalfs_get_debug_mode()){
		char buff[2048];
		g_strlcpy(buff, prefix, 2048);
		g_strlcat(buff, format, 2048);
		g_strlcat(buff, "\n",2048);
		vfprintf(stderr, buff, va);		
	}
}

/**
 * gfalFS log handler, optimized
 *
 */
inline void gfalfs_log (const gchar *log_domain, GLogLevelFlags log_level,
					 const gchar *format, ...){
	if( log_level & ( G_LOG_LEVEL_WARNING | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL)  ){
		va_list va;
		va_start(va, format);
		vsyslog(LOG_WARNING, format, va);
		va_end(va);
		va_start(va, format);
		gfalfs_log_debug("[WARNING]", format, va);
		va_end(va);
	}else if( (log_level & G_LOG_LEVEL_MESSAGE) && verbose_mode){
		va_list va;
		va_start(va, format);
		vsyslog(LOG_INFO, format, va);
		va_end(va);
		va_start(va, format);
		gfalfs_log_debug("[MESSAGE]",format, va);
		va_end(va);	
	}
			 
						 
}


