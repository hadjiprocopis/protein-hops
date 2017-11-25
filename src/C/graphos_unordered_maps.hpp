#ifndef _GRAPHOS_UNORDERED_MAPS_HPP_
#define _GRAPHOS_UNORDERED_MAPS_HPP_

/* Now this is a strange one, template classes can not be defined in .cpp files (g++?),
   instead they must be part of their header files. Great!
   http://stackoverflow.com/questions/999358/undefined-symbols-linker-error-with-simple-template-class
*/

#ifdef HAVE_BOOST
#warning "**** I am using the boost libraries for unordered and ordered map, it is worth to compare the performance with using std's (boost is generally faster, especially for large maps)."
#include <boost/config.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/tuple/tuple.hpp>
#define containers_prefix       boost
#else
#warning "**** I am using the std libraries for unordered and ordered map, it is worth to compare the performance with using boost's (boost is generally faster, especially for large maps)."
#include <tr1/unordered_map>
#include <functional>
#include <iterator>
#define containers_prefix       std::tr1
#endif

/* foreach'es */
#define const_foreach(type, i, c ) for( type::const_iterator i = (c).begin(); i != (c).end(); ++i )
#define const_reverse_foreach(type, i , c ) for( type::const_reverse_iterator i = (c).rbegin(); i != (c).rend(); ++i )
#define plain_foreach(type, i, c ) for( type::iterator i = (c).begin(); i != (c).end(); ++i )
#define plain_reverse_foreach(type, i , c ) for( type::reverse_iterator i = (c).rbegin(); i != (c).rend(); ++i )
#define foreach const_foreach
#define reverse_foreach const_reverse_foreach

/* Nested unordered maps with the primary key being a char*, the final value stored is of type T */
template <typename T>
class	GraphosUnorderedMaps {
	private:
		/* hashing function and comparator for (char *) keys to a hashmap (notice it is borrowed from gawk) */
		/* this is a hashing function of 'char *' keys, it is quite fast */
		/* taken from http://www.red-bean.com/guile/guile/new/msg00300.html and it is used in gawk, slightly modified for our needs, it is passed on to the unordered map */
		static size_t gawk_hash(const char *s/*, size_t len, unsigned long hsize*/){
			unsigned long h = 0;
			size_t len = strlen(s); /* assuming there is a terminator! */
			/*
			* This is INCREDIBLY ugly, but fast.  We break the string up into
			* 8 byte units.  On the first time through the loop we get the
			* "leftover bytes" (strlen % 8).  On every other iteration, we
			* perform 8 HASHC's so we handle all 8 bytes.  Essentially, this
			* saves us 7 cmp & branch instructions.  If this routine is
			* heavily used enough, it's worth the ugly coding.
			*
			* OZ's original sdbm hash, copied from Margo Seltzers db package.
			*/
		
			/*
			* Even more speed:
			* #define HASHC   h = *s++ + 65599 * h
			* Because 65599 = pow(2, 6) + pow(2, 16) - 1 we multiply by shifts
			*/
			#define HASHC   htmp = (h << 6); h = *s++ + htmp + (htmp << 10) - h
		
			unsigned long htmp;
		
			h = 0;
		
			/* "Duff's Device" for those who can handle it */
			if (len > 0) {
				register size_t loop = (len + 8 - 1) >> 3;
		
				switch (len & (8 - 1)) {
				case 0:
					do {	/* All fall throughs */
						HASHC;
				case 7:		HASHC;
				case 6:		HASHC;
				case 5:		HASHC;
				case 4:		HASHC;
				case 3:		HASHC;
				case 2:		HASHC;
				case 1:		HASHC;
					} while (--loop);
				}
			}
		
			//if (h >= hsize)
			//h %= hsize;
			return h;
		}

		/* the hashers */
                struct char_star_equal : std::binary_function<char *, char *, bool>{
                        bool operator()(char * const& s1, char * const& s2) const { return ((s1==NULL)&&(s2==NULL)) ? true : ((s1==NULL)||(s2==NULL)) ? true : strcmp(s1,s2)==0; }
                };
                struct char_star_hash : std::unary_function<char *, std::size_t>{
                        std::size_t operator()(char * const& x) const { return gawk_hash(x); }
                };
	public:
                typedef containers_prefix::unordered_map<int,T> unordered_map_third_level_t;
                typedef unordered_map_third_level_t* unordered_map_third_level_t_p;
                typedef containers_prefix::unordered_map<int, unordered_map_third_level_t_p> unordered_map_second_level_t;
                typedef unordered_map_second_level_t* unordered_map_second_level_t_p;
                typedef containers_prefix::unordered_map<int, unordered_map_second_level_t_p> unordered_map_first_level_t;
                typedef	unordered_map_first_level_t*	unordered_map_first_level_t_p;

		typedef	containers_prefix::unordered_map<char *,T,char_star_hash,char_star_equal>  unordered_map_third_level_char_keys_t;

		/* this is only called when the final data stored here (e.g. the <T>) is an allocated pointer and needs to be free by the supplied user-defined function */
		/* only for cases when you have GraphosUnorderedMaps<char *> BUT NOT GraphosUnorderedMaps<int>, then use the appropriate deleter functions below (with the 2 args) */
		static void delete_unordered_map_final_contents(T m, void (*deleter_function)(T)){ deleter_function(m); }

		static void delete_unordered_map_third_level_t(unordered_map_third_level_t *m){
		//	for(typename unordered_map_third_level_t::iterator _iterstl_=m->begin();_iterstl_!=m->end();++_iterstl_){ delete_unordered_map_final_contents(_iterstl_->second); } delete(m);
		//	delete(m);
		}
		static void delete_unordered_map_third_level_char_keys_t(unordered_map_third_level_char_keys_t *m){
			for(typename unordered_map_third_level_char_keys_t::iterator _iterstl_=m->begin();_iterstl_!=m->end();++_iterstl_){ free(_iterstl_->first); }
		}
		static void delete_unordered_map_third_level_t(unordered_map_third_level_t *m, void (*deleter_function)(T)){
			for(typename unordered_map_third_level_t::iterator _iterstl_=m->begin();_iterstl_!=m->end();++_iterstl_){ delete_unordered_map_final_contents(_iterstl_->second, deleter_function); }
		}
		static void delete_unordered_map_second_level_t(unordered_map_second_level_t *m){
			for(typename unordered_map_second_level_t::iterator _itersl_=m->begin();_itersl_!=m->end();++_itersl_){ delete_unordered_map_third_level_t(_itersl_->second); delete(_itersl_->second); }
		}
		static void delete_unordered_map_second_level_t(unordered_map_second_level_t *m, void (*deleter_function)(T)){
			for(typename unordered_map_second_level_t::iterator _itersl_=m->begin();_itersl_!=m->end();++_itersl_){ delete_unordered_map_third_level_t(_itersl_->second, deleter_function); delete(_itersl_->second); }
		}
		static void delete_unordered_map_first_level_t(unordered_map_first_level_t *m){
			for(typename unordered_map_first_level_t::iterator _iterfl_=m->begin();_iterfl_!=m->end();++_iterfl_){ delete_unordered_map_second_level_t(_iterfl_->second); delete(_iterfl_->second); }
		}
		static void delete_unordered_map_first_level_t(unordered_map_first_level_t *m, void (*deleter_function)(T)){
			for(typename unordered_map_first_level_t::iterator _iterfl_=m->begin();_iterfl_!=m->end();++_iterfl_){ delete_unordered_map_second_level_t(_iterfl_->second, deleter_function); delete(_iterfl_->second); }
		}
};
#endif
