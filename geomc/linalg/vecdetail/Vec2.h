/*
 * Vec2.h
 *
 *  Created on: May 9, 2009
 *      Author: Tim Babb
 */

#ifndef Vec2_H_
#define Vec2_H_

#include <cmath>
#include <geomc/linalg/vecdetail/VecBase.h>

#ifdef GEOMC_VEC_USE_SWIZZLE
    #include <string>
    #include <geomc/GeomException.h>
#endif

namespace geom {

    /** @ingroup linalg
     *  @brief 2D specialization of vector class.
     * 
     * `Vec<T,2>`'s elements may be accessed under these equivalent naming schemes:
     * 
     *     v.{x,y}
     *     v.{s,t}
     *     v.{row,col}
     * 
     * with the latter scheme intended for use as matrix coordinates. `x`, `s`, and
     * `row` all refer to the same element.
     */
    template <typename T> class Vec<T,2> : public detail::VecCommon<T,2> {

    public:
        
        /// @brief (1, 0) vector constant
        static const Vec<T,2> X_AXIS;
        /// @brief (0, 1) vector constant
        static const Vec<T,2> Y_AXIS;

        /*===========================*
         * Structors                 *
         *===========================*/
        
        /// Construct vector with both elements set to 0.
        Vec():detail::VecCommon<T,2>(){}
        
        /// Construct a vector with both elements set to `a`.
        Vec(T a):detail::VecCommon<T,2>(a){}

        /// Construct a vector with elements `(x, y)`.
        Vec(T x, T y){
            detail::VecBase<T,2>::x = x;
            detail::VecBase<T,2>::y = y;
        }
        
        /// Construct a vector with elements copied from the 2-element array `v`.
        Vec(T v[2]):detail::VecCommon<T,2>(v){}
        
        /*===========================*
         * Convenience Arithmetic    *
         *===========================*/
        
        //do not hide these; we want to overload them:
        using detail::VecCommon<T,2>::add;
        using detail::VecCommon<T,2>::sub;
        using detail::VecCommon<T,2>::scale;
        using detail::VecCommon<T,2>::dot;

        /// Addition. Convenience function with separate args for `x` and `y`.
        inline Vec<T,2> add(T dx, T dy) const {
            return Vec<T,2>(detail::VecBase<T,2>::x + dx, detail::VecBase<T,2>::y + dy);
        }

        /// Subtraction. Convenience function with separate args for `x` and `y`.
        inline Vec<T,2> sub(T dx, T dy) const {
            return Vec<T,2>(detail::VecBase<T,2>::x - dx, detail::VecBase<T,2>::y - dy);
        }

        /// Scalar multiplication. Convenience function with separate args for `x` and `y`.
        inline Vec<T,2> scale(T sx, T sy) const {
            return Vec<T,2>(detail::VecBase<T,2>::x*sx, detail::VecBase<T,2>::y*sy);
        }

        /// Dot product with the vector `(x1, y1)`.
        inline T dot(T x1, T y1) const {
            return detail::VecBase<T,2>::x*x1 + detail::VecBase<T,2>::y*y1;
        }

        /// @return A new vector with the X component reflected.
        inline Vec<T,2> reflectX() const {
            return Vec<T,2>(-detail::VecBase<T,2>::x, detail::VecBase<T,2>::y);
        }
        
        /// @return A new vector with the Y component reflected.
        inline Vec<T,2> reflectY() const {
            return Vec<T,2>(detail::VecBase<T,2>::x, -detail::VecBase<T,2>::y);
        }

        /// @return `true` if no elements are an infinity or `NaN`.
        bool isFiniteReal() const {
            return isReal(detail::VecBase<T,2>::x) && isReal(detail::VecBase<T,2>::y);
        }
        
        /*===========================*
         * 2D Geometry               *
         *===========================*/

        /** @return A vector perpendicular to `this`, created by a rotation of 
         * 90 degrees counterclockwise.
         */
        
        Vec<T,2> leftPerpendicular() const {
            return Vec<T,2>(-detail::VecBase<T,2>::y, detail::VecBase<T,2>::x);
        }
        
        /** @return A vector perpendicular to `this`, created by a rotation of 
         * 90 degrees clockwise.
         */
        Vec<T,2> rightPerpendicular() const {
            return Vec<T,2>(detail::VecBase<T,2>::y, -detail::VecBase<T,2>::x);
        }

        /** @return A vector rotated counterclockwise by the angle `radians`.
         * @param radians Rotation angle in radians
         */
        Vec<T,2> rotate(T radians) const {
            T sint = sin(radians);
            T cost = cos(radians);
            return Vec<T,2>(cost*detail::VecBase<T,2>::x - sint*detail::VecBase<T,2>::y,
                            sint*detail::VecBase<T,2>::x + cost*detail::VecBase<T,2>::y);
        }
        
        /** @return A vector rotated counterclockwise by the angle `radians` about
         * the point `center`.
         * @param center Center of rotation.
         * @param radians Angle of rotation.
         */
        Vec<T,2> rotate(Vec<T,2> center, T radians) const {
            return center + (((*this) - center).rotate(radians));
        }

        /**
         * @return The polar coordinates `(r, angle)` for this cartesian point,
         * with `angle` in radians.
         */
        inline Vec<T,2> toPolar() const {
            return Vec<T,2>(this->mag(), getAngle());
        }
        
        /**
         * @return The cartesian `(x, y)` coordinates of this point interpreted
         * as polar coordinates, with `this[0] = r` and `this[1] = radians`.
         */
        inline Vec<T,2> fromPolar() const {
            Vec<T,2> v;
            v.x = detail::VecBase<T,2>::y * cos(detail::VecBase<T,2>::x);
            v.y = detail::VecBase<T,2>::y * sin(detail::VecBase<T,2>::x);
            return v;
        }

        /**
         * @return The angle in radians to this vector from the x-axis, between 0 
         * and `2 * pi`.
         */
        inline T getAngle() const {
            T theta = (T)(atan2(detail::VecBase<T,2>::y,detail::VecBase<T,2>::x));
            if (theta < 0) theta += M_2_PI;
            return theta;
        }
        
        #ifdef GEOMC_VEC_USE_SWIZZLE
        const Vec<T,2> swizzle(const std::string& xyz) const {
            if (xyz.length() != 2){
                throw GeomException("Invalid 2D swizzle string: " + xyz);
            }
            return Vec<T,2>(swizzlecoord(xyz[0]),
                            swizzlecoord(xyz[1]));
        }
        #endif

    private:
        
        #ifdef GEOMC_VEC_USE_SWIZZLE
        T swizzlecoord(char c) const  {
            switch (tolower(c)){
                case 'x':
                    return detail::VecBase<T,2>::x;
                case 'y':
                    return detail::VecBase<T,2>::y;
                case '0':
                    return 0;
                case '1':
                    return 1;
                default:
                    throw GeomException("Invalid swizzle character: " + c);
            }
        }
        #endif

    }; //end Vec2 definition
    
    template <typename T> const Vec<T,2> Vec<T,2>::X_AXIS = Vec<T,2>(1,0);
    template <typename T> const Vec<T,2> Vec<T,2>::Y_AXIS = Vec<T,2>(0,1);

}; //end namespace geom

#endif /* Vec2_H_ */
