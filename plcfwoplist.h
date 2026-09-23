/* plcfwoplist.h - library to fetch and write data from and to Siemens PLC 
 * multiple operations configuration and process
 */

#if !defined ( __PLCFWOPLIST__ )
#define        __PLCFWOPLIST__
 
#include "plcfwop.h"

#define FW_OPTLEVEL_LIST 0
#define FW_OPTLEVEL_CONCAT 1
#define FW_OPTLEVEL_CHUNKS 2

typedef struct _plcfwdb {
	int start_address;
	int end_address;
	int lower_boundary;
	int upper_boundary;
	int id;
	int org_id;
	int operation;
	int chunks;
	struct _plcfwdb *next;
	struct _plcfwdb *replaced;
	unsigned char *response;
} plcfwdb;

int plcfw_conf_raw (int org_id, int db, int sa, int len, char *desc);
int plcfw_conf_bool (int db, int sa, int bn, char *desc);
int plcfw_conf_byte (int db, int sa, char *desc);
int plcfw_conf_int (int db, int sa, char *desc);
int plcfw_conf_numeric (int db, int sa, double off, double sl, char *desc);
int plcfw_conf_float (int db, int sa, double off, double sl, char *desc);

void plcfw_set_chunk_length (int len);
void plcfw_rewind_oplist ();

plcfwop *plcfw_next_op ();
int plcfw_process_single ();
int plcfw_process_list (int opt_level);

#endif
