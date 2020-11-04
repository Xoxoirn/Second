#include "Render.h"
#include <sstream>
#include <iostream>
#include <string>
#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>
#include "MyOGL.h"
#include <math.h>
#include <vector>
#include "Camera.h"
#include "Light.h"
#include "Primitives.h"
#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

using namespace std;

#define TOP_RIGHT 1.0f,1.0f
#define TOP_LEFT 0.0f,1.0f
#define BOTTOM_RIGHT 1.0f,0.0f
#define BOTTOM_LEFT 0.0f,0.0f

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����


//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}

GLuint texId[3];

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	

	//������ ����������� ���������  (R G B)
	RGBTRIPLE *texarray;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("FirstTexture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//���������� �� ��� ��������
	glGenTextures(1, &texId[0]);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId[0]);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#pragma region ������ ��������

	OpenGL::LoadBMP("SecondTexture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//���������� �� ��� ��������
	glGenTextures(1, &texId[1]);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId[1]);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

#pragma endregion
#pragma region ������ ��������
	OpenGL::LoadBMP("ThirdTexture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//���������� �� ��� ��������
	glGenTextures(1, &texId[2]);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId[2]);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#pragma endregion

	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH);


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;

}


void DrawTriangle(double v1[], double v2[], double v3[])
{
	glBegin(GL_TRIANGLES);
	glColor3d(0.7, 0.4, 0.8);
	glVertex3dv(v1);
	glVertex3dv(v2);
	glVertex3dv(v3);
	glEnd();
}
void NormalizeVector(double* vec)
{
	double modVector = -sqrt(pow(vec[0], 2) + pow(vec[1], 2) + pow(vec[2], 2)); // ����� �������

	for (int i = 0; i < 3; ++i)
	{
		vec[i] /= modVector;
	}
}
void FindNormal(double* a, double* b, double* c, int FlagSwap = 0) // �� ����� � ���� ��� �������
{
	double vectorA[3], vectorB[3];

	for (int i = 0; i < 3; ++i) // �������� ������ A � B
	{
		vectorA[i] = a[i] - c[i];
		vectorB[i] = b[i] - c[i];
	}

	double VectorNormal[3];

	VectorNormal[0] = vectorA[1] * vectorB[2] - vectorB[1] * vectorA[2]; //���������� ������� �� ������
	VectorNormal[1] = -vectorA[0] * vectorB[2] + vectorB[0] * vectorA[2];
	VectorNormal[2] = vectorA[0] * vectorB[1] - vectorB[0] * vectorA[1];

	NormalizeVector(VectorNormal);

	if (FlagSwap != 0)
	{
		for (int i = 0; i < 3; ++i) // �������� ������ A � B
		{
			VectorNormal[i] *= -1;
		}
	}

glNormal3dv(VectorNormal);
}
double NewCoordY(double oldCoord) // ���������� ���������� ����� � ����������� �� � "��������� �������", ������ ��� �������� �������� �� 0 �� 1
{
	double NewCoord = (oldCoord) / 15.; // 15 - ������������ ������ �������
	return NewCoord;
}
double NewCoordX(double oldCoord)
{
	double NewCoord = oldCoord / 10;
	return NewCoord;
}
void DrawWall(double v1[], double v2[])
{
	double tmp1[3] = { v1[0],v1[1],v1[2] + 2 };
	double tmp2[3] = { v2[0],v2[1],v2[2] + 2 };
	glColor3d(0.1, 0.6, 0.9);
	glBegin(GL_QUADS);
	glVertex3dv(tmp1);
	glVertex3dv(tmp2);
	glVertex3dv(v2);
	glVertex3dv(v1);
	glEnd();
}
void roof()
{

	glColor3d(0.7, 0.4, 0.8);
	double A[] = { 0, 0, 2 };
	double B[] = { 1, 4, 2 };
	double C[] = { 0, 13, 2 };
	double D[] = { 4, 12, 2 };
	double E[] = { 4, 6, 2 };
	double F[] = { 9, 11, 2 };
	double G[] = { 10, 5, 2 };

	FindNormal(C, B, A);
	glBegin(GL_TRIANGLES);
	
	//glVertex3dv(A);
	//glVertex3dv(B);
	//glVertex3dv(G);
	glTexCoord2d(NewCoordX(B[0]), NewCoordY(B[1]));
	glVertex3dv(B);
	glTexCoord2d(NewCoordX(E[0]), NewCoordY(E[1]));
	glVertex3dv(E);
	glTexCoord2d(NewCoordX(G[0]), NewCoordY(G[1]));
	glVertex3dv(G);

	glTexCoord2d(NewCoordX(E[0]), NewCoordY(E[1]));
	glVertex3dv(E);
	glTexCoord2d(NewCoordX(F[0]), NewCoordY(F[1]));
	glVertex3dv(F);
	glTexCoord2d(NewCoordX(G[0]), NewCoordY(G[1]));
	glVertex3dv(G);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2d(NewCoordX(B[0]), NewCoordY(B[1]));
	glVertex3dv(B);
	glTexCoord2d(NewCoordX(C[0]), NewCoordY(C[1]));
	glVertex3dv(C);
	glTexCoord2d(NewCoordX(D[0]), NewCoordY(D[1]));
	glVertex3dv(D);
	glTexCoord2d(NewCoordX(E[0]), NewCoordY(E[1]));
	glVertex3dv(E);

	glEnd();
}
void bottom()
{

	double A[] = { 0, 0, 0 };
	double B[] = { 1, 4, 0 };
	double C[] = { 0, 13, 0 };
	double D[] = { 4, 12, 0 };
	double E[] = { 4, 6, 0 };
	double F[] = { 9, 11, 0 };
	double G[] = { 10, 5, 0 };

	glColor3d(0.7, 0.4, 0.8);
	FindNormal(A, B, C);

	glBegin(GL_TRIANGLES);

	//glVertex3dv(A);
	//glVertex3dv(B);
	//glVertex3dv(G);

	glVertex3dv(B);
	glVertex3dv(E);
	glVertex3dv(G);


	glVertex3dv(E);
	glVertex3dv(F);
	glVertex3dv(G);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(D);
	glVertex3dv(E);

	glEnd();
}
void walls()
{
	glColor3d(0.1, 0.6, 0.9);
	
	double A1[] = { 0, 0, 0 };
	double B1[] = { 1, 4, 0 };
	double C1[] = { 0, 13, 0 };
	double D1[] = { 4, 12, 0 };
	double E1[] = { 4, 6, 0 };
	double F1[] = { 9, 11, 0 };
	double G1[] = { 10, 5, 0 };

	double A2[] = { 0, 0, 2 };
	double B2[] = { 1, 4, 2 };
	double C2[] = { 0, 13, 2 };
	double D2[] = { 4, 12, 2 };
	double E2[] = { 4, 6, 2 };
	double F2[] = { 9, 11, 2 };
	double G2[] = { 10, 5, 2 };

	glBegin(GL_QUADS);
	FindNormal(B2, A2, A1);
	glTexCoord2d(BOTTOM_RIGHT);
	glVertex3dv(A1);
	glTexCoord2d(TOP_RIGHT);
	glVertex3dv(A2);
	glTexCoord2d(TOP_LEFT);
	glVertex3dv(B2);
	glTexCoord2d(BOTTOM_LEFT);
	glVertex3dv(B1);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texId[1]);
	glBegin(GL_QUADS);
	FindNormal(C2, B2, B1);
	glTexCoord2d(BOTTOM_RIGHT);
	glVertex3dv(B1);
	glTexCoord2d(TOP_RIGHT);
	glVertex3dv(B2);
	glTexCoord2d(TOP_LEFT);
	glVertex3dv(C2);
	glTexCoord2d(BOTTOM_LEFT);
	glVertex3dv(C1);
	glEnd();


	glBindTexture(GL_TEXTURE_2D, texId[0]);
	glBegin(GL_QUADS);
	FindNormal(D2, C2, C1);

    glVertex3dv(C1);
	glVertex3dv(C2);
	glVertex3dv(D2);
	glVertex3dv(D1);

	FindNormal(E2, D2, D1);
	glTexCoord2d(BOTTOM_RIGHT);
	glVertex3dv(D1);
	glTexCoord2d(TOP_RIGHT);
	glVertex3dv(D2);
	glTexCoord2d(TOP_LEFT);
	glVertex3dv(E2);
	glTexCoord2d(BOTTOM_LEFT);
	glVertex3dv(E1);

	FindNormal(F2, E2, E1);
	glTexCoord2d(BOTTOM_RIGHT);
	glVertex3dv(E1);
	glTexCoord2d(TOP_RIGHT);
	glVertex3dv(E2);
	glTexCoord2d(TOP_LEFT);
	glVertex3dv(F2);
	glTexCoord2d(BOTTOM_LEFT);
	glVertex3dv(F1);
	glEnd();

glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_QUADS);
	glColor4d(0.41, 0.16, 0.19, 0.6);
	FindNormal(G2, F2, F1);
	glTexCoord2d(BOTTOM_RIGHT);
	glVertex3dv(F1);
	glTexCoord2d(TOP_RIGHT);
	glVertex3dv(F2);
	glTexCoord2d(TOP_LEFT);
	glVertex3dv(G2);
	glTexCoord2d(BOTTOM_LEFT);
	glVertex3dv(G1);
	glEnd();

	//FindNormal(A2, G2, G1);

//	glVertex3dv(G1);
	//glVertex3dv(G2);
	//glVertex3dv(A2);
	//glVertex3dv(A1);
	
	glEnd();
}
void NormVariant()
{
	double povorot = -14.04;
	double C[] = { 0, 13, 0 };
	double D[] = { 4, 12, 0 };
	double O[3];//�������� CD



	O[0] = (C[0] + D[0]) / 2;
	O[1] = (C[1] + D[1]) / 2;
	O[2] = 0;

	double radius = (sqrt(pow((C[0] - D[0]), 2) + pow((C[1] - D[1]), 2) + pow((C[2] - D[2]), 2))) / 2; // ���������� ����� ����� �������

	vector <double> x; //����� �� ����������
	vector <double> y;

	for (double i = povorot; i <= (180 + povorot + 1); i += 0.1)
	{
		x.push_back(radius * cos(i * M_PI / 180.0) + O[0]); // ������� push_back ��������� �������� � �����
		y.push_back(radius * sin(i * M_PI / 180.0) + O[1]);
	}

	glColor3d(0.7, 0.4, 0.8);
	
	double temp1[] = { x[0], y[0], 2 };
	double temp2[] = { x[1], y[1], 2 };
	double temp3[] = { x[2], y[2], 2 };

	glBegin(GL_POLYGON);
	FindNormal(temp1, temp2, temp3);
	for (int i = 0; i < x.size() - 1; i++)
	{
		glVertex3d(x[i], y[i], 0);	
		glVertex3d(x[i + 1], y[i + 1], 0);
	}
	glEnd();
	
	double temp11[] = { x[0], y[0], 0 };
	double temp22[] = { x[1], y[1], 0 };
	double temp33[] = { x[2], y[2], 0 };

	glBegin(GL_POLYGON);
	FindNormal(temp33, temp22, temp11);
	for (int i = 0; i < x.size() - 1; i++)
	{
		glTexCoord2d(NewCoordX(x[i]), NewCoordY(y[i]));
		glVertex3d(x[i], y[i], 2);
		glTexCoord2d(NewCoordX(x[i + 1]), NewCoordY(y[i + 1]));
		glVertex3d(x[i + 1], y[i + 1], 2);
	}
	glEnd();

	glColor3d(0.1, 0.6, 0.9);
	glBegin(GL_QUADS);
	for (double i = 0; i < x.size() - 1; i++)
	{
		double temp1[] = { x[i], y[i], 0 };
		double temp2[] = { x[i], y[i], 2 };
		double temp3[] = { x[i + 1], y[i + 1], 2 };

		FindNormal(temp1, temp2, temp3);


		glTexCoord2d(i / (x.size() - 1), 1); glVertex3d(x[i], y[i], 2); // ����� ������������ ����� �������� (����� �� �� ����� ���������������)
		glTexCoord2d(i / (x.size() - 1), 0); glVertex3d(x[i], y[i], 0);
		glTexCoord2d(i/ (x.size() - 1), 0); glVertex3d(x[i + 1], y[i + 1], 0);
		glTexCoord2d(i/ (x.size() - 1), 1); glVertex3d(x[i + 1], y[i + 1], 2);
	}
	glEnd();

}
void NormVariant2()
{

	double A[] = { 0, 0, 2 };
	double G[] = { 10, 5, 2 };
	double M[] = { 6, 4, 2 }; // �������� ����� �� ����������

	double S[3];//�������� AG
	S[0] = (A[0] + G[0]) / 2;
	S[1] = (A[1] + G[1]) / 2;
	S[2] = 0;

	double E1[] = { 4, 6, 0 };
	double E2[] = { 4, 6, 2 };

	double B1[] = { 1, 4, 0 };
	double B2[] = { 1, 4, 2 };
	//(0 - x) ^ 2 + (0 - y) ^ 2 = (6 - x) ^ 2 + (4 - y) ^ 2
	//(10 - x) ^ 2 + (5 - y) ^ 2 = (6 - x) ^ 2 + (4 - y) ^ 2
	double x0 = 12;
	double y0 = -23. / 2;
	double r = 16.6207701386; //������

	vector <double> x; //����� �� ����������
	vector <double> y;


	double povorot = (atan(A[0] - G[0] / A[1] - G[1]) * 180 / M_PI) + 90; // ������� ��� ���������� ���� �������� ����������

	double alpha_div2 = 46.28; //����� ����, ������� �� ����� ������� (������)
	for (double i = 149.2 - alpha_div2 - 6; i <= 90 + alpha_div2; i += 0.09) //������ 149.2
	{
		double y1, x1;
		x1 = r * cos(i * M_PI / 180); // ������ ���������� � ������ ���������
		y1 = r * sin(i * M_PI / 180);
		x.push_back(x1 * cos(povorot * M_PI / 180) + y1 * sin(povorot * M_PI / 180) + x0); // ������� ��� �������� ���������� ������������ ������ ��������� � ������� ������ ���������� � ����� x0y0
		y.push_back(-x1 * sin(povorot * M_PI / 180) + y1 * cos(povorot * M_PI / 180) + y0);
	}
	glBindTexture(GL_TEXTURE_2D, texId[2]);
	glBegin(GL_QUADS);
	for (double i = 0; i < x.size() - 1; i++)
	{
		double temp1[] = { x[i], y[i], 0 };
		double temp2[] = { x[i], y[i], 2 };
		double temp3[] = { x[i + 1], y[i + 1], 2 };

		FindNormal(temp3, temp2, temp1);
		glTexCoord2d(i / (x.size() - 1), 1); glVertex3d(x[i], y[i], 2); // ����� ������������ ����� �������� (����� �� �� ����� ���������������)
		glTexCoord2d(i / (x.size() - 1), 0); glVertex3d(x[i], y[i], 0);
		glTexCoord2d(i / (x.size() - 1), 0); glVertex3d(x[i + 1], y[i + 1], 0);
		glTexCoord2d(i / (x.size() - 1), 1); glVertex3d(x[i + 1], y[i + 1], 2);

	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, texId[0]);
	glColor3d(0.7, 0.4, 0.8);
	
	double temp1[] = { x[0], y[0], 2 };
	double temp2[] = { x[1], y[1], 2 };
	double temp3[] = { x[2], y[2], 2 };
	glBegin(GL_TRIANGLES);
	FindNormal(temp1, temp2, temp3);
	for (int i = 0; i < (x.size() - 1); i++)
	{
		glVertex3dv(B1);
		glVertex3d(x[i], y[i], 0);
		glVertex3d(x[i + 1], y[i + 1], 0);
	}
	
	double temp11[] = { x[0], y[0], 0 };
	double temp22[] = { x[1], y[1], 0 };
	double temp33[] = { x[2], y[2], 0 };
	FindNormal(temp33, temp22, temp11);
	for (int i = 0; i < (x.size() - 1) ; i++)
	{
		glTexCoord2d(NewCoordX(B2[0]), NewCoordY(B2[1]));
		glVertex3dv(B2);
		glTexCoord2d(NewCoordX(x[i]), NewCoordY(y[i]));
		glVertex3d(x[i], y[i], 2);
		glTexCoord2d(NewCoordX(x[i+1]), NewCoordY(y[i+1]));
		glVertex3d(x[i + 1], y[i + 1], 2);

	}
	glEnd();

}

void Render(OpenGL *ogl)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//��������� ���������
	GLfloat amb[] = { 0.5, 0.5, 0.5, 1. };
	GLfloat dif[] = { 0.8, 0.8, 0.8, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;

	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); 
		//������ �����
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);
	//===================================
	//������� ���  
	glBindTexture(GL_TEXTURE_2D, texId[0]);
	bottom();
	roof();
	
	NormVariant();
	NormVariant2();
    walls();



   //��������� ������ ������

	
	glMatrixMode(GL_PROJECTION);	//������ �������� ������� ��������. 
	                                //(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix();   //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ���� 				    
	glLoadIdentity();	  //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW);		//������������� �� �����-��� �������
	glPushMatrix();			  //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity();		  //���������� �� � ������

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(300, 200);
	rec.setPosition(10, ogl->getHeight() - 200 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //��������������� ������� �������� � �����-��� �������� �� �����.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}