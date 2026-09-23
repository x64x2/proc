/* plcfw.c - utility to fetch and write data from and to Siemens PLC from the standard Unix/Linux command-line
 *
 * Usage:
 * plcfw 
 * 		-h, --host <PLC IP address> 
 * 		-p, --port <PLC port> 
 * 		-op, --operation <fetch/write>, default is fetch 
 * 		-m, --mode <raw/bool/numeric/float>
 * 		-i, --org-ID <orig ID number>, default is 1
 * 		-db, --data-block <DB number>
 * 		-sa, --start-address <start address>
 * 		-len, --length <data length in bytes/words>
 * 		-bn, --bit-number <bit number>
 * 		-sl, --slope <slope>, default is 1.0
 * 		-off, --offset <offset>, default is 0.0
 * 		-t, --timeout <timeout in seconds>, default is 10
 * 		-ot, --operation-timeout <timeout in seconds>, default is 10 times timeout
 * 		-y, --no-tcp-nodelay
 * 		-f, --fork
 * 		-dtmax, --max-dtframe-size <number>
 * 		-desc, --description <string>
 * 		-L, --list <string>
 * 		-opt, --opt-level <optlevel 0,1,2>
 * 		-chl, --chunk-length <number>
 *      -o, --output <string>
 * 		-v, --verbose
 * 		-D, --dump-packets
 * 		-H, --help
 */

#define MAX_FILENAME_LEN 512
#define MAX_COMMANDLINE_PARAMS 32
#define MAX_LINE_LEN 256
#define MAX_RESPONSE_STRING_LEN 1024

#define FW_OPERATION_STD 0
#define FW_OPERATION_HELP -1
#define FW_OPERATION_LIST -2

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#include "plcfwlib.h"
#include "plcfwoplist.h"
#include "plcfwconn.h"

typedef struct _fw_config {
	char hostname[MAX_HOSTNAME_LEN];
	int port;
	int tmout;
	int op_tmout;
	int fork;
	int verbose;
	int no_nodelay;
	int max_dtframe_size;
	char list_filename[MAX_FILENAME_LEN];
	char output_filename[MAX_FILENAME_LEN];
	int operation;
	int mode;
	int org_id;
	int data_block;
	int start_address;
	int length;
	int bit_number;
	int opt_level;
	int chunk_length;
	double slope;
	double offset;
	char description[MAX_DESCRIPTION_LEN];
} fw_config;

fw_config cfg;

void init_config () 
{
	memset(cfg.hostname, 0x0, MAX_HOSTNAME_LEN);
	memset(cfg.list_filename, 0x0, MAX_FILENAME_LEN);
	memset(cfg.output_filename, 0x0, MAX_FILENAME_LEN);
	cfg.port = 0;
	cfg.mode = FW_OPERATION_STD;
	cfg.tmout = 10;
	cfg.op_tmout = -1;
	cfg.fork = 0;
	cfg.verbose = FW_SILENT; 
	cfg.no_nodelay = 0; 
	cfg.max_dtframe_size = -1; 
	cfg.operation = FW_OPERATION_FETCH; 
	cfg.mode = FW_MODE_RAW;
	cfg.org_id = FW_ORG_DB; 
	cfg.data_block = -1;
	cfg.start_address = -1;
	cfg.length = -1;
	cfg.bit_number = -1;
	cfg.slope = 1.0;
	cfg.offset = 0.0; 
	cfg.opt_level = FW_OPTLEVEL_LIST;
	cfg.chunk_length = -1;
	memset(cfg.description, 0x0, MAX_DESCRIPTION_LEN);	
}

int validate_config (int mode) 
{
	int rc = 0;

	if (cfg.operation == FW_OPERATION_HELP) {
		return rc;
	}

	cfg.op_tmout = (cfg.op_tmout == -1) ? 10 * cfg.tmout : cfg.op_tmout;
	if ((cfg.fork == 1) && (cfg.verbose)) {
		printf("Going to operate in the fork mode.\n");
	}

	if ((cfg.output_filename[0] != 0) && (cfg.verbose)) {
		printf("Going to redirect output to [%s].\n", cfg.output_filename);
	} 

	if (mode == FW_OPERATION_STD) {
		if (cfg.hostname[0] == 0) {
			fprintf(stderr, "Error: Hostname - the address of the PLC - is not given. Please use '-h' or '--host' parameter.\n");
			rc = -1;
		}
	
		if (cfg.port == 0) {
			fprintf(stderr, "Error: Port - the IP port on the PLC - is not given. Please use '-p' or '--port' parameter.\n");
			rc = -1;
		}

		if (rc == 0) {
			plcfw_create_connection(cfg.hostname, cfg.port, cfg.tmout);
			plcfw_set_connection_params(cfg.no_nodelay, cfg.max_dtframe_size);
		}
	
		if (cfg.list_filename[0] != 0) {
			cfg.operation = FW_OPERATION_LIST;
			return rc;
		}
	}
	
	if (rc == 0) {
		switch (cfg.mode) {
			case FW_MODE_RAW:
				rc = plcfw_conf_raw(cfg.org_id, cfg.data_block, cfg.start_address, cfg.length, cfg.description);
				break; 			
			case FW_MODE_BOOL:
				rc = plcfw_conf_bool(cfg.data_block, cfg.start_address, cfg.bit_number, cfg.description);
				break; 							
			case FW_MODE_CHAR:
				rc = plcfw_conf_byte(cfg.data_block, cfg.start_address, cfg.description);
				break; 			
			case FW_MODE_INT:
				rc = plcfw_conf_int(cfg.data_block, cfg.start_address, cfg.description);
				break; 			
			case FW_MODE_NUMERIC:
				rc = plcfw_conf_numeric(cfg.data_block, cfg.start_address, cfg.offset, cfg.slope, cfg.description);
				break; 			
			case FW_MODE_FLOAT:
				rc = plcfw_conf_float(cfg.data_block, cfg.start_address, cfg.offset, cfg.slope, cfg.description);
				break; 			
		}
	}
	return rc;
}

int parse_line (unsigned char *line) {
	unsigned char *p = line;
	while ((*p == ' ') || (*p == '\t')) {
		p++;
	}

	if ((*p == '#') || (*p == '\n')) {
		return -2;
	}

	int operation;
	unsigned char *argv = strtok(p, " \t\n"), *saved_argv;
	if (!strcmp(argv, "FETCH")) {
		cfg.operation = FW_OPERATION_FETCH;
	} else if (!strcmp(argv, "WRITE")) {
		cfg.operation = FW_OPERATION_WRITE;
	} else {
		return -1;
	}

	argv = strtok(NULL, " \t\n");

	while (argv != NULL) {
		if ((!strcmp(argv, "-m")) || (!strcmp(argv,"--mode"))) {
			argv = strtok(NULL, " \t\n");
			if ((!strcmp(argv, "b")) || (!strcmp(argv, "B")) || (!strcmp(argv, "bool"))) {
				cfg.mode = FW_MODE_BOOL;
			} else if ((!strcmp(argv, "c")) || (!strcmp(argv, "C")) || (!strcmp(argv, "char")) || (!strcmp(argv, "byte"))) {
				cfg.mode = FW_MODE_CHAR;		
			} else if ((!strcmp(argv, "i")) || (!strcmp(argv, "I")) || (!strcmp(argv, "int"))) {
				cfg.mode = FW_MODE_INT;	
			} else if ((!strcmp(argv, "n")) || (!strcmp(argv, "N")) || (!strcmp(argv, "numeric"))) {
				cfg.mode = FW_MODE_NUMERIC;
			} else if ((!strcmp(argv, "r")) || (!strcmp(argv, "R")) || (!strcmp(argv, "raw"))) {
				cfg.mode = FW_MODE_RAW;
			} else if ((!strcmp(argv, "f")) || (!strcmp(argv, "F")) || (!strcmp(argv, "float"))) {
				cfg.mode = FW_MODE_FLOAT;
			}
		} else if ((!strcmp(argv, "-i")) || (!strcmp(argv,"--org-ID"))) {
			argv = strtok(NULL, " \t\n");
			cfg.org_id = atoi(argv);
		} else if ((!strcmp(argv, "-db")) || (!strcmp(argv,"--data-block"))) {
			argv = strtok(NULL, " \t\n");
			cfg.data_block = atoi(argv);
		} else if ((!strcmp(argv, "-sa")) || (!strcmp(argv,"--start-address"))) {
			argv = strtok(NULL, " \t\n");
			cfg.start_address = atoi(argv);
		} else if ((!strcmp(argv, "-len")) || (!strcmp(argv,"--length"))) {
			argv = strtok(NULL, " \t\n");
			cfg.length = atoi(argv);
		} else if ((!strcmp(argv, "-bn")) || (!strcmp(argv,"--bit-number"))) {
			argv = strtok(NULL, " \t\n");
			cfg.bit_number = atoi(argv);	
		} else if ((!strcmp(argv, "-sl")) || (!strcmp(argv,"--slope"))) {
			argv = strtok(NULL, " \t\n");
			cfg.slope = atof(argv);
		} else if ((!strcmp(argv, "-off")) || (!strcmp(argv,"--offset"))) {
			argv = strtok(NULL, " \t\n");
			cfg.offset = atof(argv);
		} else if ((!strcmp(argv, "-desc")) || (!strcmp(argv,"--description"))) {
			argv = strtok(NULL, " \t\n");
			if (argv[0] == '\"') {
				strncpy(cfg.description, argv + 1, MAX_DESCRIPTION_LEN);
				char *p1 = cfg.description;
				while (*p1 != '\0') ++p1; --p1;
				if (*p1 == '\"') {
					*p1 = '\0';
				} else {
					strcat(cfg.description, " ");
					argv = strtok(NULL, "\"");
					strncat(cfg.description, argv, MAX_DESCRIPTION_LEN);
				}
			} else {
				strncpy(cfg.description, argv, MAX_DESCRIPTION_LEN);
			}
		} else {
			fprintf(stderr, "Warning: Unknown parameter [%s]\n", argv);
		}
		argv = strtok(NULL, " \t\n");
	}

	return (validate_config(FW_OPERATION_LIST));
}

char* get_next_arg (int argc, int i, char **argv)
{
	if (i + 1 >= argc) {
		fprintf(stderr, "Error: Missing parameter after [%s].\n", argv[i]);
		abort();
	}
	return argv[i+1];
}

int parse_command_line (int argc, char *argv[])
{
	init_config();

	if (argc == 1) {
		cfg.operation = FW_OPERATION_HELP;
		return 0;
	}

	int i = 1;
	while (i < argc) {
		if ((!strcmp(argv[i], "-h")) || (!strcmp(argv[i],"--host"))) {
			strncpy(cfg.hostname, get_next_arg(argc, i, argv), MAX_HOSTNAME_LEN);
			i += 2;
		} else if ((!strcmp(argv[i], "-p")) || (!strcmp(argv[i],"--port"))) {
			cfg.port = atoi(get_next_arg(argc, i, argv)); 
			i += 2;
		} else if ((!strcmp(argv[i], "-op")) || (!strcmp(argv[i],"--operation"))) {
			if ((!strcmp(get_next_arg(argc, i, argv), "w")) || (!strcmp(get_next_arg(argc, i, argv), "W")) || (!strcmp(get_next_arg(argc, i, argv), "write"))) {
				cfg.operation = FW_OPERATION_WRITE;
			}
			i += 2;
		} else if ((!strcmp(argv[i], "-m")) || (!strcmp(argv[i],"--mode"))) {
			if ((!strcmp(get_next_arg(argc, i, argv), "b")) || (!strcmp(get_next_arg(argc, i, argv), "B")) || (!strcmp(get_next_arg(argc, i, argv), "bool"))) {
				cfg.mode = FW_MODE_BOOL;
			} else if ((!strcmp(get_next_arg(argc, i, argv), "c")) || (!strcmp(get_next_arg(argc, i, argv), "C")) || (!strcmp(get_next_arg(argc, i, argv), "char")) || (!strcmp(get_next_arg(argc, i, argv), "byte"))) {
				cfg.mode = FW_MODE_CHAR;	
			} else if ((!strcmp(get_next_arg(argc, i, argv), "i")) || (!strcmp(get_next_arg(argc, i, argv), "I")) || (!strcmp(get_next_arg(argc, i, argv), "int"))) {
				cfg.mode = FW_MODE_INT;	
			} else if ((!strcmp(get_next_arg(argc, i, argv), "n")) || (!strcmp(get_next_arg(argc, i, argv), "N")) || (!strcmp(get_next_arg(argc, i, argv), "numeric"))) {
				cfg.mode = FW_MODE_NUMERIC;
			} else if ((!strcmp(get_next_arg(argc, i, argv), "r")) || (!strcmp(get_next_arg(argc, i, argv), "R")) || (!strcmp(get_next_arg(argc, i, argv), "raw"))) {
				cfg.mode = FW_MODE_RAW;
			} else if ((!strcmp(get_next_arg(argc, i, argv), "f")) || (!strcmp(get_next_arg(argc, i, argv), "F")) || (!strcmp(get_next_arg(argc, i, argv), "float"))) {
				cfg.mode = FW_MODE_FLOAT;
			}
			i += 2;
		} else if ((!strcmp(argv[i], "-i")) || (!strcmp(argv[i],"--orig-ID"))) {
			cfg.org_id = atoi(get_next_arg(argc, i, argv));
			i += 2;
		} else if ((!strcmp(argv[i], "-db")) || (!strcmp(argv[i],"--data-block"))) {
			cfg.data_block = atoi(get_next_arg(argc, i, argv));
			i += 2;
		} else if ((!strcmp(argv[i], "-sa")) || (!strcmp(argv[i],"--start-address"))) {
			cfg.start_address = atoi(get_next_arg(argc, i, argv));
			i += 2;
		} else if ((!strcmp(argv[i], "-len")) || (!strcmp(argv[i],"--length"))) {
			cfg.length = atoi(get_next_arg(argc, i, argv));
			i += 2;
		} else if ((!strcmp(argv[i], "-bn")) || (!strcmp(argv[i],"--bit-number"))) {
			cfg.bit_number = atoi(get_next_arg(argc, i, argv));	
			i += 2;
		} else if ((!strcmp(argv[i], "-sl")) || (!strcmp(argv[i],"--slope"))) {
			cfg.slope = atof(get_next_arg(argc, i, argv));
			i += 2;
		} else if ((!strcmp(argv[i], "-off")) || (!strcmp(argv[i],"--offset"))) {
			cfg.offset = atof(get_next_arg(argc, i, argv));
			i += 2;
		} else if ((!strcmp(argv[i], "-t")) || (!strcmp(argv[i],"--timeout"))) {
			cfg.tmout = atoi(get_next_arg(argc, i, argv));
			i += 2;
		} else if ((!strcmp(argv[i], "-ot")) || (!strcmp(argv[i],"--operation-timeout"))) {
			cfg.op_tmout = atoi(get_next_arg(argc, i, argv));
			i += 2;
		} else if ((!strcmp(argv[i], "-dtmax")) || (!strcmp(argv[i],"--max-dtframe-size"))) {
			cfg.max_dtframe_size = atoi(get_next_arg(argc, i, argv));
			i += 2;
		} else if ((!strcmp(argv[i], "-desc")) || (!strcmp(argv[i],"--description"))) {
			strncpy(cfg.description, get_next_arg(argc, i, argv), MAX_DESCRIPTION_LEN);
			i += 2;
		} else if ((!strcmp(argv[i], "-L")) || (!strcmp(argv[i],"--list"))) {
			strncpy(cfg.list_filename, get_next_arg(argc, i, argv), MAX_FILENAME_LEN);
			i += 2;
		} else if ((!strcmp(argv[i], "-o")) || (!strcmp(argv[i],"--output-filename"))) {
			strncpy(cfg.output_filename, get_next_arg(argc, i, argv), MAX_FILENAME_LEN);
			i += 2;
		} else if ((!strcmp(argv[i], "-y")) || (!strcmp(argv[i],"--no-tcp-nodelay"))) {
			cfg.no_nodelay = 1;
			i += 1;
		} else if ((!strcmp(argv[i], "-f")) || (!strcmp(argv[i],"--fork"))) {
			cfg.fork = 1;
			i += 1;
		} else if ((!strcmp(argv[i], "-v")) || (!strcmp(argv[i],"--verbose"))) {
			cfg.verbose = FW_VERBOSE;
			i += 1;
		} else if ((!strcmp(argv[i], "-D")) || (!strcmp(argv[i],"--dump-packets"))) {
			cfg.verbose = FW_DUMP_PACKETS;
			i += 1;	
		} else if ((!strcmp(argv[i], "-H")) || (!strcmp(argv[i],"--help"))) {
			cfg.operation = FW_OPERATION_HELP;
			i += 1;
		} else if ((!strcmp(argv[i], "-opt")) || (!strcmp(argv[i],"--opt-level"))) {
			cfg.opt_level = atoi(get_next_arg(argc, i, argv));
			i += 2;
		} else if ((!strcmp(argv[i], "-chl")) || (!strcmp(argv[i],"--chunk-length"))) {
			cfg.chunk_length = atoi(get_next_arg(argc, i, argv));
			i += 2;
		} else {
			fprintf(stderr, "Warning: Unknown parameter [%s] is going to be ignored.\n", argv[i]);
			i += 1;
		}
	}

	return (validate_config(FW_OPERATION_STD));
}

int parse_list () 
{
	FILE *f;
	if ((f = fopen(cfg.list_filename, "rt")) == NULL) {
		fprintf(stderr, "Error: Cannot open file [%s] for reading.\n", cfg.list_filename);
		return -1;
	}

	unsigned char line[MAX_LINE_LEN];

	while (fgets(line, MAX_LINE_LEN, f) != NULL ) {
		parse_line(line);
	}
}

void print_usage() 
{
	fprintf(stderr, "plcfw - utility to fetch and write data from and to Siemens PLC from the standard Unix/Linux command-line\n");
	fprintf(stderr, "* Usage:\n");
	fprintf(stderr, "    plcfw <parameters>\n");
	fprintf(stderr, "* Which are:\n");
    fprintf(stderr, "    -h, --host <IP address>                - address of the remote PLC\n"); 
    fprintf(stderr, "    -p, --port <port>                      - PLC port to connect to \n"); 
    fprintf(stderr, "    -op, --operation <string>              - OP code <f,F,fetch/w,W,write>, default is fetch \n"); 
    fprintf(stderr, "    -m, --mode <string>                    - mode of data interpretation <r,R,raw/b,B,bool/c,C,char,byte/i,I,int/n,N,numeric/f,F,float>, default is raw \n");
    fprintf(stderr, "    -i, --org-ID <number>                  - ORG-ID number, default is 1 (DB mode) \n");
    fprintf(stderr, "    -db, --data-block <number>             - data block number \n");
    fprintf(stderr, "    -sa, --start-address <number>          - start address \n");
    fprintf(stderr, "    -len, --data length <number>           - length of the data written or fetched, in bytes or words \n");
    fprintf(stderr, "    -bn, --bit-number <number>             - for boolean, number of querried bit \n");
    fprintf(stderr, "    -sl, --slope <float number>            - slope for numeric value, default is 1.0 \n");
    fprintf(stderr, "    -off, --offset <float number>          - offset for numeric value, default is 0.0 \n");
    fprintf(stderr, "    -t, --timeout <number>                 - timeout for both connection and data transfer (in seconds), default is 10 \n");
	fprintf(stderr, "    -ot, --operation-timeout <number>      - timeout fot the whole operation (in seconds), default is 10 times timeout \n");
	fprintf(stderr, "    -y, --no-tcp-nodelay                   - do not use TCP_NODELAY for the socket communication \n");
	fprintf(stderr, "    -f, --fork                             - fork extra process for communication with PLC, this is useful in case when in danger of getting stuck; plcfw is able to recover from this state after a timeout \n");
	fprintf(stderr, "    -desc, --description <string>          - verbose description (preferably in commas) \n");
	fprintf(stderr, "    -L, --list <string>                    - reads list of operations from a given file \n");
	fprintf(stderr, "    -opt, --opt-level <0,1,2>              - optimization level for list access; 0 - no optimization, 1 - one request per data block, 2 - chunks \n");
	fprintf(stderr, "    -chl, --chunk-length <number>			- length of chunk - for optimization level 2 \n");
	fprintf(stderr, "    -o, --output-filename <string>         - file where data output is to be redirected \n");
    fprintf(stderr, "    -v, --verbose                          - enables verbosity \n");
    fprintf(stderr, "    -D, --dump-packets                     - enables dumping of all packages sent or received \n");
    fprintf(stderr, "    -H, --help                             - prints this help message \n");
}

void process_fetch_response (plcfwop *op) 
{
	if (cfg.verbose) {
		printf("Response received:\n");
	}
	char response[MAX_RESPONSE_STRING_LEN];

	switch (op->mode) {
		case FW_MODE_RAW:
			sprintf(response, "%s = ", op->description);
			int i;
			for (i = 0; i < (2 * cfg.length); i++) {
				sprintf(response, "0x%02x ", op->raw_result[i]);
			}
			break;

		case FW_MODE_BOOL:
			sprintf(response, "%s = %d", op->description, op->int_result);
			break;

		case FW_MODE_CHAR:
			sprintf(response, "%s = %d", op->description, op->int_result);
			break;

		case FW_MODE_INT:
			sprintf(response, "%s = %d", op->description, op->int_result);
			break;

		case FW_MODE_NUMERIC:
			sprintf(response, "%s = %f", op->description, op->float_result);
			break;
			
		case FW_MODE_FLOAT:
			sprintf(response, "%s = %f", op->description, op->float_result);
			break;
	}

	if (cfg.output_filename[0] != 0) {
		FILE *f;
		if ((f = fopen(cfg.output_filename, "wt")) == NULL) {
			fprintf(stderr, "Error: Cannot open output file [%s]. Check permissions.\n", cfg.output_filename);
		}
		fprintf(f, "%s\n", response);
		fclose(f);
	} else {
		printf("%s\n", response);
	}
}


int do_operation () 
{
	int rc = 0;
	if (cfg.list_filename[0] != 0) {
		rc = plcfw_process_list(cfg.opt_level);
	} else {
		rc = plcfw_process_single();
	}

	plcfwop *op;
	plcfw_rewind_oplist();

	while ((op = plcfw_next_op()) != NULL) {
		switch (op->operation) {
			case FW_OPERATION_FETCH:
				process_fetch_response(op);
				break;
			case FW_OPERATION_WRITE:
				break;
		}
	}

	return rc;
}

int fork_and_do_operation ()
{
	int rc = 0;

	sigset_t mask;
	sigset_t orig_mask;
 
	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
 
	if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
		fprintf(stderr, "Error: Cannot block SIGCHLD. Cannot continue.\n");
		return -1;
	}

	pid_t childpid = fork();
	if (childpid == (pid_t)-1) {
		fprintf(stderr, "Error: Cannot fork child process. Cannot continue.\n");
		return -1;
	}

	if (childpid == (pid_t)0) {
		if (cfg.verbose) {
			printf("<0> Reported by child process: Forked. \n");
		}
		rc = do_operation();
		
		if (cfg.verbose) {	
			printf("<0> Reported by child process: Operation call completed. \n");
		}
		return rc;
	} else {
		if (cfg.verbose) {
			printf("<%d> Reported by parrent process: Child forked. \n", (int)childpid);
		}

		struct timespec timeout;
		timeout.tv_sec = cfg.op_tmout;
		timeout.tv_nsec = 0;

		int errno_sig;

		do {
			if (sigtimedwait(&mask, NULL, &timeout) < 0) {
				errno_sig = errno;
				switch (errno_sig) {
					case EINTR:
						continue;
					case EAGAIN:
						fprintf(stderr, "Error: Timeout elapsed waiting for the child process to terminate. \n");
						kill(childpid, SIGKILL);
						break;
					default:
						fprintf(stderr, "Error: Unexpected error while waiting for the child process, errno = %d. \n", errno_sig);
						return -1;
				}
			}
			break;
		} while (1);
 
		if (waitpid(childpid, &rc, 0) < 0) {
			errno_sig = errno;
			fprintf(stderr, "Error: Unexpected error while waiting for the child process, errno = %d. \n", errno_sig);
			return -1;
		}

		if (cfg.verbose) {
			printf("<0> Reported by parent process: Return code from child is %d.\n", rc);
		}
	}

	return rc;
}

int main (int argc, char *argv[])
{
	if (parse_command_line(argc, argv) != 0) {
		exit(-1);
	}

	if (cfg.operation == FW_OPERATION_HELP) {
		print_usage();
		exit(0);
	}

	plcfw_set_verbosity(cfg.verbose);

	int rc = 0;

	if (cfg.list_filename[0] != 0) {
		rc = parse_list();
	}

	if (rc == 0) {
		if (cfg.fork == 1) {
			rc = fork_and_do_operation();
		} else {
			rc = do_operation();
		}
	}

	return rc;
}


