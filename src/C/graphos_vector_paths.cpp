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

#include "graphos_vector_paths.hpp"

void	GraphosVectorPaths::reset(void){
	this->_delete_me();
	this->myPaths = new paths_t();
	this->myData = new paths_data_t();
}
GraphosVectorPaths::GraphosVectorPaths(){
//	printf("constructed\n");
	this->myPaths = NULL;
	this->myData = NULL;
	this->reset();
}
GraphosVectorPaths::GraphosVectorPaths(GraphosVectorPaths const&G){
	fprintf(stderr, "GraphosVectorPaths::GraphosVectorPaths : not implemented, why do you call me????????????\n");
	this->myPaths = NULL;
	this->myData = NULL;
	this->reset();
}
GraphosVectorPaths::~GraphosVectorPaths(){ this->_delete_me(); }
void	GraphosVectorPaths::_delete_me(void){
	/* order below is important, because once we destroy paths (vector) all is gone, it must be last */
	this->delete_data();
	this->delete_paths();
}
int	GraphosVectorPaths::get(int i, int j){ return this->myPaths->at(i)->at(j); } /* return jth item of ith path */
GraphosVectorPaths::path_data_t	*GraphosVectorPaths::get_data(int i){ return this->myData->at(i); } /* return jth item of ith path */
GraphosVectorPaths::path_t	*GraphosVectorPaths::get(int i){ return this->myPaths->at(i); } /* return the ith path */
GraphosVectorPaths::paths_t	*GraphosVectorPaths::paths(){ return this->myPaths; }
GraphosVectorPaths::path_t	*GraphosVectorPaths::new_path(void){ path_t *a_path = new GraphosVectorPaths::path_t(); this->myPaths->push_back(a_path); this->myData->push_back(new path_data_t()); return a_path; }
unsigned int	GraphosVectorPaths::num_paths(void){ return this->myPaths == NULL ? 0 : this->myPaths->size(); }
unsigned int	GraphosVectorPaths::num_items(int i){ return this->myPaths->at(i)->size(); }
void	GraphosVectorPaths::delete_paths(){
	if( this->myPaths != NULL ){
		int	i;
//		fprintf(stderr, "GraphosVectorPaths: free\n");
		for(i=this->num_paths();--i>-1;){
//			fprintf(stderr, "deleting %d path\n", i);
			delete this->get(i);			
		}
		delete this->myPaths;
//		BOOST_FOREACH(GraphosVectorPaths::path_t_p v1, *(this->myPaths)){ fprintf(stderr, "deleting a path\n"); delete v1; }
	}
}
void	GraphosVectorPaths::delete_data(){ 
	if( this->myData != NULL ){
		path_data_t	*a_data;
		int		i;
		for(i=this->num_paths();--i>-1;){
			if( (a_data=this->myData->at(i)) != NULL ){ delete a_data; }
		}
		delete this->myData;
	}
}
bool	GraphosVectorPaths::calculate_path_data(
	Graphos *g,
	path_calculator_function_t calculator,
	unsigned int path_index
){
	return calculator(g, this->myPaths->at(path_index), this->myData->at(path_index));
}
bool	GraphosVectorPaths::calculate_paths_data(
	Graphos *g,
	path_calculator_function_t calculator
){
	unsigned int	i, s = this->num_paths();
	bool	res = true;
	for(i=0;i<s;i++){
		res = res && this->calculate_path_data(g, calculator, i);
	}
	return res;
}
/* printer */
bool	GraphosVectorPaths::toString_path_data(
	Graphos *g,
	path_toString_function_t printer,
	unsigned int path_index,
	const char *ofs,
	std::string	*result
){
	return printer(g, this->myPaths->at(path_index), this->myData->at(path_index), ofs, result);
}
bool	GraphosVectorPaths::toString_paths_data(
	Graphos *g,
	path_toString_function_t printer,
	const char *ofs,
	std::string	*result
){
	unsigned int	i, s = this->num_paths();
	bool	res = true, ras;
	for(i=0;i<s;i++){
		if( (ras=this->toString_path_data(g, printer, i, ofs, result)) == true ){ result->push_back('\n'); }
		else { fprintf(stderr, "GraphosVectorPaths::toString_paths_data() : call to GraphosVectorPaths::toString_path_data() has failed\n"); }
		res = res && ras;
	}
	return res;
}
