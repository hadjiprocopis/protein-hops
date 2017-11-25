/*
Program by Andreas Hadjiprocopis (ICR at the time)
contact: andreashad2@gmail.com
The program is free to distribute and use but please
attribute the orginal source and author.
*/

#include <stdio.h>
#include <iostream>
#include <cstdlib>
//#include <cassert>

#include "graphos_node_data.hpp"

void	GraphosNodeData::reset(void){
	this->_delete_me();
	this->myData = (char *)malloc(100*sizeof(char));
}
GraphosNodeData::GraphosNodeData(){
//	printf("constructed\n");
	this->myData = NULL;
	this->reset();
}
GraphosNodeData::GraphosNodeData(GraphosNodeData const&G){
//	fprintf(stderr, "GraphosNodeData::GraphosNodeData : not implemented, why do you call me????????????\n");
	this->myData = NULL;
	this->reset();
}
GraphosNodeData::~GraphosNodeData(){ this->_delete_me(); }
void	GraphosNodeData::_delete_me(void){
//	fprintf(stderr, "GraphosNodeData: destructed\n");
	if( this->myData != NULL ){ free(this->myData); }
}
