#ifndef _GRAPHOS_TREE_PATHS_HPP_
#define	_GRAPHOS_TREE_PATHS_HPP_

#include <glib.h> /* for their excellent tree implementation which is similar in API to the perl Tree::Nary */

/* a Gnode data structure for the tree */
class gnode_data_t {
	public:
		int	id;
				gnode_data_t(int i);
				gnode_data_t(const gnode_data_t &g);
				gnode_data_t(const gnode_data_t *g);
				~gnode_data_t();
};

class	GraphosTreePaths {
	public:
		GraphosTreePaths(gpointer data);
		GraphosTreePaths(GNode *root);
		GraphosTreePaths(GraphosTreePaths const&G);
		~GraphosTreePaths();

		GNode	*root();
		GNode	*root(GNode *r);
		void	reset(gpointer newdata); /* the data to hold in the first node or NULL if no first node is to be created */
		std::vector<GNode *> *leaf_nodes(void);
		std::vector<GNode *> *all_nodes(void);
		
		static void	free_gnode_tree(GNode *first_node);
		static void	get_leaf_nodes(GNode *a_node, gpointer a_data);
	private:
		GNode	*myRoot;

		static gboolean	get_all_nodes(GNode *a_node, gpointer a_data);
		void	_delete_me(void); /* this is what the destructor (and reset) calls - all the delete code is here */
};
#endif
