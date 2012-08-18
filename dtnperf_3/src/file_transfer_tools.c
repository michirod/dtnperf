/*
 * file_transfer_tools.c
 *
 *  Created on: 08/ago/2012
 *      Author: michele
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <bp_abstraction_api.h>
#include "file_transfer_tools.h"
#include "bundle_tools.h"

file_transfer_info_list_t file_transfer_info_list_create()
{
	file_transfer_info_list_t * list;
	list = (file_transfer_info_list_t *) malloc(sizeof(file_transfer_info_list_t));
	list->first = NULL;
	list->last = NULL;
	list->count = 0;
	return * list;
}

void file_transfer_info_list_destroy(file_transfer_info_list_t * list)
{
	free(list);
}

file_transfer_info_t * file_transfer_info_create(bp_endpoint_id_t client_eid,
		int filename_len,
		char * filename,
		u32_t file_dim,
		u32_t last_bundle_time,
		u32_t expiration)
{
	file_transfer_info_t * info;
	info = (file_transfer_info_t *) malloc(sizeof(file_transfer_info_t));
	bp_copy_eid(&(info->client_eid), &client_eid);
	info->filename_len = filename_len;
	info->filename = (char*) malloc(filename_len + 1);
	strncpy(info->filename, filename, filename_len +1);
	info->file_dim = file_dim;
	info->last_bundle_time = last_bundle_time;
	info->expiration = expiration;
	return info;
}

void file_transfer_info_destroy(file_transfer_info_t * info)
{
	free(info->filename);
	free(info);
}

void file_transfer_info_put(file_transfer_info_list_t * list, file_transfer_info_t * info)
{
	file_transfer_info_list_item_t * new;
	new = (file_transfer_info_list_item_t *) malloc(sizeof(file_transfer_info_list_item_t));
	new->info = info;
	new->next = NULL;
	if (list->first == NULL) // empty list
	{
		new->previous = NULL;
		list->first = new;
		list->last = new;
	}
	else
	{
		new->previous = list->last;
		list->last->next = new;
		list->last = new;
	}
	list->count ++;
}

file_transfer_info_list_item_t *  file_transfer_info_get_list_item(file_transfer_info_list_t * list, bp_endpoint_id_t client)
{
	file_transfer_info_list_item_t * item = list->first;
	while (item != NULL)
	{
		if (strcmp(item->info->client_eid.uri, client.uri) == 0)
		{
			return item;
		}
		item = item->next;
	}
	return NULL;
}

file_transfer_info_t *  file_transfer_info_get(file_transfer_info_list_t * list, bp_endpoint_id_t client)
{
	file_transfer_info_list_item_t * item;
	item = file_transfer_info_get_list_item(list, client);
	if (item != NULL)
	{
		return item->info;
	}
	return NULL;
}

void file_transfer_info_del(file_transfer_info_list_t * list, bp_endpoint_id_t client)
{
	file_transfer_info_list_item_t * item;
	item = file_transfer_info_get_list_item(list, client);
	if (item != NULL)
	{
		if (item->next == NULL && item->previous == NULL) // unique element of the list
		{
			list->first = NULL;
			list->last = NULL;
		}
		else if (item->next == NULL)  // last element of list
		{
			item->previous->next = NULL;
			list->last = item->previous;
		}
		else if (item->previous == NULL) // first element of list
		{
			item->next->previous = NULL;
			list->first = item->next;
		}
		else // generic element of list
		{
			item->next->previous = item->previous;
			item->previous->next = item->next;
		}
		file_transfer_info_destroy(item->info);
		free(item);
		list->count --;
	}
}

pending_bundle_list_t pending_bundle_list_create()
{
	pending_bundle_list_t * list;
	list = (pending_bundle_list_t *) malloc(sizeof(pending_bundle_list_t));
	list->first = NULL;
	list->last = NULL;
	list->count = 0;
	return * list;
}

void pending_bundle_list_destroy(pending_bundle_list_t * list)
{
	free(list);
}

pending_bundle_t pending_bundle_create(bp_bundle_object_t * bundle)
{
	pending_bundle_t * pending_bundle;
	pending_bundle = (pending_bundle_t *) malloc(sizeof(pending_bundle_t));
	pending_bundle->bundle = bundle;
	return * pending_bundle;
}

void pending_bundle_destroy(pending_bundle_t * pending_bundle)
{
	bp_bundle_free(pending_bundle->bundle);
	free(pending_bundle);
}

void pending_bundle_put(pending_bundle_list_t * list, pending_bundle_t * pending_bundle)
{
	pending_bundle_list_item_t * new;
	new = (pending_bundle_list_item_t *) malloc(sizeof(pending_bundle_list_item_t));
	new->next = NULL;
	new->pending_bundle = pending_bundle;
	if (list->first == NULL) // empty list
	{
		new->previous = NULL;
		list->first = new;
		list->last = new;
	}
	else
	{
		new->previous = list->last;
		list->last->next = new;
		list->last = new;
	}
	list->count ++;
}

int pending_bundle_get_list_item(pending_bundle_list_t * list, bp_endpoint_id_t client, pending_bundle_list_item_t ** result)
{
	pending_bundle_list_item_t * item = list->first;
	pending_bundle_list_item_t  temp[list->count];
	bp_endpoint_id_t client_eid;
	int count = 0;

	result = NULL;

	while (item != NULL)
	{
		bp_bundle_get_source(*(item->pending_bundle->bundle), &client_eid);
		if (strcmp(client_eid.uri, client.uri) == 0)
		{
			temp[count] = *item;
			count ++;
		}
		item = item->next;
	}
	if (count > 0)
	{
		*result = (pending_bundle_list_item_t *) malloc(count * sizeof(pending_bundle_list_item_t));
		memcpy(result, temp, count * sizeof(pending_bundle_list_item_t));
	}
	return count;
}

int pending_bundle_get(pending_bundle_list_t * list, bp_endpoint_id_t client, pending_bundle_t ** result)
{
	pending_bundle_list_item_t * items;
	pending_bundle_t * pending_bundles;
	int i, count;
	count = pending_bundle_get_list_item(list, client, &items);
	pending_bundles = (pending_bundle_t *) malloc(count * sizeof(pending_bundle_t));
	for(i = 0; i < count; i++){
		pending_bundles[i] = *(items[i].pending_bundle);
	}
	if (count > 0)
		free(items);
	return count;
}

void pending_bundle_del(pending_bundle_list_t * list, bp_endpoint_id_t client)
{
	pending_bundle_list_item_t * items;
	pending_bundle_list_item_t * item;
	int count, i;
	count = pending_bundle_get_list_item(list, client, &items);
	for(i = 0; i < count; i++)
	{
		item = items + (i * sizeof(pending_bundle_list_item_t));
		if (item->next == NULL)  // last element of list
		{
			item->previous->next = NULL;
			list->last = item->previous;
		}
		else if (item->previous == NULL) // first element of list
		{
			item->next->previous = NULL;
			list->first = item->next;
		}
		else // generic element of list
		{
			item->next->previous = item->previous;
			item->previous->next = item->next;
		}
		pending_bundle_destroy(item->pending_bundle);
		free(item);
		list->count --;
	}
}

int assemble_file(file_transfer_info_t * info, bp_bundle_object_t bundle, char * dir)
{
	char * transfer;
	u32_t transfer_len;
	FILE * f = NULL;
	int fd;
	u32_t offset, pl_size;
	bp_timestamp_t timestamp;
	bp_timeval_t expiration;

	bp_bundle_get_payload_size(bundle, &pl_size);
	bp_bundle_get_expiration(bundle, &expiration);
	bp_bundle_get_creation_timestamp(bundle, &timestamp);

	// transfer length is total payload length without header,
	// congestion control char and file fragment offset
	transfer_len = pl_size - (HEADER_SIZE + 1 + sizeof(u32_t));

	// create stream from incoming bundle payload
	if (open_payload_stream_read(bundle, &f) < 0)
		return -1;

	// skip header and congestion control char
	fseek(f, HEADER_SIZE + 1, SEEK_SET);

	// read file fragment offset
	fread(&offset, sizeof(u32_t), 1, f);

	// read remaining file fragment
	transfer = (char*) malloc(transfer_len);
	if (fread(transfer, transfer_len, 1, f) != 1)
		return -1;
	fclose(f);

	// open or create destination file
	char* filename = (char*) malloc(info->filename_len + strlen(dir) +1);
	strcpy(filename, dir);
	strcat(filename, info->filename);
	fd = open(filename, O_WRONLY | O_CREAT, 0755);
	if (fd < 0)
	{
		perror("open");
		return -1;
	}


	// write fragment
	lseek(fd, offset, SEEK_SET);
	if (write(fd, transfer, transfer_len) < 0)
		return -1;
	close(fd);

	// deallocate resources
	free(filename);
	free(transfer);

	// update info
	info->bytes_recvd += transfer_len;
	info->expiration = expiration;
	info->last_bundle_time = timestamp.secs;

	// if transfer completed return 1
	if (info->bytes_recvd >= info->file_dim)
		return 1;

	return 0;

}

int process_incoming_file_transfer_bundle(file_transfer_info_list_t *info_list,
		pending_bundle_list_t * pending_list,
		bp_bundle_object_t * bundle,
		char * dir)
{
	bp_endpoint_id_t client_eid;
	file_transfer_info_t * info;

	// get info from bundle
	bp_bundle_get_source(*bundle, &client_eid);

	info = file_transfer_info_get(info_list, client_eid);
	if (info != NULL) // first bundle of transfer already received
	{
		int result;


		// assemble file
		result = assemble_file(info, *bundle, dir);
		if (result < 0) // error
			return result;
		if (result == 1) // transfer completed
		{
			// remove info from list
			file_transfer_info_del(info_list, client_eid);
		}
		return 0;
	}
	else // save in pending bundles
	{
		pending_bundle_t pending_bundle;
		pending_bundle = pending_bundle_create(bundle);
		pending_bundle_put(pending_list, &pending_bundle);
		return 1;
	}

}

int process_incoming_file_transfer_bundle_first(file_transfer_info_list_t *info_list,
		pending_bundle_list_t * pending_list,
		bp_bundle_object_t bundle,
		char * dir)
{
	FILE * pl_stream = NULL;
	bp_endpoint_id_t client_eid;
	bp_timestamp_t timestamp;
	bp_timeval_t expiration;
	int filename_len;
	char * filename;
	u32_t file_dim;
	file_transfer_info_t * info;
	pending_bundle_t * pending_bundles;
	int pending_bundles_count, i, result;
	bp_bundle_object_t * pending_bundle = NULL;

	// get bundle info
	bp_bundle_get_source(bundle, &client_eid);
	bp_bundle_get_creation_timestamp(bundle, &timestamp);
	bp_bundle_get_expiration(bundle, &expiration);

	// get payload stream to read file transfer info
	open_payload_stream_read(bundle, &pl_stream);
	result = fseek(pl_stream, HEADER_SIZE + 1, SEEK_SET);

	// get filename len
	result = fread(&filename_len, sizeof(int), 1, pl_stream);

	// get filename
	filename = (char *) malloc(filename_len + 1);
	memset(filename, 0, filename_len + 1);
	result = fread(filename, filename_len, 1, pl_stream);
	if(result < 1 )
		perror("fread");
	filename[filename_len] = '\0';

	//get file size
	fread(&file_dim, sizeof(u32_t), 1, pl_stream);

	// create file transfer info object
	info = file_transfer_info_create(client_eid, filename_len, filename, file_dim, timestamp.secs, expiration);

	// insert info into info list
	file_transfer_info_put(info_list, info);

	//check for pending bundles
	pending_bundles_count = pending_bundle_get(pending_list, client_eid, &pending_bundles);

	// assemble pending bundles
	for (i = 0; i < pending_bundles_count; i++)
	{
		pending_bundle = pending_bundles[i].bundle;
		assemble_file(info, *pending_bundle, dir);
	}

	// remove and deallocate pending bundles
	pending_bundle_del(pending_list, client_eid);
	return 0;
}

u32_t get_file_fragment_size(long payload_size)
{
	u32_t result;
	// file fragment size is payload without header, congestion ctrl char and offset
	result = payload_size - (HEADER_SIZE + 1 + sizeof(u32_t));
	return result;
}

bp_error_t prepare_file_transfer_payload(dtnperf_options_t *opt, FILE * f, int fd, boolean_t * eof)
{
	if (f == NULL)
		return BP_ENULLPNTR;

	bp_error_t result;
	u32_t fragment_len;
	char * fragment;
	u32_t offset;
	long bytes_read;

	// prepare header and congestion control
	result = prepare_payload_header(opt, f, FALSE);

	// get size of fragment and allocate fragment
	fragment_len = get_file_fragment_size(opt->bundle_payload);
	fragment = (char *) malloc(fragment_len);

	// get offset of fragment
	offset = lseek(fd, 0, SEEK_CUR);
	// write offset in the bundle
	fwrite(&offset, sizeof(offset), 1, f);

	// read fragment from file
	bytes_read = read(fd, fragment, fragment_len);
	if (bytes_read < fragment_len) // reached EOF
		*eof = TRUE;
	else
		*eof = FALSE;

	// write fragment in the bundle
	fwrite(fragment, bytes_read, 1, f);

	return result;
}

bp_error_t prepare_file_transfer_first_payload(dtnperf_options_t *opt, FILE * f, int fd,
		char * filename, u32_t file_dim)
{
	if (f == NULL)
		return BP_ENULLPNTR;

	bp_error_t result;
	int filename_len = strlen(filename);

	// prepare header and congestion control
	result = prepare_payload_header(opt, f, TRUE);

	// write filename length
	fwrite(&filename_len, sizeof(int), 1, f);

	// write filename
	fwrite(filename, filename_len, 1, f);

	//write file size
	fwrite(&file_dim, sizeof(file_dim), 1, f);

	return result;
}
