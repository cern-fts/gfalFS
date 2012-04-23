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
 * @file params.h
 * @brief header for parameters management 
 * @author Devresse Adrien
 * @version 0.1
 * @date 13/09/2011
 * */
#include <glib.h>

#define GFALFS_URL_MAX_LEN 2048

void gfalfs_set_verbose_mode(gboolean status);
gboolean gfalfs_get_verbose_mode();

void gfalfs_set_debug_mode(gboolean status);
gboolean gfalfs_get_debug_mode();


void gfalfs_log (const gchar *log_domain,
					 GLogLevelFlags log_level,
					 const gchar *format,
					 ...);
