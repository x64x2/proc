/* plcfwops.h - library to fetch and write data from and to Siemens PLC 
 * operations management module; it's purpose is to optimize access to PLC memory in case of multiple requests to the same data block etc.
 *
 */

#include "plcfwop.h"
#include <stdio.h>
#include <stdlib.h>


plcfwop* plcfw_new_operation ();

void plcfw_default (plcfwop *op);

int plcfw_validate (plcfwop *op);
void plcfw_discard ();
void plcfw_reset_oplist ();

plcfwop *plcfw_next ();

void plcfw_destroy_oplist ();
void plcfw_compute ();
