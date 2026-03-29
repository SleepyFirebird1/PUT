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

float speed = 0;
float turn = 0;
float My_PI = 3.141592653589793;

static Models::Torus kolo(0.3,0.1,12,12);
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
		if (key == GLFW_KEY_A) turn = PI / 6;
		if (key == GLFW_KEY_D) turn = -PI / 6;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_RIGHT) speed = 0;
		if (key == GLFW_KEY_LEFT) speed = 0;
		if (key == GLFW_KEY_A) turn = 0;
		if (key == GLFW_KEY_D) turn = 0;
		}
}


//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float angle, float wheelAngle) {
	//************Tutaj umieszczaj kod rysujący obraz******************
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
	glm::mat4 P = glm::perspective(glm::radians(50.0f), 1.0f, 1.0f, 50.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 2.0f, -7.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	// program cieniujacy
	spLambert->use(); 
	// macierz rzutowania
	glUniformMatrix4fv(spLambert->u("P"), 1, false, glm::value_ptr(P));
	// macierz widoku
	glUniformMatrix4fv(spLambert->u("V"), 1, false, glm::value_ptr(V));

	glm::mat4 M = glm::mat4(1.0f);
	M = glm::scale(M, glm::vec3(0.8f, 0.8f, 0.8f));
	M = glm::rotate(M, angle, glm::vec3(0.0f, 1.0f, 0.0f));

	// Macierz dla podwozia
	glm::mat4 M_Podwozie = M;
	M_Podwozie = glm::scale(M_Podwozie, glm::vec3(1.5f, 0.125f, 1.0f));
	glUniform4f(spLambert->u("color"), 1, 1, 1, 1);
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M_Podwozie));  //Załadowanie macierzy modelu do programu cieniującego
	Models::cube.drawSolid(); //Narysowanie obiektu

	// Macierz dla koła 1
	glm::mat4 M_Kolo1 = M;
	M_Kolo1 = glm::translate(M_Kolo1, glm::vec3(1.5f, 0.0f, 1.0f));
	M_Kolo1 = glm::rotate(M_Kolo1, turn, glm::vec3(0.0f, 1.0f, 0.0f));
	M_Kolo1 = glm::rotate(M_Kolo1, wheelAngle, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniform4f(spLambert->u("color"), 1, 1, 1, 1);
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M_Kolo1));  //Załadowanie macierzy modelu do programu cieniującego
	kolo.drawSolid(); //Narysowanie obiektu

	// Macierz dla koła 2
	glm::mat4 M_Kolo2 = M;
	M_Kolo2 = glm::translate(M_Kolo2, glm::vec3(1.5f, 0.0f, -1.0f));
	M_Kolo2 = glm::rotate(M_Kolo2, turn, glm::vec3(0.0f, 1.0f, 0.0f));
	M_Kolo2 = glm::rotate(M_Kolo2, wheelAngle, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniform4f(spLambert->u("color"), 1, 1, 1, 1);
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M_Kolo2));  //Załadowanie macierzy modelu do programu cieniującego
	kolo.drawSolid(); //Narysowanie obiektu

	// Macierz dla koła 3
	glm::mat4 M_Kolo3 = M;
	M_Kolo3 = glm::translate(M_Kolo3, glm::vec3(-1.5f, 0.0f, 1.0f));
	M_Kolo3 = glm::rotate(M_Kolo3, wheelAngle, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniform4f(spLambert->u("color"), 1, 1, 1, 1);
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M_Kolo3));  //Załadowanie macierzy modelu do programu cieniującego
	kolo.drawSolid(); //Narysowanie obiektu

	// Macierz dla koła 4
	glm::mat4 M_Kolo4 = M;
	M_Kolo4 = glm::translate(M_Kolo4, glm::vec3(-1.5f, 0.0f, -1.0f));
	M_Kolo4 = glm::rotate(M_Kolo4, wheelAngle, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniform4f(spLambert->u("color"), 1, 1, 1, 1);
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M_Kolo4));  //Załadowanie macierzy modelu do programu cieniującego
	kolo.drawSolid(); //Narysowanie obiektu

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

	window = glfwCreateWindow(1000, 1000, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

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
			wheelAngle += -PI / 6 * glfwGetTime();
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
