/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005/2006, Anthony Minessale II <anthmct@yahoo.com>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthmct@yahoo.com>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * 
 * Anthony Minessale II <anthmct@yahoo.com>
 * Nenad Corbic <ncorbic@sangoma.com>
 *
 * switch_buffer.c -- Data Buffering Code
 *
 */

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <switch_buffer.h>
#include <pthread.h>

#ifndef assert
#define assert(_val_) if (!(_val_)) { return -1; }
#endif

static unsigned int buffer_id = 0;


int switch_buffer_free(switch_buffer_t *buffer)
{
	if (buffer) {
		if (buffer->data) {
			free(buffer->data);
		}
		buffer->data=NULL;
		pthread_mutex_destroy(&buffer->lock);
		free(buffer);
		buffer=NULL;
	}
	return 0;
}

int switch_buffer_create(switch_buffer_t **buffer, switch_size_t max_len)
{
	switch_buffer_t *new_buffer=NULL;;

	if ((new_buffer = malloc(sizeof(switch_buffer_t))) != 0
		&& (new_buffer->data = malloc(max_len)) != 0) {
		new_buffer->datalen = max_len;
		new_buffer->id = buffer_id++;
		*buffer = new_buffer;
		pthread_mutex_init(&new_buffer->lock, NULL);
		switch_buffer_zero(new_buffer);
		return 0;
	}
	
	switch_buffer_free(new_buffer);
	
	return -1;
}



int switch_buffer_len(switch_buffer_t *buffer)
{
	
	assert(buffer != NULL);
	return buffer->datalen;

}


int switch_buffer_freespace(switch_buffer_t *buffer)
{
	
	assert(buffer != NULL);
	return (buffer->datalen - buffer->used);
}

int switch_buffer_inuse(switch_buffer_t *buffer)
{
	assert(buffer != NULL);
	return buffer->used;
}

int switch_buffer_toss(switch_buffer_t *buffer, switch_size_t datalen)
{
	switch_size_t reading = 0;

	assert(buffer != NULL);
	
 	pthread_mutex_lock(&buffer->lock);
	if (buffer->used < 1) {
		buffer->used = 0;
		pthread_mutex_unlock(&buffer->lock);
		return 0;
	} else if (buffer->used >= datalen) {
		reading = datalen;
	} else {
		reading = buffer->used;
	}
	
	
	memmove(buffer->data, buffer->data + reading, buffer->datalen - reading);
	buffer->used -= datalen;
	pthread_mutex_unlock(&buffer->lock);

	return buffer->datalen;
}

int switch_buffer_read(switch_buffer_t *buffer, void *data, switch_size_t datalen)
{
	switch_size_t reading = 0;

	assert(buffer != NULL);
	assert(data != NULL);


	pthread_mutex_lock(&buffer->lock);
	if (buffer->used < 1) {
		buffer->used = 0;
		pthread_mutex_unlock(&buffer->lock);
		return 0;
	} else if (buffer->used >= datalen) {
		reading = datalen;
	} else {
		reading = buffer->used;
	}

	memcpy(data, buffer->data, reading);
	memmove(buffer->data, buffer->data + reading, buffer->datalen - reading);
	buffer->used -= reading;
	pthread_mutex_unlock(&buffer->lock);
	
#if 0
	printf("BUFFER READ: USED %i Reading=%i\n",
		buffer->used,reading);
#endif	
	return reading;
}

int switch_buffer_write(switch_buffer_t *buffer, void *data, switch_size_t datalen)
{
	switch_size_t freespace;

	assert(buffer != NULL);
	assert(data != NULL);
	assert(buffer->data != NULL);
	
	pthread_mutex_lock(&buffer->lock);
	freespace = buffer->datalen - buffer->used;
	if (freespace < datalen) {
		pthread_mutex_unlock(&buffer->lock);
		return 0;
	} else {
		memcpy(buffer->data + buffer->used, data, datalen);
		buffer->used += datalen;
	}

	pthread_mutex_unlock(&buffer->lock);
	
#if 0
	printf("BUFFER WRITE: USED %i\n",buffer->used);
#endif	
	return buffer->used;
}

int switch_buffer_zero(switch_buffer_t *buffer)
{
	assert(buffer != NULL);
    	assert(buffer->data != NULL);

	pthread_mutex_lock(&buffer->lock);
	buffer->used = 0;
	pthread_mutex_unlock(&buffer->lock);
	
	return 0;
	
}
