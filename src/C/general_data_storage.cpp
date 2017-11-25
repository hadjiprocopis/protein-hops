/*
Program by Andreas Hadjiprocopis (ICR at the time)
contact: andreashad2@gmail.com
The program is free to distribute and use but please
attribute the orginal source and author.
*/

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstdlib>
//#include <cassert>

#include "general_data_storage.hpp"

void	GeneralDataStorage::reset(int initial_capacity){
	this->_delete_me();
	this->myData = new container_t(); this->myData->reserve(initial_capacity);
	this->myDataToFree = new container_t(); this->myDataToFree->reserve(initial_capacity); /* <<- even if it might not be all used ... */
}
void	GeneralDataStorage::reset(){
	this->_delete_me();
	this->myData = new container_t();
	this->myDataToFree = new container_t();
}
GeneralDataStorage::GeneralDataStorage(int initial_capacity){
	this->myData = NULL;
	this->myDataToFree = NULL;
	this->reset(initial_capacity);
}
GeneralDataStorage::GeneralDataStorage(){
	this->myData = NULL;
	this->myDataToFree = NULL;
	this->reset();
}
/* deletes the containers but not their contents (e.g. the pointers in there, i.e from ToFree) */
void	GeneralDataStorage::soft_delete(){
	if( this->myDataToFree != NULL ){ delete this->myDataToFree; this->myDataToFree = NULL; }
	if( this->myData != NULL ){ delete this->myData; this->myData = NULL; }
	this->_delete_me();
}	
void	GeneralDataStorage::clear(){
	/* now this basically is like calling _delete_me() and then recreating the containers, so the code below is like _delete_me()
	   if this gets complicated then ... */
	for(int i=this->myDataToFree->size();--i>-1;){ free(boost::get<char *>(*(this->myDataToFree->at(i)))); }
	for(int i=this->myData->size();--i>-1;){ delete this->myData->at(i); }
	this->myDataToFree->clear();
	this->myData->clear();
}
GeneralDataStorage::GeneralDataStorage(GeneralDataStorage const&G){ this->reset(G.size()); }
GeneralDataStorage::~GeneralDataStorage(){ this->_delete_me(); }
void	GeneralDataStorage::_delete_me(void){
	/* each value is stored in a new contents_t() this pointer is stored in Data.
	   Additionally it may be stored in DataToFree - the same thing, just to know that
	   we need to free the contents of this value which we assumed was a pointer to allocated memory.
	   so free the value inside each content in DataToFree.
	   then free all the pointers to contents_t in Data */
	if( this->myDataToFree != NULL ){
		for(int i=this->myDataToFree->size();--i>-1;){
			//printf("myDataToFree: before that it is '%p'\n",this->myDataToFree->at(i));
			//printf("I am about to delete; '%s'\n", boost::get<char *>(*(this->myDataToFree->at(i))));
			free(boost::get<char *>(*(this->myDataToFree->at(i))));
		}
		delete this->myDataToFree;
		this->myDataToFree = NULL;
	}
	if( this->myData != NULL ){
		for(int i=this->myData->size();--i>-1;){
			//printf("myData: before that it is '%p'\n",this->myData->at(i));
			delete this->myData->at(i);
		}
		delete this->myData;
		this->myData = NULL;
	}
}
/* general get, you get a variant i.e. no type basically: you need to typecast it, i.e. you need to know what it is */
GeneralDataStorage::contents_t	*GeneralDataStorage::get(int i){ return this->myData->at(i); }
GeneralDataStorage::contents_t	*GeneralDataStorage::get_last(void){ return this->myData->back(); }
GeneralDataStorage::contents_t	*GeneralDataStorage::get_first(void){ return this->myData->front(); }
/* all templated functions go the header file ! */

int	GeneralDataStorage::size(void) const{ return this->myData->size(); }

GeneralDataStorage::container_t	*GeneralDataStorage::data_to_free(void){ return this->myDataToFree; }
GeneralDataStorage::container_t	*GeneralDataStorage::data(void){ return this->myData; }

char	*GeneralDataStorage::toString(
	const char	*ofs
){
	to_string_visitor	a_vis;
	std::string	ret; /* we will cheat - the format is unknwon so we use string */
	unsigned int	s, i;
	if( (s=this->myData->size()) == 0 ){ return strdup(""); }
//printf("GeneralDataStorage::toString : the size is %d\n", s);
	s--;
	for(i=0;i<s;i++){
		boost::apply_visitor(a_vis, *(this->get(i)));
		ret += a_vis.str + ofs;
//std::cout << ret << std::endl;
	}
//printf("final:\n");
	boost::apply_visitor(a_vis, *(this->get(s)));
	ret += a_vis.str;
//std::cout << ret << std::endl;
	return strdup(ret.c_str());
}
char	*GeneralDataStorage::toString(
	const unsigned int	*indices,
	const int	num_indices,
	const char	*ofs
){
	to_string_visitor	a_vis;
	std::string	ret; /* we will cheat - the format is unknwon so we use string */
	unsigned int	s = num_indices - 1, i;
	if( this->myData->size() == 0 ){ return strdup(""); }
	for(i=0;i<s;i++){
		boost::apply_visitor(a_vis, *(this->get(indices[i])));
		ret += a_vis.str + ofs;
	}
	boost::apply_visitor(a_vis, *(this->get(indices[s])));
	ret += a_vis.str;
	return strdup(ret.c_str());
}
/* append another storage into this one */
void	GeneralDataStorage::append(GeneralDataStorage *G){
	GeneralDataStorage::container_t	*dt = G->data();
	int	s_dt = dt->size();

	if( s_dt > 0 ){ this->myData->reserve(s_dt+this->myData->size()); } else { return; /* nothing to merge */}

	GeneralDataStorage::container_t	*dtf = G->data_to_free();
	int	s_dtf = dtf->size();

	if( s_dtf > 0 ){ this->myDataToFree->reserve(s_dtf+this->myDataToFree->size()); }

	this->myData->insert(this->myData->end(), dt->begin(), dt->end());
	this->myDataToFree->insert(this->myDataToFree->end(), dtf->begin(), dtf->end());
}
