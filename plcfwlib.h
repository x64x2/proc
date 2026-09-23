/* plcfwlib.h - library to fetch and write data from and to Siemens PLC 
 * */

#if !defined ( __PLCFWLIB__ )
#define        __PLCFWLIB__

#include <string.h>

#define PLCFW_SYS_ERROR -1
#define PLCFW_CONNECT_ERROR -2
#define PLCFW_EPIPE_ERROR -3
#define PLCFW_TIMEOUT_ERROR -4
#define PLCFW_DISCONNECT_ERROR -5
#define PLCFW_FRAME_ERROR -6

#define FW_OK 0
#define FW_NONEXISTANT_BLOCK 2
#define FW_BLOCK_TOO_SMALL 3
#define FW_INVALID_ORG_ID 6
#define FW_ORG_DB 1
#define FW_BIT_MEM 2
#define FW_PII 3
#define FW_PIQ 4

#define PLCFW_HEADER_LEN 16
#define FW_MAX_DTFRAME_SIZE_DFL 2032
#define MAXTSDU_SIZE 65524

#define FW_OPERATION_FETCH 5
#define FW_OPERATION_WRITE 3
#define FW_OPERATION_DISCARDED -1

#define FW_SILENT 0
#define FW_VERBOSE 1
#define FW_DUMP_PACKETS 2

void plcfw_set_verbosity(int verbose);

void plcfw_set_protocol_parameters(int max_dtframe_size);
int plcfw_open (char *hostname, int port, int tmout, int no_nodelay);
int plcfw_send (int sock, unsigned char *buffer, size_t size);
int plcfw_recv (int sock, unsigned char *buffer, size_t expected_size, size_t *resp_size, int tmout);
int plcfw_fetch (int sock, int org_id, int data_block, int start_address, int length, unsigned char *buffer, int tmout);
void plcfw_close(int sock);
#endif


