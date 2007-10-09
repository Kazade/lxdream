/**
 * $Id: dreamcast.h,v 1.17 2007-10-09 08:12:29 nkeynes Exp $
 *
 * Public interface for dreamcast.c -
 * Central switchboard for the system. This pulls all the individual modules
 * together into some kind of coherent structure. This is also where you'd
 * add Naomi support, if I ever get a board to play with...
 *
 * Copyright (c) 2005 Nathan Keynes.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef dreamcast_H
#define dreamcast_H 1

#include <stdio.h>
#include <glib/gtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_TIMESLICE_LENGTH 1000000 /* nanoseconds */

#define MB *1024*1024
#define KB *1024

#define XLAT_NEW_CACHE_SIZE 32 MB
#define XLAT_TEMP_CACHE_SIZE 2 MB
#define XLAT_OLD_CACHE_SIZE 8 MB

#define CONFIG_TYPE_NONE 0
#define CONFIG_TYPE_FILE 1
#define CONFIG_TYPE_PATH 2
#define CONFIG_TYPE_KEY 3

#define DEFAULT_CONFIG_FILENAME "lxdream.rc"

typedef struct dreamcast_config_entry {
    const gchar *key;
    const int type;
    const gchar *default_value;
    gchar *value;
} *dreamcast_config_entry_t;

typedef struct dreamcast_config_group {
    const gchar *key;
    struct dreamcast_config_entry *params;
} *dreamcast_config_group_t;

void dreamcast_configure(void);
void dreamcast_configure_aica_only(void);
void dreamcast_init(void);
void dreamcast_reset(void);
void dreamcast_run(void);
void dreamcast_run_for( unsigned int seconds, unsigned int nanosecs );
void dreamcast_stop(void);
void dreamcast_shutdown(void);
gboolean dreamcast_is_running(void);

gboolean dreamcast_load_config( const gchar *filename );
gboolean dreamcast_save_config( const gchar *filename );
gboolean dreamcast_load_config_stream( FILE *f );
gboolean dreamcast_save_config_stream( FILE *f );

#define DREAMCAST_SAVE_MAGIC "%!-lxDream!Save\0"
#define DREAMCAST_SAVE_VERSION 0x00010000

int dreamcast_save_state( const gchar *filename );
int dreamcast_load_state( const gchar *filename );

#define SCENE_SAVE_MAGIC "%!-lxDream!Scene"
#define SCENE_SAVE_VERSION 0x00010000

extern struct dreamcast_config_group dreamcast_config_root[];

/* Global config values */
const gchar *dreamcast_get_config_value( int key );
#define CONFIG_BIOS_PATH 0
#define CONFIG_FLASH_PATH 1
#define CONFIG_DEFAULT_PATH 2
#define CONFIG_SAVE_PATH 3
#define CONFIG_BOOTSTRAP 4

#ifdef __cplusplus
}
#endif

#endif /* !dreamcast_H */
