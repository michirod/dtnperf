/*
 * file_transfer_tools.h
 *
 *  Created on: 08/ago/2012
 *      Author: michele
 */

#ifndef FILE_TRANSFER_TOOLS_H_
#define FILE_TRANSFER_TOOLS_H_

#include "dtnperf_types.h"


typedef struct file_transfer_info
{
	bp_endpoint_id_t client_eid;
	int filename_len;
	char * filename;
	u32_t file_dim;
	u32_t bytes_recvd;
	u32_t last_bundle_time; //timestamp secs
	u32_t expiration; //secs
}
file_transfer_info_t;

typedef struct file_transfer_info_list_item
{
	file_transfer_info_t * info;
	struct file_transfer_info_list_item * previous;
	struct file_transfer_info_list_item * next;
} file_transfer_info_list_item_t;

typedef struct file_transfer_info_list
{
	file_transfer_info_list_item_t * first;
	file_transfer_info_list_item_t * last;
	int count;
} file_transfer_info_list_t;


typedef struct pending_bundle
{
	bp_bundle_object_t * bundle;
} pending_bundle_t;

typedef struct pending_bundle_list_item
{
	pending_bundle_t * pending_bundle;
	struct pending_bundle_list_item * previous;
	struct pending_bundle_list_item * next;
} pending_bundle_list_item_t;

typedef struct pending_bundle_list
{
	pending_bundle_list_item_t * first;
	pending_bundle_list_item_t * last;
	int count;
} pending_bundle_list_t;


file_transfer_info_list_t file_transfer_info_list_create();
void file_transfer_info_list_destroy(file_transfer_info_list_t * list);

file_transfer_info_t * file_transfer_info_create(bp_endpoint_id_t client_eid,
		int filename_len,
		char * filename,
		u32_t file_dim,
		u32_t last_bundle_time,
		u32_t expiration);

void file_transfer_info_destroy(file_transfer_info_t * info);

void file_transfer_info_put(file_transfer_info_list_t * list, file_transfer_info_t * info);

file_transfer_info_list_item_t *  file_transfer_info_get_list_item(file_transfer_info_list_t * list, bp_endpoint_id_t client);

file_transfer_info_t *  file_transfer_info_get(file_transfer_info_list_t * list, bp_endpoint_id_t client);

void file_transfer_info_del(file_transfer_info_list_t * list, bp_endpoint_id_t client);


pending_bundle_list_t pending_bundle_list_create();

void pending_bundle_list_destroy(pending_bundle_list_t * list);

pending_bundle_t pending_bundle_create(bp_bundle_object_t * bundle);

void pending_bundle_destroy(pending_bundle_t * pending_bundle);

void pending_bundle_put(pending_bundle_list_t * list, pending_bundle_t * pending_bundle);

int pending_bundle_get_list_item(pending_bundle_list_t * list, bp_endpoint_id_t client, pending_bundle_list_item_t ** result);

int pending_bundle_get(pending_bundle_list_t * list, bp_endpoint_id_t client, pending_bundle_t ** result);

void pending_bundle_del(pending_bundle_list_t * list, bp_endpoint_id_t client);

/*
 * assemble_file() writes the file fragment contained in bundle to the file
 * indicated by info. Returns -1 if an error occurs, 0 if the fragment is written
 * succesfully, 1 if the written fragment is the last fragment of the file.
 */
int assemble_file(file_transfer_info_t * info, bp_bundle_object_t bundle, char * dir);

int process_incoming_file_transfer_bundle(file_transfer_info_list_t *info_list,
		pending_bundle_list_t * pending_list,
		bp_bundle_object_t * bundle,
		char * dir);
int process_incoming_file_transfer_bundle_first(file_transfer_info_list_t *info_list,
		pending_bundle_list_t * pending_list,
		bp_bundle_object_t bundle,
		char * dir);

u32_t get_file_fragment_size(long payload_size);

bp_error_t prepare_file_transfer_payload(dtnperf_options_t *opt, FILE * f, int fd, boolean_t * eof);

bp_error_t prepare_file_transfer_first_payload(dtnperf_options_t *opt, FILE * f, int fd,
		char * filename, u32_t file_dim);



#endif /* FILE_TRANSFER_TOOLS_H_ */
