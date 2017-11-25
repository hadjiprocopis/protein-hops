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
#include <string>
#include <vector>
#include <cassert>

#include <sys/time.h>
#include <sys/resource.h>

#include <glib.h> /* for their excellent tree implementation which is similar in API to the perl Tree::Nary */

#include "graphos.hpp"

/* a word about inline functions:
 Can speed up execution BUT they need to be defined in every cpp file using them...
 So in order to avoid a lot of hard work, only inline private functions for the time being
*/

/* using this macro does not make a difference, probably is faster than using a function */
#define	ADD_NODE(node_name, node_id)	this->vertex_names_2_ids->insert(std::make_pair((node_name),(node_id))); this->vertex_ids_2_names->insert(std::make_pair((node_id),(node_name)))

/* using these macros instead of the corresponding inline functions is almost 3% SLOWER ... see local variables */
#define	ADD_EDGE_FORWARD(from_id, to_id) {\
	unordered_map_third_level_t_p	_ADD_EDGE_FORWARD_a_third_level_map;\
	unordered_map_second_level_t::iterator  _ADD_EDGE_FORWARD_a_found_p_i;\
	if( (_ADD_EDGE_FORWARD_a_found_p_i=this->forward_search_edgelist->find((from_id))) == this->forward_search_edgelist->end() ){\
		_ADD_EDGE_FORWARD_a_third_level_map = new unordered_map_third_level_t();\
		this->forward_search_edgelist->insert(std::make_pair((from_id), _ADD_EDGE_FORWARD_a_third_level_map));\
	} else { _ADD_EDGE_FORWARD_a_third_level_map = _ADD_EDGE_FORWARD_a_found_p_i->second; }\
	_ADD_EDGE_FORWARD_a_third_level_map->insert(std::make_pair((to_id),1));\
}
#define	ADD_EDGE_BACKWARD(from_id,to_id) {\
	unordered_map_third_level_t_p	_ADD_EDGE_BACKWARD_a_third_level_map;\
	unordered_map_second_level_t::iterator  _ADD_EDGE_BACKWARD_a_found_p_i;\
	if( (_ADD_EDGE_BACKWARD_a_found_p_i=this->backward_search_edgelist->find((from_id))) == this->backward_search_edgelist->end() ){\
		_ADD_EDGE_BACKWARD_a_third_level_map = new unordered_map_third_level_t();\
		this->backward_search_edgelist->insert(std::make_pair((from_id), _ADD_EDGE_BACKWARD_a_third_level_map));\
	} else { _ADD_EDGE_BACKWARD_a_third_level_map = _ADD_EDGE_BACKWARD_a_found_p_i->second; }\
	_ADD_EDGE_BACKWARD_a_third_level_map->insert(std::make_pair((to_id),1));\
}
#define	DELETE_NODE_BY_NAME(node_name)	{\
	vertex_names_2_ids_t::iterator	a_found;\
	if( (a_found=this->vertex_names_2_ids->find(node_name)) == this->vertex_names_2_ids->end() ){ return; /* not found */ }\
	this->vertex_ids_2_names->erase(a_found->second);\
	this->vertex_names_2_ids->erase(a_found);\
}
#define	DELETE_NODE_BY_ID(node_id)	{\
	vertex_ids_2_names_t::iterator	a_found;\
	if( (a_found=this->vertex_ids_2_names->find(node_id)) == this->vertex_ids_2_names->end() ){ return; /* not found */ }\
	this->vertex_names_2_ids->erase(a_found->second);\
	this->vertex_ids_2_names->erase(a_found);\
}

#if 0
/* int: this is what we store in these maps */
/*static*/	void Graphos::delete_unordered_map_final_contents(int m){}
/*static*/	void Graphos::delete_unordered_map_third_level_t(Graphos::unordered_map_third_level_t_p m){GraphosUnorderedMaps<int>::delete_unordered_map_third_level_t(m);}
/*static*/	void Graphos::delete_unordered_map_second_level_t(Graphos::unordered_map_second_level_t_p m){ GraphosUnorderedMaps<int>::delete_unordered_map_second_level_t(m); }
/*static*/	void Graphos::delete_unordered_map_first_level_t(Graphos::unordered_map_first_level_t_p m){GraphosUnorderedMaps<int>::delete_unordered_map_first_level_t(m);}
/*static*/	void Graphos::delete_nodelist_with_hops_t(nodelist_with_hops_t *m){ Graphos::delete_unordered_map_second_level_t(m); }
/*static*/	void Graphos::delete_nodelist_t(nodelist_t *m){ Graphos::delete_unordered_map_third_level_t(m); }
/*static*/	void Graphos::delete_edgelist_with_hops_t(edgelist_with_hops_t *m){ Graphos::delete_unordered_map_first_level_t(m); }
/*static*/	void Graphos::delete_edgelist_t(edgelist_t *m){ Graphos::delete_unordered_map_second_level_t(m); }
/* there is no need to delete the keys (char*) of this because they are pointers to unordered_map_second_level_t which will be deleted then */ 
/*static*/	void Graphos::delete_vertex_ids_2_names_t(Graphos::vertex_ids_2_names_t *m){ delete m; }
/*static*/	void Graphos::delete_vertex_names_2_ids_t(Graphos::vertex_names_2_ids_t *m){ GraphosUnorderedMaps::delete_unordered_map_third_level_char_keys_t(m); }
/*static*/	void Graphos::_deleter_function_for_node_data(Graphos::graphos_node_storage_t *m){ printf("deleter function is called\n"); delete(m); }
/*static*/	void Graphos::_deleter_function_for_edge_data(Graphos::graphos_edge_storage_t *m){ delete(m); }
/*static*/	void Graphos::delete_node_data_t(Graphos::node_data_t *m){ GraphosUnorderedMaps<Graphos::graphos_node_storage_t *>::delete_unordered_map_third_level_t(m, &(Graphos::_deleter_function_for_node_data)); }
/*static*/	void Graphos::delete_forward_search_edge_data_t(Graphos::edge_data_t *m){ GraphosUnorderedMaps<Graphos::graphos_edge_storage_t *>::delete_unordered_map_second_level_t(m, &(Graphos::_deleter_function_for_edge_data)); }
/*static*/	void Graphos::delete_backward_search_edge_data_t(Graphos::edge_data_t *m){ GraphosUnorderedMaps<Graphos::graphos_edge_storage_t *>::delete_unordered_map_second_level_t(m); }
#endif

/* int: this is what we store in these maps */
/*static*/      void Graphos::delete_unordered_map_final_contents(int m){}
/*static*/      void Graphos::delete_unordered_map_third_level_t(Graphos::unordered_map_third_level_t_p m){GraphosUnorderedMaps<int>::delete_unordered_map_third_level_t(m);}
/*static*/      void Graphos::delete_unordered_map_second_level_t(Graphos::unordered_map_second_level_t_p m){ GraphosUnorderedMaps<int>::delete_unordered_map_second_level_t(m); }
/*static*/      void Graphos::delete_unordered_map_first_level_t(Graphos::unordered_map_first_level_t_p m){GraphosUnorderedMaps<int>::delete_unordered_map_first_level_t(m);}
/*static*/      void Graphos::delete_nodelist_with_hops_t(nodelist_with_hops_t *m){ Graphos::delete_unordered_map_second_level_t(m); }
/*static*/      void Graphos::delete_nodelist_t(nodelist_t *m){ Graphos::delete_unordered_map_third_level_t(m); }
/*static*/      void Graphos::delete_edgelist_with_hops_t(edgelist_with_hops_t *m){ Graphos::delete_unordered_map_first_level_t(m); }
/*static*/      void Graphos::delete_edgelist_t(edgelist_t *m){ Graphos::delete_unordered_map_second_level_t(m); }
/* there is no need to delete the keys (char*) of this because they are pointers to unordered_map_second_level_t which will be deleted then */
/*static*/      void Graphos::delete_vertex_ids_2_names_t(Graphos::vertex_ids_2_names_t *m){ delete m; }
/*static*/      void Graphos::delete_vertex_names_2_ids_t(Graphos::vertex_names_2_ids_t *m){ delete m; }
/*static*/      void Graphos::delete_node_data_t(Graphos::node_data_t *m){ GraphosUnorderedMaps<Graphos::graphos_node_storage_t *>::delete_unordered_map_third_level_t(m, &(Graphos::_deleter_function_for_node_data)); }
/*static*/      void Graphos::delete_forward_search_edge_data_t(Graphos::edge_data_t *m){ GraphosUnorderedMaps<Graphos::graphos_edge_storage_t *>::delete_unordered_map_second_level_t(m, &(Graphos::_deleter_function_for_edge_data)); }
/*static*/      void Graphos::delete_backward_search_edge_data_t(Graphos::edge_data_t *m){ GraphosUnorderedMaps<Graphos::graphos_edge_storage_t *>::delete_unordered_map_second_level_t(m); }

/* these functions are passed to some delete_ functions to delete the data structures held in the unordered containers */
/* the deleter functions are responsible to delete everything including m */
/*static*/      void Graphos::_deleter_function_for_node_data(Graphos::graphos_node_storage_t *m){ delete(m); }
/*static*/      void Graphos::_deleter_function_for_edge_data(Graphos::graphos_edge_storage_t *m){ delete(m); }


/*static*/	char	*Graphos::read_file_in_memory(const char *filename){
	int	filedes;
	if( (filedes=open(filename, O_RDONLY)) < 0 ){ fprintf(stderr, "read_file_in_memory : could not open input file '%s' for reading.\n", filename); perror(NULL); return (char *)NULL; }
	off_t	fsize = lseek(filedes, 0L, SEEK_END); lseek(filedes, 0L, SEEK_SET); /* rewind */
	char	*buffer;
	if( (buffer=(char *)malloc((fsize+1)*sizeof(char))) == NULL ){ fprintf(stderr, "read_file_in_memory : could not allocate %lld bytes for buffer.\n", fsize+1); perror(NULL); close(filedes); return (char *)NULL; }
	ssize_t	bytesread;
	if( (bytesread=read(filedes, buffer, fsize)) != fsize ){ fprintf(stderr, "read_file_in_memory : failed to read %lld bytes (whole file contents) from file '%s'. Only read %zu bytes.\n", fsize, filename, bytesread); perror(NULL); close(filedes); free(buffer); return (char *)NULL; }
	close(filedes);
	buffer[fsize] = '\0'; /* append a NULL at the end - so size if size+1 */
	fprintf(stdout, "read_file_in_memory : read %lld bytes from file '%s' in memory.\n", fsize, filename);
	return(buffer);
}
/*static*/	bool	Graphos::is_edgelist_transitive(edgelist_t *an_edgelist){
	unordered_map_third_level_t_p   sl;
	unordered_map_second_level_t::iterator  um_it;
	for(edgelist_t::iterator iter=an_edgelist->begin();(iter!=an_edgelist->end());++iter){
		sl = iter->second;
		for(unordered_map_third_level_t::iterator iter2=sl->begin();iter2!=sl->end();++iter2){
			if( ((um_it=an_edgelist->find(iter2->first)) == an_edgelist->end()) || (um_it->second->find(iter->first) == um_it->second->end()) ){ return false; }
		}
	}
	return true;
}
/*static*/	int	Graphos::IsEmptyString(char *str){
	int     i = 0;
	while( (str[i]!='\n') && (str[i]!='\0') ) if( str[i++] != ' ' ) return(FALSE); return(TRUE);
}
/* returns -1 on failure, or the number of edges it wrote (including zero, in which case no output file will be written) */
int	Graphos::write_edgelist_to_file(
	FILE *outfilehandle,
	const char *ofs
){
	return	Graphos::write_edgelist_to_file(outfilehandle, ofs, this->vertex_ids_2_names, this->forward_search_edgelist);
}
/* returns -1 on failure, or the number of edges it wrote (including zero, in which case no output file will be written) */
int	Graphos::write_edgelist_to_file(
	const char	*outfilename,
	const char *ofs
){
	return	Graphos::write_edgelist_to_file(outfilename, ofs, this->vertex_ids_2_names, this->forward_search_edgelist);
}
/* returns -1 on failure, or the number of nodes it wrote (including zero, in which case no output file will be written) */
int	Graphos::write_unique_nodes_to_file(
	FILE *outfilehandle
){
	int	num_nodes = 0;
	for(vertex_names_2_ids_t::iterator iter=this->vertex_names_2_ids->begin();iter!=this->vertex_names_2_ids->end();++iter){
		fprintf(outfilehandle, "%s\n", iter->first);
		num_nodes++;
	}
	return num_nodes;
}
/* returns -1 on failure, or the number of nodes it wrote (including zero, in which case no output file will be written) */
int	Graphos::write_unique_nodes_to_file(
	const char	*outfilename
){
	FILE	*outfilehandle;
	if( (outfilehandle=fopen(outfilename, "w")) == NULL ){ fprintf(stderr, "write_edgelist_to_file : could not open output file '%s' for writing results.\n", outfilename); perror(NULL); return(-1); }
	int	num_nodes = this->write_unique_nodes_to_file(outfilehandle);
	fclose(outfilehandle);
	//if( num_nodes == 0 ){ unlink(outfilename); }
	return(num_nodes);
}
/* returns -1 on failure, or the number of nodes it wrote (including zero, in which case no output file will be written) */
/*static*/	int	Graphos::write_nodelist_to_file(
	FILE *outfilehandle,
	Graphos::vertex_ids_2_names_t *vertex_ids_2_names,
	Graphos::nodelist_t *the_nodelist
){
	int	num_nodes = 0;
	for(nodelist_t::iterator iter=the_nodelist->begin();iter!=the_nodelist->end();++iter){
		fprintf(outfilehandle, "%s\n", (*vertex_ids_2_names)[iter->first]);
		num_nodes++;
	}
	return num_nodes;
}
/* returns -1 on failure, or the number of nodes it wrote (including zero, in which case no output file will be written) */
/*static*/	int	Graphos::write_nodelist_to_file(
	const char *outfilename,
	Graphos::vertex_ids_2_names_t *vertex_ids_2_names,
	Graphos::nodelist_t *the_nodelist
){
	FILE	*outfilehandle;
	if( (outfilehandle=fopen(outfilename, "w")) == NULL ){ fprintf(stderr, "write_edgelist_to_file : could not open output file '%s' for writing results.\n", outfilename); perror(NULL); return(-1); }
	int	num_nodes = Graphos::write_nodelist_to_file(outfilehandle, vertex_ids_2_names, the_nodelist);
	fclose(outfilehandle);
	//if( num_nodes == 0 ){ unlink(outfilename); }
	return(num_nodes);
}
/* returns -1 on failure, or the number of edges it wrote (including zero, in which case no output file will be written) */
/*static*/	int	Graphos::write_edgelist_to_file(
	FILE *outfilehandle,
	const char *ofs,
	Graphos::vertex_ids_2_names_t *vertex_ids_2_names,
	Graphos::edgelist_t *the_edgelist
){
	int	num_edges = 0;
	for(unordered_map_second_level_t::iterator iter=the_edgelist->begin();iter!=the_edgelist->end();++iter){
		for(unordered_map_third_level_t::iterator iter2=(iter->second)->begin();iter2!=(iter->second)->end();++iter2){
			fprintf(outfilehandle, "%s%s%s\n", (*vertex_ids_2_names)[iter->first], ofs, (*vertex_ids_2_names)[iter2->first]);
			num_edges++;
		}
	}
	//if( num_edges == 0 ){ unlink(outfilename); }
	return(num_edges);
}
/* returns -1 on failure, or the number of edges it wrote (including zero, in which case no output file will be written) */
/*static*/	int	Graphos::write_edgelist_to_file(
	const char *outfilename,
	const char *ofs,
	Graphos::vertex_ids_2_names_t *vertex_ids_2_names,
	Graphos::edgelist_t *the_edgelist
){
	FILE	*outfilehandle;
	if( (outfilehandle=fopen(outfilename, "w")) == NULL ){ fprintf(stderr, "write_edgelist_to_file : could not open output file '%s' for writing results.\n", outfilename); perror(NULL); return(-1); }
	int	num_edges = Graphos::write_edgelist_to_file(outfilehandle, ofs, vertex_ids_2_names, the_edgelist);
	fclose(outfilehandle);
	//if( num_edges == 0 ){ unlink(outfilename); }
	return(num_edges);
}

/* constructors here */
/* create one for reading only transitive or only non-transitive data */
Graphos::Graphos(bool make_transitive){
	this->vertex_names_2_ids = NULL;
	this->vertex_ids_2_names = NULL;
	this->forward_search_edge_data = NULL;
	this->backward_search_edge_data = NULL;
	this->node_data = NULL;
	this->forward_search_edgelist = this->backward_search_edgelist = NULL;
	this->myMakeTransitive = make_transitive;
	this->reset();
#if TEST_DELETE
	GeneralDataStorage	*xx = new GeneralDataStorage();
	xx->put(12.0);
	xx->put(54.56);
	xx->put_and_remember_to_delete(strdup("fuck and fgufuf"));
	xx->put(13);
	xx->put(154.56);
	xx->put_and_remember_to_delete(strdup("2222fuck and fgufuf2222"));
//	printf("getting %lf\n", xx->get_double(0));
	delete xx;

	int	myColumns[] = {0,1,2,3};
//	int	myColumns[4]; myColumns[0] = 0; myColumns[1] = 1; myColumns[2] = 2; myColumns[3] = 3;
	GeneralDataReader::data_type_t	myDataTypes[] = {GeneralDataReader::STRING, GeneralDataReader::DOUBLE, GeneralDataReader::DOUBLE, GeneralDataReader::STRING};
	char	mySeparator[] = "\t";
	GeneralDataReader	*gdr = new GeneralDataReader(
		4,
		myColumns,
		myDataTypes,
		(char *[] ){"NA", "-999.9", "-999.9", "NA"},
		mySeparator
	);
	char	myLineOfText[] = "abc\t\t35.5\tahahah\t1555\0";
	gdr->parse(myLineOfText);
	GeneralDataStorage *stor = gdr->storage(); gdr->make_new_storage();
	fprintf(stderr, "0=%s, 1=%lf, 2=%lf, 3=%s\n", stor->get_as<char *>(0), stor->get_as<double>(1), stor->get_as<double>(2), stor->get_as<char *>(3));

	char	myLineOfText2[] = "fuck\t76\t23.3\toffff\t82828\0";
	gdr->parse(myLineOfText2);
	GeneralDataStorage *stor2 = gdr->storage(); gdr->make_new_storage();
	fprintf(stderr, "0=%s, 1=%lf, 2=%lf, 3=%s\n", stor->get_as<char *>(0), stor->get_as<double>(1), stor->get_as<double>(2), stor->get_as<char *>(3));
	delete stor;
	delete stor2;
	delete gdr;
#endif
}
void	Graphos::_delete_me(){
	if( this->forward_search_edgelist != this->backward_search_edgelist ){
		if( this->backward_search_edgelist != NULL ){ Graphos::delete_unordered_map_second_level_t(this->backward_search_edgelist); delete this->backward_search_edgelist; }
	}
	if( this->forward_search_edgelist != NULL ){ Graphos::delete_unordered_map_second_level_t(this->forward_search_edgelist); delete this->forward_search_edgelist; }
	if( this->vertex_names_2_ids != NULL ){ foreach(vertex_names_2_ids_t, a_node_name, *(this->vertex_names_2_ids)){ free(a_node_name->first); } delete this->vertex_names_2_ids; }
	if( this->vertex_ids_2_names != NULL ){ delete this->vertex_ids_2_names; }
	if( this->forward_search_edge_data != NULL ){
		Graphos::delete_forward_search_edge_data_t(this->forward_search_edge_data); delete this->forward_search_edge_data;
		if( (this->forward_search_edge_data != this->backward_search_edge_data) && (this->backward_search_edge_data != NULL) ){ Graphos::delete_backward_search_edge_data_t(this->backward_search_edge_data); delete this->backward_search_edge_data; }
	}
	if( this->node_data != NULL ){ Graphos::delete_node_data_t(this->node_data); delete this->node_data; }
}
Graphos::~Graphos(){ this->_delete_me(); }
Graphos::Graphos(Graphos const&g){
	this->vertex_names_2_ids = NULL;
	this->vertex_ids_2_names = NULL;
	this->forward_search_edge_data = NULL;
	this->backward_search_edge_data = NULL;
	this->node_data = NULL;
	this->forward_search_edgelist = this->backward_search_edgelist = NULL;
	this->myMakeTransitive = g.myMakeTransitive;
	this->reset();
}
Graphos::Graphos(Graphos *g){
	this->vertex_names_2_ids = NULL;
	this->vertex_ids_2_names = NULL;
	this->forward_search_edge_data = NULL;
	this->backward_search_edge_data = NULL;
	this->node_data = NULL;
	this->forward_search_edgelist = this->backward_search_edgelist = NULL;
	this->myMakeTransitive = g->myMakeTransitive;
	this->reset();
}
void	Graphos::reset(){
	this->_delete_me();
	this->myPreviousFileWasTransitive = -1;
	this->vertex_names_2_ids = new vertex_names_2_ids_t();
	this->vertex_ids_2_names = new vertex_ids_2_names_t();

	this->forward_search_edgelist = new unordered_map_second_level_t();
	if( this->backward_search_edgelist != NULL ){ printf("it is not null %p abd deletn:\n", this->backward_search_edgelist); delete this->backward_search_edgelist; }
	this->backward_search_edgelist = NULL;
	if( this->node_data == NULL ){ this->node_data = new node_data_t(); }
	if( this->forward_search_edge_data == NULL ){ this->forward_search_edge_data = new edge_data_t(); }

	if( this->forward_search_edgelist != this->backward_search_edgelist ){ this->backward_search_edge_data = new edge_data_t(); } else { this->backward_search_edge_data = this->forward_search_edge_data; }

}
/* no need to allocate any memory for the two lists parents (A->{B,C}) and children (B->{A},C->{A}) but it may be that the children is
   pointing to the parents, so in either case you need to free parents and if parents != children then free children too, as thus:
	if( forward_search_edgelist != backward_search_edgelist ) free(backward_search_edgelist); free(forward_search_edgelist);
 */
int	Graphos::read_edgelist_from_file(
	const char	*filename,
	int	num_first_lines_skip, /* as header if any otherwise set to zero */
	int	from_node_name_column_number,
	int	to_node_name_column_number,
	const char	*separator /* a string delimiter: NOT each char is a separate delimiter, the whole string is the delimiter */
){
	std::vector<std::string> strs;
	int	i, strlen_separator = strlen(separator);
	unordered_map_third_level_t_p	sl;
	char	*pReadFrom, *pReadTo, *filecontents, c, *savePtr, *a_token, *from, *to, *a_str;
	vertex_names_2_ids_t::iterator	a_found_n2i;
	int	last_id = 0, from_id, to_id, col_num;
	unordered_map_second_level_t::iterator	a_found_c_i, a_found_p_i;

	if( (filecontents=Graphos::read_file_in_memory(filename)) == NULL ){ fprintf(stderr, "read_edgelist_from_file : call to read_file_in_memory has failed for input file '%s'.\n", filename); return(-1); }
	int	linenum=0, num_edges = 0;
	pReadTo = pReadFrom = &(filecontents[0]);
	for(i=0;i<num_first_lines_skip;i++){ while( ((c=*pReadTo) != '\n') && (c != '\0') ){ pReadTo++; } pReadTo++; pReadFrom = pReadTo; linenum++; }
	while(1){
		while( ((c=*pReadTo) != '\n') && (c != '\0') ){ pReadTo++; }
		c = *pReadTo; *pReadTo = '\0';
		if( c == '\0' ){ break; }
		if( pReadTo == pReadFrom ){ *pReadTo = c; pReadTo++; pReadFrom = pReadTo; continue; }
		linenum++;
//		if( linenum % 10000 == 0 ){ printf("line %d\n", linenum); fflush(stdout); }
//		printf("I will read: '%s' and c='%c'.\n", pReadFrom, c);
//		printf("the line is : '%s'\n", pReadFrom);
		/* the string pointed to by pReadFrom becomes swiss cheese by strtok but we don't care, whereas a_token points to various positions into that string (which is our line of input) */
		for(a_token=GeneralDataReader::delimiter_string_strtok_r(pReadFrom,separator,strlen_separator,&savePtr),col_num=0,from=NULL,to=NULL;a_token;a_token=GeneralDataReader::delimiter_string_strtok_r(NULL,separator,strlen_separator,&savePtr),col_num++){
			/* from, to: these need to be freed when done, re: strdup. These strings will not be in this->forward_search_edgelist
			   (because they use int IDs) or in the result tree - so free them when freeing the whole vertex_names_2_ids.
			** WARNING: the same from/to pointers are used in BOTH vertex_names_2_ids and vertex_ids_2_names - SO FREE THEM ONLY FOR ONE!!! */
			if( col_num == from_node_name_column_number ){ from = a_token; } 
			if( col_num == to_node_name_column_number ){ to = a_token; }
		}

		if( (from==NULL) || (to==NULL) ){ fprintf(stderr, "read_edgelist_from_file : something wrong with line '%s', number %d, one or both of 'from', 'to', columns %d,%d of file '%s' are empty, IFS='%s'\n", pReadFrom, linenum, from_node_name_column_number, to_node_name_column_number, filename, separator); return(-1); }
//		boost::split(strs, pReadFrom, boost::is_any_of(separator)); from = strs[from_node_name_column_number]; to = strs[to_node_name_column_number];
		if( (a_found_n2i=vertex_names_2_ids->find(from)) != vertex_names_2_ids->end() ){ from_id = a_found_n2i->second; }
		else { from_id = last_id++; a_str = strdup(from); ADD_NODE(a_str, from_id); }
		/* strdup(from) : strstroup mentions the MACRO(i++) problem where every use of i in the macro will be i++ ... http://www2.research.att.com/~bs/bs_faq2.html#macro */
#if 0
a_str = strdup(from)
vertex_names_2_ids->insert(std::make_pair(a_str, from_id)); vertex_ids_2_names->insert(std::make_pair(from_id, a_str)); }
#endif
		if( (a_found_n2i=vertex_names_2_ids->find(to)) != vertex_names_2_ids->end() ){ to_id = a_found_n2i->second; }
		else { to_id = last_id++; a_str = strdup(to); ADD_NODE(a_str, to_id); }
#if 0
a_str = strdup(to)
vertex_names_2_ids->insert(std::make_pair(a_str, to_id)); vertex_ids_2_names->insert(std::make_pair(to_id, a_str)); }
#endif
		this->add_edge_forward(from_id, to_id); /* this function call costs time ... */
		//ADD_EDGE_FORWARD(from_id, to_id); /* this function call costs time ... */
#if 0
		if( (a_found_p_i=this->forward_search_edgelist->find(from_id)) == this->forward_search_edgelist->end() ){
			a_third_level_map = new unordered_map_third_level_t();
			this->forward_search_edgelist->insert(std::make_pair(from_id, a_third_level_map));
		} else { a_third_level_map = a_found_p_i->second; }
		a_third_level_map->insert(std::make_pair(to_id,1));
#endif
		num_edges++; /* <-- if an edge is defined twice in the file, it will be counted twice, so this is an upper bound of num of edges */
		*pReadTo = c; pReadTo++; pReadFrom = pReadTo;
	}

	/* we need to know if the contents of the file and hence the this->forward_search_edgelist is transitive, so make a check */
	bool it_is_transitive = Graphos::is_edgelist_transitive(this->forward_search_edgelist);
	if( (this->myPreviousFileWasTransitive != -1) && (this->myPreviousFileWasTransitive != it_is_transitive) ){ fprintf(stderr, "read_edgelist_from_file : the transitivity of data in the previous file read and this one is not the same (previous: %d, this one: %d), current file '%s'.\n", this->myPreviousFileWasTransitive, it_is_transitive, filename); return(-1); }
	if( it_is_transitive == false ){
		if( this->myMakeTransitive == false ){
			/* no it isn't, we need to create separate list for backward search (e.g. when given '-1' -> B) and put all in forw_search backwards, e.g. if A->B, we will put B->A (this does not mean that B is connected to A, it means that A->B but when asking what is connected to B? */
			if( myPreviousFileWasTransitive == -1 ){ this->backward_search_edgelist = new unordered_map_second_level_t(); }
			for(unordered_map_second_level_t::iterator iter=this->forward_search_edgelist->begin();iter!=this->forward_search_edgelist->end();++iter){
				sl = iter->second; from_id = iter->first;
				for(unordered_map_third_level_t::iterator iter2=sl->begin();iter2!=sl->end();++iter2){
					to_id = iter2->first;
					this->add_edge_backward(to_id, from_id);
					//ADD_EDGE_BACKWARD(from_id, to_id); /* this function call costs time ... */
#if 0
					if( (a_found_p_i=this->backward_search_edgelist->find(to_id)) == this->backward_search_edgelist->end() ){
						a_third_level_map = new unordered_map_third_level_t();
						this->backward_search_edgelist->insert(std::make_pair(to_id, a_third_level_map));
					} else { a_third_level_map = a_found_p_i->second; }
					a_third_level_map->insert(std::make_pair(from_id,1));
#endif
				}
			}
		} else {
			/* here we will make the forward list transitive, i.e. for each A->B in this map add a B->A in the same map */
			/* remember that a we may get problems iterating through a map and adding elements to it - or assume so unless someone says otherwise */
			int	unique_id;
			last_id = vertex_names_2_ids->size(); /* the number of all unique nodes */
			std::vector<int>	temp_relationships(num_edges);
			for(unordered_map_second_level_t::iterator iter=this->forward_search_edgelist->begin();iter!=this->forward_search_edgelist->end();++iter){
				sl = iter->second; from_id = iter->first;
				for(unordered_map_third_level_t::iterator iter2=sl->begin();iter2!=sl->end();++iter2){
					to_id = iter2->first;
					unique_id = from_id + last_id * to_id;
					temp_relationships.push_back(unique_id);
				}
			}
			for(i=temp_relationships.size();--i>-1;){
				unique_id = temp_relationships[i];
				to_id = (int )(unique_id / last_id);
				from_id = unique_id % last_id;
				this->add_edge_forward(to_id, from_id);
				//ADD_EDGE_FORWARD(from_id, to_id); /* this function call costs time ... */
#if 0
				if( (a_found_p_i=this->forward_search_edgelist->find(to_id)) == this->forward_search_edgelist->end() ){
					a_third_level_map = new unordered_map_third_level_t();
					this->forward_search_edgelist->insert(std::make_pair(to_id, a_third_level_map));
				} else { a_third_level_map = a_found_p_i->second; }
				a_third_level_map->insert(std::make_pair(from_id, 1));
#endif
			}
			/* there is no need to create separate backward search list, because the forward one contains everything, we just made it transitive */
			if( myPreviousFileWasTransitive == -1 ){ this->backward_search_edgelist = this->forward_search_edgelist; }
		}
	} else {
		/* file contents are transitive, so for each A->B there is already a B->A, so just make references to backlist */
		if( myPreviousFileWasTransitive == -1 ){ this->backward_search_edgelist = this->forward_search_edgelist; }
	}
	this->myPreviousFileWasTransitive = it_is_transitive;
	free(filecontents);
	return linenum;
}
int	Graphos::node_name_2_id(char *n){ return this->_node_name_2_id(n); }
char	*Graphos::node_id_2_name(int n){ return this->_node_id_2_name(n); }

/* these are private versions in order to be inline, the public versions call these as they are not heavily used */
inline	int	Graphos::_node_name_2_id(char *n){
	Graphos::vertex_names_2_ids_t::iterator	a_found;
	if( (a_found=this->vertex_names_2_ids->find(n)) != this->vertex_names_2_ids->end() ){ return a_found->second; } return -2;
}
inline	char	*Graphos::_node_id_2_name(int n){
	Graphos::vertex_ids_2_names_t::iterator	a_found;
	if( (a_found=this->vertex_ids_2_names->find(n)) != this->vertex_ids_2_names->end() ){ return a_found->second; } return (char *)NULL;
}
	
/* returns true or false if found a direct link between from and to.
   if from or to == -1 (undef) then it searches for all links to or from the defined from/to node.
   it returns a pointer to a hashmap whose keys are the matches linking from/to.
   Sometimes this hashmap is created anew (when from,to are both defined). in this case
   the pointer must be freed and the 'must_free_result' result is set to true.
   Other times the resultant pointer points to existing structures and should not be free, so 'must_free_result' will be set to false
*/
bool	Graphos::link(
	char	*from,	/* a node NAME, or 'undef', i.e. all nodes linking to 'to' which must be defined */
	char	*to,	/* a node NAME, or 'undef', i.e. all nodes linking to 'from' which must be defined */ /* at most one of these two can be -1 */
	unordered_map_third_level_t **result,	/* place the result in this pointer */
	bool	*must_free_result	/* true if the result pointer must be freed */
){
	int	from_id, to_id;
	if( (from_id=this->_node_name_2_id(from)) < 0 ){ fprintf(stderr, "Graphos::link() : could not find vertex name '%s' (from) in input file.\n", from); return(-1); }
	if( (to_id=this->_node_name_2_id(to)) < 0 ){ fprintf(stderr, "Graphos::link() : could not find vertex name '%s' (to) in input file.\n", to); return(-1); }
	return this->link(from_id, to_id, result, must_free_result);
}
/* find if two nodes are linked (in the transitive and non-transitive case) */
bool	Graphos::linked(
	char	*from,	/* a node NAME, or 'undef', i.e. all nodes linking to 'to' which must be defined */
	char	*to	/* a node NAME, or 'undef', i.e. all nodes linking to 'from' which must be defined */ /* at most one of these two can be -1 */
){
	int	from_id, to_id;
	if( (from_id=this->_node_name_2_id(from)) < 0 ){ fprintf(stderr, "Graphos::linked() : could not find vertex name '%s' (from) in input file.\n", from); return(-1); }
	if( (to_id=this->_node_name_2_id(to)) < 0 ){ fprintf(stderr, "Graphos::linked() : could not find vertex name '%s' (to) in input file.\n", to); return(-1); }
	return this->linked(from_id, to_id);
}
/* finds all links between 'from' and 'to' num_hops away by recursively finding all links coming out from 'from'
   then for all of these nodes, it does the same until it reaches the specified hops range and the end node is 'to'.
   Recursion may become a problem if the number of hops is more than 10, 100? which is the depth of recursion, depending on
   your system. However depending on the connectivity, a number of hops > 3 may be unrealistic resulting to billions of links...
   so recursion may become a problem but it is not for my application.
*/
int	Graphos::links_recursive(
	char	*from,	/* node name as read from file */
	char	*to,	/* as above */
	int	min_num_hops,
	int	max_num_hops,
	int	current_num_hops,
	GNode	*results_tree /* the result, initialise this pointer first */
){
	int	from_id, to_id;
	if( (from_id=this->_node_name_2_id(from)) < 0 ){ fprintf(stderr, "Graphos::links_recursive() : could not find vertex name '%s' (from) in input file.\n", from); return(-1); }
	if( (to_id=this->_node_name_2_id(to)) < 0 ){ fprintf(stderr, "Graphos::links_recursive() : could not find vertex name '%s' (to) in input file.\n", to); return(-1); }
	return this->links_recursive(from_id, to_id, min_num_hops, max_num_hops, current_num_hops, results_tree);
}
int	Graphos::find_paths(
	char		*from,
	char		*to,
	int		min_num_hops,
	int		max_num_hops,
	GraphosTreePaths	**results_tree,
	GraphosVectorPaths	*results_paths,	/* this is supplied to the function so as to append paths in already existing paths */
	int		*_from_id=NULL,	/* will look the strings up and return the int ids */
	int		*_to_id=NULL
){
	int	from_id, to_id;
	if( (from_id=this->_node_name_2_id(from)) < 0 ){ fprintf(stderr, "Graphos::find_paths() : could not find vertex name '%s' (from) in input file.\n", from); return(-1); }
	if( (to_id=this->_node_name_2_id(to)) < 0 ){ fprintf(stderr, "Graphos::find_paths() : could not find vertex name '%s' (to) in input file.\n", to); return(-1); }
	if( _from_id != NULL ){ *_from_id = from_id; }
	if( _from_id != NULL ){ *_to_id = to_id; }
	/* this->forward_search_edgelist contains keys A, in A->B so it can be used to look up children of nodes,
	   this->backward_search_edgelist contains keys B, in A->B so it can be used to look up parents of nodes */

	*results_tree = new GraphosTreePaths(new gnode_data_t(from_id));
//	cout << "searching recursively from '" << from << "' to '" << to << " (" << from_id << " -> " << to_id << ") ..." << std::endl;
	links_recursive(from_id, to_id, min_num_hops, max_num_hops, 0, (*results_tree)->root());

//	cout << "done, now searching for the leaf nodes of the resultant tree holding the results ...";
	std::vector<GNode *>	*leaf_nodes = (*results_tree)->leaf_nodes();
	if( leaf_nodes->size() == 1 ){
		/* this is just the node we entered at the beginning, no results yielded... */
		delete leaf_nodes;
		return 0;
	}
//	cout << " done, found " << leaf_nodes->size() << std::endl;
	GNode	*a_node;
	foreach(std::vector<GNode *>, vv, *leaf_nodes){
		//fprintf(stdout, "leaf node: %s\n", (char *)((*vv)->data));
		a_node = *vv; /* the data, i.e. tree nodes to this need not be freed because they are just pointers to the tree and are freed with the tree */
		GraphosVectorPaths::path_t *a_path = results_paths->new_path();
		while(true){
			if( a_node == NULL ){ break; }
			a_path->push_back( ((gnode_data_t *)(a_node->data))->id );
			//cout << "pushing " << ((gnode_data_t *)(a_node->data))->id << std::endl;
			a_node = a_node->parent;
		}
		std::reverse(a_path->begin(), a_path->end()); /* because we started from leaf to root, we need to reverse the paths */
	}
	delete leaf_nodes;
	return results_paths->num_paths();
}
bool	Graphos::write_all_data_to_files(
	const char	*outbasename,	/* if not NULL, it saves all results to relevant files with names constructed from this basename */
	const char	*ofs,
	const int	min_num_hops,
	const int	max_num_hops,
	GraphosVectorPaths::path_toString_function_t	printer, /* optional printer for each path */
	GraphosVectorPaths	*results_paths	/* this is supplied to the function so as to append paths in already existing paths */
){
	int	num_paths;
	if( (outbasename != NULL)  && ((num_paths=results_paths->num_paths()) > 0) ){
		nodelist_with_hops_t	*results_nodelist_with_hops = new nodelist_with_hops_t(); /* hops -> nodename */
		edgelist_with_hops_t	*results_edgelist_with_hops = new edgelist_with_hops_t(); /* hops -> from -> to */
		int	num_edges_found, num_nodes_found, num_hops;
		char *outfilename = (char *)malloc(strlen(outbasename)+100), num_hops_str[10]; // a trillion hops...

		if( write_results_as_paths_to_file(outbasename, ofs, min_num_hops, max_num_hops, printer, results_paths) == false ){
			fprintf(stderr, "find_paths : call to write_paths_to_files has failed.\n");
			delete_nodelist_with_hops_t(results_nodelist_with_hops); delete results_nodelist_with_hops;
			delete_edgelist_with_hops_t(results_edgelist_with_hops); delete results_edgelist_with_hops;
			free(outfilename);
			return false;
		}
		if( convert_paths_to_edgelist(results_paths, results_edgelist_with_hops) == false ){
			fprintf(stderr, "find_paths : call to convert_paths_to_edgelist.\n");
			delete_nodelist_with_hops_t(results_nodelist_with_hops); delete results_nodelist_with_hops;
			delete_edgelist_with_hops_t(results_edgelist_with_hops); delete results_edgelist_with_hops;
			free(outfilename);
			return false;
		}
		for(unordered_map_first_level_t::iterator iter=results_edgelist_with_hops->begin();iter!=results_edgelist_with_hops->end();++iter){
			if( (num_hops=iter->first) == -1 ){ strcpy(num_hops_str, "all"); } else { sprintf(num_hops_str, "%d", num_hops); }
			sprintf(outfilename, "%s.edgelist.%shops.txt", outbasename, num_hops_str);
			if( (num_edges_found=write_edgelist_to_file(outfilename, ofs, vertex_ids_2_names, iter->second)) == -1 ){
				fprintf(stderr, "find_paths : call to write_edgelist_to_file has failed for %d hops.\n", num_hops);
				delete_nodelist_with_hops_t(results_nodelist_with_hops); delete results_nodelist_with_hops;
				delete_edgelist_with_hops_t(results_edgelist_with_hops); delete results_edgelist_with_hops;
				free(outfilename);
			}
			fprintf(stdout, "%s hops yielded %d edges", num_hops_str, num_edges_found);
			if( num_edges_found > 0 ){ fprintf(stdout, ", output file '%s'", outfilename); }
			fprintf(stdout, ".\n");
		}
		if( convert_paths_to_nodelist(results_paths, results_nodelist_with_hops) == false ){
			fprintf(stderr, "find_paths : call to convert_paths_to_nodelist.\n");
			delete_nodelist_with_hops_t(results_nodelist_with_hops); delete results_nodelist_with_hops;
			delete_edgelist_with_hops_t(results_edgelist_with_hops); delete results_edgelist_with_hops;
			free(outfilename);
			return false;
		}
		for(nodelist_with_hops_t::iterator iter=results_nodelist_with_hops->begin();iter!=results_nodelist_with_hops->end();++iter){
			if( (num_hops=iter->first) == -1 ){ strcpy(num_hops_str, "all"); } else { sprintf(num_hops_str, "%d", num_hops); }
			sprintf(outfilename, "%s.nodelist.%shops.txt", outbasename, num_hops_str);
			if( (num_nodes_found=write_nodelist_to_file(outfilename, this->vertex_ids_2_names, iter->second)) == -1 ){
				fprintf(stderr, "find_paths : call to write_nodelist_to_file has failed for %d hops.\n", num_hops);
				delete_nodelist_with_hops_t(results_nodelist_with_hops); delete results_nodelist_with_hops;
				delete_edgelist_with_hops_t(results_edgelist_with_hops); delete results_edgelist_with_hops;
				free(outfilename);
				return false;
			}
			fprintf(stdout, "%s hops yielded %d nodes", num_hops_str, num_nodes_found);
			if( num_nodes_found > 0 ){ fprintf(stdout, ", output file '%s'", outfilename); }
			fprintf(stdout, ".\n");
		}
		delete_nodelist_with_hops_t(results_nodelist_with_hops); delete results_nodelist_with_hops;
		delete_edgelist_with_hops_t(results_edgelist_with_hops); delete results_edgelist_with_hops;
		free(outfilename);
	}
	return true;
}
inline	bool	Graphos::linked(
	int	from,	/* a node (internal) id, or -1 for 'undef', i.e. all nodes linking to 'to' which must be defined */
	int	to	/* a node (internal) id, or -1 for 'undef', i.e. all nodes linking to 'from' which must be defined */ /* at most one of these two can be -1 */
){
	unordered_map_second_level_t::iterator	a_found;
	if( ((a_found=this->forward_search_edgelist->find(from))!=this->forward_search_edgelist->end()) && (a_found->second->find(to)!=a_found->second->end()) ){ return true; }
	if( this->forward_search_edgelist != this->backward_search_edgelist ){ return ( ((a_found=this->backward_search_edgelist->find(from))!=this->backward_search_edgelist->end()) && (a_found->second->find(to)!=a_found->second->end()) ); }
	return false;
}
		
inline	bool	Graphos::link(
	int	from,	/* a node (internal) id, or -1 for 'undef', i.e. all nodes linking to 'to' which must be defined */
	int	to,	/* a node (internal) id, or -1 for 'undef', i.e. all nodes linking to 'from' which must be defined */ /* at most one of these two can be -1 */
	unordered_map_third_level_t **result,	/* place the result in this pointer */
	bool	*must_free_result	/* true if the result pointer must be freed */
){
	int	f, t;
	unordered_map_second_level_t	*links;
	unordered_map_second_level_t::iterator	a_found;

	*must_free_result = false;
	*result = NULL;

	if( from == -1 ){
		if( to == -1 ){ std::cerr << "search : can not have both from and to being 'undef'." << std::endl; return false; }
		f = to; t = from; links = this->backward_search_edgelist;
	} else {
		f = from; t = to; links = this->forward_search_edgelist;
	}
	/* f is definitely a node name where is t is or it isn't a node name (if it is n't then it is undef) */
	if( t == -1 ){
		if( (a_found=links->find(f)) != links->end() ){
			//cout << "found for '"<<f<<"' mult" << print_keys_of_unordered_map<unordered_map_third_level_t>(*(a_found->second)) << std::endl;
			//for(unordered_map_second_level_t::iterator iter=links->begin();iter!=links->end();++iter){ cout << iter->first << ","; }
			//cout << std::endl;
			*result = a_found->second;
			return true;
		}
	} else {
		if( ((a_found=links->find(f))!=links->end()) && (a_found->second->find(t)!=a_found->second->end()) ){
			*result = new unordered_map_third_level_t();
			(*result)->insert(std::make_pair(t, 1));
			*must_free_result = true;
			return true;
		}
	}
	return false;
}
inline	int	Graphos::links_recursive(
	int	from_id,	/* node internal id */
	int	to_id,		/* as above */
	int	min_num_hops,
	int	max_num_hops,
	int	current_num_hops,
	GNode	*results_tree /* the result, initialise this pointer first */
){
	unordered_map_third_level_t	*all_matched;
	bool				must_free_result;

	//cout << print_tabs(current_num_hops) << "entering at " << current_num_hops << " hops, " << from << " -> " << to << ", min=" << min_num_hops << ", max=" << max_num_hops << std::endl;
	if( from_id == to_id ){
		if( (current_num_hops <= (max_num_hops+1)) && (current_num_hops > min_num_hops) ){
			//cout << print_tabs(current_num_hops) << "**** success at " << current_num_hops << " hops." << std::endl;
			return(1);
		}
	}
	if( current_num_hops > max_num_hops ){
		//cout << print_tabs(current_num_hops) << "failed at " << current_num_hops << " hops." << std::endl;
		return(0);
	}

	/* find all the links from the 'from' */
	//cout << print_tabs(current_num_hops) << "doing : link(" << from_id << ", 'undef', this->backward_search_edgelist, this->forward_search_edgelist) ...";
	if( link(from_id, -1, &all_matched, &must_free_result) == false ){ return 0; }
	//cout << " done" << std::endl;

	int		new_num_hops = current_num_hops + 1,
			index = 1, pm;
	bool		found_at_least_one_good = false;
	GNode		*a_node;
	gnode_data_t	*a_node_data;
	for(unordered_map_third_level_t::iterator iter=all_matched->begin();iter!=all_matched->end();++iter){
		/* go through each link from 'from' */
		pm = iter->first;
/*		if( (new_num_hops > max_num_hops) && (pm == to_id) ){
			cout << print_tabs(current_num_hops) << "success ((" << new_num_hops << ">=" << max_num_hops << ")&&(" << pm << " eq " << to_id << "))" << std::endl;
		} else if( new_num_hops <= max_num_hops ){
			cout << print_tabs(current_num_hops) << "calling repeated_search('" << pm << "', '" << to_id << "', min_hops=" << min_num_hops << ", max_hops=" << max_num_hops << ", current_num_hops=" << current_num_hops << ") of total :";
			//	for(unordered_map_third_level_t_p::iterator iter2=all_matched.begin();iter2!=all_matched.end();++iter2){ cout << iter2->first << ","; }
			cout << std::endl;
		}
*/
		a_node_data = new gnode_data_t(pm);
		a_node = g_node_new(a_node_data);
		//cout << print_tabs(current_num_hops) << "creating tree node: " << pm << std::endl;
		if( ((new_num_hops>max_num_hops)&&(pm==to_id)) || ((new_num_hops<=max_num_hops)&&(links_recursive(pm, to_id, min_num_hops, max_num_hops, new_num_hops, a_node)==1)) ){
			//cout << print_tabs(current_num_hops) << "saving " << from_id << " -> " << pm << std::endl;
			g_node_append(results_tree, a_node);
			found_at_least_one_good = true;
		} else {
			//cout << print_tabs(current_num_hops) << "deleting non-needed node: " << pm << std::endl;
			//delete a_node_data; g_node_destroy(a_node);
			GraphosTreePaths::free_gnode_tree(a_node);
		}
		index++;
	}
	return found_at_least_one_good;
}
/* as a hashmap num_hops -> list */
/* a num_hops=-1 means all hops put together */
/*static*/	bool	Graphos::convert_paths_to_nodelist(
	GraphosVectorPaths *the_paths,
	Graphos::nodelist_with_hops_t *ret
){
	unordered_map_third_level_t	*a_third_level_map, *all_hops_map;
	Graphos::nodelist_with_hops_t::iterator	a_found_p;
	int	num_hops;
	num_hops = -1; /* for the all hops */
	if( (a_found_p=ret->find(num_hops)) == ret->end() ){ all_hops_map = new unordered_map_third_level_t(); ret->insert(std::make_pair(num_hops, all_hops_map)); } else { all_hops_map = a_found_p->second; }

	BOOST_FOREACH(GraphosVectorPaths::path_t *vv, *(the_paths->paths())){
		// remember that they start from leaf, so reverse foreach
		num_hops = vv->size() - 2;
		if( (a_found_p=ret->find(num_hops))== ret->end() ){ a_third_level_map = new unordered_map_third_level_t(); ret->insert(std::make_pair(num_hops, a_third_level_map)); }
		else { a_third_level_map = a_found_p->second; }
		BOOST_REVERSE_FOREACH(int a_node_id, *vv){
			a_third_level_map->insert(std::make_pair(a_node_id, 1));
			all_hops_map->insert(std::make_pair(a_node_id, 1));
		}
	}
	return true;
}
/* as a hashmap num_hops -> from -> to */
/* a num_hops=-1 means all hops put together */
/*static*/	bool	Graphos::convert_paths_to_edgelist(
	GraphosVectorPaths *the_paths,
	edgelist_with_hops_t *ret
){
	edgelist_t	*a_second_level_map, *all_hops_map;
	unordered_map_third_level_t	*a_third_level_map, *a_third_level_mapa;
	edgelist_with_hops_t::iterator	a_found_p1;
	unordered_map_second_level_t::iterator	a_found_p2, a_found_p2a;
	int	first_id, second_id, num_hops;

	num_hops = -1; /* for the all hops */
	if( (a_found_p1=ret->find(num_hops)) == ret->end() ){
		all_hops_map = new edgelist_t();
		ret->insert(std::make_pair(num_hops, all_hops_map));
	} else { all_hops_map = a_found_p1->second; }

//	BOOST_FOREACH(GraphosVectorPaths::path_t *vv, the_paths->paths())){
	foreach(GraphosVectorPaths::paths_t, vv, (*the_paths->paths())){
		// remember that they start from leaf, so reverse foreach
		num_hops = (*vv)->size() - 2;
		if( (a_found_p1=ret->find(num_hops))== ret->end() ){ a_second_level_map = new unordered_map_second_level_t(); ret->insert(std::make_pair(num_hops, a_second_level_map)); }
		else { a_second_level_map = a_found_p1->second; }
		first_id = -1;
		BOOST_REVERSE_FOREACH(int a_node_id, **vv){
//		reverse_foreach(int, a_node_id, **vv){
			if( first_id == -1 ){ first_id = a_node_id; continue; }
			if( (a_found_p2=a_second_level_map->find(first_id))== a_second_level_map->end() ){ a_third_level_map = new unordered_map_third_level_t(); a_second_level_map->insert(std::make_pair(first_id, a_third_level_map)); }
			else { a_third_level_map = a_found_p2->second; }
			second_id = a_node_id;
			a_third_level_map->insert(std::make_pair(second_id, 1));

			if( (a_found_p2a=all_hops_map->find(first_id))== all_hops_map->end() ){ a_third_level_mapa = new unordered_map_third_level_t(); all_hops_map->insert(std::make_pair(first_id, a_third_level_mapa)); }
			else { a_third_level_mapa = a_found_p2a->second; }
			a_third_level_mapa->insert(std::make_pair(second_id, 1));

			first_id = second_id;
		}
	}
	//print_unordered_map_first_level(ret);
	return true;
}
/*static*/	bool	Graphos::write_results_as_paths_to_file(
	const char *outbasename,
	const char *ofs,
	const int min_num_hops,
	const int max_num_hops,
	GraphosVectorPaths::path_toString_function_t printer, /* optional printer for each path node and edge, essentially for their data */
	GraphosVectorPaths *the_paths
){
	char	*buffer, *pBuffer;
	if( (buffer=(char *)malloc(100000)) == NULL ){ fprintf(stderr, "write_results_as_paths_to_file : could not allocate %d bytes for char buffer.\n", 100000); perror(NULL); exit(1); }
	char	*sBuffer = &(buffer[0]);
	int	num_hops, total_hops_trials = max_num_hops - min_num_hops + 1, i,
		*num_paths_per_hop = (int *)malloc(total_hops_trials*sizeof(int));
	FILE	**file_handles = (FILE **)malloc(total_hops_trials*sizeof(FILE *));
	char	**output_filenames = (char **)malloc(total_hops_trials*sizeof(char *));
	for(i=0;i<total_hops_trials;i++){
		asprintf(&(output_filenames[i]), "%s.%dhops.txt", outbasename, (i+min_num_hops));
		if( (file_handles[i]=fopen(output_filenames[i], "w")) == NULL ){ fprintf(stderr, "write_results_as_paths_to_file : could not open output file '%s' for writing results.\n", output_filenames[i]); perror(NULL); return false; }
		num_paths_per_hop[i] = 0;
	}
	int	num_paths = the_paths->num_paths(), num_items, num_items_minus_one,
		j, a_node_id, next_node_id;
	graphos_node_storage_t	*a_node_data;
	graphos_edge_storage_t	*a_edge_data;
	GraphosVectorPaths::path_data_t	*a_path_data;
	char			*a_str, *a_node_name;

	if( printer == NULL ){
		for(i=0;i<num_paths;i++){
			num_items = the_paths->num_items(i); num_items_minus_one = num_items - 1;
			pBuffer = sBuffer;
			num_hops = num_items - 2;
			next_node_id=-1;
			for(j=0;j<num_items;j++){
				a_node_id = the_paths->get(i,j);
				a_node_name = (*(this->vertex_ids_2_names))[a_node_id];
				if( (a_node_data=this->_get_node_data(a_node_id)) == NULL ){
					pBuffer += sprintf(pBuffer, "%s()%s", a_node_name, ofs);
				} else {
					a_str = a_node_data->toString(",");
					pBuffer += sprintf(pBuffer, "%s,(%s)%s", a_node_name, a_str, ofs);
					free(a_str);
				}
				if( j < num_items_minus_one ){
					next_node_id = the_paths->get(i,j+1);
					a_str = (*(this->vertex_ids_2_names))[next_node_id];
					if( (a_edge_data=this->_get_edge_data(a_node_id, next_node_id)) != NULL ){
						//printf("EDGE DAA: %s->%s (%d->%d) = %p\n", a_node_name, a_str, a_node_id, next_node_id, a_edge_data);
						a_str = a_edge_data->toString(",");
						pBuffer += sprintf(pBuffer, "(%s)%s", a_str, ofs);
						free(a_str);
					}
				}
			}
			/* now do we have path data? then print it at the end */
			a_path_data = the_paths->get_data(i);
			a_str = a_path_data->toString(",");
			pBuffer += sprintf(pBuffer, "(%s)\n", a_str);
			free(a_str);
			fputs(sBuffer, file_handles[num_hops-min_num_hops]);
			num_paths_per_hop[num_hops-min_num_hops]++;
		}
	} else {
		/* a printer was given */
		for(i=0;i<num_paths;i++){
			std::string	result("");
			num_items = the_paths->num_items(i); num_items_minus_one = num_items - 1;
			num_hops = num_items - 2;
			the_paths->toString_path_data(this, printer, i, ofs, &result);
			result.push_back('\n');
			fputs(result.c_str(), file_handles[num_hops-min_num_hops]);
			num_paths_per_hop[num_hops-min_num_hops]++;
		}
	}
		
	for(i=0;i<total_hops_trials;i++){
		fclose(file_handles[i]);
		fprintf(stdout, "%d hops yielded %d paths", i+min_num_hops, num_paths_per_hop[i]);
		if( num_paths_per_hop[i] > 0 ){ fprintf(stdout, ", output file '%s'", output_filenames[i]); } /*else { unlink(output_filenames[i]); }*/
		fprintf(stdout, ".\n");
		free(output_filenames[i]);
	}
	free(buffer); free(num_paths_per_hop); free(file_handles); free(output_filenames);
	return true;
}
int	Graphos::read_edgedata_from_file(
	const char	*filename,
	int	num_first_lines_skip, /* as header if any */
	int	from_node_name_column_number,
	int	to_node_name_column_number,
	int	num_columns_to_read,
	const int	*the_columns,
	const GeneralDataReader::data_type_t *the_data_types,
	const char	**default_values,
	const char	*separator /* data separator */
){
	char	*pReadFrom, *pReadTo, *filecontents, c, *from, *to;
	if( (filecontents=Graphos::read_file_in_memory(filename)) == NULL ){ fprintf(stderr, "read_edgedata_from_file : call to read_file_in_memory has failed for input file '%s'.\n", filename); return(-1); }

	unordered_map_second_level_t::iterator  um_it;
	unordered_map_third_level_t::iterator	um_sl_it;

	edge_data_contents_t	*a_third_level_map;
	vertex_names_2_ids_t::iterator	a_found_n2i;
	int	from_id = 0, to_id = 0;
	edge_data_t::iterator	a_found_c_i, a_found_p_i;
	edge_data_contents_t::iterator	a_found_p_3;

	GeneralDataReader	*gdr = new GeneralDataReader(
		num_columns_to_read,
		the_columns,
		the_data_types,
		default_values,
		separator
	);
	GeneralDataStorage	*stor = NULL;

	int	linenum=0, i;
	pReadTo = pReadFrom = &(filecontents[0]);
	for(i=0;i<num_first_lines_skip;i++){ while( ((c=*pReadTo) != '\n') && (c != '\0') ){ pReadTo++; } pReadTo++; pReadFrom = pReadTo; linenum++; }
	while(1){
		while( ((c=*pReadTo) != '\n') && (c != '\0') ){ pReadTo++; }
		c = *pReadTo; *pReadTo = '\0';
		if( c == '\0' ){ break; }
		if( pReadTo == pReadFrom ){ *pReadTo = c; pReadTo++; pReadFrom = pReadTo; continue; }
		linenum++;
//		if( linenum % 10000 == 0 ){ printf("line %d\n", linenum); fflush(stdout); }
//		printf("I will read: '%s' and c='%c'.\n", pReadFrom, c);
//		printf("the line is : '%s'\n", pReadFrom);
		/* the string pointed to by pReadFrom becomes swiss cheese by strtok but we don't care, whereas a_token points to various positions into that string (which is our line of input) */
		gdr->parse(pReadFrom);
		stor = gdr->storage();
		from = stor->get_as<char *>(from_node_name_column_number); to = stor->get_as<char *>(to_node_name_column_number);
		if( (a_found_n2i=vertex_names_2_ids->find(from)) != vertex_names_2_ids->end() ){ from_id = a_found_n2i->second; }
		else {
//			fprintf(stderr, "Graphos::read_edgedata_from_file() : could not find FROM node '%s' (specified column %d) in the list of known edges, line %d of file '%s', will skip...\n", from, from_node_name_column_number, linenum, filename);
			stor->clear();
			*pReadTo = c; pReadTo++; pReadFrom = pReadTo;
			continue;
		}
		if( (a_found_n2i=vertex_names_2_ids->find(to)) != vertex_names_2_ids->end() ){ to_id = a_found_n2i->second; }
		else {
//			fprintf(stderr, "Graphos::read_edgedata_from_file() : could not find TO node '%s' (specified column %d) in the list of known edges, line %d of file '%s', will skip...\n", to, to_node_name_column_number, linenum, filename);
			stor->clear();
			*pReadTo = c; pReadTo++; pReadFrom = pReadTo;
			continue;
		}
		if( this->linked(from_id, to_id) == false ){
#ifdef DATA_FILE_CONSISTENCY_CHECKS
			fprintf(stderr, "Graphos::read_edgedata_from_file() : could not find any edge between FROM node '%s' (specified column %d, node id=%d) and TO node '%s' (specified column %d, node id=%d) in the list of known edges, line %d of file '%s', will skip...\n", from, from_node_name_column_number, from_id, to, to_node_name_column_number, to_id, linenum, filename);
#endif
			stor->clear(); *pReadTo = c; pReadTo++; pReadFrom = pReadTo; continue; }

		if( (a_found_p_i=this->forward_search_edge_data->find(from_id)) == this->forward_search_edge_data->end() ){
			a_third_level_map = new edge_data_contents_t();
			this->forward_search_edge_data->insert(std::make_pair(from_id, a_third_level_map));
		} else { a_third_level_map = a_found_p_i->second; }
		if( (a_found_p_3=a_third_level_map->find(to_id)) != a_third_level_map->end() ){
			/* already have data for this, so merge */
			//printf("MERGING: for %s->%s\n", from, to);
			a_found_p_3->second->append(stor);
			stor->soft_delete(); stor->reset(); /* delete the containers but not the content pointers because they are appended to the other one */
		} else {
			//printf("ADDING: for %s->%s\n", from, to);
			a_third_level_map->insert(std::make_pair(to_id,stor));
			/* the ref to stor has been saved, so make new storage */
			gdr->make_new_storage();
		}
		*pReadTo = c; pReadTo++; pReadFrom = pReadTo;
	}
	delete gdr;
	free(filecontents);

	if( this->forward_search_edgelist != this->backward_search_edgelist ){
		/* we are recreating the backward search edgedata but first erase it */
		Graphos::delete_backward_search_edge_data_t(this->backward_search_edge_data); delete this->backward_search_edge_data;
		this->backward_search_edge_data = new edge_data_t();
		for(edge_data_t::iterator iter=this->forward_search_edge_data->begin();iter!=this->forward_search_edge_data->end();++iter){
			for(edge_data_contents_t::iterator iter2=iter->second->begin();iter2!=iter->second->end();++iter2){
				if( (a_found_p_i=this->backward_search_edge_data->find(to_id)) == this->backward_search_edge_data->end() ){
					a_third_level_map = new edge_data_contents_t();
					this->backward_search_edge_data->insert(std::make_pair(to_id, a_third_level_map));
				} else { a_third_level_map = a_found_p_i->second; }
				a_third_level_map->insert(std::make_pair(from_id,stor));
			}
		}
	}
	return linenum;
}
int	Graphos::read_nodedata_from_file(
	const char *filename,
	int	num_first_lines_skip, /* as header if any */
	int	node_name_column_number,
	int	num_columns_to_read,
	const int	*the_columns,
	const GeneralDataReader::data_type_t *the_data_types,
	const char	**default_values,
	const char	*separator /* data separator */
){
	char	*pReadFrom, *pReadTo, *filecontents, c, *node_name;
	if( (filecontents=Graphos::read_file_in_memory(filename)) == NULL ){ fprintf(stderr, "read_nodedata_from_file : call to read_file_in_memory has failed for input file '%s'.\n", filename); return(-1); }

	vertex_names_2_ids_t::iterator	a_found_n2i;
	int	node_id;
	node_data_t::iterator	a_found_d;

	GeneralDataReader	*gdr = new GeneralDataReader(
		num_columns_to_read,
		the_columns,
		the_data_types,
		default_values,
		separator
	);
	GeneralDataStorage	*stor;

	int	linenum=0, i;
	pReadTo = pReadFrom = &(filecontents[0]);
	for(i=0;i<num_first_lines_skip;i++){ while( ((c=*pReadTo) != '\n') && (c != '\0') ){ pReadTo++; } pReadTo++; pReadFrom = pReadTo; linenum++; }
	while(1){
		while( ((c=*pReadTo) != '\n') && (c != '\0') ){ pReadTo++; }
		c = *pReadTo; *pReadTo = '\0';
		if( c == '\0' ){ break; }
		if( pReadTo == pReadFrom ){ *pReadTo = c; pReadTo++; pReadFrom = pReadTo; continue; }
		linenum++;
//		if( linenum % 10000 == 0 ){ printf("line %d\n", linenum); fflush(stdout); }
//		printf("I will read: '%s' and c='%c'.\n", pReadFrom, c);
//		printf("the line is : '%s'\n", pReadFrom);
		/* the string pointed to by pReadFrom becomes swiss cheese by strtok but we don't care, whereas a_token points to various positions into that string (which is our line of input) */
		gdr->parse(pReadFrom);
		stor = gdr->storage();
		node_name = stor->get_as<char *>(node_name_column_number);
		if( (a_found_n2i=vertex_names_2_ids->find(node_name)) != vertex_names_2_ids->end() ){ node_id = a_found_n2i->second; }
		else { fprintf(stderr, "Graphos::read_nodedata_from_file() : could not find node '%s' (specified column %d) in the list of known edges, line %d of file '%s', will skip...\n", node_name, node_name_column_number, linenum, filename); stor->clear(); *pReadTo = c; pReadTo++; pReadFrom = pReadTo; continue; }

		if( (a_found_d=this->node_data->find(node_id)) == this->node_data->end() ){
			//printf("ADDING NODE DATA: for %s = %s\n", node_name, stor->toString(","));
			this->node_data->insert(std::make_pair(node_id,stor));
			/* the ref to stor has been saved, so make new storage */
			gdr->make_new_storage();
		} else {
			/* already have data for this, so merge */
			//printf("MERGING NODE DATA: for %s\n", node_name);
			a_found_d->second->append(stor);
			stor->soft_delete(); stor->reset(); /* delete the containers but not the content pointers because they are appended to the other one */
		}
		*pReadTo = c; pReadTo++; pReadFrom = pReadTo;
	}
	delete gdr;
	free(filecontents);
	return linenum;
}
void	Graphos::print_node_data(FILE *fh){
	char	*a_str;
	for(node_data_t::iterator iter=this->node_data->begin();iter!=this->node_data->end();++iter){
		a_str = iter->second->toString(",");
		fprintf(fh, "%d : '%s'\n", iter->first, a_str);
		free(a_str);
	}
}
void	Graphos::print_edge_data(FILE *fh){
	char	*a_str;
	for(edge_data_t::iterator iter=this->forward_search_edge_data->begin();iter!=this->forward_search_edge_data->end();++iter){
		for(edge_data_contents_t::iterator iter2=iter->second->begin();iter2!=iter->second->end();++iter2){
			a_str = iter2->second->toString(",");
			fprintf(fh, "%d -> %d: '%s'", iter->first, iter2->first, a_str);
			free(a_str);
		}
	}
}
/* delete all edges involving this node, either from or to */
inline	void	Graphos::break_edge_starting_with(
	int	node_id
){
	Graphos::edgelist_t::iterator	a_found_e;
	Graphos::nodelist_t::iterator	a_found_n;

	if( (a_found_e=this->forward_search_edgelist->find(node_id)) == this->forward_search_edgelist->end() ){ return; /* nothing found */}

	Graphos::nodelist_t	*the_nodelist = a_found_e->second;
	/* the whole third level map has to go, but before, we have to remove each reverse edge (e.g. B->A) from back list if needed */
	if( this->myMakeTransitive == true ){
		Graphos::edgelist_t	*which_list;
		if( this->forward_search_edgelist != this->backward_search_edgelist ){
			/* we need to do the same for the backward list because they are independent */
			which_list = this->backward_search_edgelist;
		} else {
			/* lists are the same but because it's transitive, we need to erase the to->from from the forward list */
			which_list = this->forward_search_edgelist;
		}
		for(a_found_n=the_nodelist->begin();a_found_n!=the_nodelist->end();++a_found_n){
			printf("breaking edge %d->%d\n", node_id, a_found_n->first);
			Graphos::break_edge(node_id, a_found_n->first, which_list);
		}
	}
	/* now ready to remove from forward list */
	Graphos::delete_nodelist_t(a_found_e->second);
	this->forward_search_edgelist->erase(a_found_e);
}
inline	void	Graphos::break_edge_ending_with(
	int	node_id
){
	Graphos::edgelist_t::iterator	a_found_e;
	Graphos::nodelist_t::iterator	a_found_n;

	//printf("entering break_edge_ending_with id=%d\n", node_id);
	//for(edgelist_t::iterator iter=this->backward_search_edgelist->begin();iter!=this->backward_search_edgelist->end();++iter){printf("I have: this: '%d'\n", iter->first);}
	/* same as with the ending but check first the backward list */
	if( (a_found_e=this->backward_search_edgelist->find(node_id)) != this->backward_search_edgelist->end() ){
		//printf("entering break_edge_ending_with id=%d\n", node_id);
		Graphos::nodelist_t	*the_nodelist = a_found_e->second; /* erase all node_id -> each member of this nodelist */
		/* the whole third level map has to go, but before, we have to remove each reverse edge (e.g. B->A) from back list if needed */
		for(a_found_n=the_nodelist->begin();a_found_n!=the_nodelist->end();++a_found_n){
			//printf("breaking edge %d->%d from forward list\n", a_found_n->first, node_id);
			Graphos::break_edge(a_found_n->first, node_id, this->forward_search_edgelist);
		}
		/* now ready to remove from backward list */
		Graphos::delete_nodelist_t(a_found_e->second);
		this->backward_search_edgelist->erase(a_found_e);
	}
}
void	Graphos::delete_node_by_name(
	char	*node_name
){
	vertex_names_2_ids_t::iterator	a_found;
	int	node_id;
	if( (a_found=this->vertex_names_2_ids->find(node_name)) == this->vertex_names_2_ids->end() ){ return; /* not found */ }
	node_id = a_found->second;
	//printf("i am deleting this node: '%s', id=%d\n", node_name, node_id);
	/* now we need to erase all edges involving this node, this is painful */
	this->break_edge(node_id);
	/* free the data allocated to it */
	free(a_found->first);
	/* and finally remove from the list */
	this->vertex_ids_2_names->erase(node_id);
	this->vertex_names_2_ids->erase(a_found);
}
inline	void	Graphos::delete_node_by_id(
	int	node_id
){
	vertex_ids_2_names_t::iterator	a_found;
	if( (a_found=this->vertex_ids_2_names->find(node_id)) == this->vertex_ids_2_names->end() ){ return; /* not found */ }
	this->vertex_names_2_ids->erase(a_found->second);
	this->vertex_ids_2_names->erase(a_found);
}
/* break edge involving node_id, either as from or to */
inline	void	Graphos::break_edge(
	int	node_id
){
	this->break_edge_starting_with(node_id);
	this->break_edge_ending_with(node_id);
}

/* give it an edgelist and goes and erases the A->B, it does not care about transitive or whatever */
/*static*/	int	Graphos::break_edge(
	int	from_id,
	int	to_id,
	edgelist_t	*an_edgelist
){
	Graphos::edgelist_t::iterator	a_found_e;
	Graphos::nodelist_t::iterator	a_found_n;
	if( (a_found_e=an_edgelist->find(from_id)) == an_edgelist->end() ){ return -1; /* no from_id found */}
	if( (a_found_n=a_found_e->second->find(to_id)) == a_found_e->second->end() ){ return 0; /* no to_id found */ }
	a_found_e->second->erase(a_found_n); /* erase as a key, no memory delete required as it stores int as value */
	if( a_found_e->second->size() == 0 ){
		/* we need to delete from because it has no elements */
		Graphos::delete_nodelist_t(a_found_e->second);
		an_edgelist->erase(a_found_e);
		return 1;
	}
	return 2;
}

inline	void	Graphos::break_edge(
	int	from_id,
	int	to_id
){
	Graphos::edgelist_t::iterator	a_found_e;
	Graphos::nodelist_t::iterator	a_found_n;

	if( Graphos::break_edge(from_id, to_id, this->forward_search_edgelist) < 1 ){ return; /* no deletion was made */ }

	/* it means that we need to break all to->from too because the edges are transitive (e.g. for each a->b, there is a b->a)
	   and must be deleted transitively, if it isn't transitive then if there is by chance a b->a, it must be left intact when a->b is deleted */
	if( this->myMakeTransitive == true ){
		if( this->forward_search_edgelist != this->backward_search_edgelist ){
			/* we need to do the same for the backward list because they are independent */
			Graphos::break_edge(to_id, from_id, this->backward_search_edgelist);
		} else {
			/* lists are the same but because it's transitive, we need to erase the to->from from the forward list */
			Graphos::break_edge(to_id, from_id, this->forward_search_edgelist);
		}
	}
}

inline	void	Graphos::add_edge_forward(
	int	from_id,
	int	to_id
){
	Graphos::nodelist_t	*a_third_level_map;
	Graphos::edgelist_t::iterator  a_found_e;
	
	if( (a_found_e=this->forward_search_edgelist->find(from_id)) == this->forward_search_edgelist->end() ){
		a_third_level_map = new Graphos::nodelist_t();
		this->forward_search_edgelist->insert(std::make_pair(from_id, a_third_level_map));
	} else { a_third_level_map = a_found_e->second; }
	a_third_level_map->insert(std::make_pair(to_id,1));
}	
inline	void	Graphos::add_edge_backward(
	int	from_id,
	int	to_id
){
	Graphos::nodelist_t	*a_third_level_map;
	Graphos::edgelist_t::iterator  a_found_e;
	
	if( (a_found_e=this->backward_search_edgelist->find(from_id)) == this->backward_search_edgelist->end() ){
		a_third_level_map = new Graphos::nodelist_t();
		this->backward_search_edgelist->insert(std::make_pair(from_id, a_third_level_map));
	} else { a_third_level_map = a_found_e->second; }
	a_third_level_map->insert(std::make_pair(to_id,1));
}
inline	Graphos::graphos_node_storage_t *Graphos::_get_node_data(int node_id){
	Graphos::node_data_t::iterator	a_found_n;
	if( (a_found_n=this->node_data->find(node_id)) == this->node_data->end() ){ return NULL; }
	return a_found_n->second;
}
inline	Graphos::graphos_node_storage_t *Graphos::_get_edge_data(int from_id, int to_id){
	Graphos::edge_data_t::iterator	a_found_e;
	Graphos::edge_data_contents_t::iterator	a_found_c;
	if( ((a_found_e=this->forward_search_edge_data->find(from_id)) == this->forward_search_edge_data->end()) ||
	    ((a_found_c=a_found_e->second->find(to_id)) == a_found_e->second->end()) ){
	    	/* could n't find it in the forward list, can we do it with back list ? only if we are transitive */
		if( this->myMakeTransitive == true ){
			Graphos::edge_data_t	*which_list;
			if( this->forward_search_edgelist != this->backward_search_edgelist ){
				/* forward and backward list because are independent */
				which_list = this->backward_search_edge_data;
			} else { which_list = this->forward_search_edge_data; }
			if( ((a_found_e=which_list->find(to_id)) == which_list->end()) ||
			    ((a_found_c=a_found_e->second->find(from_id)) == a_found_e->second->end()) ){ return NULL; }
		} else { return NULL; }
	}
	return a_found_c->second;
}
/* the public version of the above, just calls them, because they are inlined we get problems if we let them public ... c++ glory */
Graphos::graphos_node_storage_t *Graphos::get_node_data(int node_id){
	return this->_get_node_data(node_id);
}
Graphos::graphos_node_storage_t *Graphos::get_edge_data(int from_id, int to_id){
	return this->_get_edge_data(from_id, to_id);
}
Graphos::graphos_node_storage_t *Graphos::get_node_data(char *node_name){
	int	node_id;
	if( (node_id=this->_node_name_2_id(node_name)) < 0 ){ fprintf(stderr, "Graphos::_get_node_data() : could not find vertex name '%s' in input file.\n", node_name); return NULL; }
	return this->_get_node_data(node_id);
}
Graphos::graphos_node_storage_t *Graphos::get_edge_data(char *from_name, char *to_name){
	int	from_id, to_id;
	if( (from_id=this->_node_name_2_id(from_name)) < 0 ){ fprintf(stderr, "Graphos::_get_edge_data() : could not find vertex name '%s' (from) in input file.\n", from_name); return NULL; }
	if( (to_id=this->_node_name_2_id(to_name)) < 0 ){ fprintf(stderr, "Graphos::_get_edge_data() : could not find vertex name '%s' (to) in input file.\n", to_name); return NULL; }
	return this->_get_edge_data(from_id, to_id);
}

/* public methods using node names (not private id's) */
/* they should not be inline with current setup */
/* delete all edges involving this node, either from or to */
void	Graphos::break_edge_starting_with(
	char	*node_name
){
	int	node_id;
	if( (node_id=this->_node_name_2_id(node_name)) < 0 ){ fprintf(stderr, "Graphos::break_edge_starting_with() : could not find vertex name '%s' in input file.\n", node_name); return; }
	this->break_edge_starting_with(node_id);
}
void	Graphos::break_edge_ending_with(
	char	*node_name
){
	int	node_id;
	if( (node_id=this->_node_name_2_id(node_name)) < 0 ){ fprintf(stderr, "Graphos::break_edge_ending_with() : could not find vertex name '%s' in input file.\n", node_name); return; }
	this->break_edge_ending_with(node_id);
}
/* break edge involving node_id, either as from or to */
void	Graphos::break_edge(
	char	*node_name
){
	int	node_id;
	if( (node_id=this->_node_name_2_id(node_name)) < 0 ){ fprintf(stderr, "Graphos::break_edge() : could not find vertex name '%s' in input file.\n", node_name); return; }
	this->break_edge_starting_with(node_id);
	this->break_edge_ending_with(node_id);
}
void	Graphos::break_edge(
	char	*from_name,
	char	*to_name
){
	int	from_id, to_id;
	if( (from_id=this->_node_name_2_id(from_name)) < 0 ){ fprintf(stderr, "Graphos::break_edge() : could not find vertex name '%s' (from) in input file.\n", from_name); return; }
	if( (to_id=this->_node_name_2_id(to_name)) < 0 ){ fprintf(stderr, "Graphos::break_edge() : could not find vertex name '%s' (to) in input file.\n", to_name); return; }
	this->break_edge(from_id, to_id);
}
