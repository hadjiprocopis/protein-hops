#ifndef _GENERAL_DATA_STORAGE_HPP_
#define	_GENERAL_DATA_STORAGE_HPP_

#include<vector>
#include<boost/variant.hpp>
#include<boost/lexical_cast.hpp>

/* it is not possible to write templated classes or functions in implementation files - great C++ !!! */
/* so all cpp goes here - well only the templated functions as this is not a template class */
//#include <cassert>

/* for fast access, create one such object with fixed capacity and clear() it every row of data read */
class	GeneralDataStorage {
	friend class GeneralDataReader;
	private:
		/* int,double,float -> double (because boost::variant == differentiates between types and don't know how to overload */
		typedef	boost::variant<double, char *>	contents_t;
		struct	to_string_visitor : boost::static_visitor<>{
			std::string	str;
			template <typename T>
			void operator()(T const& item){
				str = boost::lexical_cast<std::string>(item);
			}
		};
	public:
		typedef	std::vector<contents_t *>	container_t;

		GeneralDataStorage();
		GeneralDataStorage(int /*initial_capacity*/);
		GeneralDataStorage(GeneralDataStorage const&);
		~GeneralDataStorage();

		void	reset(int /*initial_capacity*/);
		void	reset();
		void	clear();
		
		contents_t *get(int idx);
		contents_t *get_first(void);
		contents_t *get_last(void);
		template <typename T>
		T	get_as(int idx){ return boost::get<T>(*(this->get(idx))); }
		template <typename T>
		void	put(T d){ this->myData->push_back(new contents_t(d)); }
		template <typename T>
		void	put_and_remember_to_delete(T *d){ contents_t *r = new contents_t(d); this->myData->push_back(r); this->myDataToFree->push_back(r); }
		template <typename T>
		void	put(T d, int idx){ this->myData->at(idx) = new contents_t(d); }
		template <typename T>
		void	put_and_remember_to_delete(T *d, int idx){ this->myData->push_back(d); this->myDataToFree->push_back(d); }

		int	size(void) const;

		/* deletes the containers but not their contents (e.g. the pointers in there, i.e from ToFree) */
		void	soft_delete(void);

		/* get the datas - this is not good as we break all encapsulation but faster for merging two of these objects */
		container_t	*data(void);
		container_t	*data_to_free(void);

		/* append another storage into this one */
		void	append(GeneralDataStorage *);

		char	*toString(const char *ofs); /* print all cols in the order they come */
		char	*toString(const unsigned int *indices, const int num_indices, const char *ofs); /* print selected cols on selected order */
	private:
		container_t	*myData;
		container_t	*myDataToFree;

		void	_delete_me(void); /* this is what the destructor (and reset) calls - all the delete code is here */
};
#endif
