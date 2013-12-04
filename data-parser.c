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
json_t* get_nodes(const char* filename) {
  json_t *root = NULL;       //< root of the document tree
  json_t *json_node = NULL;  //< for general document traversal
  json_error_t json_error;   //< error indication
  const char *key = NULL;    //< key for iterations
  json_t *value = NULL;      //< value for iterations


  // do inital read of the data + some sanity checks
  root = json_load_file(filename, 0, &json_error);
  if (!root) {
    fprintf(stderr, "%s\n", json_error.text);
    return NULL;
  }
  if (!json_is_object(root)) {
    fprintf(stderr, "JSON data at root is not an object\n");
    return NULL;
  }
  fprintf(stdout, "%i elements found at the root\n", json_object_size(root));
  json_object_foreach(root, key, value) {
    fprintf(stdout, "* \"%s\"\n", key);
  }

  // check content key
  json_node = json_object_get(root, "content");
  if (!json_node) {
    fprintf(stderr, "\"content\"-key not found\n");
    return NULL;
  }
  if (!json_is_string(json_node)) {
    fprintf(stderr, "\"content\"-key not associated with a string\n");
    return NULL;
  }
  if (0 != strcmp("network state", json_string_value(json_node))) {
    fprintf(stderr, "\"content\"-key does not indicate network state\n");
    return NULL;
  }

  // get all nodes
  json_node = json_object_get(root, "nodes");
  if (!json_node) {
    fprintf(stderr, "\"nodes\"-key not found\n");
    return NULL;
  }
  if (!json_is_array(json_node)) {
    fprintf(stderr, "no array associated with \"nodes\"-key\n");
    return NULL;
  }
  fprintf(stdout, "%i network nodes found\n", json_array_size(json_node));

  return json_node;
}


/**
 * Convert a JSON network node to a SRP Network node
 * @param node from the JSON datastructure
 * @returns SRP_NetworkNode_t pointer in case of success, NULL otherwise
 * @see json_data_to_network(json_t *nodes)
 */
SRP_NetworkNode_t* jsonNode_to_SRP_NetworkNode(json_t *node){
  json_t *element_ptr = NULL;                   //< current JSON element
  json_t *neighbour_array_ptr = NULL;           //< array of neighbours of a node
  json_t *neighbour_ptr = NULL;                 //< current neighbour from array
  size_t index = 0;                             //< array index
  SRP_NetworkNode_t *srp_node = NULL;           //< current node to be constructed
  SRP_NetworkNode_t *new_neighbour = NULL;      //< new neighbour entry to be constructed
  SRP_NetworkNode_t *old_neighbour = NULL;      //< predecessor in neighbour list

  // input sanity checks
  if (NULL == node) {
    return NULL;
  }
  if (!json_is_object(node)) {
    return NULL;
  }

  // create SRP node
  srp_node = SRP_NetworkNode_create();
  if (NULL == srp_node) {
    return NULL;
  }

  // extract node ID
  element_ptr = json_object_get(node, "node id");
  if (NULL == element_ptr) {
    return NULL;
  }
  if (!json_is_number(element_ptr)){
    return NULL;
  }
  srp_node->id = json_integer_value(element_ptr);

  // extract weight
  element_ptr = json_object_get(node, "weight");
  if (NULL == element_ptr) {
    return NULL;
  }
  if (!json_is_number(element_ptr)) {
    return NULL;
  }
  srp_node->weight = json_integer_value(element_ptr);

  // extract neighbours
  element_ptr = json_object_get(node, "neighbours");
  if (NULL == element_ptr) {
    return NULL;
  }
  if (!json_is_array(element_ptr)) {
    return NULL;
  }

  if (0 == json_array_size(element_ptr)) {
    // array can be empty -> no neighbours
    return srp_node;
  }

  neighbour_array_ptr = element_ptr;
  old_neighbour = srp_node;
  json_array_foreach(neighbour_array_ptr, index, neighbour_ptr) {
    if (!json_is_object(neighbour_ptr)) {
      return NULL;
    }
    element_ptr = json_object_get(neighbour_ptr, "node id");
    if (!json_is_number(element_ptr)) {
      return NULL;
    }

    new_neighbour = SRP_NetworkNode_create();
    if (NULL == new_neighbour) {
      return NULL;
    }
    new_neighbour->id = json_integer_value(element_ptr);
    fprintf(stderr, "* added neighbour %lli\n", new_neighbour->id);

    element_ptr = json_object_get(neighbour_ptr, "weight");
    if (!json_is_number(element_ptr)) {
      return NULL;
    }
    new_neighbour->weight = json_integer_value(element_ptr);
    old_neighbour->neighbours = (struct SRP_NetworkNode_t*)new_neighbour;
    old_neighbour = new_neighbour;
  }

  return srp_node;
}


/**
 * Convert JSON-nodes data to SRP_Network
 * @param nodes to be parsed
 * @returns SRP_Network in case of success, NULL otherwise
 * @see jsonNode_to_SRP_NetworkNode(json_t *node)
 */
SRP_Network_t* json_data_to_network(json_t *nodes){
  SRP_Network_t *srp_nw_ptr = NULL;       //< pointer to last element in network data structure
  SRP_Network_t *srp_root_ptr = NULL;     //< pointer to root of network data structure
  SRP_NetworkNode_t *new_node_ptr = NULL; //< new node to be added
  size_t node_index;
  json_t *node_ptr;

  // sanity checks
  if (NULL == nodes) {
    return NULL;
  }
  if (!json_is_array(nodes)) {
    return NULL;
  }
  if (0 == json_array_size(nodes)) {
    return NULL;
  }

  // create first (dummy) element in network
  srp_nw_ptr = SRP_Network_create();
  if (NULL == srp_nw_ptr) {
    return NULL;
  }
  srp_root_ptr = srp_nw_ptr;

  // process each node in the array
  json_array_foreach(nodes, node_index, node_ptr) {
    // crude casting print without checks
    fprintf(stdout, "processing node %lli\n", 
            json_integer_value(
                               json_object_get(node_ptr,"node id")
                               )
            );
    // convert current (JSON) node
    new_node_ptr = jsonNode_to_SRP_NetworkNode(node_ptr);
    if (NULL == new_node_ptr) {
      return NULL;
    }
    // create new network list element
    srp_nw_ptr->next = (struct SRP_Network_t*)SRP_Network_create();
    if (NULL == srp_nw_ptr->next) {
      return NULL;
    }
    // move to next element
    srp_nw_ptr = (SRP_Network_t*)srp_nw_ptr->next;
    // append data
    srp_nw_ptr->data = new_node_ptr;
  }

  // remove first dummy element
  srp_nw_ptr = srp_root_ptr;
  srp_root_ptr = (SRP_Network_t*)srp_nw_ptr->next;
  free(srp_nw_ptr);

  return srp_root_ptr;
}


/**
 * Convert JSON object to routing criterion.
 * @param json points to the JSON object to be converted
 * @returns a pointer of type SRP_RoutingCriterion_t in case of success, NULL in case of error
 * @see extract_objective_functions(const char* filename)
 */
SRP_RoutingCriterion_t* json_to_routing_criterion(json_t *json) {
  json_t* token = NULL;                     //< key to be retrieved from JSON data
  char* metric_identifier = NULL;           //< metric identifier of a rule
  char* operator = NULL;                    //< operator of a rule
  char* value = NULL;                       //< value of a rule
  char* rule = NULL;                        //< final rule string
  SRP_RoutingCriterion_t* criterion = NULL; //< routing criterion to be returned

  //fprintf(stderr, "JSON object type: %i\n", json_typeof(json));

  // do sanity checks
  if (NULL == json) {
    fprintf(stderr, "given pointer is NULL\n");
    return NULL;
  }
  if (!json_is_object(json)) {
    fprintf(stderr, "no JSON object found\n");
    return NULL;
  }

  fprintf(stdout, "** processing a rule\n");

  token = json_object_get(json, "metric");
  if (!token) {
    fprintf(stderr, "\'metric\'-key not found\n");
    return NULL;
  }
  if (!json_is_string(token)) {
    fprintf(stderr, "\'metric\'-key not associated with string value\n");
    return NULL;
  }
  metric_identifier = (char*)json_string_value(token);
  if (NULL == metric_identifier) {
    fprintf(stdout, "extracting the metric identifier failed\n");
    return NULL;
  }
  

  token = json_object_get(json, "operator");
  if (!token) {
    fprintf(stderr, "\'operator\'-key not found\n");
    return NULL;
  }
  if (!json_is_string(token)) {
    fprintf(stderr, "\'operator\'-key not associated with string value\n");
    return NULL;
  }
  operator = (char*)json_string_value(token);
  if (NULL == operator) {
    fprintf(stdout, "extracting the rule operator failed\n");
    free(metric_identifier);
    return NULL;
  }

  token = json_object_get(json, "value");
  if (!token) {
    fprintf(stderr, "\'value\'-key not found\n");
    return NULL;
  }
  if (!json_is_string(token)) {
    fprintf(stderr, "\'value\'-key not associated with string value\n");
    return NULL;
  }
  value = (char*)json_string_value(token);
  if (NULL == value) {
    fprintf(stdout, "extracting the value failed\n");
    free(metric_identifier);
    free(operator);
    return NULL;
  }

  // construct the rule
  criterion = (SRP_RoutingCriterion_t*)malloc(sizeof(SRP_RoutingCriterion_t));
  if (NULL == criterion) {
    fprintf(stderr, "allocating memory for the criterion data-structure failed\n");
    free(metric_identifier);
    free(operator);
    free(value);
    return NULL;
  }

  rule = (char*)calloc(strlen(metric_identifier)
                       + strlen(operator)
                       + strlen(value)
                       + 3, sizeof(char));

  if (!strcat(rule, metric_identifier)
      || !strcat(rule, "#")) {
    fprintf(stderr, "constructing first third of the rule failed\n");
    free(metric_identifier);
    free(operator);
    free(value);
    free(rule);
    free(criterion);
    return NULL;
  }
  free(metric_identifier);

  if (!strcat(rule, operator) 
      || !strcat(rule, "#")) {
    fprintf(stderr, "constructing second third of the rule failed\n");
    free(operator);
    free(value);
    free(rule);
    free(criterion);
    return NULL;
  }
  free(operator);

  if (!strcat(rule, value)) {
    fprintf(stderr, "constructing last third of the rule failed\n");
    free(metric_identifier);
    free(value);
    free(rule);
    free(criterion);
    return NULL;
  }
  free(value);

  // plunge it into the data-structure and spit it out
  criterion->rule = rule;
  criterion->next = NULL;
  fprintf(stdout, "rule: %s\n", criterion->rule);
  return criterion;
}


/**
 * Extract the objective functions from a given dataset.
 * @param filename contains the name of the JSON file to be parsed
 * @returns The fist element of a list of extracted objective functions, NULL in case of error
 * @see json_to_routing_criterion(json_t *json)
 */
SRP_ObjectiveFunction_t* extract_objective_functions(const char* filename){
  json_t *json;              //< generic pointer to JSON objects
  json_t *of;                //< current objective function in array
  json_t *criteria;          //< JSON object holding the criteria of an objective function
  json_t *token;             //< token of a objection function
  json_t *rulejson;          //< pointer to JSON object describing a routing rule
  json_error_t json_error;   //< generic error indication
  size_t i,j = 0;            //< index in the array
  SRP_ObjectiveFunction_t *start_ptr = NULL;                  //< start of list of objective functions
  SRP_ObjectiveFunction_t *current_objective_ptr = start_ptr; //< pointer to current element in list of objective functions
  SRP_RoutingCriterion_t *current_criterion = NULL;           //< current criterion to be worked with
  SRP_RoutingCriterion_t *last_criterion = NULL;              //< the currently last criterion in the list
  char* of_id = NULL;        //< metric identifier for an objective function


  if (NULL == filename) {
    return NULL;
  }
  
  // do inital read of the data + some sanity checks
  json = json_load_file(filename, 0, &json_error);
  if (!json) {
    fprintf(stderr, "%s\n", json_error.text);
    return NULL;
  }
  if (!json_is_object(json)) {
    fprintf(stderr, "JSON data at root is not an object\n");
    return NULL;
  }

  // get all objective functions
  json = json_object_get(json, "objective functions");
  if (!json) {
    fprintf(stderr, "\"objective functions\"-key not found\n");
    return NULL;
  }
  if (!json_is_array(json)) {
    fprintf(stderr, "no array associated with \"objective functions\"-key\n");
    return NULL;
  }
  fprintf(stdout, "%i objective functions found\n", json_array_size(json));
  if (0 == json_array_size(json)) {
    return NULL;
  }

  // iterate over all objective function JSON objects in the array
  json_array_foreach(json, i, of) {
    if (!json_is_object(of)) {
      fprintf(stderr, "current objective function %i not encoded as JSON object\n", i);
      return NULL;
    }

    token = json_object_get(of, "id");
    if (!token) {
      fprintf(stderr, "no ID of objective function found\n");
      continue;
    }
    if (!json_is_number(token)) {
      fprintf(stderr, "'id'-key not associated with a number-value\n");
      continue;
    }
    of_id = (char*)calloc(21, sizeof(char)); //based on 20 chars of unsigned long long (+\n)
    if (NULL == of_id) {
      fprintf(stdout, "allocating memory for the metric identifier failed\n");
      return NULL;
    }
    sprintf(of_id, "%lli", json_integer_value(token));
    fprintf(stdout, "* processing objective-function %s\n", of_id);
    
    criteria = json_object_get(of, "criteria");
    if (!criteria) {
      fprintf(stderr, "'criteria'-key not found\n");
      free(of_id);
      return NULL;
    }
    if (!json_is_array(criteria)) {
      fprintf(stderr, "'criteria'-key not assiciated with an array\n");
      free(of_id);
      return NULL;
    }

    start_ptr = (SRP_ObjectiveFunction_t*)malloc(sizeof(SRP_ObjectiveFunction_t));
    if (NULL == start_ptr) {
      fprintf(stderr, "allocating memory for the objective function data-structure failed\n");
      free(of_id);
      return NULL;
    }
    current_objective_ptr = start_ptr;
    current_objective_ptr->id = of_id;
    current_objective_ptr->criteria = NULL;
    free(of_id);

    //@todo fill data-structures
    json_array_foreach(criteria, j, rulejson){
      current_criterion = json_to_routing_criterion(rulejson);
      if (NULL == current_criterion) {
        continue;
      }
      if (NULL == current_objective_ptr->criteria) {
        current_objective_ptr->criteria = current_criterion;
        last_criterion = current_objective_ptr->criteria;
      }
      last_criterion->next = (struct SRP_RoutingCriterion_t*)current_criterion;
      last_criterion = (SRP_RoutingCriterion_t*)last_criterion->next;
    }
  }

  return start_ptr;
}


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
SRP_Network_t* filter_network(SRP_Network_t* network, char* criteria){
  // sanity checks network
  if (NULL == network) {
    return NULL;
  }
  if (NULL == network->data) {
    return NULL;
  }
  // sanity checks criteria
  if (NULL == criteria) {
    return network;
  }
  if (0 == strlen(criteria)) {
    return network;
  }

  return NULL;
}


int main(int argc, char* argv[]){
  json_t *nodes = NULL;
  SRP_Network_t *network = NULL;

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
  extract_objective_functions(argv[1]);

  return 0;
}
