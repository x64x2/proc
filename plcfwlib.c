/* plcfwlib.c - library to fetch and write data from and to Siemens PLC 
 */

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>

#include "plcfwlib.h"

int _max_dtframe_size = FW_MAX_DTFRAME_SIZE_DFL;
int _verbose = FW_SILENT;

void plcfw_set_verbosity (int verbose)
{
	_verbose = verbose;
	if (_verbose) {
		printf("Verbosity level set to %d.\n", _verbose);
	}
}

void plcfw_set_protocol_parameters (int max_dtframe_size)
{
	if (max_dtframe_size > FW_MAX_DTFRAME_SIZE_DFL) {
		if (_verbose) {
			printf("Setting MAX_DTFRAME_SIZE to %d.\n", max_dtframe_size);
		}
		_max_dtframe_size = max_dtframe_size;
	} else {
		fprintf(stderr, "Error: Invalid value for MAX_DTFRAME_SIZE, should be at least bigger than %d.\n", FW_MAX_DTFRAME_SIZE_DFL);
	}
}

int plcfw_sigpipe_block () 
{  
	sigset_t intmask;	
	if ((sigemptyset(&intmask) == -1) || (sigaddset(&intmask, SIGPIPE) == -1)) {
		if (_verbose) {
			fprintf(stderr, "Error: Failed to initialize the signal mask");  
		}
		return -1;
	}	
   
	if (_verbose) {
		printf("Entering SIGPIPE BLOCK state\n"); 
	}

	if (sigprocmask(SIG_BLOCK, &intmask, NULL) == -1) {
		if (_verbose) {
			fprintf(stderr, "Error: Failed to block SIGPIPE");  
		}
		return -1;
	}

	if (_verbose) {
   		printf("SIGPIPE signal blocked\n");  
	}

	return 0;
}

int plcfw_sigpipe_unblock () 
{  
	sigset_t intmask;	
	if ((sigemptyset(&intmask) == -1) || (sigaddset(&intmask, SIGPIPE) == -1)) {
		if (_verbose) {
			fprintf(stderr, "Error: Failed to initialize the signal mask");  
		}
		return -1;
	}	
   
	if (_verbose) {
		printf("Leaving SIGPIPE BLOCK state\n"); 
	}

	if (sigprocmask(SIG_UNBLOCK, &intmask, NULL) == -1) {
		if (_verbose) {
			fprintf(stderr, "Error: Failed to block SIGPIPE");  
		}
		return -1;
	}

	if (_verbose) {
   		printf("SIGPIPE signal unblocked\n");  
	}

	return 0;
}

void plcfw_fill_header (unsigned char *header, int op_code, int org_id, int data_block, int start_address, int length)
{
	header[0] = 'S';							
	header[1] = '5';							
	header[2] = 16;								
	header[3] = 1;									
	header[4] = 3;									
	header[5] = op_code;						
	header[6] = 3;									
	header[7] = 8;									
	header[8] = org_id;								
	header[9] = data_block; 		
	header[10] = 0xFF & (start_address / 256);		
	header[11] = 0xFF & (start_address % 256);	
	header[12] = 0xFF & (length / 256);			
	header[13] = 0xFF & (length % 256);			
	header[14] = 0xFF;								
	header[15] = 2;				
}

int plcfw_connect_attempt (unsigned char *hostname, int port, int no_nodelay)
{
	struct sockaddr_in rsock;	
	struct protoent *pent;
	int sock, val = 1;

	if (_verbose) {
		printf("Creating the socket...\n");
	}

	memset ((char *)&rsock,0,sizeof(struct sockaddr));  
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
        fprintf(stderr, "Error: Unable to create socket: %s\n", strerror(errno));
        return PLCFW_SYS_ERROR;
    }

	if (no_nodelay == 0) {
		if (_verbose) {
			printf("Going to obtain protocol entry for TCP.\n");
		}

		if ((pent = getprotobyname("TCP")) == NULL)
    	{
        	fprintf(stderr, "Error: Unable to obtain protocol entry: %s\n", strerror(errno));
        	return PLCFW_SYS_ERROR;
		}

		if (_verbose) {
			printf("Configuring socket with TCP_NODELAY option.\n");
		}

		if (setsockopt(sock, pent->p_proto, TCP_NODELAY, &val, sizeof(int)) == -1)
    	{
        	fprintf(stderr, "Error: Unable to configure socket: %s\n", strerror(errno));
        	return PLCFW_SYS_ERROR;
		}
	} else {
		if (_verbose) {
			printf("No TCP_NODELAY is used, so no additional socket configuration.\n");
		}
	}	
        
	if (_verbose) {
       	printf("Socket created: %i\n", sock);
	}

	if (_verbose) {
		printf("Trying to connect to %s on port %i.\n", hostname, port);
	}

	if ((rsock.sin_addr.s_addr = inet_addr(hostname)) == INADDR_NONE) {
		if (_verbose) {
			printf("Address %s is not direct IP address, trying DNS lookup.\n", hostname);
		}
		struct hostent *hp = gethostbyname(hostname);
		if (hp == NULL) {
			fprintf(stderr, "Error: DNS lookup for %s failed.", hostname);
			return PLCFW_SYS_ERROR;
		} else {
			if (_verbose) {
				printf("Result of DNS lookup is: %s\n", inet_ntoa(*(struct in_addr*)(hp->h_addr_list[0])));
			}
			rsock.sin_addr = *((struct in_addr *)hp->h_addr_list[0]);
			if (rsock.sin_addr.s_addr == INADDR_NONE) {
				fprintf(stderr, "DNS lookup failed, result is %s.\n", hp->h_addr_list[0]);
				return PLCFW_SYS_ERROR;
			}
		}
	}

	rsock.sin_family = AF_INET;
	rsock.sin_port = htons(port);

	if (connect(sock, (struct sockaddr *)(&rsock), sizeof(struct sockaddr)) == -1) 
	{
        fprintf(stderr, "Error: Unable to connect to %s on port %i: %s\n", hostname, port, strerror(errno));
		close(sock);	
		return PLCFW_CONNECT_ERROR;
	}

	if (_verbose) {
		printf("Host %s connected on port %i\n", hostname, port);
	}

	return sock;
}

int plcfw_open (char *hostname, int port, int tmout, int no_nodelay)
{
	int sock;
	long n = (tmout > 1) ? tmout : 1;
  
	do {
		if ((sock = plcfw_connect_attempt(hostname, port, no_nodelay)) > 0) break;
        sleep(1);
	} 
	while ((--n) && ((sock == PLCFW_CONNECT_ERROR) || (sock == PLCFW_TIMEOUT_ERROR)));
        
	return sock;                                                            
}

int plcfw_send (int sock, unsigned char *buffer, size_t size)
{
	int err, sigerr, send_errno;

	if (size > MAXTSDU_SIZE) {
		fprintf(stderr, "Error: Size of buffer too big to send, size is %d, MAXTSDU size is %d.\n", size, MAXTSDU_SIZE);
		return PLCFW_SYS_ERROR;
	}

	if ((sigerr = plcfw_sigpipe_block()) != 0) {
		fprintf(stderr, "Error: Cannot continue sending packet.\n");
		return PLCFW_SYS_ERROR;
	}

	if (_verbose == FW_DUMP_PACKETS) {
		printf("Packet to send [length = %d]:\n", size);
		int i;
	  	for (i = 0; i < size; i++) {
			printf("- byte [%d]: value = 0x%02x\n", i, buffer[i]);
		}
	}	

	err = send(sock, buffer, size, 0);
	send_errno = errno;
	
	if ((sigerr = plcfw_sigpipe_unblock()) != 0) {
		fprintf(stderr, "Error: Cannot continue.\n");
		return PLCFW_SYS_ERROR;
	}
                
	if (err == -1) {
		fprintf(stderr, "Error: Can't send telegram: %s\n", strerror(send_errno));
        return ((send_errno == EPIPE) ? PLCFW_EPIPE_ERROR : PLCFW_SYS_ERROR);            
	}
       
	if (_verbose) {
		printf("Telegram sent, size: %i\n", size);
	}
                  
	return 0;
}

int plcfw_recv (int sock, unsigned char *buffer, size_t expected_size, size_t *resp_size, int tmout)
{
	int rc, err, sigerr, send_errno, n;
	unsigned char frame[_max_dtframe_size];
	unsigned char *dptr = frame;
	struct timeval tv;
	fd_set fds;

	*resp_size = 0;

	tv.tv_usec = 0;
	tv.tv_sec = tmout;

	FD_ZERO(&fds);
	FD_SET(sock, &fds);
  
	do { 
    	if (tmout > 0) {
			if (_verbose) {
				printf("Going to listen on socket...\n");
			}

			if ((err = select(sock + 1, &fds, NULL, NULL, &tv)) == -1) {
				fprintf(stderr, "Error: Select returned error value: %s\n", strerror(errno));
				return PLCFW_SYS_ERROR;
			}
                        
			err = FD_ISSET(sock, &fds);        
            if (err == -1) {
				fprintf(stderr, "Error: Unable to complete select operation, error value: %s\n", strerror(errno));
				return PLCFW_SYS_ERROR;
			}

            if (err == 0) {
				if (_verbose) {
					fprintf(stderr, "Timeout on socket, this is not necessarilly an error, but anyway, it means that we didn't receive the data.\n");
				}
				return PLCFW_TIMEOUT_ERROR;
			}
		}	

		if (_verbose) {
			printf("Going to receive response packet (or packet fragment).\n");
		}
          
        if ((sigerr = plcfw_sigpipe_block()) != 0) {
			fprintf(stderr, "Error: Cannot continue receiving packet.\n");
			return PLCFW_SYS_ERROR;
		}
       
		rc = recv(sock, dptr, _max_dtframe_size, 0);
		send_errno = errno;

		if ((sigerr = plcfw_sigpipe_unblock()) != 0) {
			fprintf(stderr, "Error: Cannot continue.\n");
			return PLCFW_SYS_ERROR;
		}
			
		if (rc == -1) {
        	fprintf(stderr, "Error: Unable to read response header: %s\n", strerror(errno));
                        
			switch (send_errno) {
				case ECONNRESET: 
					return PLCFW_DISCONNECT_ERROR;
				case EPIPE:
			  		return PLCFW_EPIPE_ERROR;
                default:
					return PLCFW_SYS_ERROR;
			}
		}

		if (_verbose == FW_DUMP_PACKETS) {
			printf("Packet received [length = %d]:\n", rc);
			int i;
	  		for (i = 0; i < rc; i++) {
				printf("- byte [%d]: value = 0x%02x\n", i, dptr[i]);
			}
		}	
		
		dptr += rc;
		*resp_size += rc;

		if (_verbose) {
			if (*resp_size == expected_size) {
				printf("Full response received.\n");
			} else {
				printf("Part of the response received, need to wait for another segment.\n");
			}
		}

		switch (frame[8]) {
			case FW_OK:
				if (_verbose) {
					printf("Response is a positive ack.\n");
				}
				break;

			case FW_NONEXISTANT_BLOCK:
				fprintf(stderr, "Error: Requested data block does not exist.\n");
				return PLCFW_FRAME_ERROR;

			case FW_BLOCK_TOO_SMALL:
				fprintf(stderr, "Error: Requested block is too small.\n");
				return PLCFW_FRAME_ERROR;

			case FW_INVALID_ORG_ID:
				fprintf(stderr, "Error: Invalid ORG ID.\n");
				return PLCFW_FRAME_ERROR;

			default:
				fprintf(stderr, "Error: Unexpected error reponse received.\n");
				return PLCFW_FRAME_ERROR;
		}
	
	} while (*resp_size < expected_size);

	memcpy(buffer, frame + PLCFW_HEADER_LEN, expected_size); 
               
    return 0;
}		

int plcfw_fetch (int sock, int org_id, int data_block, int start_address, int length, unsigned char *buffer, int tmout)
{
	int rc = 0;

	unsigned char header[PLCFW_HEADER_LEN];
	unsigned char *p;
	size_t resp_size;
	size_t size_bytes;

	size_bytes = (org_id == FW_ORG_DB) ? 2 * length : length; // in DB access mode, the length is given in words, otherwise in bytes
	if (_verbose) {
		printf("Going to allocate buffer of size %d.\n", size_bytes);
	}

	if ((p = (unsigned char*)malloc(size_bytes + PLCFW_HEADER_LEN)) == NULL) {
		fprintf(stderr, "Error: Unable to allocate buffer of size %d.\n", size_bytes);
		return PLCFW_SYS_ERROR;
	}

	if (_verbose) {
		printf("Going to fill up the F/W operation header.\n");
	}

	plcfw_fill_header(header, FW_OPERATION_FETCH, org_id, data_block, start_address, length);
	
	if (_verbose) {
		printf("Sending the request to host...");	
	}

	if ((rc = plcfw_send(sock, header, PLCFW_HEADER_LEN)) != 0) {
		fprintf(stderr, "Error: There was a problem sending the request.\n");
		return rc;
	}

	rc = plcfw_recv(sock, p, size_bytes + PLCFW_HEADER_LEN, &resp_size, tmout);
	if (rc != 0) {
		fprintf(stderr, "Error: Unable to receive response from the host.\n");
		return rc;
	}
	
    memcpy(buffer, p, size_bytes);    
	free(p);
    
	return 0; 
}

void plcfw_close(int sock)
{
	close(sock);
	if (_verbose) {
		printf("Socket %d closed.\n", sock);
	}

}



