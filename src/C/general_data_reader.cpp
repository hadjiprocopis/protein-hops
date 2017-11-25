/*
Program by Andreas Hadjiprocopis (ICR at the time)
contact: andreashad2@gmail.com
The program is free to distribute and use but please
attribute the orginal source and author.
*/

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <cassert>

#include "general_data_reader.hpp"
///*static*/bool GeneralDataReader::default_string_to_any_converter_INT(char *d, GeneralDataStorage *o){ o->put(atoi(d)); return true; }
///*static*/bool GeneralDataReader::default_string_to_any_converter_FLOAT(char *d, GeneralDataStorage *o){ o->put((float )atof(d)); return true; }
/*static*/bool GeneralDataReader::default_string_to_any_converter_DOUBLE(char *d, GeneralDataStorage *o){ o->put(atof(d)); return true; }
/*static*/bool GeneralDataReader::default_string_to_any_converter_STRING(char *d, GeneralDataStorage *o){ o->put_and_remember_to_delete(strdup(d)); return true; }
/* you need to free the returned value */
///*static*/char *GeneralDataReader::default_any_to_string_converter_INT(void *d){ char *ret; asprintf(&ret, "%d", *((int *)(d))); return ret; }
///*static*/char *GeneralDataReader::default_any_to_string_converter_FLOAT(void *d){ char *ret; asprintf(&ret, "%f", *((float *)(d))); return ret; }
/*static*/char *GeneralDataReader::default_any_to_string_converter_DOUBLE(GeneralDataStorage::contents_t *d){ char *ret; asprintf(&ret, "%lf", boost::get<double>(*d)); return ret; }
/*static*/char *GeneralDataReader::default_any_to_string_converter_STRING(GeneralDataStorage::contents_t *d){ /*we need it because we free without checking type*/ char *ret; asprintf(&ret, "%s", boost::get<char *>(*d)); return ret; }
/*
bool GeneralDataReader::default_string_to_any_converter_INT(char *d, GeneralDataStorage *o, int idx){ o->put(atoi(d), idx); return true; }
bool GeneralDataReader::default_string_to_any_converter_FLOAT(char *d, GeneralDataStorage *o, int idx){ o->put((float )atof(d), idx); return true; }
bool GeneralDataReader::default_string_to_any_converter_DOUBLE(char *d, GeneralDataStorage *o, int idx){ o->put(atof(d), idx); return true; }
bool GeneralDataReader::default_string_to_any_converter_STRING(char *d, GeneralDataStorage *o, int idx){ o->put(strdup(d), idx); return true; }
*/

GeneralDataReader::GeneralDataReader(int num_columns, const int *the_columns, const GeneralDataReader::data_type_t *the_data_type_names, const char **the_default_values, const char *the_separator){
	this->myNumColumns = 0;
	this->myDefaultValues = NULL;
//	printf("src/C/general_data_reader.cpp : num col %d, %d, %d, '%s'\n", num_columns, the_columns[8], the_data_type_names[8], the_separator);
	this->reset(num_columns, the_columns, the_data_type_names, the_default_values, the_separator);
}
GeneralDataReader::GeneralDataReader(GeneralDataReader const&G){
	this->myNumColumns = 0;
	this->myDefaultValues = NULL;
	this->reset(G.get_num_columns(),
 G.get_columns(),
 G.get_data_type_names(),
 /* BUGBUG : breaks my balls */
(const char **) G.get_default_values(),
G.get_separator()
);
}
GeneralDataReader::~GeneralDataReader(){ this->_delete_me(); }
/* call this when you want to make new storage and keep the old one for its contents separate */
void	GeneralDataReader::make_new_storage(){ this->myStorage = new GeneralDataStorage(this->myNumColumns); }
//void	GeneralDataReader::string_to_any_converter(DataTypeName dtn, bool (*string_to_any_converter)(char *)){ this->string_to_any_converters[dtn] = string_to_any_converter; }
/* this is what the destructor (and reset) calls - all the delete code is here */
void	GeneralDataReader::_delete_me(void){
	delete this->myStorage;
	delete this->myColumnsBitmap;
	free(this->myColumns);
	free(this->myDataTypeNames);
	free(this->mySeparator);
	int i; for(i=this->myNumColumns;--i>-1;){ free(this->myDefaultValues[i]); } free(this->myDefaultValues);
}
/* specify which columns (given the separator) from a data line we are interested in, starts from 0 ZERO */
void	GeneralDataReader::reset(
	int num_columns,
	const int *the_columns,
	const GeneralDataReader::data_type_t	*the_data_type_names,
	const char	**the_default_values,
	const char	*the_separator
){
	int	i, x;
	if( this->myNumColumns > 0 ){ this->_delete_me(); }
	/* the problem of fast lookup whether a column is in this list, is to create a bit-map with N entries
	   where N is the this->myMaxColumnNumberSpecified column number specified here
	*/
	this->myNumColumns = num_columns;
	this->make_new_storage();
	this->myColumns = (int *)malloc(num_columns*sizeof(int));
	this->separator(the_separator);
	
	this->myMaxColumnNumberSpecified = the_columns[0];
	for(i=0;i<this->myNumColumns;i++){
		x = the_columns[i];
		this->myColumns[i] = x;
		if(x>this->myMaxColumnNumberSpecified){this->myMaxColumnNumberSpecified=x;}
	}
	/* the bitmap spans from 0 to max column number and tells us if col X was specified to be stored */
	/* this is alternative to scan the whole array of supplied columns to see if X is in there and is better for small X (<1000 say = 1000 columns!) */
	this->myColumnsBitmap = new bitmap_t(this->myMaxColumnNumberSpecified+1, false);
	for(i=0;i<this->myNumColumns;i++){ this->myColumnsBitmap->at(this->myColumns[i]) = true; }

	this->myDataTypeNames = (GeneralDataReader::data_type_t *)malloc(num_columns*sizeof(GeneralDataReader::data_type_t));
	for(i=0;i<this->myNumColumns;i++){ this->myDataTypeNames[i] = the_data_type_names[this->myColumns[i]]; }

	this->myDefaultValues = (char **)malloc(this->myNumColumns*sizeof(char *)); for(i=this->myNumColumns;--i>-1;){ this->myDefaultValues[i] = strdup(the_default_values[i]); }
}
/* get/setters */
void	GeneralDataReader::separator(const char *sep){ this->mySeparator = strdup(sep); this->myStrlenSeparator = strlen(this->mySeparator); }
char	**GeneralDataReader::get_default_values(void) const { return this->myDefaultValues; }
char	*GeneralDataReader::get_separator(void) const { return this->mySeparator; }
int	*GeneralDataReader::get_columns(void) const { return this->myColumns; }
GeneralDataReader::data_type_t	*GeneralDataReader::get_data_type_names(void) const { return this->myDataTypeNames; }
/* get the number of columns */
int	GeneralDataReader::get_num_columns(void) const { return this->myNumColumns; }
/* this does not clone, so if you want to keep this data, either clone the returned value or
   call make_new_storage() */
GeneralDataStorage	*GeneralDataReader::storage(void){ return this->myStorage; }

/*static*/ const GeneralDataReader::string_to_any_converter_t GeneralDataReader::myDataReaders[GeneralDataReader::myNumDataTypes] = {
	/* in the same order as the enum data_type_t */
//	&GeneralDataReader::default_string_to_any_converter_INT,
//	&GeneralDataReader::default_string_to_any_converter_FLOAT,
	&GeneralDataReader::default_string_to_any_converter_DOUBLE,
	&GeneralDataReader::default_string_to_any_converter_STRING
};
/*static*/ const GeneralDataReader::any_to_string_converter_t GeneralDataReader::myDataToStringConverters[GeneralDataReader::myNumDataTypes] = {
	/* in the same order as the enum data_type_t */
//	&GeneralDataReader::default_any_to_string_converter_INT,
//	&GeneralDataReader::default_any_to_string_converter_FLOAT,
	&GeneralDataReader::default_any_to_string_converter_DOUBLE,
	&GeneralDataReader::default_any_to_string_converter_STRING
};
/* the input parameter will be changed by this, you still need to free it if you allocated it, but it will be like swiss cheese */
void	GeneralDataReader::parse(char *line_of_text){
	char	*savePtr, *a_token;
	int	col_num, used_col_num;
	char	*sep = this->mySeparator;
	int	max_col = this->myMaxColumnNumberSpecified;
	for(a_token=GeneralDataReader::delimiter_string_strtok_r(line_of_text,sep,this->myStrlenSeparator,&savePtr),col_num=0,used_col_num=0;
		a_token&&(col_num<=max_col);a_token=GeneralDataReader::delimiter_string_strtok_r(NULL,this->mySeparator,this->myStrlenSeparator,&savePtr),col_num++){
		//printf("found token: '%s' with separator: '%s'\n", a_token, this->mySeparator);
		if( this->myColumnsBitmap->at(col_num) == false ){ continue; } /* not in the list of columns we are interested */
		if( *a_token == '\0' ){
			a_token = this->myDefaultValues[col_num];
#ifdef DATA_FILE_CONSISTENCY_CHECKS
			printf("token is empty, using default value '%s'\n", a_token);
#endif
		}
		//printf("i am inserting the token: %s of column %d, num_col=%d, type %d\n", a_token, this->myColumns[used_col_num], used_col_num, this->myDataTypeNames[this->myColumns[used_col_num]]);
		/* this a_token will be strdup in the reader function if a char*, if number then no */
		GeneralDataReader::myDataReaders[this->myDataTypeNames[used_col_num]](a_token, this->myStorage);
		//char *res =  myDataToStringConverters[this->myDataTypeNames[used_col_num]](this->myStorage->get_last()); printf("the result is : '%s'\n", res); free(res);
		//printf("done inserting\n");
		used_col_num++;
	}
//	fprintf(stderr, "IN HERE 0=%s, 1=%d\n", this->myStorage->get_string(0), this->myStorage->get_int(3));
}
std::string	GeneralDataReader::toString(void){
	int	i, x;
	std::string	ret("");
	for(i=0;i<this->myNumColumns;i++){
		ret += GeneralDataReader::myDataToStringConverters[this->myDataTypeNames[i]](this->myStorage->get(i)) + ',';
	}
	if( (x=ret.size()) > 0 ){ ret.resize(x-1); }
	return ret;
}
/* my version of strtok to work with a string delimiter not a string of delimiter characters matching any one of them,
   the difference between gnulibc's is the use of strpbrk
*/
/*static*/char *	GeneralDataReader::delimiter_string_strtok_r (char *s, const char *delim, const int strlen_delim, char **save_ptr){
        char *token;

        if( s == NULL ){
                /* re-entry */
                s = *save_ptr;
        }
//	printf("entering s='%s'\n", s);        
        if( (s==NULL) || (*s == '\0') ) return NULL;
        
        token = s;
        s = strstr(s, delim); /* returns the begginning of where it found delim, so we need to advance by strlen(delim)-1, see below */

        if( s == NULL ){
                /* This token finishes the string - can't find any delimiter in there */
                *save_ptr = strchr(token, '\0');
        } else {
                *s = '\0';
		s+=strlen_delim-1;
                *s = '\0';
                *save_ptr = s + 1;
        }
        return token;
}
/* this is a copy of the above excpet for the strlen(delim) */
/*static*/char *	GeneralDataReader::delimiter_string_strtok_r (char *s, const char *delim, char **save_ptr){
        char *token;

        if( s == NULL ){
                /* re-entry */
                s = *save_ptr;
        }
//	printf("entering s='%s'\n", s);        
        if( (s==NULL) || (*s == '\0') ) return NULL;
        
        token = s;
        s = strstr(s, delim); /* returns the begginning of where it found delim, so we need to advance by strlen(delim)-1, see below */

        if( s == NULL ){
                /* This token finishes the string - can't find any delimiter in there */
                *save_ptr = strchr(token, '\0');
        } else {
                *s = '\0';
		s+=strlen(delim)-1;
                *s = '\0';
                *save_ptr = s + 1;
        }
        return token;
}

