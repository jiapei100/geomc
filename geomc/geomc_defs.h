/*
 * geomc_defs.h
 *
 *  Created on: Dec 24, 2010
 *      Author: tbabb
 */

#ifndef GEOMC_DEFS_H_
#define GEOMC_DEFS_H_

#include <boost/integer.hpp>
#include <boost/utility/enable_if.hpp>
#include <limits>
#include <algorithm>

/**
 * @mainpage GEOMC Library
 * 
 * Geomc is a C++ template library for geometry and basic linear algebra. It is 
 * built to provide building blocks for 2D and 3D applications, though generalization
 * to `N` dimensions is a major design philosophy. 
 * 
 * Wherever possible, types are designed to interoperate intuitively via C++
 * arithmetic operators. Performance, templatization over element types and 
 * dimension, and minimization of dynamic memory allocations are all emphasized.
 * 
 * Source code
 * ===========
 * 
 * Download the geomc headers and source from:
 * http://github.com/trbabb/geomc
 * 
 */

// Shall a generic Vec<N> check whether an index is out-of-bounds on every access?
// Bounds checks can incur signficant cost.

  /* #define GEOMC_VEC_CHECK_BOUNDS */

// Shall a matrix check whether index is out-of-bounds on every access?

  /* #define GEOMC_MTX_CHECK_BOUNDS */

// Shall matrices verify correct dimensions before performing inter-matrix operations? (recommended)

#define GEOMC_MTX_CHECK_DIMS

// Shall matrices check and handle storage aliasing among matrices in inter-matrix operations?

#define GEOMC_MTX_CHECK_ALIASING

// Shall vectors include functions for outputting to streams?

#define GEOMC_LINALG_USE_STREAMS

// Shall vectors include swizzle functions? pulls in <string> and GeomException

#define GEOMC_VEC_USE_SWIZZLE


#define PI  (3.141592653589793238462643383)
#define TAU (6.283185307179586476925286767)

#define M_CLAMP(v,lo,hi) std::min(std::max((v),(lo)),(hi))

#define M_ENABLE_IF(cond)   typename boost::enable_if<(cond), int>::type DUMMY=0
#define M_ENABLE_IF_C(cond) typename boost::enable_if_c<(cond), int>::type DUMMY=0

#define DERIVED_TYPE(base,derived)      typename boost::enable_if<boost::is_base_of< (base), (derived) >, (derived)>::type
#define REQUIRE_INHERIT(base,derived)   typename boost::enable_if<boost::is_base_of< (base), (derived) >, int>::type dummy=0
#define REQUIRE_INHERIT_T(base,derived) typename boost::enable_if<boost::is_base_of< base, derived >, int>::type

typedef boost::int_t<std::numeric_limits<size_t>::digits>::fast index_t;

template <typename T> inline T positive_mod(T a, T b){
    T r = a % b;
    return r<0?r+b:r;
}

// How shall derivative discontinuities be handled by dual numbers?
// e.g., what value shall the derivative have at abs(0)?
// Define exactly one of these to 1.

#define DUAL_DISCONTINUITY_LEFT    0
#define DUAL_DISCONTINUITY_RIGHT   1
#define DUAL_DISCONTINUITY_NAN     0
#define DUAL_DISCONTINUITY_AVERAGE 0

#ifdef PARSING_DOXYGEN

/** @brief Namepsace of all `geomc` functions and classes. */
namespace geom { };

/** @brief Functions to extend support of stdlib to geomc classes. */
namespace std { };

#endif

#endif /* GEOMC_DEFS_H_ */
