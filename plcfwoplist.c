/* plcfwoplist.c - library to fetch and write data from and to Siemens PLC 
 * multiple operations configuration and process
 */

#include <stdio.h>
#include <stdlib.h>

#include "plcfwconn.h"
#include "plcfwlib.h"
#include "plcfwop.h"
#include "plcfwoplist.h"

#define DEFAULT_CHUNK_LEN 10 
extern int _verbose;
 
static plcfwop *ops = NULL, *last = NULL, *actual = NULL;
static plcfwdb *dbs = NULL, *db_last = NULL;
static plcfwdb *chs = NULL, *ch_last = NULL;
int _chlen = DEFAULT_CHUNK_LEN;
 
plcfwop* plcfw_new_operation ();
void plcfw_delete_operation (plcfwop *op);
void plcfw_add_operation (plcfwop *op);

int plcfw_process_list_simple ();
int plcfw_process_list_blocks (plcfwdb *start);
void plcfw_process_all_responses ();

void plcfw_concatenate ();
void plcfw_chunking ();
int plcfw_distribute_responses ();

plcfwdb* plcfw_get_db (int db, int org_id, int operation);
plcfwdb* plcfw_add_chunk (int id, int org_id, int operation, int start_address, int end_address);
plcfwdb* plcfw_get_chunk (plcfwdb *start, int id, int org_id, int operation, int start_address, int end_address);

int plcfw_conf_raw (int org_id, int db, int sa, int len, char *desc)
{
	if (_verbose) {
		printf("Configuring RAW: org_id = %d, db = %d, sa = %d, len = %d, desc = %s.\n", org_id, db, sa, len, desc);
	}

	plcfwop *op = plcfw_new_operation();
	
	plcfw_default(op);
	op->mode = FW_MODE_RAW;
	op->org_id = org_id;
	op->data_block = db;
	op->start_address = sa;
	op->length = len;
	strncpy(op->description, desc, MAX_DESCRIPTION_LEN);

	int rc;
	if ((rc = plcfw_validate(op)) == 0) {
		plcfw_add_operation(op);
	}
	return rc;
}

int plcfw_conf_bool (int db, int sa, int bn, char *desc)
{
	if (_verbose) {
		printf("Configuring BOOL: db = %d, sa = %d, bn = %d, desc = %s.\n", db, sa, bn, desc);
	}
	
	plcfwop *op = plcfw_new_operation();
	
	plcfw_default(op);
	op->mode = FW_MODE_BOOL;
	op->data_block = db;
	op->start_address = sa;
	op->bit_number = bn;
	strncpy(op->description, desc, MAX_DESCRIPTION_LEN);

	int rc;
	if ((rc = plcfw_validate(op)) == 0) {
		plcfw_add_operation(op);
	}
	return rc;
}

int plcfw_conf_byte (int db, int sa, char *desc)
{
	if (_verbose) {
		printf("Configuring BYTE: db = %d, sa = %d, desc = %s.\n", db, sa, desc);
	}

	plcfwop *op = plcfw_new_operation();
	
	plcfw_default(op);
	op->mode = FW_MODE_CHAR;
	op->data_block = db;
	op->start_address = sa;
	strncpy(op->description, desc, MAX_DESCRIPTION_LEN);

	int rc;
	if ((rc = plcfw_validate(op)) == 0) {
		plcfw_add_operation(op);
	}
	return rc;
}

int plcfw_conf_int (int db, int sa, char *desc)
{
	if (_verbose) {
		printf("Configuring INT: db = %d, sa = %d, desc = %s.\n", db, sa, desc);
	}

	plcfwop *op = plcfw_new_operation();
	
	plcfw_default(op);
	op->mode = FW_MODE_INT;
	op->data_block = db;
	op->start_address = sa;
	strncpy(op->description, desc, MAX_DESCRIPTION_LEN);
	
	int rc;
	if ((rc = plcfw_validate(op)) == 0) {
		plcfw_add_operation(op);
	}
	return rc;
}

int plcfw_conf_numeric (int db, int sa, double off, double sl, char *desc)
{
	if (_verbose) {
		printf("Configuring NUMERIC: db = %d, sa = %d, offset = %f, slope = %f, desc = %s.\n", db, sa, off, sl, desc);
	}

	plcfwop *op = plcfw_new_operation();
	
	plcfw_default(op);
	op->mode = FW_MODE_NUMERIC;
	op->data_block = db;
	op->start_address = sa;
	op->offset = off;
	op->slope = sl;
	strncpy(op->description, desc, MAX_DESCRIPTION_LEN);
	
	int rc;
	if ((rc = plcfw_validate(op)) == 0) {
		plcfw_add_operation(op);
	}
	return rc;
}

int plcfw_conf_float (int db, int sa, double off, double sl, char *desc)
{
	if (_verbose) {
		printf("Configuring FLOAT: db = %d, sa = %d, off = %f, sl = %f, desc = %s.\n", db, sa, off, sl, desc);
	}

	plcfwop *op = plcfw_new_operation();
	
	plcfw_default(op);
	op->mode = FW_MODE_FLOAT;
	op->data_block = db;
	op->start_address = sa;
	op->offset = off;
	op->slope = sl;
	strncpy(op->description, desc, MAX_DESCRIPTION_LEN);
	
	int rc;
	if ((rc = plcfw_validate(op)) == 0) {
		plcfw_add_operation(op);
	}
	return rc;
}

void plcfw_set_chunk_length (int len)
{
	_chlen = len;
}

void plcfw_rewind_oplist ()
{
	actual = ops;
}

plcfwop *plcfw_next_op ()
{
	plcfwop *m = actual;
	if (actual != NULL) {
		actual = actual->next;
	}

	return m;
}

int plcfw_process_single ()
{
	plcfwop *op = ops; 

	if (op->operation != FW_OPERATION_FETCH)
	{
		fprintf(stderr, "Only FETCH operation is supported by this version of plcfwlib.\n");
		return -1;
	}

	int rc; 
	
	rc = plcfw_connect();
	if (rc == 0) {
		rc = plcfw_call_fetch(op->org_id, op->data_block, op->start_address, op->length, op->raw_result);
		if (rc != 0) {
			fprintf(stderr, "Error: There was a problem in communication with PLC.\n");
		}
		plcfw_disconnect();
	}

	if (rc == 0) {
		plcfw_process_response(op);
	}

	return rc;
}

int plcfw_process_list (int opt_level)
{
	int rc = 0;

	switch (opt_level) {
		case FW_OPTLEVEL_LIST:
			rc = plcfw_process_list_simple();
			break;

		case FW_OPTLEVEL_CONCAT:
			plcfw_concatenate();
			rc = plcfw_process_list_blocks(dbs);
			if (rc == 0) {
				rc = plcfw_distribute_responses();
				plcfw_process_all_responses();
			}
			break;

		case FW_OPTLEVEL_CHUNKS:
			plcfw_concatenate();
			plcfw_chunking();
			rc = plcfw_process_list_blocks(chs);
			if (rc == 0) {
				rc = plcfw_distribute_responses();
				plcfw_process_all_responses();
			}
			break;
	}

	return rc;
}

int plcfw_process_list_simple () 
{
	int rc = 0;

	rc = plcfw_connect();
	if (rc == 0) {
		plcfwop *op = NULL;

		op = ops;
		while (op != NULL) {
			switch (op->operation) {
				case FW_OPERATION_FETCH:
					if (_verbose) {
						printf("Attempting to FETCH: org_id [%d] data_block [%d] start_address [%d] length [%d] description [%s]\n", op->org_id, op->data_block, op->start_address, op->length, op->description);
					}
					rc = plcfw_call_fetch(op->org_id, op->data_block, op->start_address, op->length, op->raw_result);
					if (rc == 0) {
						plcfw_process_response(op);
					} else {
						fprintf(stderr, "Error: There was a problem in communication with PLC.\n");
					}
					break;

				case FW_OPERATION_WRITE:
					fprintf(stderr, "Sorry, WRITE operation not supported by this version of plcfw.\n");
					break;
			}
			op = op->next;
		}
	}

	plcfw_disconnect();

	return 0;
}

int plcfw_process_list_blocks (plcfwdb *start)
{
	int rc, len, len_bytes;
	
	rc = plcfw_connect();
	if (rc == 0) {
		plcfwdb *db = start;

		while (db != NULL) {
			len_bytes = db->end_address - db->start_address;
			if (len_bytes > plcfw_get_bufsize()) {
				fprintf(stderr, "Buffer len for the planned operation (%d) exceeds the MAX value given by protocol/command-line (%d).\n", len_bytes, plcfw_get_bufsize());
				return -1;
			}

			len = (db->org_id == FW_ORG_DB) ? len_bytes / 2 : len_bytes;
			switch (db->operation) {
				case FW_OPERATION_FETCH:
					if (_verbose) {
						printf("Attempting to FETCH: org_id [%d] data_block [%d] start_address [%d] length [%d]\n", db->org_id, db->id, db->start_address, len);
					}
					db->response = (unsigned char*)malloc(len_bytes);
					rc = plcfw_call_fetch(db->org_id, db->id, db->start_address, len, db->response);
					if (rc != 0) {
						fprintf(stderr, "Error: There was a problem in communication with PLC.\n");
					}
					break;

				case FW_OPERATION_WRITE:
					fprintf(stderr, "Sorry, WRITE operation not supported by this version of plcfw.\n");
					break;
			}
			db = db->next;
		}
	}

	plcfw_disconnect();

	return 0;
}

void plcfw_concatenate ()
{
	plcfwop *m = ops;

	int end_address = 0;
	plcfwdb *db;

	while (m != NULL) {
		if (_verbose) {
			printf("op sa [%d] m data_block %d: %d\n", m->start_address, m->data_block, m->size_bytes);
		}		
		db = plcfw_get_db(m->data_block, m->org_id, m->operation);
		if (m->start_address < db->start_address) {
			db->start_address = m->start_address;
		} 
		end_address = m->start_address + m->size_bytes;
		if (end_address > db->end_address) {
			db->end_address = end_address;
		}
		m->chunk = db;
		m = m->next;
	}

	if (_verbose) {
		db = dbs;
		while (db != NULL) {
			printf("db: id = %d, start = %d, end = %d\n", db->id, db->start_address, db->end_address);
			db = db->next;
		}
	}
}

void plcfw_chunking ()
{
	plcfwdb *db, *ch;
	plcfwop *op;
	
	db = dbs;
	while (db != NULL) {
		if (db->end_address - db->start_address > _chlen) {
			plcfw_add_chunk(db->id, db->org_id, db->operation, db->start_address, db->start_address + 1);
			db->chunks = 1;
		} else {
			db->chunks = 0;
		}
		db = db->next;
	}
	
	if (_verbose) {
		printf("Chunking - round 1:\n");
		ch = chs;
		while (ch != NULL) {
			printf("ch: id = %d, start = %d, end = %d\n", ch->id, ch->start_address, ch->end_address);
			ch = ch->next;
		}
	}

	op = ops;
	while (op != NULL) {
		ch = plcfw_get_chunk(chs, op->data_block, op->org_id, op->operation, op->start_address, op->start_address + op->size_bytes);
		if (ch == NULL) {
			ch = plcfw_add_chunk(op->data_block, op->org_id, op->operation, op->start_address, op->start_address + op->size_bytes);
		}
		op->chunk = ch;
		op = op->next;
	}

	if (_verbose) {
		printf("Chunking - round 2:\n");
		ch = chs;
		while (ch != NULL) {
			printf("ch: this = %p, id = %d, start = %d, end = %d\n", ch, ch->id, ch->start_address, ch->end_address);
			ch = ch->next;
		}
		
		op = ops;
		while (op != NULL) {
			printf("op: this = %p, db = %d, start = %d, end = %d, chunk = %p\n", op, op->data_block, op->start_address, op->start_address + op->size_bytes, op->chunk);
			op = op->next;
		}
	}
	plcfwdb *inch;

	ch = chs;
	while (ch != NULL) {
		inch = plcfw_get_chunk(ch->next, ch->id, ch->org_id, ch->operation, ch->start_address, ch->end_address);
		if (inch != NULL) {
			ch->replaced = inch;
		}
		ch = ch->next;
	}

	if (_verbose) {
		printf("Chunking - round 3:\n");
		ch = chs;
		while (ch != NULL) {
			printf("ch: this = %p, id = %d, start = %d, end = %d, replaced = %p\n", ch, ch->id, ch->start_address, ch->end_address, ch->replaced);
			ch = ch->next;
		}
	}

	op = ops;
	while (op != NULL) {
		ch = op->chunk;
		while (ch->replaced != NULL) {
			op->chunk = ch->replaced;
			ch = ch->replaced;
		}
		op = op->next;
	}
	
	if (_verbose) {
		printf("Chunking - round 4\n");
		op = ops;
		while (op != NULL) {
			printf("op: this = %p, db = %d, start = %d, end = %d, chunk = %p\n", op, op->data_block, op->start_address, op->start_address + op->size_bytes, op->chunk);
			op = op->next;
		}
	}
}

plcfwop* plcfw_new_operation ()
{
	plcfwop *m = (plcfwop *)malloc(sizeof(plcfwop));
	return m;
}

void plcfw_delete_operation (plcfwop *op)
{
	if (op != NULL) {
		free(op);
	}
}

void plcfw_add_operation (plcfwop *op)
{
	if (ops == NULL) {
		ops = op;
		last = ops;
	} else {
		last->next = op;
		last = last->next;
	}

	last->next = NULL;
}

plcfwdb* plcfw_get_db (int db, int org_id, int operation)
{
	plcfwdb *m = dbs;
	while (m != NULL) {
		if ((m->id == db) && (m->org_id == org_id) && (m->operation == operation)) {
			break;
		}
		m = m->next;
	}

	if (m == NULL) {
		if (db_last == NULL) {
			db_last = (plcfwdb *)malloc(sizeof(plcfwdb));
			dbs = db_last;
		} else {
			db_last->next = (plcfwdb *)malloc(sizeof(plcfwdb));
			db_last = db_last->next;
		}
		m = db_last;
		m->id = db;
		m->org_id = org_id;
		m->operation = operation;
		m->start_address = 32768;
		m->end_address = 0;
		m->next = NULL;
		m->response = NULL;
	}

	return m;
}

plcfwdb *plcfw_add_chunk (int id, int org_id, int operation, int start_address, int end_address)
{
	plcfwdb *m;
	m = (plcfwdb *)malloc(sizeof(plcfwdb));
	m->id = id;
	m->org_id = org_id;
	m->operation = operation;
	m->start_address = start_address;
	m->end_address = end_address;
	m->next = NULL;
	m->replaced = NULL;
	m->response = NULL;
	
	if (ch_last == NULL) {
		ch_last = m;
		chs = m;
	} else {
		ch_last->next = m;
		ch_last = m;
	}

	return m;
}

plcfwdb *plcfw_get_chunk (plcfwdb *start, int id, int org_id, int operation, int start_address, int end_address)
{
	plcfwdb *m;

	m = start;
	while (m != NULL) {
		if (((m->id == id) && (m->org_id == org_id) && (m->operation == operation)) &&
			(((start_address > (m->start_address - _chlen)) && (start_address < (m->end_address + _chlen))) ||
			((end_address > (m->start_address - _chlen)) && (end_address < (m->end_address + _chlen))))) {
			if (_verbose) {
				printf("Found: %d, %d is in [%d, %d]\n", start_address, end_address, m->start_address, m->end_address);
			}
			break;
		}
		m = m->next;
	}

	if (m != NULL) {
		if (m->start_address > start_address) {
			m->start_address = start_address;
		}
		if (m->end_address < end_address) {
			m->end_address = end_address;
		}
	}

	return m;
}

int plcfw_distribute_responses ()
{
	plcfwop *op;
	plcfwdb *db;

	int rc = 0;
	int offset;

	op = ops;
	while (op != NULL) {
		db = op->chunk;
		if ((db != NULL) && (db->response != NULL)) {
			offset = op->start_address - db->start_address;
			memcpy(op->raw_result, db->response + offset, op->size_bytes);
			if (_verbose == FW_DUMP_PACKETS) {
				printf("op sa: [%d], db sa: [%d], offset [%d], size [%d]\n", op->start_address, db->start_address, offset, op->size_bytes);
			}
		} else {
			fprintf(stderr, "Error: operation db [%d], sa: [%d] has no chunk assigned.\n", op->data_block, op->start_address);
			rc = -1;
		}
		op = op->next;
	}

	db = dbs;
	while (db != NULL) {
		free(db->response);
		db = db->next;
	}

	return rc;
}

void plcfw_process_all_responses ()
{
	plcfwop *op;
	op = ops;
	while (op != NULL) {
		plcfw_process_response(op);
		op = op->next;
	}
}
