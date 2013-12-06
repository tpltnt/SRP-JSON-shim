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
  json_t *nodes = NULL;					//< nodes from JSON
  SRP_Network_t *network = NULL;		//< network data-structure in SRP-compliant format
  SRP_ObjectiveFunction_t *of = NULL;	//< objective function / routing criteria
  SRP_node_list_t *path = NULL;			//< path from start to finish
  SRP_node_list_element_t *hop = NULL;	//< path from start to finish

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

  // adjust weight according to objective function
  network = SRP_adjust_Network(network, of);
  if (NULL == network) {
	  return 4;
  }

  // find path
  path = SRP_route(network, 23, 42);
  if (NULL == path) {
	  return 5;
  }

  fprintf(stdout,"calculated route: ");
  hop = path->start;
  while (NULL != hop->next) {
	  fprintf(stdout, "%llu --> ", hop->id);
	  hop = (SRP_node_list_element_t*)hop->next;
  }
  fprintf(stdout, "%llu\n", hop->id);

  return 0;
}
