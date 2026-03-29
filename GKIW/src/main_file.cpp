/**
 * @file main_file.cpp
 * @brief Główny plik programu OpenGL.
 */

/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/


#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "shaderprogram.h"

using namespace glm;

float speed = 0;
float turn = 0;
float My_PI = 3.141592653589793;


//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
    initShaders();
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************	
	glClearColor(0, 0, 0, 1);//Ustaw czarny kolor czyszczenia ekranu
	glEnable(GL_DEPTH_TEST);

	// Kompilacja na mac wymaga tego
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    freeShaders();
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************	
}

void key_callback(GLFWwindow* window, int key,
	int scancode, int action, int mods){
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_RIGHT) speed = 3.14;
		if (key == GLFW_KEY_LEFT) speed = -3.14;
		if (key == GLFW_KEY_A) turn =  3.14;
		if (key == GLFW_KEY_D) turn = -3.14;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_RIGHT) speed = 0;
		if (key == GLFW_KEY_LEFT) speed = 0;
		if (key == GLFW_KEY_A) turn = 0;
		if (key == GLFW_KEY_D) turn = 0;
		}
}
void drawCuboid(mat4 M, float sizeX, float sizeY, float sizeZ) {
    mat4 M_scaled = scale(M, vec3(sizeX, sizeY, sizeZ));
	glUniform4f(spLambert->u("color"), 1, 1, 1, 1);
    glUniformMatrix4fv(spLambert->u("M"), 1, false, value_ptr(M_scaled));
    Models::cube.drawSolid();
}

void drawingFinger(mat4 M, float turn) {
    mat4 M_joint1 = M;
    M_joint1 = rotate(M_joint1, turn, vec3(0.0f, 0.0f, 1.0f));
    M_joint1 = translate(M_joint1, vec3(1.0f, 0.0f, 0.0f));
	drawCuboid(M_joint1, 1.0f, 0.25f, 0.5f);

    mat4 M_joint2 = M_joint1;
    M_joint2 = translate(M_joint2, vec3(1.0f, 0.0f, 0.0f));
    M_joint2 = rotate(M_joint2, turn, vec3(0.0f, 0.0f, 1.0f));
    M_joint2 = translate(M_joint2, vec3(1.0f, 0.0f, 0.0f));
	drawCuboid(M_joint2, 1.0f, 0.25f, 0.5f);

    mat4 M_joint3 = M_joint2;
    M_joint3 = translate(M_joint3, vec3(1.0f, 0.0f, 0.0f));
    M_joint3 = rotate(M_joint3, turn, vec3(0.0f, 0.0f, 1.0f));
    M_joint3 = translate(M_joint3, vec3(1.0f, 0.0f, 0.0f));
	drawCuboid(M_joint3, 1.0f, 0.25f, 0.5f);

}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float angle, float wheelAngle) {
	//************Tutaj umieszczaj kod rysujący obraz******************
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
	mat4 P = perspective(radians(50.0f), 1.0f, 1.0f, 50.0f);
	mat4 V = lookAt(vec3(0.0f, 2.0f, -7.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	// program cieniujacy
	spLambert->use(); 
	// macierz rzutowania
	glUniformMatrix4fv(spLambert->u("P"), 1, false, value_ptr(P));
	// macierz widoku
	glUniformMatrix4fv(spLambert->u("V"), 1, false, value_ptr(V));
	// srodrecze
	mat4 M = mat4(1.0f);
	M = scale(M, vec3(0.5f, 0.5f, 0.5f));
	M = rotate(M, angle, vec3(0.0f, 1.0f, 0.0f));
	drawCuboid(M, 0.5f, 0.25f, 0.5f);
	// palec 1
	mat4 M_p1 = M;
	M_p1 = translate(M_p1, vec3(0.5f, 0.0f, 0.0f));
	drawingFinger(M_p1, wheelAngle);
	// palec 2
	mat4 M_p2 = M;
	M_p2 = rotate(M_p2, My_PI/2, vec3(0.0f, 1.0f, 0.0f));
	M_p2 = translate(M_p2, vec3(0.5f, 0.0f, 0.0f));
	drawingFinger(M_p2, wheelAngle);
	// palec 3
	mat4 M_p3 = M;
	M_p3 = rotate(M_p3, My_PI, vec3(0.0f, 1.0f, 0.0f));
	M_p3 = translate(M_p3, vec3(0.5f, 0.0f, 0.0f));
	drawingFinger(M_p3, wheelAngle);
	// palec 4
	mat4 M_p4 = M;
	M_p4 = rotate(M_p4, 3*My_PI/2, vec3(0.0f, 1.0f, 0.0f));
	M_p4 = translate(M_p4, vec3(0.5f, 0.0f, 0.0f));
	drawingFinger(M_p4, wheelAngle);

	glfwSwapBuffers(window);
}


int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	#ifdef __APPLE__
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	window = glfwCreateWindow(2000, 1000, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące
	glfwSetKeyCallback(window, key_callback);

	//Główna pętla	
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{		
		float angle=0;
		float wheelAngle = 0;
		glfwSetTime(0);
		while (!glfwWindowShouldClose(window)) {
			angle+=speed*glfwGetTime();
			wheelAngle += turn * glfwGetTime();
			glfwSetTime(0);
			drawScene(window,angle, wheelAngle);
			glfwPollEvents();
			} 
		//Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
