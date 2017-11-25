#ifndef _GRAPHOS_HPP_
#define	_GRAPHOS_HPP_

#include <stdio.h>
#include <glib.h> /* for their excellent tree implementation which is similar in API to the perl Tree::Nary */

#include "general_cpu_time_monitor.hpp"
#include "general_data_storage.hpp"
#include "general_data_reader.hpp"
#include "graphos_node_data.hpp"
#include "graphos_edge_data.hpp"
#include "graphos_unordered_maps.hpp"
#include "graphos_tree_paths.hpp"
#include "graphos_vector_paths.hpp"

class	Graphos {
	public:
/*		typedef	GraphosEdgeData	graphos_edge_storage_t;
		typedef	GraphosNodeData	graphos_node_storage_t;
*/
		typedef	GeneralDataStorage	graphos_edge_storage_t;
		typedef	GeneralDataStorage	graphos_node_storage_t;
	private:
		typedef	GraphosUnorderedMaps<int>::unordered_map_third_level_t	unordered_map_third_level_t;
		typedef	GraphosUnorderedMaps<int>::unordered_map_second_level_t	unordered_map_second_level_t;
		typedef	GraphosUnorderedMaps<int>::unordered_map_first_level_t	unordered_map_first_level_t;
		typedef	GraphosUnorderedMaps<int>::unordered_map_third_level_t_p	unordered_map_third_level_t_p;
		typedef	GraphosUnorderedMaps<int>::unordered_map_second_level_t_p	unordered_map_second_level_t_p;
		typedef	GraphosUnorderedMaps<int>::unordered_map_first_level_t_p	unordered_map_first_level_t_p;
		static void delete_unordered_map_final_contents(int m);
		static void delete_unordered_map_third_level_t(unordered_map_third_level_t_p m);
		static void delete_unordered_map_second_level_t(unordered_map_second_level_t_p m);
		static void delete_unordered_map_first_level_t(unordered_map_first_level_t_p m);
		/* these functions are called when unordered_maps data structures store allocated memory as their final data (e.g. NOT GraphosUnorderedMaps<int>, YES GraphosUnorderedMaps<char *>) */
		static void _deleter_function_for_edge_data(graphos_edge_storage_t *m);
		static void _deleter_function_for_node_data(graphos_node_storage_t *m);
	public:
		typedef GraphosUnorderedMaps<int>::unordered_map_third_level_char_keys_t vertex_names_2_ids_t;
		typedef containers_prefix::unordered_map<int, char *>			vertex_ids_2_names_t;
		typedef GraphosUnorderedMaps<int>::unordered_map_second_level_t	edgelist_t;
		typedef	GraphosUnorderedMaps<int>::unordered_map_first_level_t	edgelist_with_hops_t; /* includes number of hops */
		typedef	GraphosUnorderedMaps<int>::unordered_map_third_level_t	nodelist_t;
		typedef	GraphosUnorderedMaps<int>::unordered_map_second_level_t	nodelist_with_hops_t;

		typedef GraphosUnorderedMaps<graphos_node_storage_t *>::unordered_map_third_level_t	node_data_t;
		typedef GraphosUnorderedMaps<graphos_edge_storage_t *>::unordered_map_second_level_t	edge_data_t;
		typedef GraphosUnorderedMaps<graphos_edge_storage_t *>::unordered_map_third_level_t	edge_data_contents_t;

		static void delete_nodelist_with_hops_t(nodelist_with_hops_t *);
		static void delete_nodelist_t(nodelist_t *);
		static void delete_edgelist_with_hops_t(edgelist_with_hops_t *);
		static void delete_edgelist_t(edgelist_t *);
		static void delete_node_data_t(node_data_t *);
		static void delete_forward_search_edge_data_t(edge_data_t *);
		static void delete_backward_search_edge_data_t(edge_data_t *);
	private:
		edgelist_t				*backward_search_edgelist,
							*forward_search_edgelist;
		node_data_t				*node_data;
		edge_data_t				*backward_search_edge_data, *forward_search_edge_data;
		vertex_names_2_ids_t			*vertex_names_2_ids;
		vertex_ids_2_names_t			*vertex_ids_2_names;
		static void delete_vertex_ids_2_names_t(vertex_ids_2_names_t *);
		static void delete_vertex_names_2_ids_t(vertex_names_2_ids_t *);
		void	_delete_me(void);
	public:
		void	print_edge_data(FILE *);
		void	print_node_data(FILE *);
		static	int	IsEmptyString(char *str);
		static	bool	is_edgelist_transitive(edgelist_t *an_edgelist);
		static	std::string	print_tabs(int n);
		/* as a hashmap num_hops -> list */
		/* a num_hops=-1 means all hops put together */
		static	bool	convert_paths_to_nodelist(
			GraphosVectorPaths *the_paths,
			Graphos::nodelist_with_hops_t *ret
		);

		/* constructors here */
		/* create one for reading only transitive or only non-transitive data */
		Graphos(bool make_transitive);
		~Graphos();
		Graphos(Graphos const&g);
		Graphos(Graphos *g);

		void	reset();
		int	find_paths(
			char		*from,
			char		*to,
			int		min_num_hops,
			int		max_num_hops,
			GraphosTreePaths	**results_tree, /* this is created in the function */
			GraphosVectorPaths	*results_paths,	/* this is supplied to the function so as to append paths in already existing paths */
			int		*_from_id,	/* will look the strings up and return the int ids */
			int		*_to_id
		);
		/* no need to allocate any memory for the two lists parents (A->{B,C}) and children (B->{A},C->{A}) but it may be that the children is
		   pointing to the parents, so in either case you need to free parents and if parents != children then free children too, as thus:
			if( forward_search_edgelist != backward_search_edgelist ) free(backward_search_edgelist); free(forward_search_edgelist);
		 */
		int	read_edgelist_from_file(
			const char *filename,
			int	num_first_lines_skip, /* as header if any */
			int	from_column_number,
			int	to_column_number,
			const char	*separator /* a string delimiter: NOT each char is a separate delimiter, the whole string is the delimiter */
		);
		int	read_edgedata_from_file(
			const char *filename,
			int	num_first_lines_skip, /* as header if any */
			int	from_node_name_column_number, /* the col numbers (given separator below) containing the names/ids of the from and to nodes */
			int	to_node_name_column_number,
			int	num_columns_to_read,
			const int	*the_columns,
			const GeneralDataReader::data_type_t *the_data_types,
			const char	**default_values,
			const char	*separator /* a string delimiter: NOT each char is a separate delimiter, the whole string is the delimiter */
		);
		int	read_nodedata_from_file(
			const char	*filename,
			int	num_first_lines_skip, /* as header if any */
			int	node_name_column_number,
			int	num_columns_to_read,
			const int	*the_columns,
			const GeneralDataReader::data_type_t *the_data_types,
			const char	**default_values,
			const char	*separator /* a string delimiter: NOT each char is a separate delimiter, the whole string is the delimiter */
		);
		int	node_name_2_id(char *n);
		char	*node_id_2_name(int n);
		char	*read_file_in_memory(const char *filename);
		/* returns true or false if found a direct link between from and to.
		   if from or to == -1 (undef) then it searches for all links to or from the defined from/to node.
		   it returns a pointer to a hashmap whose keys are the matches linking from/to.
		   Sometimes this hashmap is created anew (when from,to are both defined). in this case
		   the pointer must be freed and the 'must_free_result' result is set to true.
		   Other times the resultant pointer points to existing structures and should not be free, so 'must_free_result' will be set to false
		*/
		bool	link(
			char	*from,	/* a node NAME, or 'undef', i.e. all nodes linking to 'to' which must be defined */
			char	*to,	/* a node NAME, or 'undef', i.e. all nodes linking to 'from' which must be defined */ /* at most one of these two can be -1 */
			unordered_map_third_level_t **result,	/* place the result in this pointer */
			bool	*must_free_result	/* true if the result pointer must be freed */
		);
		/* find if two nodes are linked (in the transitive and non-transitive case) */
		bool	linked(
			char	*from,	/* a node NAME, or 'undef', i.e. all nodes linking to 'to' which must be defined */
			char	*to	/* a node NAME, or 'undef', i.e. all nodes linking to 'from' which must be defined */ /* at most one of these two can be -1 */
		);
		/* finds all links between 'from' and 'to' num_hops away by recursively finding all links coming out from 'from'
		   then for all of these nodes, it does the same until it reaches the specified hops range and the end node is 'to'.
		   Recursion may become a problem if the number of hops is more than 10, 100? which is the depth of recursion, depending on
		   your system. However depending on the connectivity, a number of hops > 3 may be unrealistic resulting to billions of links...
		   so recursion may become a problem but it is not for my application.
		*/
		int	links_recursive(
			char	*from,	/* node name as read from file */
			char	*to,	/* as above */
			int	min_num_hops,
			int	max_num_hops,
			int	current_num_hops,
			GNode	*results_tree /* the result, initialise this pointer first */
		);
		void	break_edge(
			char	*from_name,
			char	*to_name
		);
		void	break_edge(
			char	*node_name
		);
		void	break_edge_starting_with(
			char	*node_name
		);
		void	break_edge_ending_with(
			char	*node_name
		);
		int	add_node(
			char	*node_name
		);
		graphos_node_storage_t	*get_node_data(
			char	*node_name
		);
		graphos_edge_storage_t	*get_edge_data(
			char	*from_node_name,
			char	*to_node_name
		);
		void	delete_node_by_name(
			char	*node_name
		);
		/* write etc. */
		/* returns -1 on failure, or the number of nodes it wrote (including zero, in which case no output file will be written) */
		int	write_unique_nodes_to_file(
			const char *outfilename
		);
		/* returns -1 on failure, or the number of edges it wrote (including zero, in which case no output file will be written) */
		int	write_edgelist_to_file(
			const char *outfilename,
			const char *ofs
		);
		/* returns -1 on failure, or the number of nodes it wrote (including zero, in which case no output file will be written) */
		int	write_unique_nodes_to_file(
			FILE *outfilehandle
		);
		/* returns -1 on failure, or the number of edges it wrote (including zero, in which case no output file will be written) */
		int	write_edgelist_to_file(
			FILE *outfilehandle,
			const char *ofs
		);
		bool	write_results_as_paths_to_file(
			const char *outbasename,
			const char *ofs,
			const int min_num_hops,
			const int max_num_hops,
			GraphosVectorPaths::path_toString_function_t printer, /* optional printer for each path - set to NULL in order to use default */
			GraphosVectorPaths *the_paths
		);
		bool    write_all_data_to_files(
			const char	*outbasename,
			const char	*ofs,
			const int	min_num_hops,
			const int	max_num_hops,
			GraphosVectorPaths::path_toString_function_t printer, /* optional printer for each path - set to NULL in order to use default */
			GraphosVectorPaths *results_paths
		);
		graphos_node_storage_t	*get_node_data(
			int	node_id
		);
		graphos_edge_storage_t	*get_edge_data(
			int	from_node_id,
			int	to_node_id
		);
	private:
		bool	myMakeTransitive;
		int	myPreviousFileWasTransitive;

		int	links_recursive(
			int	from_id,	/* node internal ID */
			int	to_id,	/* as above */
			int	min_num_hops,
			int	max_num_hops,
			int	current_num_hops,
			GNode	*results_tree /* the result, initialise this pointer first */
		);
		bool	link(
			int	from_id,	/* a node (internal) id, or -1 for 'undef', i.e. all nodes linking to 'to' which must be defined */
			int	to_id,	/* a node (internal) id, or -1 for 'undef', i.e. all nodes linking to 'from' which must be defined */ /* at most one of these two can be -1 */
			unordered_map_third_level_t **result,	/* place the result in this pointer */
			bool	*must_free_result	/* true if the result pointer must be freed */
		);
		/* find if two nodes are linked (in the transitive and non-transitive case) */
		bool	linked(
			int	from_id,	/* a node (internal) id, or -1 for 'undef', i.e. all nodes linking to 'to' which must be defined */
			int	to_id	/* a node (internal) id, or -1 for 'undef', i.e. all nodes linking to 'from' which must be defined */ /* at most one of these two can be -1 */
		);
		void	add_edge_forward(
			int	from_id,
			int	to_id
		);
		void	add_edge_backward(
			int	from_id,
			int	to_id
		);
		void	break_edge(
			int	from_id,
			int	to_id
		);
		static	int	break_edge(
			int	from_id,
			int	to_id,
			edgelist_t	*an_edgelist
		);
		void	break_edge(
			int	node_id
		);
		void	break_edge_starting_with(
			int	node_id
		);
		void	break_edge_ending_with(
			int	node_id
		);
		int	add_node(
			int	node_id
		);
		void	delete_node_by_id(
			int	node_id
		);
/* these are inlined and thus works better if private otherwise we need to put the code here etc., so their public version just calls these */
		graphos_node_storage_t	*_get_node_data(
			int	node_id
		);
		graphos_edge_storage_t	*_get_edge_data(
			int	from_node_id,
			int	to_node_id
		);
		/* write etc */
		/* as a hashmap num_hops -> from -> to */
		/* a num_hops=-1 means all hops put together */
		static	bool	convert_paths_to_edgelist(
			GraphosVectorPaths *the_paths,
			Graphos::edgelist_with_hops_t *ret
		);
		/* returns -1 on failure, or the number of nodes it wrote (including zero, in which case no output file will be written) */
		static	int	write_nodelist_to_file(
			const char *outfilename,
			Graphos::vertex_ids_2_names_t *vertex_ids_2_names,
			Graphos::nodelist_t *the_nodelist
		);
		/* returns -1 on failure, or the number of edges it wrote (including zero, in which case no output file will be written) */
		static	int	write_edgelist_to_file(
			const char *outfilename,
			const char *ofs,
			Graphos::vertex_ids_2_names_t *vertex_ids_2_names,
			Graphos::edgelist_t *the_edgelist
		);

		/* returns -1 on failure, or the number of nodes it wrote (including zero, in which case no output file will be written) */
		static	int	write_nodelist_to_file(
			FILE *outfilehandle,
			Graphos::vertex_ids_2_names_t *vertex_ids_2_names,
			Graphos::nodelist_t *the_nodelist
		);
		/* returns -1 on failure, or the number of edges it wrote (including zero, in which case no output file will be written) */
		static	int	write_edgelist_to_file(
			FILE *outfilehandle,
			const char *ofs,
			Graphos::vertex_ids_2_names_t *vertex_ids_2_names,
			Graphos::edgelist_t *the_edgelist
		);
		static	char	*delimiter_string_strtok_r (char *s, const char *delim, char **save_ptr);

		int	_node_name_2_id(char *n);
		char	*_node_id_2_name(int n);
};
#endif
