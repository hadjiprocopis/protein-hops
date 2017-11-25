/*
Program by Andreas Hadjiprocopis (ICR at the time)
contact: andreashad2@gmail.com
The program is free to distribute and use but please
attribute the orginal source and author.
*/

/* this is tailored-made for STRING-DB v9 (and higher) files */
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

#include <getopt.h>

#include "graphos.hpp"

#define	graphos_enquiry_for_STRING_VERSION "version: 1.0.0"

#define	MAX_CHARS_PER_LINE_OF_SEARCH_FILE	10000

/* STRING-DB file format specifics: */
#define	DEFAULT_NODEDATA_FILE_FS	"," /* separates items in a node data */
#define	DEFAULT_ACTIONS_FILE_FS	"," /* separates items in an actions data */
#define	DEFAULT_LINKS_FILE_FS	"," /* separates items in a links data */
#define DEFAULT_LINKS_FILE_HEADER_LINES	1 /* number of header lines to ignore at the top of file */
#define	DEFAULT_LINKS_FILE_FROM_PROTEIN_NAME_COLUMN 0 /* which column is the 'from' protein? */
#define	DEFAULT_LINKS_FILE_TO_PROTEIN_NAME_COLUMN 1 /* which column is the 'to' protein? */

#define DEFAULT_ACTIONS_FILE_HEADER_LINES	1 /* number of header lines to ignore at the top of file */
#define	DEFAULT_ACTIONS_FILE_FROM_PROTEIN_NAME_COLUMN 0 /* which column is the 'from' protein? */
#define	DEFAULT_ACTIONS_FILE_TO_PROTEIN_NAME_COLUMN 1 /* which column is the 'to' protein? */
#define DEFAULT_NODEDATA_FILE_HEADER_LINES	1 /* number of header lines to ignore at the top of file */
#define	DEFAULT_NODEDATA_FILE_PROTEIN_NAME_COLUMN 0	/* which column is the protein? */
#define DEFAULT_SEARCH_FILE_HEADER_LINES	0 /* number of header lines to ignore at the top of file */

#define	DEFAULT_NODE_2_NODE_DATA_FS	"|" /* separates consecutive node data batches */
#define	DEFAULT_LINKS_2_LINKS_DATA_FS	"|" /* separates link / actions data for each edge */
#define	DEFAULT_INPUT_FILE_FS " "
#define	DEFAULT_OUTPUT_FILE_FS "\t"
#define	DEFAULT_SEARCH_FILE_FS "\t"
#define	DEFAULT_SEARCH_FILE_HEADER_LINES	0
#define	COMMAND_LINE_SEARCH_IFS ","	/* when specifying search from command line use this separator, e.g. --search "from,to" */

/* this refers to STRING-DB file : protein.links.detailed.v9.0.txt */
#define	DEFAULT_LINKS_FILE_NUM_COLUMNS_TO_READ 10 /* <-- the following [] arrays have this size */
static	const int	DEFAULT_LINKS_FILE_COLUMNS_TO_READ[] = {0,1,2,3,4,5,6,7,8,9};
static	const GeneralDataReader::data_type_t DEFAULT_LINKS_FILE_DATATYPE_PER_COLUMN[] = {
	/* from, to names */ GeneralDataReader::STRING, GeneralDataReader::STRING,
	/* 7 scores, 1 combined score */ GeneralDataReader::DOUBLE,GeneralDataReader::DOUBLE,GeneralDataReader::DOUBLE,GeneralDataReader::DOUBLE,GeneralDataReader::DOUBLE,GeneralDataReader::DOUBLE,GeneralDataReader::DOUBLE,GeneralDataReader::DOUBLE
};
static	const char	*DEFAULT_LINKS_FILE_DEFAULT_VALUE_IF_ITEM_IS_EMPTY[DEFAULT_LINKS_FILE_NUM_COLUMNS_TO_READ] = {
	"NA", "NA", "0", "0", "0", "0", "0", "0", "0", "0"
};
/* this refers to STRING-DB file : 9606.protein.actions.detailed.vlatest.txt */
#define	DEFAULT_ACTIONS_FILE_NUM_COLUMNS_TO_READ 8 /* <-- the following [] arrays have this size */
static	const int	DEFAULT_ACTIONS_FILE_COLUMNS_TO_READ[] = {0,1,2,3,4,5,6,7};
static	const GeneralDataReader::data_type_t DEFAULT_ACTIONS_FILE_DATATYPE_PER_COLUMN[] = {
	/* from, to names */ GeneralDataReader::STRING, GeneralDataReader::STRING,
	/* mode */ GeneralDataReader::STRING, /*(e.g. binding)*/
	/* action */ GeneralDataReader::STRING, /* (e.g. inhibition or empty)*/
	/* a_is_acting */ GeneralDataReader::DOUBLE, /* (0 or 1 for the first node acting on the second) */
	/* score */ GeneralDataReader::DOUBLE, /* combined score as given also in the Links file */
	/* sources */ GeneralDataReader::STRING,
	/* transferred_sources */ GeneralDataReader::STRING
};
static	const char	*DEFAULT_ACTIONS_FILE_DEFAULT_VALUE_IF_ITEM_IS_EMPTY[DEFAULT_ACTIONS_FILE_NUM_COLUMNS_TO_READ] = {
	"NA", "NA", "NA",  "NA", "0", "0", "NA", "NA",
};
/* this refers to a generic node data file which has the nodename followed by a double value */
#define	DEFAULT_NODEDATA_FILE_NUM_COLUMNS_TO_READ 2 /* <-- the following [] arrays have this size */
static	const int	DEFAULT_NODEDATA_FILE_COLUMNS_TO_READ[] = {0,1};
static	const GeneralDataReader::data_type_t DEFAULT_NODEDATA_FILE_DATATYPE_PER_COLUMN[] = {
	/* from, to names */ GeneralDataReader::STRING, GeneralDataReader::DOUBLE
};
static	const char	*DEFAULT_NODEDATA_FILE_DEFAULT_VALUE_IF_ITEM_IS_EMPTY[DEFAULT_NODEDATA_FILE_NUM_COLUMNS_TO_READ] = {
	"NA", "0"
};
/* END * STRING-DB file format specifics */

bool	a_path_calculator(Graphos *g, GraphosVectorPaths::path_t *the_path, GraphosVectorPaths::path_data_t *result);
bool	a_path_toString_printer(Graphos *g, GraphosVectorPaths::path_t *the_path, GraphosVectorPaths::path_data_t *the_path_data, const char *ofs, std::string *result);

void	Usage(FILE *fh, char *app_name);

struct filename_and_ifs_combo_t {
	char	*filename,
		*ifs;
	int	skip_header_lines;
	const	GeneralDataReader::data_type_t	*data_types;
	const	int *columns_to_read;
	int	num_columns_to_read;
	int	from_column_number, to_column_number; /* these may or may not apply to all, if just 1 col (e.g. node data), then use from_column_number */
	const char **default_values;
	filename_and_ifs_combo_t(){ filename = NULL; ifs = NULL; skip_header_lines = 0; from_column_number = 0; to_column_number = 1;}
	~filename_and_ifs_combo_t(){ if(filename!=NULL){ free(filename); } if(ifs!=NULL){ free(ifs); } }
};

int	main(int argc, char **argv){
	std::vector<filename_and_ifs_combo_t*>	actions_data_files, links_data_files,
						nodes_data_files, search_data_files;
	filename_and_ifs_combo_t	*input_data_file = new filename_and_ifs_combo_t(),
					*output_basespecs = new filename_and_ifs_combo_t(),
					*last_filecombo_inserted = NULL, *filecombo_to_insert = NULL;
	std::vector<filename_and_ifs_combo_t*>::size_type	ui;
	int	sp, num_search_patterns;
	GeneralCPUTimeMonitor	myMonitorTotal, myMonitor;

	myMonitorTotal.reset();

	char	*from = NULL, *to = NULL,
		*a_p, *dummy;
	int	min_num_hops = 1, max_num_hops = 1,
		chars_per_line_of_search_file = MAX_CHARS_PER_LINE_OF_SEARCH_FILE;
	std::vector<char *>	*search_patterns_from = new std::vector<char *>(),
				*search_patterns_to = new std::vector<char *>();
	bool	make_transitive = false;

	static	struct	option	long_options[] = {
		{"input", required_argument, 0, 'i'},
		{"output", required_argument, 0, 'o'},
		{"search", required_argument, 0, 's'},
		{"search_file", required_argument, 0, 'f'},
		{"min_num_hops", required_argument, 0, 'm'},
		{"max_num_hops", required_argument, 0, 'M'},
		{"skip_header_lines", required_argument, 0, 'H'},
		{"from_column_number", required_argument, 0, 'F'},
		{"to_column_number", required_argument, 0, 'T'},
		{"ifs", required_argument, 0, 'I'},
		{"make_transitive", no_argument, 0, 'R'},
		{"actions_data_file", required_argument, 0, 'a'},
		{"links_data_file", required_argument, 0, 'l'},
		{"nodes_data_file", required_argument, 0, 'n'},
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'},
		{0, 0, 0, 0}
	};
	int	c, option_index;
	while(1){
		option_index = 0;
		c = getopt_long(argc, argv, "i:o:s:f:m:M:H:F:T:I:Ra:l:n:C:hv", long_options, &option_index);
		if( c == -1 ) break;
		switch(c){
			case -1: break;
			case 'i': last_filecombo_inserted = input_data_file;
input_data_file->filename = strdup(optarg);
last_filecombo_inserted->skip_header_lines = DEFAULT_LINKS_FILE_HEADER_LINES;
last_filecombo_inserted->ifs = strdup(DEFAULT_LINKS_FILE_FS);
last_filecombo_inserted->skip_header_lines = DEFAULT_LINKS_FILE_HEADER_LINES;
last_filecombo_inserted->from_column_number = DEFAULT_LINKS_FILE_FROM_PROTEIN_NAME_COLUMN;
last_filecombo_inserted->to_column_number = DEFAULT_LINKS_FILE_TO_PROTEIN_NAME_COLUMN;
 break;
			case 'o': last_filecombo_inserted = output_basespecs;
last_filecombo_inserted->filename = strdup(optarg);
last_filecombo_inserted->ifs = strdup(DEFAULT_OUTPUT_FILE_FS);
 break;
			case 'a': last_filecombo_inserted = new filename_and_ifs_combo_t(); actions_data_files.push_back(last_filecombo_inserted);
last_filecombo_inserted->filename = strdup(optarg);
last_filecombo_inserted->default_values = DEFAULT_ACTIONS_FILE_DEFAULT_VALUE_IF_ITEM_IS_EMPTY;
last_filecombo_inserted->num_columns_to_read = DEFAULT_ACTIONS_FILE_NUM_COLUMNS_TO_READ;
last_filecombo_inserted->columns_to_read = DEFAULT_ACTIONS_FILE_COLUMNS_TO_READ;
last_filecombo_inserted->data_types = DEFAULT_ACTIONS_FILE_DATATYPE_PER_COLUMN;
last_filecombo_inserted->skip_header_lines = DEFAULT_ACTIONS_FILE_HEADER_LINES;
last_filecombo_inserted->ifs = strdup(DEFAULT_ACTIONS_FILE_FS);
last_filecombo_inserted->from_column_number = DEFAULT_ACTIONS_FILE_FROM_PROTEIN_NAME_COLUMN;
last_filecombo_inserted->to_column_number = DEFAULT_ACTIONS_FILE_TO_PROTEIN_NAME_COLUMN;
 break;
			case 'l': last_filecombo_inserted = new filename_and_ifs_combo_t(); links_data_files.push_back(last_filecombo_inserted);
last_filecombo_inserted->filename = strdup(optarg);
last_filecombo_inserted->default_values = DEFAULT_LINKS_FILE_DEFAULT_VALUE_IF_ITEM_IS_EMPTY;
last_filecombo_inserted->num_columns_to_read = DEFAULT_LINKS_FILE_NUM_COLUMNS_TO_READ;
last_filecombo_inserted->columns_to_read = DEFAULT_LINKS_FILE_COLUMNS_TO_READ;
last_filecombo_inserted->data_types = DEFAULT_LINKS_FILE_DATATYPE_PER_COLUMN;
last_filecombo_inserted->skip_header_lines = DEFAULT_LINKS_FILE_HEADER_LINES;
last_filecombo_inserted->ifs = strdup(DEFAULT_LINKS_FILE_FS);
last_filecombo_inserted->from_column_number = DEFAULT_LINKS_FILE_FROM_PROTEIN_NAME_COLUMN;
last_filecombo_inserted->to_column_number = DEFAULT_LINKS_FILE_TO_PROTEIN_NAME_COLUMN;
 break;
			case 'n': last_filecombo_inserted = new filename_and_ifs_combo_t(); nodes_data_files.push_back(last_filecombo_inserted);
last_filecombo_inserted->filename = strdup(optarg);
last_filecombo_inserted->default_values = DEFAULT_NODEDATA_FILE_DEFAULT_VALUE_IF_ITEM_IS_EMPTY;
last_filecombo_inserted->num_columns_to_read = DEFAULT_NODEDATA_FILE_NUM_COLUMNS_TO_READ;
last_filecombo_inserted->columns_to_read = DEFAULT_NODEDATA_FILE_COLUMNS_TO_READ;
last_filecombo_inserted->data_types = DEFAULT_NODEDATA_FILE_DATATYPE_PER_COLUMN;
last_filecombo_inserted->skip_header_lines = DEFAULT_NODEDATA_FILE_HEADER_LINES;
last_filecombo_inserted->ifs = strdup(DEFAULT_NODEDATA_FILE_FS);
last_filecombo_inserted->from_column_number = DEFAULT_NODEDATA_FILE_PROTEIN_NAME_COLUMN;
last_filecombo_inserted->to_column_number = -1; /* does not apply */
 break;
			case 'f': last_filecombo_inserted = new filename_and_ifs_combo_t(); last_filecombo_inserted->filename = strdup(optarg); search_data_files.push_back(last_filecombo_inserted);
last_filecombo_inserted->skip_header_lines = DEFAULT_SEARCH_FILE_HEADER_LINES;
last_filecombo_inserted->ifs = strdup(DEFAULT_SEARCH_FILE_FS);
			case 'I': if( last_filecombo_inserted == NULL ){ fprintf(stderr, "%s : prior to specifying an IFS, a filename must be specified. The idea is that each filename has its own IFS and user specifies one filename and then its ifs, then another filename and its ifs, etc.\n", argv[0]); Usage(stderr, argv[0]); exit(1); } last_filecombo_inserted->ifs = strdup(optarg); break;
			case 'F': if( last_filecombo_inserted == NULL ){ fprintf(stderr, "%s : prior to specifying a FROM column number, a filename must be specified. The idea is that each filename has its own IFS and user specifies one filename and then its ifs, then another filename and its ifs, etc.\n", argv[0]); Usage(stderr, argv[0]); exit(1); } last_filecombo_inserted->from_column_number = atoi(optarg); break;
			case 'T': if( last_filecombo_inserted == NULL ){ fprintf(stderr, "%s : prior to specifying a TO column number, a filename must be specified. The idea is that each filename has its own IFS and user specifies one filename and then its ifs, then another filename and its ifs, etc.\n", argv[0]); Usage(stderr, argv[0]); exit(1); } last_filecombo_inserted->to_column_number = atoi(optarg); break;
			case 'H': if( last_filecombo_inserted == NULL ){ fprintf(stderr, "%s : prior to specifying a 'skip_header_lines', a filename must be specified. The idea is that each filename has its own IFS and user specifies one filename and then its ifs, then another filename and its ifs, etc.\n", argv[0]); Usage(stderr, argv[0]); exit(1); } last_filecombo_inserted->skip_header_lines = atoi(optarg); break;
			case 'm': min_num_hops = atoi(optarg); break;
			case 'M': max_num_hops = atoi(optarg); break;
			case 's': dummy = strdup(optarg); if( (a_p=strstr(dummy, COMMAND_LINE_SEARCH_IFS)) == NULL ){ fprintf(stderr, "%s : invalid search pattern '%s'. It must consist of a 'from' and a 'to' string separated by the search IFS character or string, currently the default is '%s'.\n", argv[0], dummy, COMMAND_LINE_SEARCH_IFS); Usage(stderr, argv[0]); exit(1); }
				  *a_p = '\0'; search_patterns_from->push_back(dummy); a_p += strlen(COMMAND_LINE_SEARCH_IFS); search_patterns_to->push_back(strdup(a_p));
				  break;
			case 'C': chars_per_line_of_search_file = atoi(optarg); break;
			case 'R': make_transitive = true; break;
			case '?':
			case 'h': Usage(stderr, argv[0]); exit(0);
			case 'v': printf("%s\n", graphos_enquiry_for_STRING_VERSION); exit(0);
			default : Usage(stderr, argv[0]); fprintf(stderr, "%s : unknown option -%c.\n", argv[0], c); exit(1);
		}
	}
	if (optind < argc){
		fprintf(stderr, "%s : these options were not recognised: ", argv[0]);
		while (optind < argc) fprintf(stderr, "%s ", argv[optind++]);
		fprintf(stderr, "\n");
		exit(1);
	}

	if( input_data_file->filename == NULL ){ Usage(stderr, argv[0]); fprintf(stderr, "%s : an input edgelist file (--input) must be specified.\n", argv[0]); exit(1); }
	if( output_basespecs->filename == NULL ){ Usage(stderr, argv[0]); fprintf(stderr, "%s : an output basename (--output) must be specified.\n", argv[0]); exit(1); }
	if( (min_num_hops > max_num_hops) || (min_num_hops<0) || (max_num_hops<0) ){ Usage(stderr, argv[0]); fprintf(stderr, "%s : invalid min and max num hops combination (min:%d, max:%d), must be non-negative and min <= max.\n", argv[0], min_num_hops, max_num_hops); exit(1); }

	if( input_data_file->ifs == NULL ){ input_data_file->ifs = strdup(DEFAULT_INPUT_FILE_FS); }
	if( output_basespecs->ifs == NULL ){ output_basespecs->ifs = strdup(DEFAULT_OUTPUT_FILE_FS); }
	for(ui=0;ui<actions_data_files.size();ui++){ if( actions_data_files.at(ui)->ifs == NULL ){ fprintf(stderr, "%s : actions data file '%s' has not its IFS specified, use --ifs to specify this after the --actions_data_file parameter.\n", argv[0], actions_data_files.at(ui)->filename); Usage(stderr, argv[0]); exit(1); } }
	for(ui=0;ui<links_data_files.size();ui++){ if( links_data_files.at(ui)->ifs == NULL ){ fprintf(stderr, "%s : links data file '%s' has not its IFS specified, use --ifs to specify this after the --links_data_file parameter.\n", argv[0], links_data_files.at(ui)->filename); Usage(stderr, argv[0]); exit(1); } }
	for(ui=0;ui<nodes_data_files.size();ui++){ if( nodes_data_files.at(ui)->ifs == NULL ){ fprintf(stderr, "%s : nodes data file '%s' has not its IFS specified, use --ifs to specify this after the --nodes_data_file parameter.\n", argv[0], nodes_data_files.at(ui)->filename); Usage(stderr, argv[0]); exit(1); } }

	for(ui=0;ui<search_data_files.size();ui++){
		filename_and_ifs_combo_t *a_combo = search_data_files.at(ui);
		char	*a_filename = a_combo->filename;
		if( a_combo->ifs == NULL ){ a_combo->ifs = strdup(DEFAULT_SEARCH_FILE_FS); }
		char	*a_ifs =  a_combo->ifs;
		char	*buffer, *pBuffer;
		FILE	*a_filehandle;
		if( (buffer=(char *)malloc((chars_per_line_of_search_file+1)*sizeof(char))) == NULL ){ fprintf(stderr, "%s : could not allocate %zd bytes for buffer.\n", argv[0], (chars_per_line_of_search_file+1)*sizeof(char)); perror(NULL); exit(1); }
		if( (a_filehandle=fopen(a_filename, "r")) == NULL ){ fprintf(stderr, "%s : could not open file with search patterns '%s' for reading.\n", argv[0], a_filename); perror(NULL); exit(1); }
		int	line_num = 0;

		while(1){
			fgets(buffer, chars_per_line_of_search_file, a_filehandle);
			if( feof(a_filehandle) ){ break; }
			line_num++;

			buffer[strlen(buffer)-1] = '\0'; /* remove the last newline */
			/* remove comments */
			for(pBuffer=&(buffer[0]);*pBuffer!='\0';pBuffer++){ if( *pBuffer=='#' ){ *pBuffer = '\0'; break; } }
			if( Graphos::IsEmptyString(buffer) ){ continue; }
			//printf("line is '%s'\n", buffer);
			if( (a_p=strstr(buffer, a_ifs)) == NULL ){ fprintf(stderr, "%s : invalid search pattern '%s' read from file '%s', line %d. It must consist of a 'from' and a 'to' string separated by the search IFS character or string, currently this is '%s' *IMPORTANT: in order to specify the ifs for each file, use the --ifs option after the option specifying the filename, e.g. --search_file 'abc' --ifs 'aaa'.\n", argv[0], buffer, a_filename, line_num, a_ifs); Usage(stderr, argv[0]); exit(1); }
			*a_p = '\0'; search_patterns_from->push_back(strdup(buffer));
			//printf("then '%s'\n", buffer);
			a_p += strlen(a_ifs);
			//printf("and '%s'\n", a_p);
			search_patterns_to->push_back(strdup(a_p)); /* and we free it at the end */
		}
		free(buffer);
	}
	if( (num_search_patterns=search_patterns_from->size()) == 0 ){ fprintf(stderr, "%s : could not read any valid search pattern from either a file or the command line. Use --search and/or --search_file to specify at least one pattern.\n", argv[0]); exit(1); }

//	char	infilename[] = "/Users/Andreas/data/STRING/9606.protein.links.v9.0.txt";
//	char	infilename[] = "9606.protein.links.v9.0.txt";
//	char	infilename[] = "small.txt";
//	string	from = "0", to = "5";
//	string	from = "9606.ENSP00000231004", // LOX
//		to = "9606.ENSP00000312455"; // CFLAR
//	string	from = "9606.ENSP00000000233", // random
//		to = "9606.ENSP00000058691"; // random

	Graphos	*my_graphos = new Graphos(make_transitive);

	int	linenum;

	/* read the input - we will read only edge names, not the data with it */
	std::cout << argv[0] << " : reading edges from file '" << input_data_file->filename << "' ... " << std::endl;
	myMonitor.reset();
	if( (linenum=my_graphos->read_edgelist_from_file(
		input_data_file->filename,
		input_data_file->skip_header_lines, /* header lines to skip */
		input_data_file->from_column_number,
		input_data_file->to_column_number,
		input_data_file->ifs /* a char or string separator (no regex, no list of alternative separators, just one separator) */
	)) <= 0 ){ fprintf(stderr, "%s : call to read_edgelist_from_file has failed for file '%s'.\n", (char * )argv[0], input_data_file->filename); exit(1); }
	fprintf(stdout, "done, read %d lines (including header & empty), ", linenum); myMonitor.record(); myMonitor.print(stdout); fprintf(stdout, "\n");

	for(ui=0;ui<links_data_files.size();ui++){
		filename_and_ifs_combo_t	*a_combo = links_data_files.at(ui);
		char	*a_filename = a_combo->filename;
		char	*a_ifs = a_combo->ifs;
		fprintf(stdout, "%s : reading links data from file '%s'...\n", argv[0], a_filename);
		myMonitor.reset();
		if( (linenum=my_graphos->read_edgedata_from_file(
			a_filename,
			a_combo->skip_header_lines, /* header lines to skip */
			a_combo->from_column_number,
			a_combo->to_column_number,
			a_combo->num_columns_to_read,
			a_combo->columns_to_read,
			a_combo->data_types,
			a_combo->default_values,
			a_ifs /* a char or string separator (no regex, no list of alternative separators, just one separator) */
		)) <= 0 ){ fprintf(stderr, "%s : call to read_edgedata_from_file has failed.\n", (char * )argv[0]); exit(1); }
		fprintf(stdout, "done, read %d lines of links data (including header & empty), ", linenum); myMonitor.record(); myMonitor.print(stdout); fprintf(stdout, "\n");
		//my_graphos->print_node_data(stdout);
	}

	for(ui=0;ui<actions_data_files.size();ui++){
		filename_and_ifs_combo_t	*a_combo = actions_data_files.at(ui);
		char	*a_filename = a_combo->filename;
		char	*a_ifs = a_combo->ifs;
		fprintf(stdout, "%s : reading actions data from file '%s'...\n", argv[0], a_filename);
		myMonitor.reset();
		if( (linenum=my_graphos->read_edgedata_from_file(
			a_filename,
			a_combo->skip_header_lines, /* header lines to skip */
			a_combo->from_column_number,
			a_combo->to_column_number,
			a_combo->num_columns_to_read,
			a_combo->columns_to_read,
			a_combo->data_types,
			a_combo->default_values,
			a_ifs /* a char or string separator (no regex, no list of alternative separators, just one separator) */
		)) <= 0 ){ fprintf(stderr, "%s : call to read_edgedata_from_file has failed.\n", (char * )argv[0]); exit(1); }
		fprintf(stdout, "done, read %d lines of actions data (including header & empty), ", linenum); myMonitor.record(); myMonitor.print(stdout); fprintf(stdout, "\n");
		//my_graphos->print_node_data(stdout);
	}
	for(ui=0;ui<nodes_data_files.size();ui++){
		filename_and_ifs_combo_t	*a_combo = nodes_data_files.at(ui);
		char	*a_filename = a_combo->filename;
		char	*a_ifs = a_combo->ifs;
		fprintf(stdout, "%s : reading node data from file '%s'...\n", argv[0], a_filename);
		myMonitor.reset();
		if( (linenum=my_graphos->read_nodedata_from_file(
			a_filename,
			a_combo->skip_header_lines, /* header lines to skip */
			a_combo->from_column_number,
			a_combo->num_columns_to_read,
			a_combo->columns_to_read,
			a_combo->data_types,
			a_combo->default_values,
			a_ifs /* a char or string separator (no regex, no list of alternative separators, just one separator) */
		)) <= 0 ){ fprintf(stderr, "%s : call to read_nodedata_from_file has failed.\n", (char * )argv[0]); exit(1); }
		fprintf(stdout, "done, read %d lines of node data (including header & empty), \n", linenum); myMonitor.record(); myMonitor.print(stdout); fprintf(stdout, "\n");
		//my_graphos->print_node_data(stdout);
	}

	int	from_id, to_id, num_paths_found, num_searches_failed = 0;
	char	*new_outbasename;
	for(sp=0;sp<num_search_patterns;sp++){
		from = (*search_patterns_from)[sp]; to = (*search_patterns_to)[sp];
		fprintf(stderr, "%s : %d of %d) recursive search %s -> %s, min num hops: %d, max num hops: %d\n", argv[0], (sp+1), num_search_patterns, from, to, min_num_hops, max_num_hops);
		GraphosVectorPaths	*results_paths = new GraphosVectorPaths();
		GraphosTreePaths	*results_tree = NULL;
		asprintf(&new_outbasename, "%s.%s.%s", output_basespecs->filename, from, to);
		myMonitor.reset();
		if( (num_paths_found=my_graphos->find_paths(
			from, to,
			min_num_hops, max_num_hops,
			&results_tree, results_paths,
			&from_id, &to_id
		)) == -1 ){
			fprintf(stderr, "%s : %d of %d) call to find_paths has failed for '%s' -> '%s' for min hops:%d and max hops:%d. Skippping this ...\n", argv[0], sp+1, num_search_patterns, from, to, min_num_hops, max_num_hops);
			if( results_tree != NULL ){ delete results_tree; }
			free(new_outbasename);
			delete results_paths; if( results_tree != NULL ) delete results_tree;
//			BOOST_FOREACH(path_t *vv, *results_paths){ delete vv; } delete results_paths;
			num_searches_failed++;
			continue;
		} else if( num_paths_found == 0 ){
			fprintf(stderr, "%s : %d of %d) no paths found for the criteria specified, ", argv[0], sp+1, num_search_patterns);
			myMonitor.record(); myMonitor.print(stdout); fprintf(stdout, "\n\n");
			free(new_outbasename); delete results_paths; if( results_tree != NULL ) delete results_tree;
			num_searches_failed++;
			continue;
		}
		fprintf(stdout, "%s : %d of %d) found %d paths with criteria specified, ", argv[0], sp+1, num_search_patterns, num_paths_found);
		myMonitor.record(); myMonitor.print(stdout); fprintf(stdout, "\n\n");

		fprintf(stdout, "%s : doing calculations on the paths ... ", argv[0]);
		myMonitor.reset();
		results_paths->calculate_paths_data(my_graphos, &a_path_calculator);
		myMonitor.record(); myMonitor.print(stdout); fprintf(stdout, "\n\n"); 

		fprintf(stdout, "%s : %d of %d) saving paths to files (%s.*) ...\n", argv[0], sp+1, num_search_patterns, new_outbasename);
		myMonitor.reset();
		if( my_graphos->write_all_data_to_files(
			new_outbasename, output_basespecs->ifs,
			min_num_hops, max_num_hops, 
			&a_path_toString_printer,
			results_paths
		   ) == false ){
			fprintf(stderr, "%s : %d of %d) call to write_all_data_files() has failed for '%s' -> '%s' for min hops:%d and max hops:%d (outbasename: '%s'). Skipping this...\n", argv[0], sp+1, num_search_patterns, from, to, min_num_hops, max_num_hops, new_outbasename);
			if( results_tree != NULL ){ delete results_tree; }
			continue;
		}
		myMonitor.record(); myMonitor.print(stdout); fprintf(stdout, "\n");

		free(new_outbasename);
		delete results_paths;
		if( results_tree != NULL ) delete results_tree;
	}

	delete my_graphos;
	BOOST_FOREACH(char *vv, *search_patterns_from){ free(vv); } delete search_patterns_from;
	BOOST_FOREACH(char *vv, *search_patterns_to){ free(vv); } delete search_patterns_to;

	BOOST_FOREACH(filename_and_ifs_combo_t *vv, actions_data_files){ delete(vv); }
	BOOST_FOREACH(filename_and_ifs_combo_t *vv, search_data_files){ delete(vv); }
	BOOST_FOREACH(filename_and_ifs_combo_t *vv, nodes_data_files){ delete(vv); }
	BOOST_FOREACH(filename_and_ifs_combo_t *vv, links_data_files){ delete(vv); }
	delete input_data_file; delete output_basespecs;
	
	fprintf(stdout, "%s : done, %d (%.1lf%%) searches failed, %d (%.1lf%%) searches succeeded, of a total of %d.\n", argv[0], num_searches_failed, (100.0*num_searches_failed)/num_search_patterns, (num_search_patterns-num_searches_failed), (100.0*(num_search_patterns-num_searches_failed)/num_search_patterns), num_search_patterns);
	myMonitorTotal.record(); myMonitorTotal.print(stdout); fprintf(stdout, "\n");
//	sleep(10000);
}
bool	a_path_calculator(Graphos *g, GraphosVectorPaths::path_t *the_path, GeneralDataStorage *result){
	double	res = 1.0;
	int	s = the_path->size(), i;
	int	current_node_id = the_path->at(0), next_node_id;
	//Graphos::graphos_node_storage_t	*current_node_data = g->get_node_data(current_node_id);
	//Graphos::graphos_node_storage_t	*next_node_data;
	Graphos::graphos_edge_storage_t	*edge_data;

	for(i=1;i<s;i++){
		next_node_id = the_path->at(i);
//		next_node_data =  g->get_node_data(next_node_id);
		edge_data = g->get_edge_data(current_node_id, next_node_id);
		//printf("getting edge data %d -> %d = %p\n", current_node_id, next_node_id, edge_data);
		if( edge_data != NULL ){
			res *= edge_data->get_as<double>(9) / 1000.0; /* combined string score - this is application specific */
		}
		current_node_id = next_node_id;
//		current_node_data = next_node_data;
	}
	result->put(res);
	return true;
}
bool	a_path_toString_printer(Graphos *g, GraphosVectorPaths::path_t *the_path, GraphosVectorPaths::path_data_t *the_path_data, const char *ofs, std::string *result){
	int	s = the_path->size(), i;
	int	current_node_id = the_path->at(0), next_node_id;
	Graphos::graphos_node_storage_t	*current_node_data = g->get_node_data(current_node_id), /* <- this can be null too ! */
		*next_node_data;
	Graphos::graphos_edge_storage_t	*edge_data;

	char	*current_node_name = g->node_id_2_name(current_node_id),
		*next_node_name;
	static const unsigned int node_indices[] = {1}; /* 0: name of node, skip this */
	static const unsigned int edge_link_indices[] = {2,3,4,5,6,7,8,9}; /* 0,1: name of node, skip this */
	static const unsigned int edge_action_indices[] = {12,13,14,15}; /* it's appended to the link which always exists, first 2: name of node, skip this */
	/* the first node */
	char	*a_str;
	result->append(current_node_name); result->push_back('(');
	if( current_node_data != NULL ){ 
		a_str = current_node_data->toString(node_indices, 1, DEFAULT_NODEDATA_FILE_FS); result->append(a_str); free(a_str);
	}
	result->push_back(')'); result->append(ofs);

	for(i=1;i<s;i++){
		next_node_id = the_path->at(i);
		next_node_name = g->node_id_2_name(next_node_id);

		edge_data = g->get_edge_data(current_node_id, next_node_id);
		//printf("getting edge data %d -> %d = %p\n", current_node_id, next_node_id, edge_data);
		result->push_back('(');
		if( edge_data != NULL ){
			//printf("the size of data is %d\n", edge_data->size());
			a_str = edge_data->toString(edge_link_indices, 8, DEFAULT_LINKS_FILE_FS); result->append(a_str); free(a_str);
			if( edge_data->size() > 10 ){
				/* we have also actions */
				result->append(DEFAULT_LINKS_2_LINKS_DATA_FS); /* give it a pipe */
				a_str = edge_data->toString(edge_action_indices, 4, DEFAULT_ACTIONS_FILE_FS); result->append(a_str); free(a_str);
			}
		}
		result->push_back(')'); result->append(ofs);

		result->append(next_node_name);result->push_back('(');
		if( (next_node_data=g->get_node_data(next_node_id)) != NULL ){
			a_str = next_node_data->toString(node_indices, 1, DEFAULT_NODE_2_NODE_DATA_FS); result->append(a_str); free(a_str);
		}
		result->push_back(')');
		result->append(ofs);

		current_node_id = next_node_id;
		current_node_data = next_node_data;
		current_node_name = next_node_name;
	}
	/* the path may also have some data which must have been calculated before */
	result->push_back('('); a_str = the_path_data->toString(","); result->append(a_str); free(a_str); result->push_back(')');
	return true;
}

void	Usage(FILE *fh, char *app_name){
	fprintf(fh, "Proteins are associated with other proteins, somehow.\nSTRING-DB (https://string-db.org) provides\na free, public database of functional associations\nbetween proteins (for many species including human).\n\nThe data is of the form:\n    A B 123\nwhere A and B are proteins and 123 is the confidence\nthat they are functionally associated. Some of these\nassociations are transitive and some are not.\n\nFor humans (the 9606 files in string-db.org) there are\n11353057 associations (not unique proteins but pairs of\nassociated proteins).\n\nThis program reads a STRING-DB file, with the format\nA B 123\n...\n\nand then takes the names of two proteins (the 'from' and\nthe 'to') and a range of the number of hops.\nIt then proceeds to search the database as a Graph\nand report all associations which start with 'from',\nend with 'to' and have as many intermediates as the\nnumber of hops specified.\n\nFor example, running the command\n\ngraphos_enquiry_for_STRING --input /data/DATASETS/STRING_DB/9606.protein.links.v10.5.txt --ifs ' ' --skip_header_lines 1 --max_num_hops 2 --search '9606.ENSP00000000233,9606.ENSP00000301744' --output out\n\nwill report all associations between 9606.ENSP00000000233 and \n9606.ENSP00000301744 with a maximum of two hops between them,\ni.e.\n\n9606.ENSP00000000233 9606.ENSP00000301744 (the direct one if it exists)\n9606.ENSP00000000233 A 9606.ENSP00000301744 (1st degree, 1 intermediary protein 'A')\n9606.ENSP00000000233 A B 9606.ENSP00000301744 (2nd degree, 2 intermediaries 'A' and 'B')\n\nThis is version 4.2 and it is a preliminary version\nprovided as a proof-of-concept for my idea.\n\nVersion 11.0 does many many more things.\nIt does search. Very well.\n\nBut it also builds a network of\ninteractions with the protein intermediaries too.\nIt clusters this network either wrt to function or\nwith say expression if specified.\nIt then assesses the statistical\nsignificance of the clustering using monte carlo\nsimulation and t-test.\n\nV11 can be provided upon request and subsequent interviewing.\nIf successful, the candidate user will be provided with V11\nand supported.\n\nauthor (idea and implementation): Andreas Hadjiprocopis\ncontact: andreashad2@gmail.com\nwhen: at the time I was working at the Institute of Cancer Research.\nAlthough the idea and implementation are mine, I used\nICR's resources.\n");
	fprintf(fh, "Usage: %s options...\n", app_name);
	fprintf(fh, "  --input filename       The name of the file containing the input edge list. \
The format of the file is an optional number of header lines at the beginning, followed by data \
lines each of which contains a \
'from' node, a 'to' node and an optional number of parameters which are ignored. The data items \
in a data line are separated by a separator string. The number of header lines to skip \
and the IFS (separator) can be specified immediately after the '--input filename' option \
using the '--ifs IFS' and '--skip_header_lines N' options. Default IFS for the input \
is '%s' and default header lines = %d. The column \
numbers the 'from' and 'to' are, are assumed to be fixed for all data lines and can be specified \
using --from_column_number and --to_column_number (defaults are %d for 'from' and %d for 'to'). Empty \
lines are ignored. Therefore each non-header line represents an edge with required from and to and \
optional other data.\n", DEFAULT_LINKS_FILE_FS, DEFAULT_LINKS_FILE_HEADER_LINES,
DEFAULT_LINKS_FILE_FROM_PROTEIN_NAME_COLUMN, DEFAULT_LINKS_FILE_TO_PROTEIN_NAME_COLUMN);
	fprintf(fh, "  --min_num_hops min     Specify the minimum number of hops between the\
from and to nodes. All links with less hops will be ignored and not reported. The number of\
hops does not include from and to nodes. E.g. A->X->Y->B has 2 hops. and A->B has 0 hops.\n");
	fprintf(fh, "  --max_num_hops max     Specify the maximum number of hops between the\
from and to nodes. All links with more hops will be ignored and not reported. Depending\
on the number of edges in the input file, this number may explode the search space.\
For STRING-DB, 2 is a safe option. 3 may never terminate...\n");
	fprintf(fh, "  --output basename       The base name to form all output filenames. \
The field separator may be specified immediately after this parameter using --ifs (i know).\n");
	fprintf(fh, "  --search 'from%sto'     Initiate a search to find all paths between 'from'\
and 'to' nodes - the names provided are the same as in the links, input, actions files etc.\
Repeated searches are possible by adding more '-search ' options. Alternatively use a search\
patterns file (--search_file). All --search and --search_file patterns found will be merged together.\n",\
COMMAND_LINE_SEARCH_IFS);
	fprintf(fh, "  --search_file filename  A file containing 'from%sto' search patterns (without quotes).\
Empty lines are ignored, comments are allowed, comment character is '#'.\
The format of this file is again from and to pairs (in a new line) separated by the field separator.\
The field separator may be specified immediately after this parameter using --ifs (i know), the\
default is '%s'. If there are any header lines to be ignored, then either precede them by comment\
character (#) or use the '--skip_header_lines N' option immediately after this parameter (before\
or after the --ifs does not matter). The default header lines is %d.\n",
DEFAULT_SEARCH_FILE_FS, DEFAULT_SEARCH_FILE_FS, DEFAULT_SEARCH_FILE_HEADER_LINES);
	fprintf(fh, "  The following options will apply to any of the following options: --input, --links_data_file, --actions_data_file, --node_data_file, --search_file --output, immediately preceding them:\n");
	fprintf(fh, "  --skip_header_lines N  Skip the first 'N' lines from the file because they are 'header'.\n");
	fprintf(fh, "  --from_column_number F The 'from' node is in column F, first column is zero (this option may be used to specify column numbers in single node cases like node data.\n");
	fprintf(fh, "  --to_column_number T   The 'to' node is in column T, first column is zero.\n");
	fprintf(fh, "  --ifs FS               The field separator separating items of data.\n");
	fprintf(fh, "  End of the 4 options here.\n");
	fprintf(fh, "  --actions_data_file F  Specify the name of the 'actions' file (edge data like what kind of interaction this is, e.g. binding, etc.). Specify as many files as necessary.\n");
	fprintf(fh, "  --links_data_file F    Specify the links file (edge data with STRING-DB scores). Specify as many files as necessary.\n");
	fprintf(fh, "  --nodes_data_file F    Specify the links file (edge data with STRING-DB scores). Specify as many files as necessary.\n");
	fprintf(fh, "  --make_transitive      Flag to indicate that for each edge A->B, the program must create a B->A if it does not exist already. This has the effect that a search A->B to match also B->A, i.e. direction is not important. This flag will affect also edge data (e.g. the data for A->B will also be the same for B->A). No further checks are made so make sure that input files are consistent, e.g. they should contain only A->B and not both A->B,B->A, this will increase memory and if adding data will create a mess.\n");
	fprintf(fh, "\nauthor: Andreas Hadjiprocopis (andreashad2@gmai.com).\n");
}


