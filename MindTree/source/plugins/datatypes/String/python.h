#ifndef PYTHON_TF8KXSU0

#define PYTHON_TF8KXSU0

#include "boost/python.hpp"

namespace BPy=boost::python;

BOOST_PYTHON_MODULE(pystringcache){
    BPy::class_<StringCache, BPy::bases<DataCache>>("StringCache");
}

#endif /* end of include guard: PYTHON_TF8KXSU0 */
