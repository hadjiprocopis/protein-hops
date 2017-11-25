#ifndef _GRAPHOS_EDGE_DATA_HPP_
#define	_GRAPHOS_EDGE_DATA_HPP_
#include <vector>
#include <cassert>

#include "general_data_storage.hpp"

class	GraphosEdgeData {
	public:
		GraphosEdgeData();
		GraphosEdgeData(GraphosEdgeData const&G);
		~GraphosEdgeData();

		void	reset(void);
	private:
		GeneralDataStorage	*myData;
		void	_delete_me(void); /* this is what the destructor (and reset) calls - all the delete code is here */
};
#endif
