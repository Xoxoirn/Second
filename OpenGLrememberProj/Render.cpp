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

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
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
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
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
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
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

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света


//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
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

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("FirstTexture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId[0]);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId[0]);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#pragma region Вторая текстура

	OpenGL::LoadBMP("SecondTexture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId[1]);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId[1]);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

#pragma endregion
#pragma region Третья текстура
	OpenGL::LoadBMP("ThirdTexture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId[2]);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId[2]);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#pragma endregion

	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH);


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

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
	double modVector = -sqrt(pow(vec[0], 2) + pow(vec[1], 2) + pow(vec[2], 2)); // длина вектора

	for (int i = 0; i < 3; ++i)
	{
		vec[i] /= modVector;
	}
}
void FindNormal(double* a, double* b, double* c, int FlagSwap = 0) // из точки А идут оба вектора
{
	double vectorA[3], vectorB[3];

	for (int i = 0; i < 3; ++i) // Получаем вектор A и B
	{
		vectorA[i] = a[i] - c[i];
		vectorB[i] = b[i] - c[i];
	}

	double VectorNormal[3];

	VectorNormal[0] = vectorA[1] * vectorB[2] - vectorB[1] * vectorA[2]; //нахождение вектора по точкам
	VectorNormal[1] = -vectorA[0] * vectorB[2] + vectorB[0] * vectorA[2];
	VectorNormal[2] = vectorA[0] * vectorB[1] - vectorB[0] * vectorA[1];

	NormalizeVector(VectorNormal);

	if (FlagSwap != 0)
	{
		for (int i = 0; i < 3; ++i) // Получаем вектор A и B
		{
			VectorNormal[i] *= -1;
		}
	}

glNormal3dv(VectorNormal);
}
double NewCoordY(double oldCoord) // отправляем координату точки и нормализцем ее в "единичный отрезок", потому что текстура рисуется от 0 до 1
{
	double NewCoord = (oldCoord) / 15.; // 15 - максимальный размер рисунка
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
	double O[3];//середина CD



	O[0] = (C[0] + D[0]) / 2;
	O[1] = (C[1] + D[1]) / 2;
	O[2] = 0;

	double radius = (sqrt(pow((C[0] - D[0]), 2) + pow((C[1] - D[1]), 2) + pow((C[2] - D[2]), 2))) / 2; // расстояние между двумя точками

	vector <double> x; //точки на окружности
	vector <double> y;

	for (double i = povorot; i <= (180 + povorot + 1); i += 0.1)
	{
		x.push_back(radius * cos(i * M_PI / 180.0) + O[0]); // функция push_back добавляет значение в конец
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


		glTexCoord2d(i / (x.size() - 1), 1); glVertex3d(x[i], y[i], 2); // берем определенную часть текстуры (делим ее на много прямоугольников)
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
	double M[] = { 6, 4, 2 }; // заданная точка на окружности

	double S[3];//середина AG
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
	double r = 16.6207701386; //радиус

	vector <double> x; //точки на окружности
	vector <double> y;


	double povorot = (atan(A[0] - G[0] / A[1] - G[1]) * 180 / M_PI) + 90; // Формула для вычисления угла поворота окружности

	double alpha_div2 = 46.28; //длина дуги, которую мы будем строить (подбор)
	for (double i = 149.2 - alpha_div2 - 6; i <= 90 + alpha_div2; i += 0.09) //подбор 149.2
	{
		double y1, x1;
		x1 = r * cos(i * M_PI / 180); // Строим окружность в начале координат
		y1 = r * sin(i * M_PI / 180);
		x.push_back(x1 * cos(povorot * M_PI / 180) + y1 * sin(povorot * M_PI / 180) + x0); // Формула для поворота окружности относительно начала координат и перенос центра окружности в точку x0y0
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
		glTexCoord2d(i / (x.size() - 1), 1); glVertex3d(x[i], y[i], 2); // берем определенную часть текстуры (делим ее на много прямоугольников)
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

	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//настройка материала
	GLfloat amb[] = { 0.5, 0.5, 0.5, 1. };
	GLfloat dif[] = { 0.8, 0.8, 0.8, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); 
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  
	glBindTexture(GL_TEXTURE_2D, texId[0]);
	bottom();
	roof();
	
	NormVariant();
	NormVariant2();
    walls();



   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 200);
	rec.setPosition(10, ogl->getHeight() - 200 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}