/*
 * Quaternion.h
 *
 *  Created on: Mar 30, 2013
 *      Author: tbabb
 */

#ifndef QUATERNION_H_
#define QUATERNION_H_

#include <cmath>
#include <geomc/linalg/Vec.h>

namespace geom {

/** @ingroup linalg 
 *  @brief Quaternion class.
 * 
 * `(x, y, z)` is the vector part, while `w` is the real part. This class differs
 * slightly from convention in that the real part is the last coordinate rather than
 * the first; this scheme was chosen to maintain naming consistency with the
 * rest of the library.
 */
template <typename T>
class Quat : public Vec<T,4> {
public:
    
    /*******************************
     * Constructors                *
     *******************************/
    
    /// Construct a quaternion with all elements 0
    Quat():Vec<T,4>(){}
    /// Construct a quaternion with elements `(x, y, z, w)`
    Quat(T x, T y, T z, T w):Vec<T,4>(x,y,z,w){}
    /// Construct a quaternion from the contents of the 4-element array `v`
    Quat(T v[4]):Vec<T,4>(v){}
    /// Construct a quaternion with vector part `v` and real part `w`.
    Quat(const Vec<T,3> &v, T w):Vec<T,4>(v,w){}
    /// Construct a quaternion from the 4D vector `v`.
    Quat(const Vec<T,4> &v):Vec<T,4>(v){} // is this explicitly necessary?
    
    /*******************************
     * Static constructors         *
     *******************************/
    
    /**
     * Rotation quaternion from an axis and angle
     * @param axis Axis of rotation (not necessesarily a unit vector)
     * @param angle Angle of rotation in radians
     * @return A unit quaternion representing a rotation of `angle` radians
     * about `axis`.
     */
    static Quat<T> rotFromAxisAngle(const Vec<T,3> &axis, T angle) {
        if (axis.x == 0 && axis.y == 0 && axis.z == 0){
            return Vec<T,4>(axis, 1);
        } else {
            return Vec<T,4>(axis.unit() * std::sin(angle*0.5), std::cos(angle*0.5));
        }
    }

    /*******************************
     * Operators                   *
     *******************************/
    
    /// Quaternion multiplication
    friend inline Quat<T> operator*(const Quat<T> &q1, const Quat<T> &q2) {
        return q1.mult(q2);
    }
    
    /// Quaternion-vector multiplication (`v`'s real part is zero)
    friend inline Quat<T> operator*(const Quat<T> &q, const Vec<T,3> &v) {
        return q.mult(Quat<T>(v, 0));
    }
    
    /*******************************
     * Methods                     *
     *******************************/
    
    /// @return The vector part of this quaternion
    inline Vec<T,3> vectorPart() const {
        return Vec<T,3>(
                detail::VecBase<T,4>::x,
                detail::VecBase<T,4>::y,
                detail::VecBase<T,4>::z);
    }
    
    /// @return The scalar part of this quaternion
    inline T scalarPart() const {
        return detail::VecBase<T,4>::w;
    }
    
    /// @return The complex conjugate of this quaternion; also the inverse rotation.
    inline Quat<T> conj() const {
        return Quat<T>(-detail::VecBase<T,4>::x, 
                       -detail::VecBase<T,4>::y, 
                       -detail::VecBase<T,4>::z, 
                        detail::VecBase<T,4>::w);
    }
    
    /// Quaternion multiplication
    inline Quat<T> mult(const Quat<T> &q) const {
        Quat<T> result;
        
        const T &x = detail::VecBase<T,4>::x;
        const T &y = detail::VecBase<T,4>::y;
        const T &z = detail::VecBase<T,4>::z;
        const T &w = detail::VecBase<T,4>::w;
        
        result.x = w*q.x + x*q.w + y*q.z - z*q.y;
        result.y = w*q.y - x*q.z + y*q.w + z*q.x;
        result.z = w*q.z + x*q.y - y*q.x + z*q.w;
        result.w = w*q.w - x*q.x - y*q.y - z*q.z;
        
        return result;
    }
    
    /// Apply this unit quaternion as a rotation to `v`
    Vec<T,3> applyRotation(const Vec<T,3> &v) const {
        const Quat<T> &q = *this;
        Quat<T> qv(v,0);
        Quat<T> result = q * qv * q.conj();
        return result.vectorPart();
    }
    
    /** Convert this unit quaternion to an axis-angle rotation representation
     *  `(x, y, z, radians)`.
     */
    Vec<T,4> rotToAxisAngle() const {
        T w_clamp = std::min(std::max(detail::VecBase<T,4>::w, -1.0), 1.0);
        double alpha = 2*std::acos(w_clamp);
        if (detail::VecBase<T,4>::x == 0 && detail::VecBase<T,4>::y == 0 && detail::VecBase<T,4>::z == 0){
            return Vec<T,4>(1, 0, 0, 0);
        } else {
            return Vec<T,4>(vectorPart().unit(), alpha);
        }
    }
    
    /**
     * Convert this unit quaternion to an angular velocity representation, with
     * the magnitude of the axis representing the rate of rotation in radians.
     */
    Vec<T,3> rotToAngularVelocity() const {
        // assumes a unit quaternion.
        T w_clamp = std::min(std::max(detail::VecBase<T,4>::w, -1.0), 1.0);
        T alpha = 2 * std::acos(w_clamp);
        if (vectorPart().isZero()){
            return Vec<T,3>::zeros;
        } else {
            return alpha * vectorPart().unit();
        }
    }
    
    /** Interpolate this unit quaternion with the null rotation according
     *  to the interpolation parameter `0 <= t <= 1`.
     */
    
    // this may go the "long way" around. that what we want?
    Quat<T> rotScale(T t) const {
        // assumes a unit quaternion.
        const T w = std::max(-1, std::min(Vec<T,4>::w, 1));
        T omega = acos(w);
        T theta = omega * t;
        return Quat<T>(sin(theta) * vectorPart().unit(), cos(theta));
    }

    /**
     * Spherical linear interpolation, per Ken Shoemake. Interpolate smoothly
     * between two quaternion orientation states.
     * @param q1 The rotation at `t = 1`.
     * @param t Interpolation parameter between 0 and 1.
     * @return 
     */
    Quat<T> slerp(const Quat<T> &q1, T t) const {
        // assumed unit quaternion.
        const Quat<T> &q0 = *this;
        T dot = q0.dot(q1);
        if (dot < 0){ //angle is greater than 90. Avoid "long path" rotations.
            dot = -dot;
            q1 = -q1;
        }
        dot = std::max(0, std::min<T>(1, dot)); //keep <dot> in the domain of acos.
        T omega = acos(dot);
        T theta = omega * t;
        Quat<T> q2 = (q1 - q0 * dot).unit();
        return cos(theta) * q0 + sin(theta) * q2;
    }
    
    /*
    
    // TODO: test this.
    Quat<T> exp() const {
        T m = vectorPart().mag();
        T e = exp(detail::VecBase<T,4>::w);
        return e * Quat<T>(sin(m) * vectorPart() / m,
                           cos(m));
    }
    
    // TODO: test this.
    Quat<T> ln() const {
        T mv = vectorPart().mag();
        T mq = mag();
        T w = detail::VecBase<T,4>::w;
        return Quat<T>(
            std::acos(s/mq) * vectorPart() / mv,
            std::ln(mq));
    }
    
    // TODO: test this.
    Quat<T> pow(T a) const {
        // this doesn't look like it can be right.
        // taken from a formula 
        return Quat<T>(
                std::exp(std::ln(detail::VecBase::x) * a),
                std::exp(std::ln(detail::VecBase::y) * a),
                std::exp(std::ln(detail::VecBase::z) * a),
                std::exp(std::ln(detail::VecBase::w) * a));
    }
    */
    
};


}; //namespace geom

#endif /* QUATERNION_H_ */
