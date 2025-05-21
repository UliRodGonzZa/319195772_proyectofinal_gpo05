/*
* Elaborado por: Bravo Luna Ivonne Monserrat
*/
#include <iostream>
#include <stdlib.h>

// GLAD: Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator
// https://glad.dav1d.de/
#include <glad/glad.h>

// GLFW: https://www.glfw.org/
#include <GLFW/glfw3.h>

// GLM: OpenGL Math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Model loading classes
#include <shader_m.h>
#include <camera.h>
#include <animatedmodel.h>
#include <model.h>
#include <material.h>
#include <light.h>
#include <cubemap.h>

#include <irrKlang.h>
using namespace irrklang;

float proceduralTime = 0.0f;

// Functions
bool Start();
bool Update();

// Definición de callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Gobals
GLFWwindow* window;

// Tamaño en pixeles de la ventana
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Definición de cámara
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));

// Controladores para el movimiento del mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Variables para la velocidad de reproducción
// de la animación
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float elapsedTime = 0.0f;
float door_rotation = 0.0f;
float rotateCharacter = 0.0f;
float     trdpersonOffset = 1.5f;
float     scaleV = 0.025f;
Camera camera3rd(glm::vec3(0.0f, 0.0f, 0.0f));


glm::vec3 position(7.0f, -1.7f, -3.0f);

// Shaders
Shader* phongShader;
Shader* basicShader;
Shader* cubemapShader;
Shader* proceduralShader;
Shader* dynamicShader;



// Carga la información del modelo
Model* model1;
Model* model2;
Model* puerta;
float doorAngle = 0.0f;
Model* lightDummy;
Model* moon;

// Modelos animados
AnimatedModel* character01;


float tradius = 10.0f;
float theta = 0.0f;
float alpha = 0.0f;

// Cubemap
CubeMap* mainCubeMap;

// Materiales
Material material;
Material glassMaterial;
Light    light;

// Audio
ISoundEngine* SoundEngine = createIrrKlangDevice();
glm::vec3 forwardView(0.0f, 0.0f, 1.0f);
bool thirdPersonMode = false;
float angleAroundTarget = 0.0f;
float distanceToTarget = 10.0f;
float heightOffset = 4.0f;
glm::vec3 targetPosition = glm::vec3(0.0f);

// Entrada a función principal
int main()
{
	if (!Start())
		return -1;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		if (!Update())
			break;
	}

	glfwTerminate();
	return 0;

}

bool Start() {
	// Inicialización de GLFW

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creación de la ventana con GLFW
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Illumination Models", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Ocultar el cursor mientras se rota la escena
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: Cargar todos los apuntadores
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	// Activación de buffer de profundidad
	glEnable(GL_DEPTH_TEST);

	// Compilación y enlace de shadersstatic
	phongShader = new Shader("shaders/11_BasicPhongShader.vs", "shaders/11_BasicPhongShader.fs");
	basicShader = new Shader("shaders/10_vertex_simple.vs", "shaders/10_fragment_simple.fs");
	cubemapShader = new Shader("shaders/10_vertex_cubemap.vs", "shaders/10_fragment_cubemap.fs");
	proceduralShader = new Shader("shaders/12_ProceduralAnimation.vs", "shaders/12_ProceduralAnimation.fs");
	moon = new Model("models/IllumModels/moon.fbx");
	dynamicShader = new Shader("shaders/10_vertex_skinning-IT.vs", "shaders/10_fragment_skinning-IT.fs");
	character01 = new AnimatedModel("models/IllumModels/KAYA.fbx");

	// Dibujar en malla de alambre
	// glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	// Máximo número de huesos: 100
	dynamicShader->setBonesIDs(MAX_RIGGING_BONES);



	model1 = new Model("models/IllumModels/casaBraLun.fbx");
	model2 = new Model("models/IllumModels/vidrio.fbx");
	puerta = new Model("models/IllumModels/puertaExt.fbx");


	// Cubemap
	vector<std::string> faces
	{
		"textures/cubemap/01/posx.png",
		"textures/cubemap/01/negx.png",
		"textures/cubemap/01/posy.png",
		"textures/cubemap/01/negy.png",
		"textures/cubemap/01/posz.png",
		"textures/cubemap/01/negz.png"
	};
	mainCubeMap = new CubeMap();
	mainCubeMap->loadCubemap(faces);

	// Parámetros de la Iluminación
	light.Position = glm::vec3(0.0f, 5.0f, 0.0f);
	light.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	light.Power = glm::vec4(60.0f, 60.0f, 60.0f, 1.0f);
	light.alphaIndex = 10;
	light.distance = 5.0f;

	//mateerial normal
	material.ambient = glm::vec4(0.3f);
	material.diffuse = glm::vec4(0.7f);
	material.specular = glm::vec4(1.0f);
	material.transparency = 1.0f;
	//material vidrio
	glassMaterial.ambient = glm::vec4(0.1f, 0.2f, 0.3f, 1.0f);
	glassMaterial.diffuse = glm::vec4(0.3f, 0.4f, 0.5f, 0.3f);
	glassMaterial.specular = glm::vec4(0.9f, 0.95f, 1.0f, 0.8f);
	glassMaterial.transparency = 0.3f;


	lightDummy = new Model("models/IllumModels/lightDummy.fbx");

	// SoundEngine->play2D("sound/EternalGarden.mp3", true);

	return true;
}

bool Update() {
	// Cálculo del framerate
	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// Procesa la entrada del teclado o mouse
	processInput(window);

	if (thirdPersonMode) {
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			angleAroundTarget -= 50.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			angleAroundTarget += 50.0f * deltaTime;

		float offsetX = distanceToTarget * sin(glm::radians(angleAroundTarget));
		float offsetZ = distanceToTarget * cos(glm::radians(angleAroundTarget));
		camera.Position = targetPosition + glm::vec3(offsetX, heightOffset, offsetZ);
		camera.Front = glm::normalize(targetPosition - camera.Position);
		camera.Yaw = -90.0f + angleAroundTarget;
		camera.Pitch = -glm::degrees(atan(heightOffset / distanceToTarget));
	}

	// Renderizado R - G - B - A
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
	glm::mat4 view = camera.GetViewMatrix();

	// Cubemap (fondo)
	{
		mainCubeMap->drawCubeMap(*cubemapShader, projection, view);
	}

	// Dibujamos un objeto cualquiera
	{
		// Activamos el shader de Phong
		phongShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		phongShader->setMat4("projection", projection);
		phongShader->setMat4("view", view);

		// Configuramos propiedades de fuentes de luz
		phongShader->setVec4("LightColor", light.Color);
		phongShader->setVec4("LightPower", light.Power);
		phongShader->setInt("alphaIndex", light.alphaIndex);
		phongShader->setFloat("distance", light.distance);
		phongShader->setVec3("lightPosition", light.Position);
		phongShader->setVec3("lightDirection", light.Direction);
		phongShader->setVec3("eye", camera.Position);

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		phongShader->setMat4("model", model);

		// Aplicamos propiedades materiales
		// Modelo 1 normal
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			phongShader->setMat4("model", model);
			phongShader->setVec4("MaterialAmbientColor", material.ambient);
			phongShader->setVec4("MaterialDiffuseColor", material.diffuse);
			phongShader->setVec4("MaterialSpecularColor", material.specular);
			phongShader->setFloat("transparency", material.transparency);
			model1->Draw(*phongShader);
		}


		// Modelo 2 vidrio
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			phongShader->setMat4("model", model);
			phongShader->setVec4("MaterialAmbientColor", glassMaterial.ambient);
			phongShader->setVec4("MaterialDiffuseColor", glassMaterial.diffuse);
			phongShader->setVec4("MaterialSpecularColor", glassMaterial.specular);
			phongShader->setFloat("transparency", glassMaterial.transparency);
			model2->Draw(*phongShader);
		}


	}

	glUseProgram(0);

	// Dibujamos un objeto cualquiera
	{
		// Activamos el shader de Phong
		phongShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		phongShader->setMat4("projection", projection);
		phongShader->setMat4("view", view);

		// Configuramos propiedades de fuentes de luz
		phongShader->setVec4("LightColor", light.Color);
		phongShader->setVec4("LightPower", light.Power);
		phongShader->setInt("alphaIndex", light.alphaIndex);
		phongShader->setFloat("distance", light.distance);
		phongShader->setVec3("lightPosition", light.Position);
		phongShader->setVec3("lightDirection", light.Direction);
		phongShader->setVec3("eye", camera.Position);

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
		phongShader->setMat4("model", model);

		{
			phongShader->setMat4("model", model);
			phongShader->setVec4("MaterialAmbientColor", material.ambient);
			phongShader->setVec4("MaterialDiffuseColor", material.diffuse);
			phongShader->setVec4("MaterialSpecularColor", material.specular);
			phongShader->setFloat("transparency", material.transparency);
		}


		// Paso 2: aplicar la rotación

		model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));


		model = glm::translate(glm::mat4(1.0f), glm::vec3(9.9f,-0.66f,-6.6f));
		//model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.8f, -0.7f));

		model = glm::rotate(model, glm::radians(door_rotation), glm::vec3(0.0f, 1.0f, 0.0f));

		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	
		phongShader->setMat4("model", model);
		puerta->Draw(*phongShader);
	}

	glUseProgram(0);

	{
		character01->UpdateAnimation(deltaTime);

		// Activación del shader del personaje
		dynamicShader->use();

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		dynamicShader->setMat4("projection", projection);
		dynamicShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down

		dynamicShader->setMat4("model", model);

		dynamicShader->setMat4("gBones", MAX_RIGGING_BONES, character01->gBones);

		// Dibujamos el modelo
		character01->Draw(*dynamicShader);
	}

		glUseProgram(0);



	{
		// Activamos el shader de Phong
		proceduralShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		proceduralShader->setMat4("projection", projection);
		proceduralShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(10.0f, 10.0f, -12.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		proceduralShader->setMat4("model", model);

		proceduralShader->setFloat("time", proceduralTime);
		proceduralShader->setFloat("radius", 4.0f);
		proceduralShader->setFloat("height", 0.0f);

		moon->Draw(*proceduralShader);
		proceduralTime += 0.002;

	}



	// Dibujamos el indicador auxiliar de la fuente de iluminación
	{
		basicShader->use();

		basicShader->setMat4("projection", projection);
		basicShader->setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, light.Position);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		basicShader->setMat4("model", model);

		lightDummy->Draw(*basicShader);

	}

	glUseProgram(0);

	// glfw: swap buffers 
	glfwSwapBuffers(window);
	glfwPollEvents();

	return true;
}

// Procesamos entradas del teclado
void processInput(GLFWwindow* window)
{
	static bool tKeyPressed = false;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
		if (!tKeyPressed) {
			thirdPersonMode = !thirdPersonMode;
			tKeyPressed = true;
		}
	}
	else {
		tKeyPressed = false;
	}

	if (!thirdPersonMode) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, deltaTime);
	}
	else {
		if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
			targetPosition.z -= 5.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
			targetPosition.z += 5.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
			targetPosition.x -= 5.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
			targetPosition.x += 5.0f * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		light.Position.x -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		light.Position.x += 0.1f;

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);


	// Character movement
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {

		position = position + scaleV * forwardView;
		camera3rd.Front = forwardView;
		camera3rd.ProcessKeyboard(FORWARD, deltaTime);
		camera3rd.Position = position;
		camera3rd.Position.y += 1.7f;
		camera3rd.Position -= trdpersonOffset * forwardView;

	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		position = position - scaleV * forwardView;
		camera3rd.Front = forwardView;
		camera3rd.ProcessKeyboard(BACKWARD, deltaTime);
		camera3rd.Position = position;
		camera3rd.Position.y += 1.7f;
		camera3rd.Position -= trdpersonOffset * forwardView;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		rotateCharacter += 0.5f;

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 viewVector = model * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		forwardView = glm::vec3(viewVector);
		forwardView = glm::normalize(forwardView);

		camera3rd.Front = forwardView;
		camera3rd.Position = position;
		camera3rd.Position.y += 1.7f;
		camera3rd.Position -= trdpersonOffset * forwardView;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		rotateCharacter -= 0.5f;

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 viewVector = model * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		forwardView = glm::vec3(viewVector);
		forwardView = glm::normalize(forwardView);

		camera3rd.Front = forwardView;
		camera3rd.Position = position;
		camera3rd.Position.y += 1.7f;
		camera3rd.Position -= trdpersonOffset * forwardView;
	}	

	// Abrir puerta con tecla O
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
			door_rotation += 60.0f * deltaTime; // Ajusta la velocidad de apertura
	}
	// Cerrar puerta con tecla P
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		
			door_rotation -= 60.0f * deltaTime; // Ajusta la velocidad de cierre
	}
}

// glfw: Actualizamos el puerto de vista si hay cambios del tamaño
// de la ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: Callback del movimiento y eventos del mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (thirdPersonMode) return;
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos;

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: Complemento para el movimiento y eventos del mouse
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}