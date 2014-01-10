/* Author: Arash Ghodsi (aghodsi)
   Class: CMPS161 - Animation & Visualization
   Term: Winter 2011
   File: main.cpp - The main program; encompasses user input, window initialization, and rendering.
   prog3: Simulate a hula skirt using physically based animation. The animation is generated using
          Hooke's law for springs on the edges of the triangle mesh skirt, and rotation quaternions
          or versors for the oscillatory motion.
          The user can control the amplitude and frequency of the oscillation and whether the motion
          is 2-dimensional about the z-axis or 3-dimensional about both the x-axis and z-axis,
          independently. Finally, the user can switch in and out of wireframe rendering. Please see
          the README for controls.
 */

#include "skirt.h"
#include <cstdlib> //used for exit() and EXIT_SUCCESS
#include <GL/gl.h> //used for various gl types and functions
#include <GL/glu.h> //used for gluPerspective()
#include <GL/glut.h> //used for various glut-based functions and constants

//Global Constants
const GLint WINDOW_WIDTH = 720, WINDOW_HEIGHT = 720, WIN_POS_X = 200, WIN_POS_Y = 100;
const GLdouble FOV = 45, CLIP_NEAR = 0.1, CLIP_FAR = 100;

//Global Variables
Skirt skirt;
int xPrev, horizAngle = 90;
bool isWireframe = false;
GLdouble aspectRatio = 1.0;

//initializes the OpenGL framework such as lighting, shading, depth, culling, and materials
GLvoid init();
//links GLUT functions for windowing, keyboard, and mouse utilization
GLvoid initWindow();
//reshapes the program window if the window size has been modified
GLvoid reshape(int w, int h);
//primary rendering function from which all rendering takes place
GLvoid display();
//called from display. where all of the custom rendering takes place
GLvoid drawScene();
//used to change the skirt motion between 2D and 3D
GLvoid keyboard(unsigned char key, int mouseX, int mouseY);
//used to change the amplitude or frequency of the skirt's oscillatory motion
GLvoid keyboardArrows(int key, int x, int y);
//used to render in wireframe mode
GLvoid mouseButtonState(int button, int state, int x, int y);
//used to rotate the camera around the skirt horizontally
GLvoid mouseMove(int x, int y);

//::MAIN:://////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowPosition(WIN_POS_X, WIN_POS_Y);
   glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
   glutCreateWindow("Physically based animation: Skirt motion simulator");
   /*if fullscreen is desired, comment out this line and the preceding three lines
   glutGameModeString("1680x1050:32@60");
   glutEnterGameMode();
   //*/
   init();
   initWindow();
   
   glutMainLoop();
   
   return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

/* initializes the OpenGL framework such as lighting, shading, depth, culling, and materials
 */
GLvoid init()
{
   skirt.loadTexture();
   glEnable(GL_TEXTURE_2D);
   
   glShadeModel(GL_SMOOTH);
   glEnable(GL_NORMALIZE);
   glClearColor(0.4f, 0.4f, 0.7f, 0.0f);
   glClearDepth(1.0f);
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   
   //lighting init
   GLfloat lightPos[] = {1, 0.25, 0.5, 0};
   glEnable(GL_LIGHTING);
   glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
   glEnable(GL_LIGHT0);
   glEnable(GL_COLOR_MATERIAL);
   
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

/* links GLUT functions for windowing, keyboard, and mouse utilization
 */
GLvoid initWindow()
{
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutIdleFunc(display);
   glutKeyboardFunc(keyboard);
   glutSpecialFunc(keyboardArrows);
   glutMouseFunc(mouseButtonState);
   glutMotionFunc(mouseMove);
}

/* reshapes the program window if the window size has been modified
 */
GLvoid reshape(int w, int h)
{
   h = h?h:1;
   aspectRatio = (GLdouble)w/h;
   glViewport(0, 0, w, h);
   
   //update projection matrix
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(FOV, aspectRatio, CLIP_NEAR, CLIP_FAR);
   
   //init model-view matrix
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

/* primary rendering function from which all rendering takes place
 */
GLvoid display()
{
   glLoadIdentity();

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
   drawScene();
   glutSwapBuffers(); //contains an implicit glFlush call
}

/* called from display. where all of the custom rendering takes place
 */
GLvoid drawScene()
{
   glTranslatef(0, 1.5*skirt.getHeight(), -7); //centers the skirt in front of the camera
   glRotatef(horizAngle, 0,1,0); //rotates the skirt so the texture is centered
   skirt.draw();
}

/* captures and processes keyboard input
 * press 1 to have the skirt move in 2D
 * press 2 to have the skirt move in 3D
 */
GLvoid keyboard(unsigned char key, int mouseX, int mouseY)
{
   switch(key){
      case '1': skirt.rotate2D();
         break;
      case '2': skirt.rotate3D();
         break;
      //Esc Key
      case 27:  exit(EXIT_SUCCESS);
         break;
   }
}

/* captures and processes arrow keys
 * up/down keys change the skirt motion's amplitude
 * left/right keys change the skirt motion's frequency
 */
GLvoid keyboardArrows(int key, int x, int y)
{
   switch(key){
      case GLUT_KEY_UP: skirt.incAmplitude();
         break;
      case GLUT_KEY_DOWN: skirt.decAmplitude();
         break;
      case GLUT_KEY_RIGHT: skirt.incFrequency();
         break;
      case GLUT_KEY_LEFT: skirt.decFrequency();
         break;
   }
}

/* determines the state of the mouse
 * right click toggles wireframe rendering
 */
GLvoid mouseButtonState(int button, int state, int x, int y)
{
   if((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)){
      isWireframe = !isWireframe;
      if(isWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   }
}

/* used to rotate the camera around the skirt horizontally
 */
GLvoid mouseMove(int x, int y)
{
   horizAngle += (x > xPrev) ? 2 : -2;
   xPrev = x;
}
