/* plcfwconn.h - library to fetch and write data from and to Siemens PLC 
 * single operations configuration and process
 */

#if !defined ( __PLCFWCONN__ )
#define        __PLCFWCONN__

#define MAX_HOSTNAME_LEN 1024

typedef struct _plcfwconn {
	unsigned char hostname[MAX_HOSTNAME_LEN];
	int port;
	int tmout;
	int no_nodelay;
	int max_dtframe_size;
	int sock;
	int to_reconnect;
} plcfwconn;


plcfwconn* plcfw_create_connection (char *host, int port, int tmout);

void plcfw_set_connection_params (int no_nodelay, int max_dtframe_size);
int plcfw_connect ();
void plcfw_disconnect ();
int plcfw_call_fetch (int org_id, int data_block, int start_address, int length, unsigned char *buffer);

int plcfw_get_bufsize ();
#endif
