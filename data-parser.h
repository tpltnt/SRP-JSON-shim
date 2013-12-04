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

/**
 * Return JSON node "nodes" from given file
 * @param filename of file to be parsed
 * @returns JSON document node in case of success, NULL otherwise
 */
json_t* get_nodes(const char* filename);


/**
 * Convert a JSON network node to a SRP Network node
 * @param node from the JSON datastructure
 * @returns SRP_NetworkNode_t pointer in case of success, NULL otherwise
 * @see json_data_to_network(json_t *nodes)
 */
SRP_NetworkNode_t* jsonNode_to_SRP_NetworkNode(json_t *node);

/**
 * Convert JSON-nodes data to SRP_Network
 * @param nodes to be parsed
 * @returns SRP_Network in case of success, NULL otherwise
 * @see jsonNode_to_SRP_NetworkNode(json_t *node)
 */
SRP_Network_t* json_data_to_network(json_t *nodes);


/**
 * Convert JSON object to routing criterion.
 * @param json points to the JSON object to be converted
 * @returns a pointer of type SRP_RoutingCriterion_t in case of success, NULL in case of error
 * @see extract_objective_functions(const char* filename)
 */
SRP_RoutingCriterion_t* json_to_routing_criterion(json_t *json);


/**
 * Extract the objective functions from a given dataset.
 * @param filename contains the name of the JSON file to be parsed
 * @returns The fist element of a list of extracted objective functions, NULL in case of error
 * @see json_to_routing_criterion(json_t *json)
 */
SRP_ObjectiveFunction_t* extract_objective_functions(const char* filename);


/**
 * @brief Filter a given network according to a given objective function.
 * This function reduces the network to all the nodes fullfilling the conditions
 * defined in the given objective funtion. All conditions must be fullfilled by
 * a node in order to be kept in the network.
 * In case "criteria" is NULL or of length zero, the given network will be returned unfiltered.
 * The "criteria" string uses the character '#' as a delimiter. This character is therefore not
 * allowed to be contained in any value/description. (There is no check enforcing this (so far)).
 *
 * @param network to be filtered
 * @param criteria is a char* (string) indicating the objective function to be used for filtering
 * @returns a pointer to a filtered network in case of success, NULL otherwise
 */
SRP_Network_t* filter_network(SRP_Network_t* network, char* criteria);
