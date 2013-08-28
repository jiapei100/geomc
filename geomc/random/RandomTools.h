/*
 * RandomTools.h
 *
 *  Created on: Apr 10, 2011
 *      Author: tbabb
 */

//TODO: test whether using sines/cosines vs. test and discard is faster for 
//      unit ball/sphere random vec generation
//TODO: namespace?

#ifndef RANDOMTOOLS_H_
#define RANDOMTOOLS_H_

#include <vector>

#include <geomc/linalg/Vec.h>
#include <geomc/shape/Rect.h>
#include <geomc/random/Random.h>

namespace geom {

/**
 * @addtogroup random
 * @{
 */
 
/**
 * @ingroup random
 * Return a "default" convenience pseudorandom number generator, seeded with the system clock.
 * This generator is the default generator shared among many functions and classes
 * if no other generator is provided. Note that because it is shared and global,
 * it (and any code that calls it) is not re-entrant or threadsafe. For
 * thread-safe random number generation, construct your own per-thread `Random` objects
 * and pass them to your sampling functions and objects.
 */
Random* getRandom();

/**
 * Permute the elements of `objs` in-place.
 * 
 * Non re-entrant unless a safely-managed random number generator is supplied.
 * 
 * @param objs A list of objects to be permuted.
 * @param len Count of elements in `objs`.
 * @param rng A random number generator.
 */
template <typename T> void permute(T objs[], size_t len, Random& rng=*getRandom()) {
    for (size_t i = 0; i < len; i++) {
        size_t swapIdx = rng.rand(len - i) + i;
        T tmp = objs[swapIdx];
        objs[swapIdx] = objs[i];
        objs[i] = tmp;
    }
}

/**
 * Permute the elements of `objs` in-place.
 * 
 * Non re-entrant unless a safely-managed random number generator is supplied.
 * 
 * @param objs A list of objects to be permuted.
 * @param rng A random number generator.
 */
template <typename T> void permute(std::vector<T> &objs, Random& rng=*getRandom()) {
    for (unsigned long i = 0; i < objs.size(); i++) {
        unsigned long swapIdx = rng.rand(objs.size() - i) + i;
        T tmp = objs[swapIdx];
        objs[swapIdx] = objs[i];
        objs[i] = tmp;
    }
}

/**
 * @brief A class for sampling a variety of regions over `R`<sup>`N`</sup> space.
 * 
 * Example
 * -------
 * A random vector of unit length may be generated by:
 * 
 *     Random *rng = new MTRand(mySeed);
 *     Sampler<double> rvg(rng);
 *     Vec<double,3> p = rvg.unit<3>();
 * 
 */
template <typename T>
class Sampler {
public:
    Random *rng;
    
    /**********************************
     * Structors                      *
     **********************************/
    
    /**
     * Construct a new `Sampler` using the default (non-reentrant) 
     * pseudorandom number generator as the source of random bits.
     */
    Sampler():rng(getRandom()) {
        //do nothing else
    }
    
    /**
     * Construct a new `Sampler` with the supplied random number
     * generator as the source of random bits.
     * @param rng A (pseudo-) random number generator.
     */
    Sampler(Random *rng):rng(rng) {
        //do nothing else
    }
    
    /**********************************
     * Random N-D vectors             *
     **********************************/
    
    /**
     * @tparam N Dimension of generated sample.
     * @return A random vector of unit length. Distribution is uniform over
     * the surface of the `R`<sup>`N`</sup> sphere.
     */
    template <index_t N> Vec<T,N> unit() {
        Vec<T,N> v;
        do {
            for (index_t i = 0; i < N; i++) {
                // pick a point inside the unit cube
                v[i] = this->rng->template rand<T>(-1,1);
            }
            // discard points outside the unit ball
        } while (v.mag2() > 1);
        return v.unit(); // return point on ball surface
    }
    
    /**
     * @tparam N Dimension of generated sample.
     * @param radius Length of generated vector.
     * @return A random vector with length `radius`. Distribution is uniform over
     * the surface of the `R`<sup>`N`</sup> sphere.
     */
    template <index_t N> inline Vec<T,N> unit(T radius) {
        return unit<N>() * radius;
    }
    
    /**
     * @tparam N Dimension of generated sample.
     * @return A random vector inside the unit sphere. Samples are uniformly distributed
     * throughout the volume.
     */
    template <index_t N> Vec<T,N> solidball() {
        Vec<T,N> v;
        do {
            for (index_t i = 0; i < N; i++) {
                v[i] = this->rng->template rand<T>(-1,1);
            }
        } while (v.mag2() > 1);
        return v;
    }
    
    /**
     * @tparam N Dimension of generated sample.
     * @param radius Radius of sphere in which to sample.
     * @return A random vector inside the sphere of radius `radius`. Samples
     * are uniformly distributed throughout the volume.
     */
    template <index_t N> inline Vec<T,N> solidball(T radius) {
        T r = pow(this->rng->template rand<T>(), ((T)1)/N);
        return solidball<N>() * r * radius;
    }
    
    /**
     * @tparam N Dimension of generated sample.
     * @param minradius Minimum length of generated sample.
     * @param maxradius Maximum length of generated sample.
     * 
     * @return A random vector between the surfaces of two 
     * co-centric spheres of differing radius. Samples are uniformly distributed
     * throughout the volume.
     */
    template <index_t N> Vec<T,N> shell(T minradius, T maxradius) {
        T radius = pow(this->rng->template rand<T>(minradius/maxradius, 1), ((T)1)/N);
        return this->unit<N>() * radius * maxradius;
    }
    
    /**
     * @tparam N Dimension of generated sample.
     * @return A random vector inside the box `(0, 1)`<sup>`N`</sup>.
     */
    template <index_t N> Vec<T,N> box() {
        Vec<T,N> v;
        for (index_t i = 0; i < N; i++) {
            v[i] = this->rng->template rand<T>();
        }
        return v;
    }
    
    /**
     * @tparam N Dimension of generated sample.
     * @param lo Minimum coordinates of sample.
     * @param hi Maximum coordinates of sample.
     * @return A random vector inside the rectangle bounded by `lo` and `hi`.
     */
    template <index_t N> Vec<T,N> box(Vec<T,N> lo, Vec<T,N> hi) {
        Vec<T,N> v;
        for (index_t i = 0; i < N; i++) {  
            v[i] = this->rng->template rand<T>(lo[i], hi[i]);
        }
        return v;
    }
    
    /**
     * @tparam N Dimension of generated sample.
     * @param box Region to sample.
     * @return A random vector inside the provided region.
     */
    template <index_t N> Vec<T,N> box(const Rect<T,N> &box) {
        Vec<T,N> v;
        for (index_t i = 0; i < N; i++) {
            v[i] = this->rng->template rand<T>(box.min()[i], box.max()[i]);
        }
        return v;
    }
    
    /**********************************
     * Random 3D vectors              *
     **********************************/
    
    // todo: what if we made a bunch of Distribution objects?
    // that way, they could cache certain internal parameters and
    // dump out many samples quickly.
    
    /**
     * @param half_angle_radians Angle from the z+ axis to the cap's edge.
     * @return A random 3D vector of unit length uniformly distributed
     * over the spherical cap with internal angle `half_angle_radians`.
     */
    Vec<T,3> cap(T half_angle_radians) {
        // we pick a height with uniform probability
        // because a slice through the sphere with thickness t at height h
        // has unchanging area regardless of h. 
        T min_h = cos(half_angle_radians);
        T h = this->rng->template rand<T>(min_h, 1);
        T r = sqrt(1 - h * h);
        T theta = this->rng->template rand<T>(2*M_PI);
        return Vec<T,3>(r * cos(theta), r * sin(theta), h);
    }
    
    /**
     * @param dir Direction about which to sample.
     * @param half_angle_radians Angle from `dir` to sample within.
     * @return A random 3D vector of unit length uniformly distributed over
     * the spherical cap with center along direction `dir`, with angle `half_angle_radians`
     * between the axis and edge of the cap.
     */
    inline Vec<T,3> cap(Vec<T,3> dir, T half_angle_radians) {
        Vec<T,3> v = this->cap(half_angle_radians);
        Vec<T,3> rotAxis = Vec<T,3>(0,0,1).cross(dir);
        if (rotAxis.isZero()) return v;
        return v.rotate(rotAxis, std::acos(dir.z/dir.mag()));
    }
    
    /**
     * @param dir Direction about which to sample.
     * @param half_angle_radians Angle from `dir` to sample within.
     * @param minR Minimum distance of sample from origin.
     * @param maxR Maximum distance of sample from origin.
     * @return A random 3D vector inside the cone with axial angle `half_angle_radians`
     * and between the surfaces of two spheres with radii `minR` and `maxR`, uniformly
     * distributed throughout the volume.
     */
    inline Vec<T,3> solidcap(Vec<T,3> dir, T half_angle_radians, T minR, T maxR) {
        T r = this->rng->template rand<T>(minR / maxR, 1);
        r = maxR * cbrt(r);
        return r * this->cap(dir, half_angle_radians);
    }
    
    /**
     * @param normal Normal of the disk to sample.
     * @param r Radius of disk to sample.
     * @return A random 3D vector on the surface of a disk of radius `r` and normal `n`.
     */
    Vec<T,3> oriented_disk(Vec<T,3> normal, T r) {
        //choose a point on 2D disk first, then
        //rotate the disk to be aligned with <axis>
        Vec<T,3> raxis = Vec<T,3>(0,0,1).cross(normal);
        Vec<T,3> disk = this->disk(r).template resized<3>();
        if (raxis.isZero()) return disk;
        return disk.rotate(raxis, std::acos(normal.z/normal.mag()));
    }

    /**
     * @return A random 3D vector inside the axis-aligned box bounded by the provided coordinates.
     */
    inline Vec<T,3> box(T minx, T miny, T minz,
                        T maxx, T maxy, T maxz) {
        return Vec<T,3>(
                this->rng->template rand<T>(minx, maxx),
                this->rng->template rand<T>(miny, maxy),
                this->rng->template rand<T>(minz, maxz)
        );
    }
    
    /**********************************
     * Random 2D vectors              *
     **********************************/
    
    /**
     * Synonym for `solidball<2>()`.
     * 
     * @return A random 2D vector inside a circle with radius 1 and centered at the origin.
     * Samples are uniformly distributed across the area of the disk.
     */
    inline Vec<T,2> disk() {
        return solidball<2>();
    }
    
    /**
     * Synonym for `solidball<2>(T)`.
     * 
     * @param radius
     * @return A random 2D vector inside a circle with radius `r` and centered at the origin.
     * Samples are uniformly distributed across the area of the disk.
     */
    inline Vec<T,2> disk(T radius) {
        return solidball<2>() * radius;
    }
    
    /**
     * Generate a unit 2D vector with random angle. The entire rangle of angles 
     * between `min_radians` and `max_radians` may be sampled, so consider whether 
     * to use a negative minimum for ranges of direction which cross the +x axis.
     * 
     * @param min_radians Minimum angle of generated vector counterclockwise from the +x axis, in radians.
     * @param max_radians Maximum angle of generated vector counterclockwise from the +x axis, in radians.
     * 
     * @return A random 2D vector of unit length with heading between `min_radians`
     * and `max_radians`. 
     */
    inline Vec<T,2> arc(T min_radians, T max_radians) {
        T angle = this->rng->template rand<T>(min_radians, max_radians);
        return Vec<T,2>(cos(angle), sin(angle));
    }
    
    /**
     * 
     * Generate a 2D vector with random angle and a random length. The entire rangle of angles 
     * between `min_radians` and `max_radians` may be sampled, so consider whether 
     * to use a negative minimum for ranges of direction which cross the +x axis. The 
     * generated samples will be uniformly distributed throughout the sampled area.
     * 
     * @param min_radians Minimum angle of generated vector counterclockwise from the +x axis, in radians.
     * @param max_radians Maximum angle of generated vector counterclockwise from the +x axis, in radians.
     * @param min_radius Minimum length of generated vector.
     * @param max_radius Maximum length of generated vector.
     * 
     * @return A random 2D vector of length between `min_radius` and `max_radius` 
     * and heading between `min_radians` and `max_radians`. 
     */
    Vec<T,2> solidarc(T min_radians, T max_radians,
                      T min_radius,  T max_radius) {
        T angle = this->rng->template rand<T>(min_radians, max_radians);
        T len   = max_radius * std::sqrt(this->rng->template rand<T>(min_radius / max_radius, 1));
        return Vec<T,2>(len,angle).fromPolar();
    }
    
    /**
     * @return A random 2D vector inside the axis-aligned box bounded by the provided coordinates.
     */
    inline Vec<T,2> box(T minx, T miny, 
                        T maxx, T maxy) {
        return Vec<T,2>(this->rng->template rand<T>(minx, maxx),
                        this->rng->template rand<T>(miny, maxy));
    }
};

/// @} ingroup random

}; //end namespace geom

#endif /* RANDOMTOOLS_H_ */
