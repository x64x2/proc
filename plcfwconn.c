/* plcfwconn.h - library to fetch and write data from and to Siemens PLC 
 * single operations configuration and process
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "plcfwlib.h"
#include "plcfwconn.h"

extern int _verbose;
static plcfwconn *conn;

int plcfw_reconnect (); 

plcfwconn* plcfw_create_connection(char *host, int port, int tmout)
{
	conn = (plcfwconn*)malloc(sizeof(plcfwconn));

	strncpy(conn->hostname, host, MAX_HOSTNAME_LEN);
	conn->port = port;
	conn->tmout = tmout;

	return conn;
}

void plcfw_set_connection_params (int no_nodelay, int max_dtframe_size)
{
	conn->no_nodelay = no_nodelay;
	conn->max_dtframe_size = max_dtframe_size;
}

int plcfw_get_bufsize ()
{
	return (conn->max_dtframe_size < FW_MAX_DTFRAME_SIZE_DFL) ? FW_MAX_DTFRAME_SIZE_DFL : conn->max_dtframe_size;
}

int plcfw_connect ()
{
	if (conn->max_dtframe_size != -1) {
		plcfw_set_protocol_parameters(conn->max_dtframe_size);
	}

	if ((conn->sock = plcfw_open(conn->hostname, conn->port, conn->tmout, conn->no_nodelay)) <= 0) {
		fprintf(stderr, "Error: Cannot connect to hostname %s, port %d.\n", conn->hostname, conn->port);
		return -1;
	}

	conn->to_reconnect = 0;
	
	return 0;
}

void plcfw_disconnect ()
{
	plcfw_close(conn->sock);
}

int plcfw_call_fetch (int org_id, int data_block, int start_address, int length, unsigned char *buffer)
{
	int rc = 0;
	if (conn->to_reconnect == 1) {
		rc = plcfw_reconnect();
	}

	if (rc == 0) {
		rc = plcfw_fetch(conn->sock, org_id, data_block, start_address, length, buffer, conn->tmout);
		if (rc != 0) {
			fprintf(stderr, "Error: There was a problem in communication with PLC on %s, port %d.\n", conn->hostname, conn->port);
			conn->to_reconnect = 1;
		}
	}

	return rc;
}

int plcfw_reconnect () 
{
	if (_verbose) {
		printf("Last time, there was a problem with communication with PLC, now closing the socket and reconnecting.\n");
	}

	plcfw_close(conn->sock);
	sleep(1);

	int rc = 0;

	if ((conn->sock = plcfw_open(conn->hostname, conn->port, conn->tmout, conn->no_nodelay)) <= 0) {
		fprintf(stderr, "Error: Cannot connect to hostname %s, port %d.\n", conn->hostname, conn->port);	
		rc = -1;
	}
}

