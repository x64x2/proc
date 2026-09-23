/* plcfwops.c - library to fetch and write data from and to Siemens PLC 
 * operations management module; it's purpose is to optimize access to PLC memory in case of multiple requests to the same data block etc.
 *
 */

#include "plcfwlib.h"
#include "plcfwop.h"
#include "plcfwoplist.h"
#include "plcfwops.h"

extern int _verbose;

static plcfwop *ops = NULL, *last = NULL;
static plcfwdb *dbs = NULL, *db_last = NULL;

void plcfw_chunking ();

plcfwop* plcfw_new_operation ()
{
	if ((last != NULL ) && (last->operation == FW_OPERATION_DISCARDED)) {
		last->operation = FW_OPERATION_FETCH; 
		return last;
	}

	if (ops == NULL) {
		ops = (plcfwop *)malloc(sizeof(plcfwop));
		last = ops;
	} else {
		last->next = (plcfwop *)malloc(sizeof(plcfwop));
		last = last->next;
	}

	last->next = NULL;
	plcfw_default(last);

	return last;
}

void plcfw_default (plcfwop *op)
{
	op->operation = FW_OPERATION_FETCH; 
	op->mode = FW_MODE_RAW;
	op->org_id = FW_ORG_DB; 
	op->data_block = -1;
	op->start_address = -1;
	op->length = -1;
	op->bit_number = -1;
	op->slope = 1.0; 
	op->offset = 0.0; 
	op->chunk = NULL;
	memset(op->description, 0x0, MAX_DESCRIPTION_LEN);
}

int plcfw_validate (plcfwop *op)
{
	int rc = 0;
	
	if ((op->mode == FW_MODE_RAW) && (op->length == -1)) {
		fprintf(stderr, "Error: Data length is not given. Please use '-len' or '--length' parameter.\n");		
		rc = -1;
	}

	if (op->data_block == -1) {
		fprintf(stderr, "Error: Data block address is not given. Please use '-db' or '--data-block' parameter.\n");		
		rc = -1;
	}

	if (op->start_address == -1) {
		fprintf(stderr, "Error: Start address is not given. Please use '-sa' or '--start-address' parameter.\n");		
		rc = -1;
	}
			
	if ((op->mode == FW_MODE_BOOL) && (op->bit_number == -1)) {
		fprintf(stderr, "Error: Bit number of the boolean request is not given. Please use '-bn' or '--bit-number' parameter.\n");		
		rc = -1;
	}

	if ((op->mode != FW_MODE_RAW) && (op->org_id != FW_ORG_DB)) {
		fprintf(stderr, "Error: Sorry, in non DB memory access mode, only raw operation (not bool, not numeric, not float) is supported.\n");		
		rc = -1;
	}

	if (rc != -1) {
		if ((op->mode == FW_MODE_NUMERIC) || (op->mode == FW_MODE_INT) || (op->mode == FW_MODE_CHAR) || (op->mode == FW_MODE_BOOL)) {
			if (_verbose == 1) {
				printf("Due to bool or numeric mode, the data length is set to 1\n");
			}
			op->length = 1;
		}
		if (op->mode == FW_MODE_FLOAT) {
			if (_verbose == 1) {
				printf("Due to float mode, the data length is set to 2\n");
			}
			op->length = 2;
		}
	}

	if (op->description[0] == 0) {
		switch (op->mode) {
			case FW_MODE_RAW:
				sprintf(op->description, "Value (raw bytes) of data block [%d], on address [%d], length [%d] bytes/words", op->data_block, op->start_address, op->length);
				break;

			case FW_MODE_BOOL:
				sprintf(op->description, "Value (boolean) of data block [%d], on address [%d], bit number [%d]", op->data_block, op->start_address, op->bit_number, op->length);
				break;

			case FW_MODE_CHAR:
				sprintf(op->description, "Value (byte) of data block [%d], on address [%d]", op->data_block, op->start_address, op->length);
				break;

			case FW_MODE_INT:
				sprintf(op->description, "Value (integer) of data block [%d], on address [%d], bit number [%d]", op->data_block, op->start_address, op->bit_number, op->length);
				break;

			case FW_MODE_NUMERIC:
				sprintf(op->description, "Value (numeric) of data block [%d], on address [%d], slope [%f], offset [%f]", op->data_block, op->start_address, op->slope, op->offset);
				break;

			case FW_MODE_FLOAT:
				sprintf(op->description, "Value (float) of data block [%d], on address [%d], slope [%f], offset [%f]", op->data_block, op->start_address, op->slope, op->offset);
				break;
		}
	}
	
	return rc;
}

void plcfw_discard ()
{
	last->operation = FW_OPERATION_DISCARDED;
}

void plcfw_reset_oplist ()
{
	last = ops;
}

plcfwop *plcfw_next ()
{
	plcfwop *rc = NULL;	
	if ((last != NULL) && (last->operation != FW_OPERATION_DISCARDED)) {
		rc = last;
		last = last->next;
	}
	return rc;
}

void plcfw_destroy_oplist ()
{
	plcfwop *m = ops;
	while (ops != NULL) {
		m = ops->next;
		free(ops);
		ops = m;
	}
}

plcfwdb* plcfw_get_db (int db)
{
	plcfwdb *m = dbs;
	while (m != NULL) {
		if (m->id == db) {
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
		m->start_address = 32768;
		m->end_address = 0;
		m->next = NULL;
	}

	return m;
}

void plcfw_compute ()
{
	plcfw_reset_oplist();
	plcfwop *m;

	int size_bytes, end_address = 0;
	plcfwdb *db;

	while ((m = plcfw_next()) != NULL) {
		size_bytes = (m->org_id == FW_ORG_DB) ? 2 * m->length : m->length;
		printf("m data_block %d: %d\n", m->data_block, size_bytes);
		db = plcfw_get_db(m->data_block);
		if (m->start_address < db->start_address) {
			db->start_address = m->start_address;
		} 
		end_address = m->start_address + size_bytes;
		if (end_address > db->end_address) {
			db->end_address = end_address;
		}
		m->chunk = db;
	}

	db = dbs;
	while (db != NULL) {
		printf("db: id = %d, start = %d, end = %d\n", db->id, db->start_address, db->end_address);
		db = db->next;
	}

	plcfw_chunking();
}

#define MAX_CHUNK 10 
plcfwdb *chs = NULL, *ch_last = NULL;
plcfwdb *plcfw_add_chunk (int id, int start_address, int end_address)
{
	plcfwdb *m;
	m = (plcfwdb *)malloc(sizeof(plcfwdb));
	m->id = id;
	m->start_address = start_address;
	m->end_address = start_address + MAX_CHUNK;
	m->next = NULL;
	m->replaced = NULL;
	
	if (ch_last == NULL) {
		ch_last = m;
		chs = m;
	} else {
		ch_last->next = m;
		ch_last = m;
	}

	return m;
}

plcfwdb *plcfw_get_chunk (plcfwdb *start, int id, int start_address, int end_address)
{
	plcfwdb *m;

	m = start;
	while (m != NULL) {
		if ((m->id == id) && 
			(((start_address > (m->start_address - MAX_CHUNK)) && (start_address < (m->end_address + MAX_CHUNK))) ||
			((end_address > (m->start_address - MAX_CHUNK)) && (end_address < (m->end_address + MAX_CHUNK))))) {
			printf("Found: %d, %d is in [%d, %d]\n", start_address, end_address, m->start_address, m->end_address);
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

void plcfw_chunking ()
{
	plcfwdb *db;

	db = dbs;
	while (db != NULL) {
		if (db->end_address - db->start_address > 2 * MAX_CHUNK) {
			plcfw_add_chunk(db->id, db->start_address, db->start_address + MAX_CHUNK);
			db->chunks = 1;
		} else {
			db->chunks = 0;
		}
		db = db->next;
	}

	printf("Chunking - round 1\n");

	plcfwdb *ch;

	ch = chs;
	while (ch != NULL) {
		printf("ch: id = %d, start = %d, end = %d\n", ch->id, ch->start_address, ch->end_address);
		ch = ch->next;
	}

	plcfwop *op;
	int size_bytes;

	op = ops;
	while (op != NULL) {
		size_bytes = (op->org_id == FW_ORG_DB) ? 2 * op->length : op->length;	
		ch = plcfw_get_chunk(chs, op->data_block, op->start_address, op->start_address + size_bytes);
		if (ch == NULL) {
			ch = plcfw_add_chunk(op->data_block, op->start_address, op->start_address + size_bytes);
		}
		op->chunk = ch;
		op = op->next;
	}

	printf("Chunking - round 2\n");

	ch = chs;
	while (ch != NULL) {
		printf("ch: this = %p, id = %d, start = %d, end = %d\n", ch, ch->id, ch->start_address, ch->end_address);
		ch = ch->next;
	}
	
	op = ops;
	while (op != NULL) {
		size_bytes = (op->org_id == FW_ORG_DB) ? 2 * op->length : op->length;
		printf("op: this = %p, db = %d, start = %d, end = %d, chunk = %p\n", op, op->data_block, op->start_address, op->start_address + size_bytes, op->chunk);
		op = op->next;
	}

	plcfwdb *inch;

	ch = chs;
	while (ch != NULL) {
		inch = plcfw_get_chunk(ch->next, ch->id, ch->start_address, ch->end_address);
		if (inch != NULL) {
			ch->replaced = inch;
		}
		ch = ch->next;
	}

	printf("Chunking - round 3\n");

	ch = chs;
	while (ch != NULL) {
		printf("ch: this = %p, id = %d, start = %d, end = %d, replaced = %p\n", ch, ch->id, ch->start_address, ch->end_address, ch->replaced);
		ch = ch->next;
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

	printf("Chunking - round 4\n");

	op = ops;
	while (op != NULL) {
		size_bytes = (op->org_id == FW_ORG_DB) ? 2 * op->length : op->length;
		printf("op: this = %p, db = %d, start = %d, end = %d, chunk = %p\n", op, op->data_block, op->start_address, op->start_address + size_bytes, op->chunk);
		op = op->next;
	}
	
}

