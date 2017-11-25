#ifndef _GRAPHOS_VECTOR_PATHS_HPP_
#define	_GRAPHOS_VECTOR_PATHS_HPP_
#include <vector>
#include <cassert>

#include "general_data_storage.hpp"
class	GeneralDataStorage;
class	Graphos;

class	GraphosVectorPaths {
	public:
		/* the int here is the internal id given to each node i the Graphos class during reading */
		typedef	std::vector<int>	path_t;
		typedef	path_t	*path_t_p;
		typedef	GeneralDataStorage	path_data_t;
		typedef	std::vector<path_data_t *>	paths_data_t;
		typedef	std::vector<path_t *>	paths_t;

		typedef	bool (*path_calculator_function_t)(Graphos *g, path_t *the_path, path_data_t *result);
		typedef	bool (*path_toString_function_t)(Graphos *g, path_t *the_path, path_data_t *the_path_data, const char *ofs, std::string *result);

		GraphosVectorPaths();
		GraphosVectorPaths(GraphosVectorPaths const&G);
		~GraphosVectorPaths();

		void	reset(void);
		path_t	*get(int i);
		int	get(int i, int j);
		path_data_t	*get_data(int i);
		paths_t	*paths();
		path_t 	*new_path(void);
		unsigned int	num_paths();
		unsigned int	num_items(int i); /* number of items in path i */
		void	delete_paths(void);
		void	delete_data(void);
		bool	calculate_path_data(Graphos *g, path_calculator_function_t calculator, unsigned int path_index);
		bool	calculate_paths_data(Graphos *g, path_calculator_function_t calculator);
		bool	toString_path_data(Graphos *g, path_toString_function_t printer, unsigned int path_index, const char *ofs, std::string *result);
		bool	toString_paths_data(Graphos *g, path_toString_function_t printer, const char *ofs, std::string *result);
	private:
		paths_t	*myPaths;
		paths_data_t	*myData; /* holding a storage item per path (e.g. may contain total probability of path) */
		void	_delete_me(void); /* this is what the destructor (and reset) calls - all the delete code is here */
};
#endif
