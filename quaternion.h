/* Author: Arash Ghodsi (aghodsi)
   Class: CMPS161 - Animation & Visualization
   Term: Winter 2011
   File: quaternion.h - Interface for the Quaternion class
   prog3: Simulate a hula skirt using physically based animation. The animation is generated using
          Hooke's law for springs on the edges of the triangle mesh skirt, and rotation quaternions
          or versors for the oscillatory motion.
          The user can control the amplitude and frequency of the oscillation and whether the motion
          is 2-dimensional about the z-axis or 3-dimensional about both the x-axis and z-axis,
          independently. Finally, the user can switch in and out of wireframe rendering. Please see
          the README for controls.
 */

#ifndef QUATERNION_H
#define QUATERNION_H

/* A wrapper class for quaternions.
 */
class Quaternion
{
public:
//::PUBLIC CONSTANTS:://
   static const float TO_RADIANS;
   
   //constructors
   Quaternion(float x = 0, float y = 0, float z = 0);
   Quaternion(float angle, float x, float y, float z);
   Quaternion(const Quaternion &Q);
   
   //calculates and returns the inverse of this quaternion
   Quaternion inverse() const;
   
//::ACCESSORS:://
   float getS() const { return q[S]; }
   float getX() const { return q[X]; }
   float getY() const { return q[Y]; }
   float getZ() const { return q[Z]; }
   
//::MUTATORS:://
   //makes this quaternion a unit quaternion
   void normalize();

//::OVERLOADED OPERATORS:://
   Quaternion& operator=(const Quaternion &Q);
   Quaternion operator+(const Quaternion &Q) const;
   Quaternion operator*(const Quaternion &Q) const;
   
//::FRIEND FUNCTIONS:://
   //calculates slerp (spherical linear interpolation) based off two versors
   friend Quaternion slerp(Quaternion &q1, Quaternion &q2, float step);
   
private:
//::PRIVATE CONSTANTS:://
   static const int S, X, Y, Z;
   
//::VARIABLES:://
   float q[4];
};

#endif // QUATERNION_H
