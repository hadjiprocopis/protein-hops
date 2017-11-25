/*
Program by Andreas Hadjiprocopis (ICR at the time)
contact: andreashad2@gmail.com
The program is free to distribute and use but please
attribute the orginal source and author.
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>

#include <sys/time.h>
#include <sys/resource.h>

#include <glib.h> /* for their excellent tree implementation which is similar in API to the perl Tree::Nary */

#include "graphos_tree_paths.hpp"
#include "graphos.hpp"

using namespace std;

gnode_data_t::gnode_data_t(int i) : id(i){ /*cout << "create const" << endl; */}
gnode_data_t::gnode_data_t(const gnode_data_t &g) : id(g.id){/*cout << "copy 1" << endl;*/}
gnode_data_t::gnode_data_t(const gnode_data_t *g) : id(g->id){/*cout << "copy 2" << endl;*/}
gnode_data_t::~gnode_data_t(){/* nothing at the moment printf("deleting ~gnode_data_t with id=%d\n", this->id); */ }

/*static*/ void	GraphosTreePaths::free_gnode_tree(GNode *first_node){
	std::vector<GNode *> *all_nodes = new std::vector<GNode *>();
	g_node_traverse(first_node, G_IN_ORDER, G_TRAVERSE_ALL, -1, &GraphosTreePaths::get_all_nodes, all_nodes);
	foreach(std::vector<GNode *>, a_node, *all_nodes){
		//printf("deleting: a node: %d\n", ((gnode_data_t *)((*a_node)->data))->id); fflush(stdout);
		delete ((gnode_data_t *)((*a_node)->data));
	}
	if( all_nodes->size() > 0 ){ g_node_destroy(first_node); }
	delete all_nodes;
}
GraphosTreePaths::GraphosTreePaths(gpointer data){ this->myRoot = NULL; this->reset(data); }
GraphosTreePaths::GraphosTreePaths(GNode *root){ this->myRoot = root; }
GraphosTreePaths::GraphosTreePaths(GraphosTreePaths const&G){}
GraphosTreePaths::~GraphosTreePaths(){ this->_delete_me(); }
 /* this is what the destructor (and reset) calls - all the delete code is here */
void	GraphosTreePaths::_delete_me(void){
	if( this->myRoot != NULL ){ GraphosTreePaths::free_gnode_tree(this->myRoot); }
}
void	GraphosTreePaths::reset(gpointer newdata){
	this->_delete_me();
	this->myRoot = NULL;
	if( newdata != NULL ){ this->myRoot = g_node_new(newdata); }
}
GNode	*GraphosTreePaths::root(){ return this->myRoot; }
GNode	*GraphosTreePaths::root(GNode *r){ this->myRoot = r; return this->myRoot; } /* function overloading by return type with a useless return -- it should have been void */
/*static*/  void	GraphosTreePaths::get_leaf_nodes(GNode *a_node, gpointer a_data){
	//printf("leaf: '%d'\n", ((gnode_data_t *)(a_node->data))->id);
	if( G_NODE_IS_LEAF(a_node) ){ ((std::vector<GNode *> *)a_data)->push_back(a_node); }
	else { g_node_children_foreach(a_node, G_TRAVERSE_ALL, &get_leaf_nodes, a_data); }
}
/*static*/ gboolean	GraphosTreePaths::get_all_nodes(GNode *a_node, gpointer a_data){
	((std::vector<GNode *> *)a_data)->push_back(a_node);
	return false;
}
std::vector<GNode *> *GraphosTreePaths::leaf_nodes(void){
	std::vector<GNode *> *ret = new std::vector<GNode *>();
	GraphosTreePaths::get_leaf_nodes(this->myRoot, (gpointer *)ret);
	return ret;
}
std::vector<GNode *> *GraphosTreePaths::all_nodes(void){
	std::vector<GNode *> *ret = new std::vector<GNode *>();
	GraphosTreePaths::get_all_nodes(this->myRoot, (gpointer *)ret);
	return ret;
}
