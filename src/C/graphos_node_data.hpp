#ifndef _GRAPHOS_NODE_DATA_HPP_
#define	_GRAPHOS_NODE_DATA_HPP_
#include <vector>
#include <cassert>

class	GraphosNodeData {
	public:
		GraphosNodeData();
		GraphosNodeData(GraphosNodeData const&G);
		~GraphosNodeData();

		void	reset(void);
	private:
		char	*myData;
		void	_delete_me(void); /* this is what the destructor (and reset) calls - all the delete code is here */
};
#endif
