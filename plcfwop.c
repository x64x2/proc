/* plcfwop.c - library to fetch and write data from and to Siemens PLC 
 * single operations configuration and process
 */

#include <stdio.h>
#include <stdlib.h>

#include "plcfwlib.h"
#include "plcfwop.h"

extern int _verbose;
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
	op->size_bytes = 1;
	op->int_result = -1;
	op->float_result = -10000.0;
	op->raw_result = NULL;
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

	if (rc == 0) {
		op->size_bytes = (op->org_id == FW_ORG_DB) ? 2 * op->length : op->length;
		op->raw_result = (unsigned char*)malloc(op->size_bytes);
	}
	
	return rc;
}

void plcfw_process_response (plcfwop *op)
{
	unsigned char *resp = op->raw_result;

	switch (op->mode) {
		case FW_MODE_RAW:
			break;

		case FW_MODE_BOOL:
			op->int_result = 0x01 & (resp[0] >> op->bit_number);
			if (_verbose) {
				printf("byte value [0x%02x] bit number[%d]: value %d\n", resp[0], op->bit_number, op->int_result);
			}
			break;

		case FW_MODE_CHAR:
			op->int_result = 0xFF & resp[0];
			if (_verbose) {
				printf("obtained value %d\n", op->int_result);
			}
			break;

		case FW_MODE_INT:
			op->int_result = (0xFF & (int)resp[0]) * 256 + (0xFF & (int)resp[1]);
			if (op->int_result > 32768) {
				op->int_result -= 65536;
			}
			if (_verbose) {
				printf("obtained value [%d]\n", op->int_result);
			}
			break;

		case FW_MODE_NUMERIC:
			op->int_result = (0xFF & (int)resp[0]) * 256 + (0xFF & (int)resp[1]);
			if (op->int_result > 32768) {
				op->int_result -= 65536;
			}
			op->float_result = op->offset + op->slope * (double)op->int_result;
			if (_verbose) {
				printf("obtained [%d], slope [%f], offset [%f], result [%f]\n", op->int_result, op->slope, op->offset, op->float_result);
			}
			break;
			
		case FW_MODE_FLOAT:
			op->float_result = 0.0;
			float obtained = 0.0;
			unsigned char *p = (unsigned char*)&obtained;
			p[0] = 0xFF & resp[3];
			p[1] = 0xFF & resp[2];
			p[2] = 0xFF & resp[1];
			p[3] = 0xFF & resp[0];
			op->float_result = op->offset + op->slope * (double)obtained;
			if (_verbose) {
				printf("obtained value [%f] slope [%f], offset [%f], result [%f]\n", obtained, op->slope, op->offset, op->float_result);
			}
			break;
	}
}
