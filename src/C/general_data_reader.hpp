#ifndef _GENERAL_DATA_READER_HPP_
#define	_GENERAL_DATA_READER_HPP_
#include <string>
#include <vector>
#include <cassert>

#include "general_data_storage.hpp"

/* for fast access, create one such object with fixed capacity and clear() it every row of data read */
class	GeneralDataReader {
	public:
//		typedef			bool (GeneralDataReader::*string_to_any_converter_t)(char *); /* pointer to member function: may be slower but needs not passing the object as param */
		typedef			bool (*string_to_any_converter_t)(char *, GeneralDataStorage *);
		typedef			char * (*any_to_string_converter_t)(GeneralDataStorage::contents_t *);
		enum data_type_t {
//			INT=0, FLOAT=1, DOUBLE=2, STRING=3 /* don't forget to change numdata_type_ts below to the size of this enum (minus the UNKNOWN) */
			DOUBLE=0, STRING=1 /* this is all we need really */
		};
		static const int	myNumDataTypes = 2; /* size of the data_type_t enum (minus the UNKNOWN) */
//		static bool		default_string_to_any_converter_INT(char *, GeneralDataStorage *);
//		static bool		default_string_to_any_converter_FLOAT(char *, GeneralDataStorage *);
		static bool		default_string_to_any_converter_DOUBLE(char *, GeneralDataStorage *);
		static bool		default_string_to_any_converter_STRING(char *, GeneralDataStorage *);
//		static char		*default_any_to_string_converter_INT(void *);
//		static char		*default_any_to_string_converter_FLOAT(void *);
		static char		*default_any_to_string_converter_DOUBLE(GeneralDataStorage::contents_t *d);
		static char		*default_any_to_string_converter_STRING(GeneralDataStorage::contents_t *d);

	private:
		GeneralDataStorage	*myStorage;
		int			myNumColumns, *myColumns, myMaxColumnNumberSpecified, myStrlenSeparator;
		data_type_t	*myDataTypeNames;
		char		*mySeparator;
		typedef std::vector<bool>	bitmap_t;
		bitmap_t		*myColumnsBitmap;
		static	const string_to_any_converter_t	myDataReaders[myNumDataTypes]; /* can't initialise array in class definition (but you can do functions! silly language) */
		static	const any_to_string_converter_t	myDataToStringConverters[myNumDataTypes]; /* can't initialise array in class definition (but you can do functions! silly language) */
	public:
		GeneralDataReader(int num_colums, const int *the_columns, const data_type_t *the_data_type_names, const char **the_default_values, const char *the_separator);
		GeneralDataReader(GeneralDataReader const&G);
		~GeneralDataReader();

		/* specifies the spec, allocates memory for data structures etc. */
		void    reset(
			int num_colums,
			const int *the_columns,
			const data_type_t    *the_data_type_names,
			const char	**default_values,
			const char    *the_separator
		);
		void	columns(const int *);
		void	data_type_names(const data_type_t *);
		void	separator(const char *);
		int	*get_columns(void) const;
		data_type_t *get_data_type_names(void) const;
		char	*get_separator(void) const;
		int	get_num_columns(void) const;
		char	**get_default_values(void) const;

		std::string	toString(void);
		char	*toString(GeneralDataStorage::contents_t *);
		char	*toString(int);

		GeneralDataStorage	*storage(void);
		void	make_new_storage(void);

		void	parse(char *line_of_text);

		/* my version of strtok to work with a string delimiter not a string of delimiter characters matching any one of them,
		   the difference between gnulibc's is the use of strpbrk. same usage
		*/
		static	char *delimiter_string_strtok_r (char *s, const char *delim, char **save_ptr);
		static	char *delimiter_string_strtok_r (char *s, const char *delim, const int strlen_delim, char **save_ptr);
	private:
		void	_delete_me(void); /* this is what the destructor (and reset) calls - all the delete code is here */
		void	init(int);
		char	**myDefaultValues; /* if token for col x is \0, then use the corresponding default value (which is a char *) as token */
};
#endif
