/*
 * Rect.h
 * 
 *  Created on: Oct 7, 2010
 *      Author: Tim Babb
 */


#ifndef Rect_H_
#define Rect_H_

#include <algorithm>
#include <cmath>
#include <functional>

#include <geomc/Hash.h>
#include <geomc/shape/Bounded.h>
#include <geomc/linalg/Ray.h>
#include <geomc/linalg/Vec.h>
#include <geomc/shape/shapedetail/Hit.h>

namespace geom {

/****************************
 * Rect Class               *
 ****************************/

/** @ingroup shape
 *  @brief An N-dimensional axis-aligned box.
 *
 * This class works naturally with both 1D and multi-dimensional ranges.
 * 1D ranges have point type `T`, while multi-dimensional ranges have point
 * type `Vec<T,N>`. A typedef of this may be accessed via:
 * 
 *     Rect<T,N>::point_t
 *
 * Example: 
 * 
 * * `Rect<double,4>::point_t` is `Vec<double,4>`;  `getMin()` returns a `Vec4d`.
 * * `Rect<double,1>::point_t` is `double`; `getMin()` returns a `double`.
 * 
 * Rects obey the standard interval conventions, in that lower extremes are inclusive
 * and upper extemes are exclusive.
 */
template <typename T, index_t N> class Rect : virtual public Bounded<T,N> {
protected:
    typedef PointType<T,N> ptype;

public:
    /// Type of object confined by this Rect
    typedef typename ptype::point_t point_t;

protected:
    /// Lower extremes
    point_t mins;
    /// Upper extremes
    point_t maxs;

public:

    /**
     * @brief Construct a Rect with extremes `lo` and `hi`. 
     * If for any axis `lo > hi`, the Rect is empty.
     * 
     * @param lo Lower extreme
     * @param hi Upper extreme
     */
    Rect(point_t lo, point_t hi):mins(lo),maxs(hi) {}

    /// Construct a unit rectangle, with the lower extreme at the origin.
    Rect():
        mins((T)0),
        maxs((T)1) {
        //do nothing else
    }

    /****************************
     * Convenience functions    *
     ****************************/

    /**
     * Construct a Rect from a center point and extent.
     * @param center Center of the new Rect
     * @param dims Lengths of each axis; may be negative.
     * @return A new Rect with its center at `center`.
     */
    inline static Rect<T,N> fromCenter(typename Rect<T,N>::point_t center,
                                typename Rect<T,N>::point_t dims) {
        return Rect<T,N>::spanningCorners(center-dims/2, center+dims/2);
    }

    /**
     * Construct a Rect from a corner point and an extent.
     * @param corner Arbitrary corner point.
     * @param dims Lengths of each axis relative to given corner. Lengths may be negative.
     * @return A new Rect with one corner at `corner`.
     */
    inline static Rect<T,N> fromCorner(typename Rect<T,N>::point_t corner,
                                typename Rect<T,N>::point_t dims) {
        return Rect<T,N>::spanningCorners(corner, corner+dims);
    }
    
    /**
     * Construct a Rect bounded by corners `c1` and `c2`. 
     * @param c1 A corner of the Rect 
     * @param c2 The corner opposite `c1`
     * @return A new Rect with corners at `c1` and `c2`.
     */
    static Rect<T,N> spanningCorners(typename Rect<T,N>::point_t c1,
                                     typename Rect<T,N>::point_t c2) {
        return Rect<T,N>(std::min(c1,c2), std::max(c1,c2));
    }

    /**
     * Test whether a point is in the N-dimensional range `[min, max)`.
     * @param min Lower extreme
     * @param max Upper extreme
     * @param pt Test point
     * @return `true` if `pt` is inside the exremes
     */
    static bool contains(typename Rect<T,N>::point_t min,
                         typename Rect<T,N>::point_t max,
                         typename Rect<T,N>::point_t pt) {
        min = std::min(min,max);
        max = std::max(min,max);
        for (index_t axis = 0; axis < N; axis++) {
            T v = ptype::iterator(pt)[axis];
            if (v < ptype::iterator(min)[axis] or v >= ptype::iterator(max)[axis]) {
                return false;
            }
        }
        return true;
    }

    /*****************************
     * Operators                 *
     *****************************/

    /**
     * @brief Box union.
     * 
     * @return A Rect fully containing `this` and box `b`.
     */
    inline Rect<T,N> operator|(const Rect<T,N> &b) const {
        return rangeUnion(b);
    }
    
    /**
     * @brief Box union.
     * 
     * Extend `this` to fully contain `b`.
     * @return A reference to `this`, for convenience.
     */
    Rect<T,N>& operator|=(const Rect<T,N> &b) {
        //box union
        maxs = std::max(b.maxs, maxs);
        mins = std::min(b.mins, mins);
        return *this;
    }
    
    /**
     * @brief Box intersection.
     * 
     * @return A Rect representing the area overlapped by both `this` and `b`.
     */
    inline Rect<T,N> operator&(const Rect<T,N> &b) const {
        return rangeIntersection();
    }
    
    /**
     * @brief Box intersection.
     * 
     * Confine this Rect to the area overlapping `b`.
     * @return A reference to `this` for convenience.
     */
    Rect<T,N>& operator&=(const Rect<T,N> &b) {
        mins = std::max(mins, b.mins);
        maxs = std::min(maxs, b.maxs);
    }

    /**
     * @brief Translation.
     * 
     * @param dx Amount by which to translate this region.
     * @return A translated Rect.
     */
    inline Rect<T,N> operator+(point_t dx) const {
        return Rect<T,N>(mins + dx, maxs + dx);
    }
    

    /**
     * @brief Translation.
     * 
     * @param dx Amount by which to translate this region.
     * @return  A reference to `this`, for convenience.
     */
    inline Rect<T,N>& operator+=(T dx) {
        maxs += dx;
        mins += dx;
        return *this;
    }

    /**
     * @brief Equality test.
     * 
     * @return `true` if and only if all the corresponding extremes of `b` are the same.
     */
    bool operator==(Rect<T,N> b) const {
        return (maxs == b.maxs) && (mins == b.mins);
    }
    
    /**
     * @brief Inequality test.
     * 
     * @return `true` if and only if any extreme of `b` is different from
     * the corresponding extreme in `this`.
     */
    bool operator!=(Rect<T,N> b) const {
        return (maxs != b.maxs) || (mins != b.mins);
    }

    /**
     * @brief Uniform scale.
     * 
     * @param a Scale factor.
     * @return A new Rect, scaled about the origin by factor `a`.
     */
    Rect<T,N> operator*(point_t a) const {
        return Rect<T,N>(mins*a, maxs*a);
    }

    /**
     * @brief Uniform scale. 
     * 
     * Scale this Rect about the origin by factor `a`. 
     * 
     * @param a Scale factor
     * @return A reference to `this`, for convenience.
     */
    Rect<T,N>& operator*=(point_t a) {
        mins *= a;
        maxs *= a;
        return *this;
    }

    /**
     * @brief Uniform scale.
     * 
     * @param a Scale factor.
     * @return A new Rect, scaled about the origin by multiple `1 / a`.
     */
    Rect<T,N> operator/(point_t a) const {
        return Rect<T,N>(mins/a, maxs/a);
    }

    /**
     * @brief Uniform scale. 
     * 
     * Scale this Rect about the origin by factor `1 / a`. 
     * @param a Scale factor
     * @return A reference to `this`, for convenience.
     */
    Rect<T,N>& operator/=(point_t a) {
        mins /= a;
        maxs /= a;
        return *this;
    }

    /**
     * @brief Element-wise typecast.
     * 
     * @return A new Rect, with elements all of type `U`.
     */
    template <typename U, index_t M> operator Rect<U,M>() {
        return Rect<U,M>((typename Rect<U,M>::point_t) mins,
                         (typename Rect<U,M>::point_t) maxs);
    }


    /*****************************
     * Public Methods              *
     *****************************/

    /**
     * @return `true` if and only if `pt` is inside this rectangle.
     * Lower extremes are inclusive, while upper are exclusive.
     */
    bool contains(T pt) const {
        for (index_t axis = 0; axis < N; axis++) {
            T v = ptype::iterator(pt)[axis];
            if (v < ptype::iterator(min)[axis] or v >= ptype::iterator(max)[axis]) {
                return false;
            }
        }
        return true;
    }
 
    /**
     * @return `true` if an only if this region intersects with `box`. Lower extremes
     * are considered inclusive, while upper are exclusive.
     */
    bool intersects(const Rect<T,N> &box) const {
        for (index_t axis = 0; axis < N; axis++) {
            // disjoint on this axis?
            if (ptype::iterator(maxs)[axis]     <= ptype::iterator(box.mins)[axis] or 
                ptype::iterator(box.maxs)[axis] <= ptype::iterator(mins)[axis]) {
                return false;
            }
        }
        return true;
    }

    /**
     * @return The lower extremes of this region.
     */
    inline point_t min() const {
        return mins;
    }

    /** 
     * @return The upper extremes of this region. 
     */
    inline point_t max() const {
        return maxs;
    }
    
    /**
     * @return The center point of this region.
     */
    point_t getCenter() const {
        return (maxs+mins)/2;
    }

    /**
     * @return The size of this region along each axis.
     */
    inline point_t getDimensions() const {
        return maxs - mins;
    }

    /**
     * Change the size of this region, adjusting about its center.
     * @param dim New lengths along each axis.
     */
    void setDimensions(point_t dim) {
        dim = std::abs(dim);
        point_t diff = (dim - (maxs-mins)) / 2;
        mins = mins-diff;
        maxs = maxs+diff;
    }

    /**
     * Re-configure this region to exactly contain the two given points.
     */
    void setCorners(point_t corner1, point_t corner2) {
        maxs = std::max(corner1, corner2);
        mins = std::min(corner1, corner2);
    }
    
    /**
     * Preserving its size, translate the center of this region to the point
     * given by `center`.
     * @param center New center point.
     */
    void setCenter(point_t center) {
        point_t tx = center - getCenter();
        maxs += tx;
        mins += tx;
    }

    /**
     * Translation
     * @param tx Amount by which to translate this region.
     */
    void translate(point_t tx) {
        maxs += tx;
        mins += tx;
    }
    
    /**
     * Compute the N-dimensional volume of this region as the product of its
     * extents. Result measures a length if `N` is 1; an area if `N` is 2; a 
     * volume if 3; etc. 
     * 
     * @return The volume of this region.
     */
    T volume() const {
        T vol = 1;
        point_t dim = getDimensions();
        for (index_t axis = 0; axis < N; axis++) {
            vol *= std::max(ptype::iterator(dim)[axis], 0);
        }
        return vol;
    }
    
    /**
     * @return `true` if and only if this region contains no points.
     */
    bool isEmpty() const {
        for (index_t axis = 0; axis < N; axis++) {
            T hi = ptype::iterator(maxs)[axis];
            T lo = ptype::iterator(mins)[axis];
            if (hi <= lo) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * @return A new Rect representing the area overlapped by both `this` and `b`.
     */
    Rect<T,N> rangeIntersection(const Rect<T,N> &b) const {
        return Rect<T,N>(std::max(mins, b.mins),
                         std::min(maxs, b.maxs));
    }

    /**
     * @return A new Rect completely containing both `this` and `b`.
     */
    Rect<T,N> rangeUnion(const Rect<T,N> &b) const {
        return Rect<T,N>(std::min(mins, b.mins),
                         std::max(maxs, b.maxs));
    }

    /**
     * @return `p`'s fractional position within this Rect.
     */
    point_t boxFraction(point_t p) const {
        return (point_t(1)-p) * mins + p * maxs ;
    }
    
    /**
     * Box-ray intersection test.
     * 
     * @param r A ray
     * @param sides Whether to hit-test the front-facing or back-facing surfaces.
     * @return A ray Hit describing whether and where the ray intersects this Rect,
     * as well as the normal, side hit, and ray parameter.
     */
    Hit<T,N> trace(const Ray<T,N> &r, HitSide sides) const {
        T near_root = std::numeric_limits<T>::lowest(); // non-denormal extremes, for speed (as opposed to inf).
        T far_root  = std::numeric_limits<T>::max();    // besides, infinity may not be defined for T.
        index_t near_axis = 0;
        index_t far_axis  = 0;
        T near_coord  = 0; // for guaranteeing that the hit point is exactly on the rect surface,
        T far_coord   = 0; // regardless of precision and rounding error in the ray arithmetic.
        Hit<T,N> hit  = Hit<T,N>(r, sides); // defaults to miss
        
        // test ray intersection with an infinite slab along each axis
        // box intersections are the farthest near hit and nearest far hit
        for (index_t axis = 0; axis < N; axis++) {
            if (r.direction[axis] == 0) {
                // ray direction tangent to test planes, no intersection along this axis
                if (r.origin[axis] < mins[axis] || r.origin[axis] > maxs[axis]) {
                    // origin outside of test slab; miss
                    return hit;
                }
            } else {
                // coordinate of hit, along tested axis
                T c1 = maxs[axis];
                T c2 = mins[axis];
                // ray multiple of hit
                T s1 = (c1 - r.origin[axis]) / r.direction[axis];
                T s2 = (c2 - r.origin[axis]) / r.direction[axis];
                
                // order s1, s2 to (near, far) (aka front, back)
                if (s2 < s1) {
                    std::swap(s1,s2);
                    std::swap(c1,c2);
                }
                if (s1 > near_root) {
                    near_root  = s1;
                    near_axis  = axis; 
                    near_coord = c1;
                }
                if (s2 < far_root) {
                    far_root  = s2;
                    far_axis  = axis;
                    far_coord = c2;
                }
            }
        }
        
        if (near_root > far_root) {
            // miss
            return hit;
        } else if (near_root > 0 and (sides & HIT_FRONT)) {
            // hit front
            hit.p = r * near_root;
            hit.p[near_axis] = near_coord;
            hit.s = near_root;
            hit.n[near_axis] = -copysign(1, r.direction[near_axis]);
            hit.side = HIT_FRONT;
            hit.hit = true;
        } else if (far_root > 0 and (sides & HIT_BACK)) {
            // hit back
            hit.p = r * far_root;
            hit.p[far_axis] = far_coord;
            hit.s = far_root;
            hit.n[far_axis] = copysign(1, r.direction[far_axis]);
            hit.side = HIT_BACK;
            hit.hit = true;
        }
        // if encountered cases above; hit.
        // else miss (box entirely behind ray origin)
        return hit;
    }

    /*****************************
     * Inherited Methods         *
     *****************************/

    Rect<T,N> bounds() {
        return *this;
    }

}; // end Rect class

} //end namespace geom

//allows RectBound<T,N>s to work with std::tr1 hash containers
namespace std { namespace tr1 {

   template <typename T, index_t N>
   struct hash< geom::Rect<T,N> > : public unary_function<geom::Rect<T,N>, size_t> {
       inline size_t operator() (const geom::Rect<T,N>& box) const {
           return geom::general_hash(&box, sizeof(geom::Rect<T,N>));
       }
   };

}}

#endif /* Rect_H_ */
