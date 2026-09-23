/* plcfwop.h - library to fetch and write data from and to Siemens PLC 
 * single operations configuration and process
 */

#if !defined ( __PLCFWOP__ )
#define        __PLCFWOP__
#define MAX_DESCRIPTION_LEN 128

#define FW_MODE_RAW 0
#define FW_MODE_BOOL 1
#define FW_MODE_CHAR 2
#define FW_MODE_INT 3
#define FW_MODE_NUMERIC 4
#define FW_MODE_FLOAT 5

typedef struct _plcfwop {
	int operation;
	int mode;
	int org_id;
	int data_block;
	int start_address;
	int length;
	int bit_number;
	double slope;
	double offset;
	char description[MAX_DESCRIPTION_LEN];
	struct _plcfwop *next;
	struct _plcfwdb *chunk;
	int int_result;
	float float_result;
	unsigned char *raw_result;
	int size_bytes;
} plcfwop;

void plcfw_default (plcfwop *op);
int plcfw_validate (plcfwop *op);
void plcfw_process_response (plcfwop *op);

#endif
