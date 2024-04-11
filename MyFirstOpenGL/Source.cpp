#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <conio.h>
#include <thread>
#include <chrono>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

std::vector<GLuint> compiledPrograms;

struct GameObject {
	glm::vec3 position = glm::vec3(0.f);
	glm::vec3 rotation = glm::vec3(0.f);
	glm::vec3 scale = glm::vec3(1.f);
	glm::vec3 forward = glm::vec3(0.f, 1.f, 0.f);
	float fAngularVelocity = 0.05f;
	float fScaleVelocity = 0.0001f;
};

//Variables comunes
float fVelocity = 0.0005f;
bool pause = false;
bool wireframeMode;
bool printPiramide;
bool printCube;

enum color
{
	RED,
	GREEN,
	YELLOW
};

color colorPiramide = RED;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	
	if (!pause)
	{
		if (key == GLFW_KEY_M && action == GLFW_PRESS) {
			std::cout << "Transformations velocity +10%" << std::endl;
			fVelocity = fVelocity * 1.1f;
		}
		if (key == GLFW_KEY_N && action == GLFW_PRESS) {
			std::cout << "Transformations velocity -10%" << std::endl;
			fVelocity -= fVelocity * 0.1f;
		}
		if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
			std::cout << "Cube" << std::endl;
			printCube = !printCube;
		}
		if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
			std::cout << "Piramide" << std::endl;
			printPiramide = !printPiramide;
		}
		if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
			wireframeMode = !wireframeMode;
			if (wireframeMode) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
			}
		}
	}
	
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		
		if (pause)
		{
			pause = false;
			std::cout << "Reanudar Programa" << std::endl;
		}
		else
		{
			std::cout << "Pausar Programa" << std::endl;
			pause = true;
		}
	}
}

	struct InputManager
	{
		float fvelocity = 0.0005f;
	};

struct ShaderProgram {

	GLuint vertexShader = 0;
	GLuint geometryShader = 0;
	GLuint fragmentShader = 0;
};

void Resize_Window(GLFWwindow* window, int iFrameBufferWidth, int iFrameBufferHeight) {

	//Definir nuevo tama�o del viewport
	glViewport(0, 0, iFrameBufferWidth, iFrameBufferHeight);

	glUniform2f(glGetUniformLocation(compiledPrograms[0], "windowSize"), iFrameBufferWidth, iFrameBufferHeight);
	
}

glm::mat4 GenerateTranslationMatrix(glm::vec3 translation) {

	return glm::translate(glm::mat4(1.0f), translation);
}

glm::mat4 GenerateRotationMatrix(glm::vec3 axis, float fDegrees) {

	return glm::rotate(glm::mat4(1.0f), glm::radians(fDegrees), glm::normalize(axis));
}

glm::mat4 GenerateScaleMatrix(glm::vec3 scaleAxis) {

	return glm::scale(glm::mat4(1.0f),scaleAxis);
}

//Funcion que devolvera una string con todo el archivo leido
std::string Load_File(const std::string& filePath) {

	std::ifstream file(filePath);

	std::string fileContent;
	std::string line;
	
	//Lanzamos error si el archivo no se ha podido abrir
	if (!file.is_open()) {
		std::cerr << "No se ha podido abrir el archivo: " << filePath << std::endl;
		std::exit(EXIT_FAILURE);
	}

	//Leemos el contenido y lo volcamos a la variable auxiliar
	while (std::getline(file, line)) {
		fileContent += line + "\n";
	}

	//Cerramos stream de datos y devolvemos contenido
	file.close();

	return fileContent;
}

GLuint LoadFragmentShader(const std::string& filePath) {

	// Crear un fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//Usamos la funcion creada para leer el fragment shader y almacenarlo 
	std::string sShaderCode = Load_File(filePath);
	const char* cShaderSource = sShaderCode.c_str();

	//Vinculamos el fragment shader con su c�digo fuente
	glShaderSource(fragmentShader, 1, &cShaderSource, nullptr);

	// Compilar el fragment shader
	glCompileShader(fragmentShader);

	// Verificar errores de compilaci�n
	GLint success;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	//Si la compilacion ha sido exitosa devolvemos el fragment shader
	if (success) {

		return fragmentShader;

	}
	else {

		//Obtenemos longitud del log
		GLint logLength;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);

		//Obtenemos el log
		std::vector<GLchar> errorLog(logLength);
		glGetShaderInfoLog(fragmentShader, logLength, nullptr, errorLog.data());

		//Mostramos el log y finalizamos programa
		std::cerr << "Se ha producido un error al cargar el fragment shader:  " << errorLog.data() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}


GLuint LoadGeometryShader(const std::string& filePath) {

	// Crear un vertex shader
	GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);

	//Usamos la funcion creada para leer el vertex shader y almacenarlo 
	std::string sShaderCode = Load_File(filePath);
	const char* cShaderSource = sShaderCode.c_str();

	//Vinculamos el vertex shader con su c�digo fuente
	glShaderSource(geometryShader, 1, &cShaderSource, nullptr);

	// Compilar el vertex shader
	glCompileShader(geometryShader);

	// Verificar errores de compilaci�n
	GLint success;
	glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);

	//Si la compilacion ha sido exitosa devolvemos el vertex shader
	if (success) {

		return geometryShader;

	}
	else {

		//Obtenemos longitud del log
		GLint logLength;
		glGetShaderiv(geometryShader, GL_INFO_LOG_LENGTH, &logLength);

		//Obtenemos el log
		std::vector<GLchar> errorLog(logLength);
		glGetShaderInfoLog(geometryShader, logLength, nullptr, errorLog.data());

		//Mostramos el log y finalizamos programa
		std::cerr << "Se ha producido un error al cargar el vertex shader:  " << errorLog.data() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

GLuint LoadVertexShader(const std::string& filePath) {

	// Crear un vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//Usamos la funcion creada para leer el vertex shader y almacenarlo 
	std::string sShaderCode = Load_File(filePath);
	const char* cShaderSource = sShaderCode.c_str();

	//Vinculamos el vertex shader con su c�digo fuente
	glShaderSource(vertexShader, 1, &cShaderSource, nullptr);

	// Compilar el vertex shader
	glCompileShader(vertexShader);

	// Verificar errores de compilaci�n
	GLint success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	//Si la compilacion ha sido exitosa devolvemos el vertex shader
	if (success) {

		return vertexShader;

	}else {

		//Obtenemos longitud del log
		GLint logLength;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);

		//Obtenemos el log
		std::vector<GLchar> errorLog(logLength);
		glGetShaderInfoLog(vertexShader, logLength, nullptr, errorLog.data());

		//Mostramos el log y finalizamos programa
		std::cerr << "Se ha producido un error al cargar el vertex shader:  " << errorLog.data() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

//Funci�n que dado un struct que contiene los shaders de un programa generara el programa entero de la GPU
GLuint CreateProgram(const ShaderProgram& shaders) {

	//Crear programa de la GPU
	GLuint program = glCreateProgram();

	//Verificar que existe un vertex shader y adjuntarlo al programa
	if (shaders.vertexShader != 0) {
		glAttachShader(program, shaders.vertexShader);
	}

	if (shaders.geometryShader != 0) {
		glAttachShader(program, shaders.geometryShader);
	}

	if (shaders.fragmentShader != 0) {
		glAttachShader(program, shaders.fragmentShader);
	}

	// Linkear el programa
	glLinkProgram(program);

	//Obtener estado del programa
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	//Devolver programa si todo es correcto o mostrar log en caso de error
	if (success) {

		//Liberamos recursos
		if (shaders.vertexShader != 0) {
			glDetachShader(program, shaders.vertexShader);
		}

		//Liberamos recursos
		if (shaders.geometryShader != 0) {
			glDetachShader(program, shaders.geometryShader);
		}

		//Liberamos recursos
		if (shaders.fragmentShader != 0) {
			glDetachShader(program, shaders.fragmentShader);
		}

		return program;
	}
	else {

		//Obtenemos longitud del log
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

		//Almacenamos log
		std::vector<GLchar> errorLog(logLength);
		glGetProgramInfoLog(program, logLength, nullptr, errorLog.data());

		std::cerr << "Error al linkar el programa:  " << errorLog.data() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

void contador() {
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(2));
		std::cout << "Han pasado 2 segundos." << std::endl;
		switch (colorPiramide)
		{
		case RED:
			colorPiramide = GREEN;
			break;
		case GREEN:
			colorPiramide = YELLOW;
			break;
		case YELLOW:
			colorPiramide = RED;
			break;
		default:
			break;
		}
	}
}

void main() {

	//Definir semillas del rand seg�n el tiempo
	srand(static_cast<unsigned int>(time(NULL)));

	//timer
	std::thread t(contador); // Inicia el hilo del contador
	t.detach(); // Permite que el hilo se ejecute en segundo plano

	//Inicializamos GLFW para gestionar ventanas e inputs
	glfwInit();

	//Configuramos la ventana
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	//Inicializamos la ventana
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "My Engine", NULL, NULL);

	//Cargamos los inputs
	glfwSetKeyCallback(window, KeyCallback);

	//Asignamos funci�n de callback para cuando el frame buffer es modificado
	glfwSetFramebufferSizeCallback(window, Resize_Window);

	//Definimos espacio de trabajo
	glfwMakeContextCurrent(window);

	//Permitimos a GLEW usar funcionalidades experimentales
	glewExperimental = GL_TRUE;

	//Activamos cull face
	glEnable(GL_CULL_FACE);

	//Indicamos lado del culling
	glCullFace(GL_BACK);

	//Inicializamos GLEW y controlamos errores
	if (glewInit() == GLEW_OK) {
		GameObject cube;
		cube.position = glm::vec3(-0.65f, 0.0f, 0.0f);
		GameObject piramide;
		piramide.position = glm::vec3(0.7f, 0.0f, 0.0f);
		float anglePiramide = 0.0f;
		glm::vec3 currentColorPiramide = glm::vec3(1.0f, 1.0f, 1.0f);
		GameObject ortoedro;
		ortoedro.position = glm::vec3(0.f, 0.f, 0.f);
		ortoedro.scale = glm::vec3(1.f, 2.8f, 1.f);

		//Colores 
		glm::vec3 redColor = glm::vec3(0.0f, 1.0f, 0.0f);

		//Declarar vec2 para definir el offset
		glm::vec2 offset = glm::vec2(0.f, 0.f);

		//Compilar shaders
		ShaderProgram myFirstProgram;
		myFirstProgram.vertexShader = LoadVertexShader("MyFirstVertexShader.glsl");
		myFirstProgram.geometryShader = LoadGeometryShader("MyFirstGeometryShader.glsl");
		myFirstProgram.fragmentShader = LoadFragmentShader("MyFirstFragmentShader.glsl");

		ShaderProgram mySecondProgram;
		mySecondProgram.vertexShader = LoadVertexShader("MySecondVertexShader.glsl");
		mySecondProgram.geometryShader = LoadGeometryShader("MySecondGeometryShader.glsl");
		mySecondProgram.fragmentShader = LoadFragmentShader("MySecondFragmentShader.glsl");

		//Compilar programa
		compiledPrograms.push_back(CreateProgram(myFirstProgram));
		compiledPrograms.push_back(CreateProgram(mySecondProgram));

		//Definimos color para limpiar el buffer de color
		glClearColor(0.f, 0.f, 0.f, 1.f);

		GLuint vaoPuntos, vboPuntos;
		GLuint vaoPiramide, vboPiramide;
		GLuint vaoOrtoedro, vboOrtoedro;

		//Definimos cantidad de vao a crear y donde almacenarlos 
		glGenVertexArrays(1, &vaoPuntos);
		glGenVertexArrays(1, &vaoPiramide);
		glGenVertexArrays(1, &vaoOrtoedro);
	

		//Definimos cantidad de vbo a crear y donde almacenarlos
		glGenBuffers(1, &vboPuntos);
		glGenBuffers(1, &vboPiramide);
		glGenBuffers(1, &vboOrtoedro);

		//Indico que el VBO activo es el que acabo de crear y que almacenar� un array. Todos los VBO que genere se asignaran al �ltimo VAO que he hecho glBindVertexArray
		glBindBuffer(GL_ARRAY_BUFFER, vboPuntos);
		glBindBuffer(GL_ARRAY_BUFFER, vboPiramide);
		glBindBuffer(GL_ARRAY_BUFFER, vboOrtoedro);

		//Posici�n X e Y del punto
		GLfloat cuboPuntos[] = {
		-0.25f,  0.25f, -0.25f,
		 0.25f,  0.25f, -0.25f,
		-0.25f, -0.25f, -0.25f,
		 0.25f, -0.25f, -0.25f,
		 0.25f, -0.25f,  0.25f,
		 0.25f,  0.25f, -0.25f,
		 0.25f,  0.25f,  0.25f,
		-0.25f,  0.25f, -0.25f,
		-0.25f,  0.25f,  0.25f,
		-0.25f, -0.25f, -0.25f,
		-0.25f, -0.25f,  0.25f,
		 0.25f, -0.25f,  0.25f,
		-0.25f,  0.25f,  0.25f,
		 0.25f,  0.25f,  0.25f
		};

		GLfloat ortoedroPuntos[] = {
		-0.25f,  0.25f, -0.25f,
		 0.25f,  0.25f, -0.25f,
		-0.25f, -0.25f, -0.25f,
		 0.25f, -0.25f, -0.25f,
		 0.25f, -0.25f,  0.25f,
		 0.25f,  0.25f, -0.25f,
		 0.25f,  0.25f,  0.25f,
		-0.25f,  0.25f, -0.25f,
		-0.25f,  0.25f,  0.25f,
		-0.25f, -0.25f, -0.25f,
		-0.25f, -0.25f,  0.25f,
		 0.25f, -0.25f,  0.25f,
		-0.25f,  0.25f,  0.25f,
		 0.25f,  0.25f,  0.25f
		};

		GLfloat piramidePuntos[] = {
		// Base de la pir�mide (m�s peque�a)
		-0.25f, -0.25f, -0.25f,  // V�rtice 1
		 0.25f, -0.25f, -0.25f,  // V�rtice 2
		 0.25f, -0.25f,  0.25f,  // V�rtice 3
		-0.25f, -0.25f,  0.25f,  // V�rtice 4

		// Caras laterales
		0.0f,  0.25f,  0.0f,  // V�rtice 5 (punta)
		-0.25f, -0.25f, -0.25f,  // V�rtice 1
		-0.25f, -0.25f,  0.25f,  // V�rtice 4

		0.0f,  0.25f,  0.0f,  // V�rtice 5 (punta)
		-0.25f, -0.25f,  0.25f,  // V�rtice 4
		 0.25f, -0.25f,  0.25f,  // V�rtice 3

		0.0f,  0.25f,  0.0f,  // V�rtice 5 (punta)
		 0.25f, -0.25f,  0.25f,  // V�rtice 3
		 0.25f, -0.25f, -0.25f,  // V�rtice 2

		0.0f,  0.25f,  0.0f,  // V�rtice 5 (punta)
		 0.25f, -0.25f, -0.25f,  // V�rtice 2
		-0.25f, -0.25f, -0.25f,  // V�rtice 1
		};


		//Definimos modo de dibujo para cada cara
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//Indico que el VAO activo de la GPU es el que acabo de crear
		glBindVertexArray(vaoPuntos);
		//Indico que el VBO activo es el que acabo de crear y que almacenar� un array. Todos los VBO que genere se asignaran al �ltimo VAO que he hecho glBindVertexArray
		glBindBuffer(GL_ARRAY_BUFFER, vboPuntos);
		//Ponemos los valores en el VBO creado
		glBufferData(GL_ARRAY_BUFFER, sizeof(cuboPuntos), cuboPuntos, GL_DYNAMIC_DRAW);
		//Indicamos donde almacenar y como esta distribuida la informaci�n
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		//Indicamos que la tarjeta gr�fica puede usar el atributo 0
		glEnableVertexAttribArray(0);
		//Desvinculamos VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		//Desvinculamos VAO
		glBindVertexArray(0);

		glBindVertexArray(vaoPiramide);
		glBindBuffer(GL_ARRAY_BUFFER, vboPiramide);
		glBufferData(GL_ARRAY_BUFFER, sizeof(piramidePuntos), piramidePuntos, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glBindVertexArray(vaoOrtoedro);
		glBindBuffer(GL_ARRAY_BUFFER, vboOrtoedro);
		glBufferData(GL_ARRAY_BUFFER, sizeof(ortoedroPuntos), ortoedroPuntos, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		//Indicar a la tarjeta GPU que programa debe usar
		glUseProgram(compiledPrograms[1]);

		//Asignar valores iniciales al programa
		glUniform2f(glGetUniformLocation(compiledPrograms[1], "windowSize"), WINDOW_WIDTH, WINDOW_HEIGHT);

		//Generamos el game loop

		while (!glfwWindowShouldClose(window)) {

			//Pulleamos los eventos (botones, teclas, mouse...)
			glfwPollEvents();
			if (pause == false)
			{
				
				//Limpiamos los buffers
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				//Definimos que queremos usar el VAO con los puntos
				glBindVertexArray(vaoPuntos);
				
				//Generar el modelo de la matriz MVP
				glm::mat4 cubemodelMatrix = glm::mat4(1.0f);

				//Calculamos la nueva posicion del cubo
				cube.position = cube.position + cube.forward * fVelocity;
				cube.rotation = cube.rotation + glm::vec3(0.f, 0.05f, 0.f) * cube.fAngularVelocity;

				//invertimos direccion si se sale de los limites
				if (cube.position.y >= 0.5f || cube.position.y <= -0.5f) {

					cube.forward = cube.forward * -1.f;
				}

				//Generar una matriz de traslacion
				glm::mat4 cubetranslationMatrix = GenerateTranslationMatrix(cube.position);
				glm::mat4 cubeRotationMatrix = GenerateRotationMatrix(glm::vec3(0.f, 1.f, 0.f), cube.rotation.y);

				//Aplico las matrices
				cubemodelMatrix = cubetranslationMatrix * cubeRotationMatrix * cubemodelMatrix;

				glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "transform"), 1, GL_FALSE, glm::value_ptr(cubemodelMatrix));
				
				// Calcula la posici�n vertical del cubo con respecto al centro de la ventana
				float verticalPosition = (cube.position.y + 1.0f) * (WINDOW_HEIGHT / 2);

				// Establece el color del cubo basado en su posici�n vertical
				if (verticalPosition > WINDOW_HEIGHT / 2) {
					glUniform3f(glGetUniformLocation(compiledPrograms[0], "colorAbove"), 1.0f, 1.0f, 0.0f); // Amarillo
				}
				else {
					glUniform3f(glGetUniformLocation(compiledPrograms[0], "colorBelow"), 1.0f, 0.5f, 0.0f); // Naranja
				}

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);

				//PIRAMIDE-------------------------------
					//aplicamos color
					switch (colorPiramide)
					{
					case RED:
						currentColorPiramide = glm::vec3(1.0f, 0.0f, 0.0f);
						break;
					case GREEN:
						currentColorPiramide = glm::vec3(0.0f, 1.0f, 0.0f);
						break;
					case YELLOW:
						currentColorPiramide = glm::vec3(0.0f, 0.0f, 1.0f);
						break;
					default:
						break;
					}
					glUniform3fv(glGetUniformLocation(compiledPrograms[0], "objectColor"), 1, glm::value_ptr(currentColorPiramide));

					//Generar el modelo de la matriz MVP
				glBindVertexArray(vaoPiramide);

				glm::mat4 piramideModelMatrix = glm::mat4(1.0f);

					//Rotaci�n
					piramideModelMatrix = glm::rotate(piramideModelMatrix, glm::radians(anglePiramide), glm::vec3(1.0f, 1.0f, 0.0f));

					//Calculamos la nueva posicion del cubo
					piramide.position += piramide.forward * fVelocity;

					//invertimos direccion si se sale de los limites
					if (piramide.position.y >= 0.5f || piramide.position.y <= -0.5f) {

						piramide.forward = piramide.forward * -1.f;
					}

					//Generar una matriz de traslacion
					glm::mat4 piramideTranslationMatrix = GenerateTranslationMatrix(piramide.position);

					//Aplico las matrices
					piramideModelMatrix = piramideTranslationMatrix * piramideModelMatrix;

					glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "transform"), 1, GL_FALSE, glm::value_ptr(piramideModelMatrix));

				//Definimos que queremos dibujar
				
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 18);

				//Definimos que queremos usar el VAO con los puntos
				glBindVertexArray(vaoOrtoedro);

				//Generar el modelo de la matriz MVP
				glm::mat4 ortoedromodelMatrix = glm::mat4(1.0f);

				//Calculamos la nueva posicion del cubo
				ortoedro.rotation = ortoedro.rotation + glm::vec3(0.f, 0.05f, 0.f) * ortoedro.fAngularVelocity;
				ortoedro.scale = ortoedro.scale + glm::vec3(0.f, 1.f, 0.f) * ortoedro.fScaleVelocity;

				if (ortoedro.scale.y >= 3.f || ortoedro.scale.y <= 1.f) {

					ortoedro.fScaleVelocity = ortoedro.fScaleVelocity * -1.f;
				}

				//Generar una matriz de traslacion
				glm::mat4 ortoedroRotationMatrix = GenerateRotationMatrix(glm::vec3(0.f, 0.f, 1.f), ortoedro.rotation.z);
				glm::mat4 ortoedroScaleMatrix = GenerateScaleMatrix(ortoedro.scale);

				//Aplico las matrices
				ortoedromodelMatrix = ortoedroRotationMatrix * ortoedroScaleMatrix * ortoedromodelMatrix;

				glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "transform"), 1, GL_FALSE, glm::value_ptr(ortoedromodelMatrix));

				// Calcula la posici�n vertical del cubo con respecto al centro de la ventana
				float verticalPositionOrtoedro = (ortoedro.position.y + 1.0f) * (WINDOW_HEIGHT / 2);

				// Establece el color del cubo basado en su posici�n vertical
				if (verticalPositionOrtoedro > WINDOW_HEIGHT / 2) {
					glUniform3f(glGetUniformLocation(compiledPrograms[0], "colorAbove"), 1.0f, 1.0f, 0.0f); // Amrillo
				}
				else {
					glUniform3f(glGetUniformLocation(compiledPrograms[0], "colorBelow"), 1.0f, 0.5f, 0.0f); // Naranja
				}

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
				//if(!printCube)
				//glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
				if(!printPiramide)
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 18);

				//Dejamos de usar el VAO indicado anteriormente
				glBindVertexArray(0);



				//Cambiamos buffers
				glFlush();
				glfwSwapBuffers(window);

				anglePiramide += fVelocity * 10;
			}

			
		}

		//Desactivar y eliminar programa
		glUseProgram(0);
		glDeleteProgram(compiledPrograms[0]);

	}
	else {
		std::cout << "Ha petao." << std::endl;
		glfwTerminate();
	}

	//Finalizamos GLFW
	glfwTerminate();

}