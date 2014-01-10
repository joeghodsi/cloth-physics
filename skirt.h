/* Author: Arash Ghodsi (aghodsi)
   Class: CMPS161 - Animation & Visualization
   Term: Winter 2011
   File: skirt.h - Interface for the Skirt class
   prog3: Simulate a hula skirt using physically based animation. The animation is generated using
          Hooke's law for springs on the edges of the triangle mesh skirt, and rotation quaternions
          or versors for the oscillatory motion.
          The user can control the amplitude and frequency of the oscillation and whether the motion
          is 2-dimensional about the z-axis or 3-dimensional about both the x-axis and z-axis,
          independently. Finally, the user can switch in and out of wireframe rendering. Please see
          the README for controls.
 */

#ifndef SKIRT_H
#define SKIRT_H

#include <GL/gl.h> //used for various gl types and functions

/* The primary class for the program. Performs the physically based animation of a cloth/spring
 * system used to render a skirt.
 * This class is responsible for the following:
 * 1: Generating the skirt as a triangular mesh
 * 2: Texturing the mesh as a hula skirt
 * 3: Maintaining the spring force system governing the animation of the skirt
 * 4: Animating the skirt using rotation quaternions (versors)
 */
class Skirt
{
public:
   //constructor
   Skirt();
   //destructor
   ~Skirt();
   //draws the skirt mesh using triangle strips after calling subroutines to update the skirt state.
   void draw();
   //loads a texture for the skirt. The texture image must be a P6 RAW ppm.
   void loadTexture() const;
   
//::ACCESSORS:://
   GLfloat getHeight() const { return height; }
   
//::MUTATORS:://
   //changes the animation to a 2D rotation about the z-axis
   void rotate2D() { is3DRotation = false; }
   //changes the animation to a 3D rotation about the x-axis and the z-axis independently
   void rotate3D() { is3DRotation = true; }
   //decreases the amplitude of the motion
   void decAmplitude() { if(amplitude > AMP_MIN) amplitude -= AMP_INC; }
   //increases the amplitude of the motion
   void incAmplitude() { if(amplitude < AMP_MAX) amplitude += AMP_INC; }
   //decreases the frequency of the motion
   void decFrequency() { if(frequency > FREQ_MIN) frequency -= FREQ_INC; }
   //increases the frequency of the motion
   void incFrequency() { if(frequency < FREQ_MAX) frequency += FREQ_INC; }
   
private:
//::STRUCTS:://
   struct Vertex { GLfloat x, y, z; };
   struct Vector { GLfloat x, y, z; };

//::CONSTANTS:://
   static const int   X_RES, Y_RES;
   static const float GRAVITY, Ks, KsDiag, Kd, Hp, Hv, AMP_MIN, AMP_MAX, AMP_INC,\
                      FREQ_MIN, FREQ_MAX, FREQ_INC;
   
//::VARIABLES:://
   Vertex *initialPos, **position;
   Vector **velocity, **vertexNormals;
   GLfloat height, restLength, amplitude, frequency, theta;
   bool is3DRotation;
   
//::PRIVATE MEMBER FUNCTIONS:://
   //generates the initial state/position of the skirt vertices 
   void generateVertices();
   //calls subroutines for recalculating the vertex positions, velocities, and normals 
   void updateSkirt();
   //updates the vertex positions via Euler integration of the vertex velocities
   void updatePosition();
   //updates the vertex velocities via Euler integration using the spring forces, gravity, and
   //oscillatory forces as accelerations
   void updateVelocity();
   //calculates the oscillatory acceleration applied to the top row of free-motion vertices
   void calcOscillatoryAcc();
   //determines the x components of the spring forces
   GLfloat springX(int c, int r, float Fs1, float Fs2, float Fs3,
                                 float Fs4, float Fs5, float Fs6) const;
   //determines the y components of the spring forces
   GLfloat springY(int c, int r, float Fs1, float Fs2, float Fs3,
                                 float Fs4, float Fs5, float Fs6) const;
   //determines the z components of the spring forces
   GLfloat springZ(int c, int r, float Fs1, float Fs2, float Fs3,
                                 float Fs4, float Fs5, float Fs6) const;
   //helper for springX. Used to determine the percent of a force to give to x
   GLfloat Fx(int col1, int row1, int col2, int row2) const;
   //helper for springY. Used to determine the percent of a force to give to y
   GLfloat Fy(int col1, int row1, int col2, int row2) const;
   //helper for springZ. Used to determine the percent of a force to give to z
   GLfloat Fz(int col1, int row1, int col2, int row2) const;
   //returns the current length of a spring defined by the parameters
   GLfloat currentLength(int col1, int row1, int col2, int row2) const;
   //calculates the vertex normals
   void calcNorms();
   //resets the vertex normals to zero so they can be recalculated
   void resetNorms();
   //calculates the face normals for the triangles used to generate the skirt mesh
   Vector calcFaceNorm(Vector v1, Vector v2) const;
   //updates the normals of the vertices which share the same polygon to include its face normal
   void updateVertNorms(Vector faceNorm, Vector &vert1Norm, Vector &vert2Norm, Vector &vert3Norm);
};

#endif //SKIRT_H
