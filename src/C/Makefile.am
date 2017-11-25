##                      Graph-searcher for STRING-DB
##
##
##                        by Andreas Hadjiprocopis
##                         (andreashad2@gmail.com)
##                Copyright Andreas Hadjiprocopis, 2010, ICR
##         Free to copy and distribute but please attribute the author

OPTIONS_AUTOMAKE=gnu
ACLOCAL_AMFLAGS=-I ./m4

bin_PROGRAMS = graphos_enquiry_for_STRING #graphos_enquiry graph_enquiry graph_enquiry_v5 graph_enquiry_v4 graph_enquiry_v3 graph_enquiry_v2 graph_enquiry_v1

lib_LIBRARIES = libgraphos.a
libgraphos_a_SOURCES = general_data_storage.cpp general_data_reader.cpp graphos_node_data.cpp graphos_edge_data.cpp graphos_tree_paths.cpp graphos_vector_paths.cpp graphos.cpp
libgraphos_a_CXXFLAGS = -I. ${OPTIMISATION_FLAGS} ${CXXFLAGS} ${BOOST_CFLAGS} ${GLIB_CFLAGS}
libgraphos_a_includedir=${includedir}/graphos
libgraphos_a_include_HEADERS = general_cpu_time_monitor.hpp general_data_storage.hpp general_data_reader.hpp graphos.hpp graphos_tree_paths.hpp graphos_unordered_maps.hpp graphos_vector_paths.hpp graphos_edge_data.hpp graphos_node_data.hpp

graphos_enquiry_for_STRING_SOURCES = graphos_enquiry_for_STRING.cpp
graphos_enquiry_for_STRING_CXXFLAGS = -I. ${OPTIMISATION_FLAGS} ${CXXFLAGS} ${BOOST_CFLAGS} ${GLIB_CFLAGS}
graphos_enquiry_for_STRING_LDFLAGS = -L. ${LDFLAGS} ${BOOST_LDFLAGS} ${GLIB_LDFLAGS}
graphos_enquiry_for_STRING_LDADD = ${GLIB_LIBS} ${BOOST_LIBS} -lm -lgraphos
graphos_enquiry_for_STRING_DEPENDENCIES = libgraphos.a

#graphos_enquiry_SOURCES = graphos_enquiry.cpp
#graphos_enquiry_CXXFLAGS = -I. ${OPTIMISATION_FLAGS} ${CXXFLAGS} ${BOOST_CFLAGS} ${GLIB_CFLAGS}
#graphos_enquiry_LDFLAGS = -L. ${LDFLAGS} ${BOOST_LDFLAGS} ${GLIB_LDFLAGS}
#graphos_enquiry_LDADD = ${GLIB_LIBS} ${BOOST_LIBS} -lm -lgraphos
#graphos_enquiry_DEPENDENCIES = libgraphos.a

#graph_enquiry_SOURCES = graph_enquiry.cpp
#graph_enquiry_CXXFLAGS = ${CXXFLAGS} ${BOOST_CXXFLAGS} ${GLIB_CFLAGS}
#graph_enquiry_LDFLAGS = ${LDFLAGS} ${BOOST_LDFLAGS} ${GLIB_LIBS}
#graph_enquiry_LDADD = -lglib -lm

#graph_enquiry_v5_SOURCES = graph_enquiry_v5.cpp
#graph_enquiry_v5_CXXFLAGS = ${CXXFLAGS} ${BOOST_CXXFLAGS} ${GLIB_CFLAGS}
#graph_enquiry_v5_LDFLAGS = ${LDFLAGS} ${BOOST_LDFLAGS} ${GLIB_LIBS}
#graph_enquiry_v5_LDADD = -lglib -lm

#graph_enquiry_v4_SOURCES = graph_enquiry_v4.cpp
#graph_enquiry_v4_CXXFLAGS = ${CXXFLAGS} ${BOOST_CXXFLAGS} ${GLIB_CFLAGS}
#graph_enquiry_v4_LDFLAGS = ${LDFLAGS} ${BOOST_LDFLAGS} ${GLIB_LIBS}
#graph_enquiry_v4_LDADD = -lglib -lm

#graph_enquiry_v3_SOURCES = graph_enquiry_v3.cpp
#graph_enquiry_v3_CXXFLAGS = ${CXXFLAGS} ${BOOST_CXXFLAGS} ${GLIB_CFLAGS}
#graph_enquiry_v3_LDFLAGS = ${LDFLAGS} ${BOOST_LDFLAGS} ${GLIB_LIBS}
#graph_enquiry_v3_LDADD = -lglib -lm

#graph_enquiry_v2_SOURCES = graph_enquiry_v2.cpp
#graph_enquiry_v2_CXXFLAGS = ${CXXFLAGS} ${BOOST_CXXFLAGS} ${GLIB_CFLAGS}
#graph_enquiry_v2_LDFLAGS = ${LDFLAGS} ${BOOST_LDFLAGS} ${GLIB_LIBS}
#graph_enquiry_v2_LDADD = -lglib -lm

#graph_enquiry_v1_SOURCES = graph_enquiry_v1.cpp
#graph_enquiry_v1_CXXFLAGS = ${CXXFLAGS} ${BOOST_CXXFLAGS} ${GLIB_CFLAGS}
#graph_enquiry_v1_LDFLAGS = ${LDFLAGS} ${BOOST_LDFLAGS} ${GLIB_LIBS}
#graph_enquiry_v1_LDADD = -lglib -lm

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = graphos.pc
pkginclude_HEADERS = graphos.hpp
pkginclude_LIBS = libgraphos
