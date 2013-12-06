/* JSON data parser for SRP
 * written by Johannes Schneemann in 2013
 *
 * This parser reads JSON data and creates the data structures
 * needed inside the SRP module of TWISNet.
 * This file is licensed under APGL(v3) or later.
 */
#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include "SRP/srp.h"
#ifndef SRPDATATYPES_H_
	#include "srp_datatypes.h"
#endif
#include "data-parser.h"


int main(int argc, char* argv[]){
  json_t *nodes = NULL;
  SRP_Network_t *network = NULL;
  SRP_ObjectiveFunction_t *of = NULL;

  // check for correct number of arguments
  if (2 != argc) {
    fprintf(stderr, "invalid number of arguments given\n");
    fprintf(stdout, "usage: %s <network data JSON file>\n", argv[0]);
  }

  //@todo sanity checks for argv[1]
  nodes = get_nodes(argv[1]);
  if (!nodes) {
    fprintf(stderr, "extracting nodes failed\n");
    return 1;
  }

  network = json_data_to_network(nodes);
  if (NULL == network) {
    return 2;
  }

  //@todo sanity checks for argv[1]
  of = extract_objective_functions(argv[1]);
  if (NULL == of) {
	  return 3;
  }

  return 0;
}
