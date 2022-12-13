/*---------------------------------------------------------*/
/* ----------------  Práctica 10                  -----------*/
/*-----------------    2023-1   ---------------------------*/
/*------------- Alumno: Martinez Zambrano Rodrigo  ---------------*/
/*------------- No. Cuenta: 316204707                  ---------------*/

#include <Windows.h>

#include <glad/glad.h>	//Permite cargar los modelos 3D
#include <glfw3.h>		//main
#include <stdlib.h>		
#include <glm/glm.hpp>	//camera y model
#include <glm/gtc/matrix_transform.hpp>	//camera y model
#include <glm/gtc/type_ptr.hpp>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>	//Texture

#define SDL_MAIN_HANDLED
#include <SDL/SDL.h>

#include <shader_m.h>
#include <camera.h>
#include <modelAnim.h>
#include <model.h>
#include <Skybox.h>
#include <iostream>

#include <stdlib.h> //Librerias adicionales
#include <string.h>
#include <fstream>

//#include <SDL/SDL_mixer.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void my_input(GLFWwindow* window, int key, int scancode, int action, int mods);
void animate(void);


// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWmonitor *monitors;

void getResolution(void);

// camera
Camera camera(glm::vec3(0.0f, 500.0f, 1500.0f));
float MovementSpeed = 0.1f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
const int FPS = 60;
const int LOOP_TIME = 1000 / FPS; // = 16 milisec // 1000 millisec == 1 sec
double	deltaTime = 0.0f,
		lastFrame = 0.0f;

//Lighting
glm::vec3 lightPosition(0.0f, 4.0f, -10.0f); //apunta de afuera del escenario, como si fuera un proyector
glm::vec3 lightDirection(0.0f, -1.0f, -1.0f);
glm::vec3 lightPosition2(1.0f, 1.0f, 5.0f);

// posiciones
float	movAuto_x = 0.0f,
movAuto_z = 0.0f,
orienta = 0.0f;

bool	animacion = false,
		recorrido1 = true,
		recorrido2 = false,
		recorrido3 = false,
		recorrido4 = false;

float tiempo = 0.0f;

//Animaciones peces
float movCola = 0.0f;

//Keyframes (Manipulación y dibujo)
float	posX = 0.0f,
		posY = 0.0f,
		posZ = 0.0f,
		saltoDelfinY = 0.0f,
		saltoDelfinZ = 0.0f;

float	incX = 0.0f,
		incY = 0.0f,
		incZ = 0.0f,
		saltoDelfIncY = 0.0f,
		saltoDelfIncZ = 0.0f;

#define SAVED_FRAMES 0
#define MAX_FRAMES 9  //Se asigna la cantidad max de cuadros clave que puedo guardar
int i_max_steps = 60;  //Determina los cuadros intermedios, determina la velocidad a la que se mueve la animacion
int i_curr_steps = 0;
typedef struct _frame
{
	//Variables para GUARDAR Key Frames
	float posX;		//Variable para PosicionX
	float posY;		//Variable para PosicionY
	float posZ;		//Variable para PosicionZ
	float saltoDelfinY, saltoDelfinZ;
}FRAME;


FRAME KeyFrame[MAX_FRAMES]; //el arreglo contiene la cantidad de elementos guardados por las variables anteriores
int FrameIndex = SAVED_FRAMES;			//Variable que guarda los cuadros clave, si es !0 quiere decir que ya hay guardados
bool play = false;
int playIndex = 0;

void saveFrame(void) //Permite guardar los frames
{
	std::cout << "Frame Index = " << FrameIndex << std::endl;

	KeyFrame[FrameIndex].posX = posX;
	KeyFrame[FrameIndex].posY = posY;
	KeyFrame[FrameIndex].posZ = posZ;

	KeyFrame[FrameIndex].saltoDelfinY = saltoDelfinY;
	KeyFrame[FrameIndex].saltoDelfinZ = saltoDelfinZ;

	FrameIndex++;
}

void resetElements(void)  //Realiza un reseteo cuando se manda a reproducir la animacion
{
	posX = KeyFrame[0].posX;  //El arreglo comienza desde el primer cuadro clave guardado i=0 en el arreglo
	posY = KeyFrame[0].posY;
	posZ = KeyFrame[0].posZ;

	saltoDelfinY = KeyFrame[0].saltoDelfinY;
	saltoDelfinZ = KeyFrame[0].saltoDelfinZ;
}

void interpolation(void) //Incremento = posFinal - posInicial / #cuadrosClave
{
	incX = (KeyFrame[playIndex + 1].posX - KeyFrame[playIndex].posX) / i_max_steps; //PlayIndex determina el cuadro inicial de cada seccion
	incY = (KeyFrame[playIndex + 1].posY - KeyFrame[playIndex].posY) / i_max_steps;
	incZ = (KeyFrame[playIndex + 1].posZ - KeyFrame[playIndex].posZ) / i_max_steps;

	saltoDelfIncY = (KeyFrame[playIndex + 1].saltoDelfinY - KeyFrame[playIndex].saltoDelfinY) / i_max_steps;
	saltoDelfIncZ = (KeyFrame[playIndex + 1].saltoDelfinZ - KeyFrame[playIndex].saltoDelfinZ) / i_max_steps;
}

void animate(void) //Ayuda a animar los objetos de manera automática
{
	movCola = 30 * cos(tiempo);
	tiempo += .3f;

	if (play)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				std::cout << "Animation ended" << std::endl;
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Reset counter
								  //Interpolation
				interpolation();
			}
		}
		else
		{
			//Draw animation
			posX += incX;
			posY += incY;
			posZ += incZ;

			saltoDelfinY += saltoDelfIncY;
			saltoDelfinZ += saltoDelfIncZ;

			i_curr_steps++;
		}
	}

	//if (animacion)
	//{}
}

void getResolution()
{
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	SCR_WIDTH = mode->width;
	SCR_HEIGHT = (mode->height) - 80;
}

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	/*SDL_Init(SDL_INIT_AUDIO);
	//************* audio
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	Mix_Music* music = Mix_LoadMUS("resources/audio/delfin.wav");
	if (!music) {
		cout << "Music error:" << Mix_GetError() << endl;
	}
	//Mix_Chunk* sound = Mix_LoadWAV();
	/*if (!sound) {
		cout << "Sound error:" << Mix_GetError() << endl;
	}

	Mix_PlayMusic(music, -1);*/

	// glfw window creation
	// --------------------
	// --------------------
	monitors = glfwGetPrimaryMonitor();
	getResolution();

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CGeIHC", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(window, 0, 30);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, my_input);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader staticShader("Shaders/shader_Lights.vs", "Shaders/shader_Lights_mod.fs");
	Shader skyboxShader("Shaders/skybox.vs", "Shaders/skybox.fs");
	Shader animShader("Shaders/anim.vs", "Shaders/anim.fs");

	vector<std::string> faces //Se cargan las texturas del fondo
	{
		"resources/skybox/right_2.jpg",
		"resources/skybox/left_2.jpg",
		"resources/skybox/top_2.jpg",
		"resources/skybox/bottom_2.jpg",
		"resources/skybox/front_2.jpg",
		"resources/skybox/back_2.jpg"
	};

	Skybox skybox = Skybox(faces);

	// Shader configuration
	// --------------------
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	//************************* Carga de modelos ***********************************
	Model base("modelos/build/build.obj"); //Escenario

	//******************* Animales ************************
	Model delfin("modelos/delfin/delfin_aro.obj"); //Delfin
	Model orca("modelos/Peces/whale.obj");

	//***************** Decoración ************************
	Model aro("modelos/juegos/aro.obj");
	Model jeep("modelos/auto/Jeep_Renegade_2016_obj/Jeep_Renegade_2016.obj");

	//**************** Animados ***************************
	Model tib("modelos/Shark_/cuerpoTib.obj"); //Tiburon
	Model tibTail("modelos/Shark_/colaTib.obj");

	Model tuna("modelos/Peces/cuerpoTuna.obj"); //Atún
	Model tunaTail("modelos/Peces/colaTuna.obj");
	
	Model medusa("modelos/medusa/medusa.obj");
	//Model brazoDer("modelos/Pinguino/anim/brazoDer.obj");

	ModelAnim pezBoca("modelos/Animados/pezBoca/pezMovBoca.fbx");
	pezBoca.initShaders(animShader.ID);

//*************************** Key Frames ***************************************
	//Inicialización de KeyFrames
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		KeyFrame[i].posX = 0;
		KeyFrame[i].posY = 0;
		KeyFrame[i].posZ = 0;
		KeyFrame[i].saltoDelfinY = 0;
		KeyFrame[i].saltoDelfinZ = 0;
	}

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		skyboxShader.setInt("skybox", 0);
		
		// per-frame time logic
		// --------------------
		lastFrame = SDL_GetTicks();

		// input
		// -----
		animate();

		// render
		// ------
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		staticShader.use();
		//Setup Advanced Lights ----- iluminación direccional (sol)
		staticShader.setVec3("viewPos", camera.Position);
		staticShader.setVec3("dirLight.direction", lightDirection);
		staticShader.setVec3("dirLight.ambient", glm::vec3(0.5f, 0.5f, 0.5f));//Ilumina las caras iluminadas
		staticShader.setVec3("dirLight.diffuse", glm::vec3(0.8f, 0.8f, 0.8f)); //Ilumina las caras más ilumindas
		staticShader.setVec3("dirLight.specular", glm::vec3(0.0f, 0.0f, 0.0f)); //Reflejos de las superficies

		staticShader.setVec3("pointLight[0].position", lightPosition); //Iluminación posicional 
		staticShader.setVec3("pointLight[0].ambient", glm::vec3(0.5f, 0.5f, 0.5f));
		staticShader.setVec3("pointLight[0].diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		staticShader.setVec3("pointLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[0].constant", 0.08f); //Distancia a la que iluminan los rayos 
		staticShader.setFloat("pointLight[0].linear", 0.009f); // ""
		staticShader.setFloat("pointLight[0].quadratic", 0.032f);//potencia luz

		staticShader.setVec3("pointLight[1].position", glm::vec3(-80.0, 3.0f, 0.0f));
		staticShader.setVec3("pointLight[1].ambient", glm::vec3(0.0f, 0.2f, 0.0f));

		staticShader.setVec3("pointLight[1].position", lightPosition2);
		staticShader.setVec3("pointLight[1].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[1].constant", 1.0f);
		staticShader.setFloat("pointLight[1].linear", 0.009f);
		staticShader.setFloat("pointLight[1].quadratic", 0.032f);

		staticShader.setVec3("spotLight[0].position", glm::vec3(0.0f, 20.0f, 10.0f));
		staticShader.setVec3("spotLight[0].direction", glm::vec3(0.0f, -1.0f, 0.0f));
		staticShader.setVec3("spotLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("spotLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("spotLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("spotLight[0].cutOff", glm::cos(glm::radians(10.0f)));
		staticShader.setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(60.0f)));
		staticShader.setFloat("spotLight[0].constant", 1.0f);
		staticShader.setFloat("spotLight[0].linear", 0.009f);
		staticShader.setFloat("spotLight[0].quadratic", 0.0005f);

		staticShader.setFloat("material_shininess", 32.0f);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 tmp = glm::mat4(1.0f);
		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 50000.0f);  //aumentar para la mayor vision
		glm::mat4 view = camera.GetViewMatrix();
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);

		//// Light
		glm::vec3 lightColor = glm::vec3(0.6f);
		glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.75f);
		

		// -------------------------------------------------------------------------------------------------------------------------
		// Personaje Animacion
		// -------------------------------------------------------------------------------------------------------------------------
		//Remember to activate the shader with the animation
		animShader.use();
		animShader.setMat4("projection", projection);
		animShader.setMat4("view", view);
	
		animShader.setVec3("material.specular", glm::vec3(0.5f));
		animShader.setFloat("material.shininess", 32.0f);
		animShader.setVec3("light.ambient", ambientColor);
		animShader.setVec3("light.diffuse", diffuseColor);
		animShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		animShader.setVec3("light.direction", lightDirection);
		animShader.setVec3("viewPos", camera.Position);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-40.3f, 1.75f, 0.3f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.2f));	// it's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		animShader.setMat4("model", model);
		//animacionPersonaje.Draw(animShader);

		// -------------------------------------------------------------------------------------------------------------------------
		// Segundo Personaje Animacion
		// -------------------------------------------------------------------------------------------------------------------------

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-2000.0f, 600.0f, -4400.0f)); // translate it down so it's at the center of the scene
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, glfwGetTime()));
		model = glm::scale(model, glm::vec3(6.0f));	// it's a bit too big for our scene, so scale it down
		animShader.setMat4("model", model);
		//pezBoca.Draw(animShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-3000.0f, 900.0f, -4400.0f)); // translate it down so it's at the center of the scene
		//model = glm::translate(model, glm::vec3(0.0f, 0.0f, glfwGetTime()));
		model = glm::scale(model, glm::vec3(4.0f));	// it's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		animShader.setMat4("model", model);
		//pezBoca.Draw(animShader);

		// -------------------------------------------------------------------------------------------------------------------------
		// Escenario
		// -------------------------------------------------------------------------------------------------------------------------
		staticShader.use();
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		base.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(1500.0f, 600.0f, -2500.0f));
		model = glm::scale(model, glm::vec3(7.0f));
		staticShader.setMat4("model", model);
		aro.Draw(staticShader);

		//Jeep
		model = glm::mat4(1.0f);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(2300.0f, 0.0f, 5500.0f));
		model = glm::scale(model, glm::vec3(120.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		staticShader.setMat4("model", model);
		jeep.Draw(staticShader);

		// -------------------------------------------------------------------------------------------------------------------------
		// Animales
		// -------------------------------------------------------------------------------------------------------------------------

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-7000.0f, 50.0f, 2000.0f));
		model = glm::scale(model, glm::vec3(45.0f));
		staticShader.setMat4("model", model);
		orca.Draw(staticShader);

		// -------------------------------------------------------------------------------------------------------------------------
		// Animales animados
		// -------------------------------------------------------------------------------------------------------------------------

		//Cuerpo tiburon
		tmp = model = glm::translate(glm::mat4(1.0f), glm::vec3(-4000.0f, 100.0f, 1200.0f));
		model = glm::scale(model, glm::vec3(8.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		staticShader.setMat4("model", model);
		tib.Draw(staticShader);
		//Cola tiburon
		model = glm::translate(tmp, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(movCola), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(8.0f));
		staticShader.setMat4("model", model);
		tibTail.Draw(staticShader);

		//Medusa
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-1800.0f, 500.0f, -4500.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(90.0f));
		staticShader.setMat4("model", model);
		medusa.Draw(staticShader);

		//Atun
		tmp = model = glm::translate(glm::mat4(1.0f), glm::vec3(-1100.0f, 600.0f, -2800.0f));
		model = glm::scale(model, glm::vec3(70.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		staticShader.setMat4("model", model);
		tuna.Draw(staticShader);
		//COLA atun
		model = glm::translate(tmp, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(movCola), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(90.0f));
		staticShader.setMat4("model", model);
		tunaTail.Draw(staticShader);

		// -------------------------------------------------------------------------------------------------------------------------
		// Animados KeyFrames
		// -------------------------------------------------------------------------------------------------------------------------
			//DELFIN
			model = glm::mat4(1.0f);
			tmp = model = glm::translate(model, glm::vec3(1500.0f, -100.0f, -500.0f));
			model = glm::translate(tmp, glm::vec3(0.0f, saltoDelfinY, saltoDelfinZ));
			model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(8.0f));
			staticShader.setMat4("model", model);
			delfin.Draw(staticShader);

		// -------------------------------------------------------------------------------------------------------------------------
		// Termina Escenario
		// -------------------------------------------------------------------------------------------------------------------------

		//-------------------------------------------------------------------------------------
		// draw skybox as last
		// ------------------------------------------------------------------------------------
		skyboxShader.use();
		skybox.Draw(skyboxShader, view, projection, camera);

		// Limitar el framerate a 60
		deltaTime = SDL_GetTicks() - lastFrame; // time for full 1 loop
		//std::cout <<"frame time = " << frameTime << " milli sec"<< std::endl;
		if (deltaTime < LOOP_TIME)
		{
			SDL_Delay((int)(LOOP_TIME - deltaTime));
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	skybox.Terminate();

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void my_input(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, (float)deltaTime);
	//To Configure Model
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
		saltoDelfinY += 50.0f;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		saltoDelfinY -= 50.0f;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		saltoDelfinZ += 50.0f;
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		saltoDelfinZ -= 50.0f;
	//Mix_PlayChannel(-1, sound, 0);


	//Car animation
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		animacion ^= true; //XOR, asigna el contrario de lo que tiene asignado

	//To play KeyFrame animation 
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		if (play == false && (FrameIndex > 1))
		{
			std::cout << "Play animation" << std::endl;
			resetElements();
			//First Interpolation				
			interpolation();

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
			std::cout << "Not enough Key Frames" << std::endl;
		}
	}

	//To Save a KeyFrame
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		if (FrameIndex < MAX_FRAMES)
		{
			saveFrame();
		}
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}