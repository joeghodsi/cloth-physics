/* Author: Arash Ghodsi (aghodsi)
   Class: CMPS161 - Animation & Visualization
   Term: Winter 2011
   File: skirt.cpp - Implementation for the Skirt class
   prog3: Simulate a hula skirt using physically based animation. The animation is generated using
          Hooke's law for springs on the edges of the triangle mesh skirt, and rotation quaternions
          or versors for the oscillatory motion.
          The user can control the amplitude and frequency of the oscillation and whether the motion
          is 2-dimensional about the z-axis or 3-dimensional about both the x-axis and z-axis,
          independently. Finally, the user can switch in and out of wireframe rendering. Please see
          the README for controls.
 */

#include "skirt.h"
#include "quaternion.h"
#include <cstdlib> //used for exit() and EXIT_FAILURE
#include <cstdio> //used for fclose(), fopen(), printf(), fscanf(), sscanf(), fgetc(), fread(), FILE
#include <cmath> //used for pow(), sqrt(), sin(), cos()
#include <limits> //used for numeric_limits<float>::infinity()
#include <cstring> //used for strncmp()
#include <GL/glu.h> //used for gluBuild2DMipmaps()

using namespace std;

//::CONSTANTS:://
const int   Skirt::X_RES = 120, Skirt::Y_RES = 18;
const float Skirt::GRAVITY = 0.015*(-9.8), Skirt::Ks = 1.5, Skirt::KsDiag = 0.7, Skirt::Kd = 0.01,
            Skirt::Hp = 0.15, Skirt::Hv = 0.1,
            Skirt::AMP_MIN = 0, Skirt::AMP_MAX = 30, Skirt::AMP_INC = 2,
            Skirt::FREQ_MIN = 0, Skirt::FREQ_MAX = 0.1, Skirt::FREQ_INC = 0.02;

/* Skirt - CONSTRUCTOR
 */
Skirt::Skirt() 
{
   initialPos = new Vertex[X_RES];
   position = new Vertex*[X_RES];
   velocity = new Vector*[X_RES];
   vertexNormals = new Vector*[X_RES];
   for(int i = 0; i < X_RES; i++){
      position[i] = new Vertex[Y_RES];
      velocity[i] = new Vector[Y_RES];
      vertexNormals[i] = new Vector[Y_RES];
      for(int j = 0; j < Y_RES; j++){
         velocity[i][j].x = velocity[i][j].y = velocity[i][j].z = 0;
         vertexNormals[i][j].x = vertexNormals[i][j].y = vertexNormals[i][j].z = 0;
      }
   }
   generateVertices();
   
   amplitude = AMP_MIN;
   frequency = FREQ_MIN;
   theta = 0;
   is3DRotation = true;
}

/* Skirt - DESTRUCTOR
 */
Skirt::~Skirt()
{
   for(int i = 0; i < X_RES; i++){
      delete position[i];
      delete velocity[i];
      delete vertexNormals[i];
   }
   delete initialPos;
   delete position;
   delete velocity;
   delete vertexNormals;
}

/* draws the skirt mesh using triangle strips after calling subroutines to update the skirt state.
 */
void Skirt::draw()
{
   updateSkirt();
   for(int j = 0; j < Y_RES-1; j++){
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0, GLfloat(j)/Y_RES);
      glVertex3f(position[0][j].x, position[0][j].y, position[0][j].z);
      glTexCoord2f(0, GLfloat(j+1)/Y_RES);
      glVertex3f(position[0][j+1].x, position[0][j+1].y, position[0][j+1].z);
      for(int i = 1; i < X_RES; i++){
         glNormal3f(vertexNormals[i][j].x, vertexNormals[i][j].y, vertexNormals[i][j].z);
         glTexCoord2f(GLfloat(i)/(X_RES+10), GLfloat(j)/Y_RES);
         glVertex3f(position[i][j].x, position[i][j].y, position[i][j].z);
         glNormal3f(vertexNormals[i][j+1].x, vertexNormals[i][j+1].y, vertexNormals[i][j+1].z);
         glTexCoord2f(GLfloat(i)/(X_RES+10), GLfloat(j+1)/Y_RES);
         glVertex3f(position[i][j+1].x, position[i][j+1].y, position[i][j+1].z);
      }
      glNormal3f(vertexNormals[0][j].x, vertexNormals[0][j].y, vertexNormals[0][j].z);
      glVertex3f(position[0][j].x, position[0][j].y, position[0][j].z);
      glNormal3f(vertexNormals[0][j+1].x, vertexNormals[0][j+1].y, vertexNormals[0][j+1].z);
      glVertex3f(position[0][j+1].x, position[0][j+1].y, position[0][j+1].z);
      glEnd();
   }
}

/* loads a texture for the skirt. The texture image must be a P6 RAW ppm.
 */
void Skirt::loadTexture() const
{
   GLuint texture;
   int i = 0, texWidth, texHeight, junk;
   char header[70];
   unsigned char *image;
   FILE *in = fopen("assets/skirt_texture.ppm", "rb");
   if(!in){
      printf("Unable to open texture for reading\n");
      exit(EXIT_FAILURE);
   }
   //read in header data
   fscanf(in, "%s", header);
   if(strncmp(header, "P6", 2)){
      printf("Incompatible image format. Please load a P6 (RAW) PPM.");
      exit(EXIT_FAILURE);
   }
   while(i < 3){
      fscanf(in, "%s", header);
      if(header[0] != '#'){
         if(i == 0)       i += sscanf(header, "%i %i %i", &texWidth, &texHeight, &junk);
         else if (i == 1) i += sscanf(header, "%i %i", &texHeight, &junk);
         else if (i == 2) i += sscanf(header, "%i", &junk);
      }
   }
   
   fgetc(in);
   //read in pixel data
   image = new unsigned char[texWidth*texHeight*3];
   fread(image, sizeof(unsigned char), texWidth*texHeight*3, in);
   fclose(in);
   
   //initialize texturing using image pixel data
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   //using modulate to mix texture with color for shading
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texWidth,  texHeight, GL_RGB, GL_UNSIGNED_BYTE, image);
   
   delete image;
}

//::PRIVATE MEMBER FUNCTIONS:://////////////////////////////////////////////////////////////////////

/* generates the initial state/position of the skirt vertices 
 */
void Skirt::generateVertices()
{
   const GLfloat girth = 0.6;
   
   restLength = 2*sin(Quaternion::TO_RADIANS*(360.0/X_RES)/2); //secant or chord length
   height = Y_RES*restLength;
   for(int j = 0; j < Y_RES; j++){
      for(int i = 0; i < X_RES; i++){
         position[i][j].x = (0.1*j+1)*cos(i*Quaternion::TO_RADIANS*(360.0/X_RES))*girth;
         position[i][j].z = (0.1*j+1)*sin(i*Quaternion::TO_RADIANS*(360.0/X_RES));
         position[i][j].y = -1*(j+10)*restLength;
      }
   }
   for(int i = 0; i < X_RES; i++){
      initialPos[i].x = position[i][0].x;
      initialPos[i].y = position[i][0].y;
      initialPos[i].z = position[i][0].z;
   }
   calcNorms();
}

/* calls subroutines for recalculating the vertex positions, velocities, and normals 
 */
void Skirt::updateSkirt()
{
   updateVelocity();
   updatePosition();
   calcNorms();
}

/* updates the vertex positions via Euler integration of the vertex velocities
 */
void Skirt::updatePosition()
{
   for(int j = 1; j < Y_RES; j++)
      for(int i = 0; i < X_RES; i++){
         position[i][j].x += Hp*velocity[i][j].x;
         position[i][j].y += Hp*velocity[i][j].y;
         position[i][j].z += Hp*velocity[i][j].z;
      }
}

/* updates the vertex velocities via Euler integration using the spring forces, gravity, and
 * oscillatory forces as accelerations
 */
void Skirt::updateVelocity()
{
   float FsBelow, FsAbove, FsLeft, FsRight, FsDiagAbove, FsDiagBelow, ks, kd;
   
   //Velocity Update: Oscillation
   calcOscillatoryAcc();
   for(int j = 2; j < Y_RES; j++){
      ks = Ks + 2*(Y_RES - j);
      kd = Kd + 0.005*(Y_RES - j);
      for(int i = 0; i < X_RES; i++){
         FsBelow = (j == Y_RES-1) ? 0 : ks*(currentLength(i,j, i,j+1) - restLength);
         FsAbove = ks*(currentLength(i,j, i,j-1) - restLength);
         FsLeft = (i == 0) ? ks*(currentLength(i,j, X_RES-1,j) - restLength) :
                                  ks*(currentLength(i,j, i-1,j) - restLength);
         FsRight = (i == X_RES-1) ? ks*(currentLength(i,j, 0,j) - restLength) :
                                  ks*(currentLength(i,j, i+1,j) - restLength);
         FsDiagBelow = (j == Y_RES-1) ? 0 :
                       (i == X_RES-1) ? ks*(currentLength(i,j, 0,j+1) - restLength) :
                                  ks*(currentLength(i,j, i+1,j+1) - restLength);
         FsDiagAbove = (i == 0) ? ks*(currentLength(i,j, X_RES-1,j-1) - restLength) :
                                  ks*(currentLength(i,j, i-1,j-1) - restLength);
         //Velocity Update: Spring Forces
         velocity[i][j].x += 
            Hv*springX(i, j, FsBelow, FsAbove, FsLeft, FsRight, FsDiagBelow, FsDiagAbove);
         velocity[i][j].y += 
            Hv*springY(i, j, FsBelow, FsAbove, FsLeft, FsRight, FsDiagBelow, FsDiagAbove);
         velocity[i][j].z += 
            Hv*springZ(i, j, FsBelow, FsAbove, FsLeft, FsRight, FsDiagBelow, FsDiagAbove);
         //Velocity Update: Gravity
         velocity[i][j].y += Hv*GRAVITY;
         //Velocity Update: Spring Damping
         velocity[i][j].x -= kd*velocity[i][j].x;
         velocity[i][j].y -= kd*velocity[i][j].y;
         velocity[i][j].z -= kd*velocity[i][j].z;
      }
   }
}

/* calculates the oscillatory acceleration applied to the top row of free-motion vertices
 */
void Skirt::calcOscillatoryAcc()
{
   int minVertex, maxVertex;
   float yMin = numeric_limits<float>::infinity(), yMax = -yMin;
   bool isOscillating = false;
   
   theta += frequency;
   Quaternion xrot(amplitude*cos(-theta), 1, 0, 0);
   Quaternion zrot(amplitude*sin(-theta), 0, 0, 1);
   for(int i = 0; i < X_RES; i++){
      Quaternion p(initialPos[i].x, initialPos[i].y, initialPos[i].z), rot(xrot*p*xrot.inverse());
      if(is3DRotation) rot = zrot*rot*zrot.inverse();
      if(!isOscillating && ((position[i][0].x - rot.getX() != 0) ||
         (position[i][0].y - rot.getY() != 0) || (position[i][0].z - rot.getZ() != 0)))
         isOscillating = true;
         
      position[i][0].x = position[i][1].x = rot.getX();
      position[i][0].y = position[i][1].y = rot.getY();
      position[i][0].z = position[i][1].z = rot.getZ();
      position[i][1].y -= 5*restLength;
      
      if(yMin > position[i][0].y){
         yMin = position[i][0].y;
         minVertex = i;
      }
      if(yMax < position[i][0].y){
         yMax = position[i][0].y;
         maxVertex = i;
      }
   }
   
   if(isOscillating){
      float mag = sqrt(pow(position[maxVertex][0].x - position[minVertex][0].x,2) +
                       pow(position[maxVertex][0].y - position[minVertex][0].y,2) +
                       pow(position[maxVertex][0].z - position[minVertex][0].z,2));
      if(mag != 0){
         Vector angularForce;
         angularForce.x = (position[maxVertex][0].x - position[minVertex][0].x)/(10*mag);
         angularForce.y = (position[maxVertex][0].y - position[minVertex][0].y)/(10*mag);
         angularForce.z = (position[maxVertex][0].z - position[minVertex][0].z)/(10*mag);
         //applies the oscillatory acceleration to the top row of free-motion vertices
         for(int i = 0; i < X_RES; i++){
            velocity[i][2].x += Hv*angularForce.x;
            velocity[i][2].y += Hv*angularForce.y;
            velocity[i][2].z += Hv*angularForce.z;
         }
      }
   }
}

/* determines the x components of the spring forces
 */
GLfloat Skirt::springX(int c, int r,
                       float Fs1, float Fs2, float Fs3, float Fs4, float Fs5, float Fs6) const
{
   int forceDir = (r == Y_RES-1) ? 0 : (position[c][r].x - position[c][r+1].x < 0) ? 1 : -1;
   float Fs1_x, Fs2_x, Fs3_x, Fs4_x, Fs5_x, Fs6_x; //the % of the force in x
                     
   Fs1_x = (r == Y_RES-1) ? 0 : forceDir*Fx(c,r, c,r+1);
   forceDir = (position[c][r].x - position[c][r-1].x < 0) ? 1 : -1;
   Fs2_x = forceDir*Fx(c,r, c,r-1);
   forceDir = (c == 0) ? ((position[c][r].x - position[X_RES-1][r].x < 0) ? 1 : -1) :
                         ((position[c][r].x - position[c-1][r].x < 0) ? 1 : -1);
   Fs3_x = (c == 0) ? forceDir*Fx(c,r, X_RES-1,r) : forceDir*Fx(c,r, c-1,r);
   forceDir = (c == X_RES-1) ? ((position[c][r].x - position[0][r].x < 0) ? 1 : -1) :
                         ((position[c][r].x - position[c+1][r].x < 0) ? 1 : -1);
   Fs4_x = (c == X_RES-1) ? forceDir*Fx(c,r, 0,r) : forceDir*Fx(c,r, c+1,r);
   forceDir = (r == Y_RES-1) ? 0 : (c == X_RES-1) ? 
                         ((position[c][r].x - position[0][r+1].x < 0) ? 1 : -1) :
                         ((position[c][r].x - position[c+1][r+1].x < 0) ? 1 : -1);
   Fs5_x = (r == Y_RES-1) ? 0 : (c == X_RES-1) ?
                         forceDir*Fx(c,r, 0,r+1) : forceDir*Fx(c,r, c+1,r+1);
   forceDir = (c == 0) ? ((position[c][r].x - position[X_RES-1][r-1].x < 0) ? 1 : -1) :
                         ((position[c][r].x - position[c-1][r-1].x < 0) ? 1 : -1);
   Fs6_x = (c == 0) ? forceDir*Fx(c,r, X_RES-1,r-1) : forceDir*Fx(c,r, c-1,r-1);
   
   return Fs1_x*Fs1 + Fs2_x*Fs2 + Fs3_x*Fs3 + Fs4_x*Fs4 + Fs5_x*Fs5 + Fs6_x*Fs6;
}

/* determines the y components of the spring forces
 */
GLfloat Skirt::springY(int c, int r,
                       float Fs1, float Fs2, float Fs3, float Fs4, float Fs5, float Fs6) const
{
   int forceDir = (r == Y_RES-1) ? 0 : (position[c][r].y - position[c][r+1].y < 0) ? 1 : -1;
   float Fs1_y, Fs2_y, Fs3_y, Fs4_y, Fs5_y, Fs6_y; //the % of the force in y
                     
   Fs1_y = (r == Y_RES-1) ? 0 : forceDir*Fy(c,r, c,r+1);
   forceDir = (position[c][r].y - position[c][r-1].y < 0) ? 1 : -1;
   Fs2_y = forceDir*Fy(c,r, c,r-1);
   forceDir = (c == 0) ? ((position[c][r].y - position[X_RES-1][r].y < 0) ? 1 : -1) :
                         ((position[c][r].y - position[c-1][r].y < 0) ? 1 : -1);
   Fs3_y = (c == 0) ? forceDir*Fy(c,r, X_RES-1,r) : forceDir*Fy(c,r, c-1,r);
   forceDir = (c == X_RES-1) ? ((position[c][r].y - position[0][r].y < 0) ? 1 : -1) :
                         ((position[c][r].y - position[c+1][r].y < 0) ? 1 : -1);
   Fs4_y = (c == X_RES-1) ? forceDir*Fy(c,r, 0,r) : forceDir*Fy(c,r, c+1,r);
   forceDir = (r == Y_RES-1) ? 0 : (c == X_RES-1) ? 
                         ((position[c][r].y - position[0][r+1].y < 0) ? 1 : -1) :
                         ((position[c][r].y - position[c+1][r+1].y < 0) ? 1 : -1);
   Fs5_y = (r == Y_RES-1) ? 0 : (c == X_RES-1) ?
                         forceDir*Fy(c,r, 0,r+1) : forceDir*Fy(c,r, c+1,r+1);
   forceDir = (c == 0) ? ((position[c][r].y - position[X_RES-1][r-1].y < 0) ? 1 : -1) :
                         ((position[c][r].y - position[c-1][r-1].y < 0) ? 1 : -1);
   Fs6_y = (c == 0) ? forceDir*Fy(c,r, X_RES-1,r-1) : forceDir*Fy(c,r, c-1,r-1);
   
   return Fs1_y*Fs1 + Fs2_y*Fs2 + Fs3_y*Fs3 + Fs4_y*Fs4 + Fs5_y*Fs5 + Fs6_y*Fs6;
}

/* determines the z components of the spring forces
 */
GLfloat Skirt::springZ(int c, int r,
                       float Fs1, float Fs2, float Fs3, float Fs4, float Fs5, float Fs6) const
{
   int forceDir = (r == Y_RES-1) ? 0 : (position[c][r].z - position[c][r+1].z < 0) ? 1 : -1;
   float Fs1_z, Fs2_z, Fs3_z, Fs4_z, Fs5_z, Fs6_z; //the % of the force in z
                     
   Fs1_z = (r == Y_RES-1) ? 0 : forceDir*Fz(c,r, c,r+1);
   forceDir = (position[c][r].z - position[c][r-1].z < 0) ? 1 : -1;
   Fs2_z = forceDir*Fz(c,r, c,r-1);
   forceDir = (c == 0) ? ((position[c][r].z - position[X_RES-1][r].z < 0) ? 1 : -1) :
                         ((position[c][r].z - position[c-1][r].z < 0) ? 1 : -1);
   Fs3_z = (c == 0) ? forceDir*Fz(c,r, X_RES-1,r) : forceDir*Fz(c,r, c-1,r);
   forceDir = (c == X_RES-1) ? ((position[c][r].z - position[0][r].z < 0) ? 1 : -1) :
                         ((position[c][r].z - position[c+1][r].z < 0) ? 1 : -1);
   Fs4_z = (c == X_RES-1) ? forceDir*Fz(c,r, 0,r) : forceDir*Fz(c,r, c+1,r);
   forceDir = (r == Y_RES-1) ? 0 : (c == X_RES-1) ? 
                         ((position[c][r].z - position[0][r+1].z < 0) ? 1 : -1) :
                         ((position[c][r].z - position[c+1][r+1].z < 0) ? 1 : -1);
   Fs5_z = (r == Y_RES-1) ? 0 : (c == X_RES-1) ?
                         forceDir*Fz(c,r, 0,r+1) : forceDir*Fz(c,r, c+1,r+1);
   forceDir = (c == 0) ? ((position[c][r].z - position[X_RES-1][r-1].z < 0) ? 1 : -1) :
                         ((position[c][r].z - position[c-1][r-1].z < 0) ? 1 : -1);
   Fs6_z = (c == 0) ? forceDir*Fz(c,r, X_RES-1,r-1) : forceDir*Fz(c,r, c-1,r-1);
   
   return Fs1_z*Fs1 + Fs2_z*Fs2 + Fs3_z*Fs3 + Fs4_z*Fs4 + Fs5_z*Fs5 + Fs6_z*Fs6;
}

/* helper for springX. Used to determine the percent of a force to give to x
 */
GLfloat Skirt::Fx(int col1, int row1, int col2, int row2) const
{
   float mag = sqrt(pow(position[col1][row1].x - position[col2][row2].x,2) +
                    pow(position[col1][row1].y - position[col2][row2].y,2) +
                    pow(position[col1][row1].z - position[col2][row2].z,2));
   return pow((position[col1][row1].x - position[col2][row2].x)/mag, 2);
}

/* helper for springY. Used to determine the percent of a force to give to y
 */
GLfloat Skirt::Fy(int col1, int row1, int col2, int row2) const
{
   float mag = sqrt(pow(position[col1][row1].x - position[col2][row2].x,2) +
                    pow(position[col1][row1].y - position[col2][row2].y,2) +
                    pow(position[col1][row1].z - position[col2][row2].z,2));
   return pow((position[col1][row1].y - position[col2][row2].y)/mag, 2);
}

/* helper for springZ. Used to determine the percent of a force to give to z
 */
GLfloat Skirt::Fz(int col1, int row1, int col2, int row2) const
{
   float mag = sqrt(pow(position[col1][row1].x - position[col2][row2].x,2) +
                    pow(position[col1][row1].y - position[col2][row2].y,2) +
                    pow(position[col1][row1].z - position[col2][row2].z,2));
   return pow((position[col1][row1].z - position[col2][row2].z)/mag, 2);
}

/* returns the current length of a spring defined by the parameters
 */
GLfloat Skirt::currentLength(int col1, int row1, int col2, int row2) const
{
   return sqrt(pow(position[col1][row1].x - position[col2][row2].x,2) +
               pow(position[col1][row1].y - position[col2][row2].y,2) +
               pow(position[col1][row1].z - position[col2][row2].z,2));
}

/* calculates the vertex normals
 */
void Skirt::calcNorms()
{
   Vector v1, v2;
   resetNorms();
   for(int j = 0; j < Y_RES-1; j++){
      for(int i = 1; i < X_RES; i++){
         v1.x =   position[i][j].x - position[i-1][j].x;
         v1.y =   position[i][j].y - position[i-1][j].y;
         v1.z =   position[i][j].z - position[i-1][j].z;
         v2.x =   position[i][j+1].x - position[i-1][j].x;
         v2.y =   position[i][j+1].y - position[i-1][j].y;
         v2.z =   position[i][j+1].z - position[i-1][j].z;
         updateVertNorms(calcFaceNorm(v1, v2),
                          vertexNormals[i][j], vertexNormals[i-1][j], vertexNormals[i][j+1]);
         v1.x =   position[i-1][j+1].x - position[i-1][j].x;
         v1.y =   position[i-1][j+1].y - position[i-1][j].y;
         v1.z =   position[i-1][j+1].z - position[i-1][j].z;
         updateVertNorms(calcFaceNorm(v2, v1),
                          vertexNormals[i-1][j+1], vertexNormals[i-1][j], vertexNormals[i][j+1]);
      }
      v1.x =   position[0][j].x - position[X_RES-1][j].x;
      v1.y =   position[0][j].y - position[X_RES-1][j].y;
      v1.z =   position[0][j].z - position[X_RES-1][j].z;
      v2.x =   position[0][j+1].x - position[X_RES-1][j].x;
      v2.y =   position[0][j+1].y - position[X_RES-1][j].y;
      v2.z =   position[0][j+1].z - position[X_RES-1][j].z;
      updateVertNorms(calcFaceNorm(v1, v2),
                       vertexNormals[0][j], vertexNormals[X_RES-1][j], vertexNormals[0][j+1]);
      v1.x =   position[X_RES-1][j+1].x - position[X_RES-1][j].x;
      v1.y =   position[X_RES-1][j+1].y - position[X_RES-1][j].y;
      v1.z =   position[X_RES-1][j+1].z - position[X_RES-1][j].z;
      updateVertNorms(calcFaceNorm(v2, v1), vertexNormals[X_RES-1][j+1], 
                       vertexNormals[X_RES-1][j], vertexNormals[0][j+1]);
   }
}

/* resets the vertex normals to zero so they can be recalculated
 */
void Skirt::resetNorms()
{
   for(int j = 0; j < Y_RES-1; j++)
      for(int i = 1; i < X_RES; i++){
         vertexNormals[i][j].x = 0;
         vertexNormals[i][j].y = 0;
         vertexNormals[i][j].z = 0;
      }
}

/* calculates the face normals for the triangles used to generate the skirt mesh
 */
Skirt::Vector Skirt::calcFaceNorm(Vector v1, Vector v2) const
{
   Vector normal;
   
   normal.x = v1.y*v2.z - v1.z*v2.y;
   normal.y = v1.z*v2.x - v1.x*v2.z;
   normal.z = v1.x*v2.y - v1.y*v2.x;
   
   return normal;
}

/* updates the normals of the vertices which share the same polygon to include its face normal
 */
void Skirt::updateVertNorms(Vector faceNorm, Vector &vert1Norm, Vector &vert2Norm,
                            Vector &vert3Norm)
{
   vert1Norm.x += faceNorm.x;
   vert1Norm.y += faceNorm.y;
   vert1Norm.z += faceNorm.z;
   vert2Norm.x += faceNorm.x;
   vert2Norm.y += faceNorm.y;
   vert2Norm.z += faceNorm.z;
   vert3Norm.x += faceNorm.x;
   vert3Norm.y += faceNorm.y;
   vert3Norm.z += faceNorm.z;
}
