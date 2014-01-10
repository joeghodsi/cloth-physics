/* Author: Arash Ghodsi (aghodsi)
   Class: CMPS161 - Animation & Visualization
   Term: Winter 2011
   File: quaternion.cpp - Implementation for the Quaternion class
   prog3: Simulate a hula skirt using physically based animation. The animation is generated using
          Hooke's law for springs on the edges of the triangle mesh skirt, and rotation quaternions
          or versors for the oscillatory motion.
          The user can control the amplitude and frequency of the oscillation and whether the motion
          is 2-dimensional about the z-axis or 3-dimensional about both the x-axis and z-axis,
          independently. Finally, the user can switch in and out of wireframe rendering. Please see
          the README for controls.
 */

#include "quaternion.h"
#include <cmath> //used for pow(), sqrt(), sin(), cos(), acos()

const int Quaternion::S = 0, Quaternion::X = 1, Quaternion::Y = 2, Quaternion::Z = 3;
const float Quaternion::TO_RADIANS = acos(-1.0)/180;

/* Quaternion - 3 argument CONSTRUCTOR for a quaternion representation of a point
 */
Quaternion::Quaternion(float x, float y, float z)
{
   q[S] = 0;
   q[X] = x;
   q[Y] = y;
   q[Z] = z;
}

/* Quaternion - 4 argument CONSTRUCTOR for constructing a versor
 */
Quaternion::Quaternion(float angle, float x, float y, float z)
{
   float sinCoeff = sin((TO_RADIANS*angle)/2), mag = sqrt(pow(x,2) + pow(y,2) + pow(z,2));
   
   q[S] = cos((TO_RADIANS*angle)/2);
   q[X] = (x/mag)*sinCoeff;
   q[Y] = (y/mag)*sinCoeff;
   q[Z] = (z/mag)*sinCoeff;
}

/* Quaternion - copy CONSTRUCTOR
 */
Quaternion::Quaternion(const Quaternion &Q)
{
   q[S] = Q.q[S];
   q[X] = Q.q[X];
   q[Y] = Q.q[Y];
   q[Z] = Q.q[Z];
}

/* calculates and returns the inverse of this quaternion
 */
Quaternion Quaternion::inverse() const
{
   Quaternion inverse;
   
   inverse.q[S] = q[S];
   inverse.q[X] = -q[X];
   inverse.q[Y] = -q[Y];
   inverse.q[Z] = -q[Z];
   
   return inverse;
}

/* makes this quaternion a unit quaternion
 */
void Quaternion::normalize()
{
   float mag = sqrt(pow(q[X],2) + pow(q[Y],2) + pow(q[Z],2));
   q[X] /= mag;
   q[Y] /= mag;
   q[Z] /= mag;
}

/* Overloaded assignment operator
 */
Quaternion& Quaternion::operator=(const Quaternion &Q)
{
   if(this != &Q){
      q[S] = Q.q[S];
      q[X] = Q.q[X];
      q[Y] = Q.q[Y];
      q[Z] = Q.q[Z];
   }
   
   return *this;
}

/* Overloaded addition operator
 */
Quaternion Quaternion::operator+(const Quaternion &Q) const
{
   Quaternion sum;
   
   sum.q[S] = q[S] + Q.q[S];
   sum.q[X] = q[X] + Q.q[X];
   sum.q[Y] = q[Y] + Q.q[Y];
   sum.q[Z] = q[Z] + Q.q[Z];
   
   return sum;
}

/* Overloaded product operator
 */
Quaternion Quaternion::operator*(const Quaternion &Q) const
{
   Quaternion product;
   
   product.q[S] = q[S]*Q.q[S] - q[X]*Q.q[X] - q[Y]*Q.q[Y] - q[Z]*Q.q[Z];
   product.q[X] = q[S]*Q.q[X] + q[X]*Q.q[S] + q[Y]*Q.q[Z] - q[Z]*Q.q[Y];
   product.q[Y] = q[S]*Q.q[Y] - q[X]*Q.q[Z] + q[Y]*Q.q[S] + q[Z]*Q.q[X];
   product.q[Z] = q[S]*Q.q[Z] + q[X]*Q.q[Y] - q[Y]*Q.q[X] + q[Z]*Q.q[S];
   
   return product;
}

/* calculates slerp (spherical linear interpolation) based off two versors
 */
Quaternion slerp(Quaternion &q1, Quaternion &q2, float step)
{
   float theta = acos(q1.q[1]*q2.q[1] + q1.q[2]*q2.q[2] + q1.q[3]*q2.q[3]),
         q1Coeff = sin((1-step)*theta)/sin(theta), q2Coeff = sin(step*theta)/sin(theta);
   Quaternion result;
   
   result.q[1] = q1Coeff*q1.q[1] + q2Coeff*q2.q[1];
   result.q[2] = q1Coeff*q1.q[2] + q2Coeff*q2.q[2];
   result.q[3] = q1Coeff*q1.q[3] + q2Coeff*q2.q[3];
   
   return result;
}
