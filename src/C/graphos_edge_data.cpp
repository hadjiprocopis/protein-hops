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

#include "graphos_edge_data.hpp"

void	GraphosEdgeData::reset(void){
	this->_delete_me();
	this->myData = new GeneralDataStorage();
}
GraphosEdgeData::GraphosEdgeData(){
//	printf("constructed\n");
	this->myData = NULL;
	this->reset();
}
GraphosEdgeData::GraphosEdgeData(GraphosEdgeData const&G){
//	fprintf(stderr, "GraphosEdgeData::GraphosEdgeData : not implemented, why do you call me????????????\n");
	this->myData = NULL;
	this->reset();
}
GraphosEdgeData::~GraphosEdgeData(){ this->_delete_me(); }
void	GraphosEdgeData::_delete_me(void){
//	fprintf(stderr, "GraphosEdgeData: destructed\n");
	if( this->myData != NULL ){ delete this->myData; }
}
