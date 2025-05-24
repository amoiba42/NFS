#ifndef __DEFS_H
#define __DEFS_H

#include "ss.h"
#include "../utilities/api.h"


//stream.c
#include <vlc/vlc.h>
#define AUDIO_CHUNK_SIZE 4096
void* ss_handle_audio(void* arg);

//read.c
void* ss_handle_read(void* arg);

//write.c
void* ss_handle_write(void* arg);

//create.c
void* ss_handle_create(void* arg, int flag);

// delete.c
void* ss_handle_delete(void* arg, int flag);

//ping.c
void* ss_handle_ping(void* arg);

//info.c
void* ss_handle_info(void* arg);

#endif