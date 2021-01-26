#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <fstream>
#include <math.h>
#include <iostream>

using namespace std;

float xangle = 0;
float yangle = 0;
float zangle = 0;

const int SPACE = 2;
const int SIZE = 500 / SPACE;

class Surface
{
   public:
      double Px[SIZE][SIZE];
      double Py[SIZE][SIZE];
      double Pz[SIZE][SIZE];
      double Nx[SIZE][SIZE];
      double Ny[SIZE][SIZE];
      double Nz[SIZE][SIZE];
      float R[SIZE][SIZE];
      float G[SIZE][SIZE];
      float B[SIZE][SIZE];
};

double red = 0;
double green = 0;
double blue = 0;

Surface penny;

int style = 2;

//---------------------------------------
// convert values to window size
//---------------------------------------
float convertX(float val)
{
	float out = 0;
	out = ((val / 500) * 4) - 1;
	return out;
}
float convertY(float val)
{
	float out = 0;
	out = ((val / 500) * -4) + 1;
	return out;
}

//---------------------------------------
// Calculate surface normals
//---------------------------------------
void surface_normals(Surface &s)
{
   // Find surface normals
   for (int u = 0; u < SIZE-1; u++)
   for (int v = 0; v < SIZE-1; v++)
   {
      // Get two tangent vectors
      float Ux = s.Px[u+1][v] - s.Px[u][v];
      float Uy = s.Py[u+1][v] - s.Py[u][v];
      float Uz = s.Pz[u+1][v] - s.Pz[u][v];
      float Vx = s.Px[u][v+1] - s.Px[u][v];
      float Vy = s.Py[u][v+1] - s.Py[u][v];
      float Vz = s.Pz[u][v+1] - s.Pz[u][v];

      // Do cross product
      s.Nx[u][v] = Uy * Vz - Uz * Vy;
      s.Ny[u][v] = Uz * Vx - Ux * Vz;
      s.Nz[u][v] = Ux * Vy - Uy * Vx;

      float length = sqrt(
         s.Nx[u][v] * s.Nx[u][v] +
         s.Ny[u][v] * s.Ny[u][v] +
         s.Nz[u][v] * s.Nz[u][v]);

      if (length > 0)
      {
         s.Nx[u][v] /= length;
         s.Ny[u][v] /= length;
         s.Nz[u][v] /= length;
      }
   }

   // Wrap around values for last row and col
   for (int u = 0; u < SIZE; u++)
   {
      s.Nx[u][SIZE-1] = s.Nx[u][0];
      s.Ny[u][SIZE-1] = s.Ny[u][0];
      s.Nz[u][SIZE-1] = s.Nz[u][0];
   }

   for (int v = 0; v < SIZE; v++)
   {
      s.Nx[SIZE-1][v] = s.Nx[0][v];
      s.Ny[SIZE-1][v] = s.Ny[0][v];
      s.Nz[SIZE-1][v] = s.Nz[0][v];
   }
}

//---------------------------------------
// put number values in an array to draw
//---------------------------------------
void data_extractor(Surface &s)
{
   ifstream inColor;
   ifstream inDepth;

   string buffer1 = "penny-image.txt";
   string buffer2 = "penny-depth.txt";

   //cout << "Please enter RGB txt file name: ";
   //cin >> buffer1;
   //cout << "Pease enter depth txt file name: ";
   //cin >> buffer2;

   inColor.open(buffer1);
   inDepth.open(buffer2);

   float R = 0.0;
   float G = 0.0;
   float B = 0.0;

   int x = 0;
   int y = 0;
   int X = 0;
   int Y = 0;
   int Z = 0;

   while(inDepth >> Z)
   {
     inColor >> R >> G >> B;

   	if((R > 245 && G > 245)
	   || (B > 245 && R > 245)
	   || (G > 245 && B > 245))
      {
		   R = 0;
		   G = 0;
		   B = 0;
		   Z = 0;
	   } else {
		   R = R/255;
		   G = G/255;
		   B = B/255;
		   Z = (12*Z)/255;
	   }

   	x++;

	   if(x % 500 == 0)
      {
		   x = 0;
		   y++;
	   }

   	if(x % SPACE == 0 && y % SPACE == 0)
      {
	      s.Px[X][Y] = convertX(X);
	      s.Py[X][Y] = convertY(Y);
	      s.Pz[X][Y] = convertX(Z);
	      s.R[X][Y] = R;
	      s.G[X][Y] = G;
	      s.B[X][Y] = B;
	      red += R;
	      green += G;
	      blue += B;

	      X++;

	      if(X % SIZE == 0) {
		      X = 0;
		      Y++;
	      }
	   }
   }

   red = red / (SIZE * SIZE * 1.5);
   green = green / (SIZE * SIZE * 1.5);
   blue = blue / (SIZE * SIZE * 1.5);

   inColor.close();
   inDepth.close();

   surface_normals(s);
}


float Ka = 0.2;
float Kd = 0.4;
float Ks = 1;
float Kp = 100;

//---------------------------------------
// Initialize material properties
//---------------------------------------
void init_material(float Ka, float Kd, float Ks, float Kp,
                   float Mr, float Mg, float Mb)
{
   // Material variables
   float ambient[] = { Ka * Mr, Ka * Mg, Ka * Mb, 1.0 };
   float diffuse[] = { Kd * Mr, Kd * Mg, Kd * Mb, 1.0 };
   float specular[] = { Ks * Mr, Ks * Mg, Ks * Mb, 1.0 };

   // Initialize material
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, Kp);
}

//---------------------------------------
// Initialize light source
//---------------------------------------
void init_light(int light_source, float Lx, float Ly, float Lz,
                float Lr, float Lg, float Lb)
{
   // Light variables
   float light_position[] = { Lx, Ly, Lz, 0.0 };
   float light_color[] = { Lr, Lg, Lb, 1.0 };

   // Initialize light source
   glEnable(GL_LIGHTING);
   glEnable(light_source);
   glLightfv(light_source, GL_POSITION, light_position);
   glLightfv(light_source, GL_AMBIENT, light_color);
   glLightfv(light_source, GL_DIFFUSE, light_color);
   glLightfv(light_source, GL_SPECULAR, light_color);
   glLightf(light_source, GL_CONSTANT_ATTENUATION, 1.0);
   glLightf(light_source, GL_LINEAR_ATTENUATION, 0.0);
   glLightf(light_source, GL_QUADRATIC_ATTENUATION, 0.0);
   glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

//---------------------------------------
// Display surface coordinates and normals.
//---------------------------------------

int color = 0;
void display_surfaceRGB(Surface &s)
{
   for (int u = 0; u < SIZE-1; u++)
      for (int v = 0; v < SIZE-1; v++)
      {
	      if(s.Pz[u][v] != convertX(0))
         {
	         glBegin(style);
	         glColor3f(s.R[u][v], s.G[u][v], s.B[u][v]);
            glNormal3f(s.Nx[u][v]*SPACE, s.Ny[u][v]*SPACE, s.Nz[u][v]);
            glVertex3f(s.Px[u][v]*SPACE, s.Py[u][v]*SPACE, s.Pz[u][v]);
            glNormal3f(s.Nx[u + 1][v]*SPACE, s.Ny[u + 1][v]*SPACE, s.Nz[u + 1][v]);
            glVertex3f(s.Px[u + 1][v]*SPACE, s.Py[u + 1][v]*SPACE, s.Pz[u + 1][v]);
            glNormal3f(s.Nx[u + 1][v + 1]*SPACE, s.Ny[u + 1][v + 1]*SPACE, s.Nz[u + 1][v + 1]);
            glVertex3f(s.Px[u + 1][v + 1]*SPACE, s.Py[u + 1][v + 1]*SPACE, s.Pz[u + 1][v + 1]);
            glNormal3f(s.Nx[u][v + 1]*SPACE, s.Ny[u][v + 1]*SPACE, s.Nz[u][v + 1]);
            glVertex3f(s.Px[u][v + 1]*SPACE, s.Py[u][v + 1]*SPACE, s.Pz[u][v + 1]);
	         glEnd();
	      }
      }
}

void display_surfaceBW(Surface &s)
{
   for (int u = 0; u < SIZE-1; u++)
      for (int v = 0; v < SIZE-1; v++)
      {
	      if(s.Pz[u][v] != convertX(0))
         {
	         glBegin(style);
	         glColor3f(0.7, 0.7, 0.7);
            glNormal3f(s.Nx[u][v]*SPACE, s.Ny[u][v]*SPACE, s.Nz[u][v]);
            glVertex3f(s.Px[u][v]*SPACE, s.Py[u][v]*SPACE, s.Pz[u][v]);
            glNormal3f(s.Nx[u + 1][v]*SPACE, s.Ny[u + 1][v]*SPACE, s.Nz[u + 1][v]);
            glVertex3f(s.Px[u + 1][v]*SPACE, s.Py[u + 1][v]*SPACE, s.Pz[u + 1][v]);
            glNormal3f(s.Nx[u + 1][v + 1]*SPACE, s.Ny[u + 1][v + 1]*SPACE, s.Nz[u + 1][v + 1]);
            glVertex3f(s.Px[u + 1][v + 1]*SPACE, s.Py[u + 1][v + 1]*SPACE, s.Pz[u + 1][v + 1]);
            glNormal3f(s.Nx[u][v + 1]*SPACE, s.Ny[u][v + 1]*SPACE, s.Nz[u][v + 1]);
            glVertex3f(s.Px[u][v + 1]*SPACE, s.Py[u][v + 1]*SPACE, s.Pz[u][v + 1]);
	         glEnd();
	      }
      }
}

//---------------------------------------
// Init function for OpenGL
//---------------------------------------
void init()
{
   glClearColor(0.0, 0.0, 0.0, 1.0);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-2, 2, -2, 2, -4, 4);
   glEnable(GL_DEPTH_TEST);
   glShadeModel(GL_SMOOTH);
   init_light(GL_LIGHT0, 0, 1, 1, 1, 1, 1);
   init_light(GL_LIGHT1, 1, 0, 1, 1, 1, 1);
   init_light(GL_LIGHT2, 1, 1, 0, 1, 1, 1);

   data_extractor(penny);

   // Initialize material properties
   init_material(Ka, Kd, Ks, 100 * Kp, red, green, blue);
   glDisable(GL_LIGHTING);
}

//---------------------------------------
// Display callback for OpenGL
//---------------------------------------
void display()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glRotatef(xangle, 1.0, 0.0, 0.0);
   glRotatef(yangle, 0.0, 1.0, 0.0);
   glRotatef(zangle, 0.0, 0.0, 1.0);

   if(color == 1)
   	display_surfaceRGB(penny);
   else
   	display_surfaceBW(penny);

   glFlush();
}

//---------------------------------------
// Keyboard callback for OpenGL
//---------------------------------------
void keyboard(unsigned char key, int x, int y)
{
   // Update angles
   if (key == 's') {
      xangle -= 5;
   } else if (key == 'w') {
      xangle += 5;
   } else if (key == 'd') {
      yangle -= 5;
   } else if (key == 'a') {
      yangle += 5;
   } else if (key == 'e') {
      zangle -= 5;
   } else if (key == 'q') {
      zangle += 5;
   } else if(key == '3') {
   	glEnable(GL_LIGHTING);
	   style = 6;
   } else if (key == '2') {
	   glDisable(GL_LIGHTING);
	   style = 6;
	   color = 1;
   } else if(key == '1') {
	   glDisable(GL_LIGHTING);
	   style = 2;
	   color = 0;
   }

   // Redraw objects
   glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowSize(500, 500);
   glutInitWindowPosition(250, 250);
   glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH);
   glutCreateWindow("Homework 4");
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   init();

   printf("Keyboard commands:\n");
   printf("   '1' - B&W wire frame\n");
   printf("   '2' - color polygons\n");
   printf("   '3' - phong shading\n");
   printf("   'w' - rotate x-axis +5 degrees\n");
   printf("   's' - rotate x-axis -5 degrees\n");
   printf("   'a' - rotate y-axis -5 degrees\n");
   printf("   'd' - rotate y-axis +5 degrees\n");
   printf("   'q' - rotate z-axis -5 degrees\n");
   printf("   'e' - rotate z-axis +5 degrees\n");

   glutMainLoop();
   return 0;
}
