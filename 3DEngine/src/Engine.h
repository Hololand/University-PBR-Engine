#pragma once
#include "Camera.h"
#include "libs.h"

// Code for debugging GL errors.
void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

// Window size
void framebuffer_resize_callback(GLFWwindow* window, int fbW, int fbH)
{
	glViewport(0, 0, fbW, fbH);
};

// Enums for easy tracking of multiple shaders, texture, materials etc...
enum shader_enum{SHADER_CORE_PROGRAM = 0, SHADER_CORE_BLINN, SHADER_EQUIRECTANGULAR_TO_CUBEMAP, SHADER_IRRADIANCE, SHADER_REFLECTION, SHADER_BRDFLUT, SHADER_SKYBOX};
enum texture_enum{TEX_CURRENT_A_PBR = 0, TEX_CURRENT_M_PBR, TEX_CURRENT_R_PBR, TEX_CURRENT_N_PBR};
enum material_enum {MATERIAL_1 = 0};
enum mesh_enum {MESH_QUAD = 0};

class Engine
{
public:
	Engine(const char* title, const int width, const int height, bool resizable)
		: windowWidth(width), windowHeight(height), camera(glm::vec3(0.0f, 1.0f, 4.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f))
	{
		this->window = nullptr;
		this->frameBufferWidth = this->windowWidth;
		this->frameBufferHeight = this->windowHeight;

		// View Matrix
		this->camPosition = glm::vec3(0.0f, 0.0f, 1.5f);
		this->worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		this->camFront = glm::vec3(0.0f, 0.0f, -1.0f);

		// Projection Matrix
		this->fov = 90.0f;
		this->nearPlane = 0.1f;
		this->farPlane = 1000.0f;

		this->dt = 0.0f;
		this->curTime = 0.0f;
		this->lastTime = 0.0f;

		this->lastMouseX = 0.0f;
		this->lastMouseY = 0.0f;
		this->mouseX = 0.0f;
		this->mouseY = 0.0f;
		this->mouseOffsetX = 0.0f;
		this->mouseOffsetY = 0.0f;
		this->firstMouse = true;


		// Functions to initialise key components of the engine
		/////////////////////////////////////////////////////////////////
		// 
		// Initialise GLFW and window
		this->initGLFW();
		this->initWindow(title, resizable);

		// Initialise GLEW with valid GLFW context
		this->initGLEW();

		// Stuff for debugging
		int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(glDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}

		// Configure OpenGL
		this->initOpenGLOptions();

		// Initialise necessary data for rendering
		this->initMatrices();
		this->initShaders();
		this->initTextures();
		this->initIBL("Assets/environment.hdr");
		this->initMaterials();
		this->initModel("Assets/model.obj");
		this->initLights();
		this->initUniforms();

		// Initialise GUI
		this->initImGUI();

	}

	~Engine()
	{
		// Destroy GLFW window
		glfwDestroyWindow(this->window);
		glfwTerminate();
		// Delete all data
		for (size_t i = 0; i < this->shaders.size(); i++)
		{
			delete this->shaders[i];
		}
		for (size_t i = 0; i < this->textures.size(); i++)
		{
			delete this->textures[i];
		}
		for (size_t i = 0; i < this->materials.size(); i++)
		{
			delete this->materials[i];
		}
		for (auto*& i : this->models)
		{
			delete i;
		}
		for (size_t i = 0; i < this->pointLights.size(); i++)
		{
			delete this->pointLights[i];
		}
		// Destroy ImGUI context
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	/////////////////////////////////////////////////
	/// UPDATE FUNCTIONS CALLED ONCE PER FRAME //////
	/////////////////////////////////////////////////

	// Get mouse and keyboard input using GLFW event handler
	void updateInput()
	{
		glfwPollEvents();
		this->updateKeyInput();
		this->updateMouseInput();
		this->camera.updateInput(dt, -1, this->mouseOffsetX, this->mouseOffsetY);
	}
	void updateKeyInput()
	{
		if (glfwGetKey(this->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			setWindowShouldClose();
		}
		if (glfwGetKey(this->window, GLFW_KEY_W) == GLFW_PRESS)
		{
			this->camera.move(this->dt, FORWARD);
		}
		if (glfwGetKey(this->window, GLFW_KEY_A) == GLFW_PRESS)
		{
			this->camera.move(this->dt, LEFT);
		}
		if (glfwGetKey(this->window, GLFW_KEY_S) == GLFW_PRESS)
		{
			this->camera.move(this->dt, BACKWARD);
		}
		if (glfwGetKey(this->window, GLFW_KEY_D) == GLFW_PRESS)
		{
			this->camera.move(this->dt, RIGHT);
		}
		if (glfwGetKey(this->window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			this->camera.move(this->dt, UP);
		}
		if (glfwGetKey(this->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			this->camera.move(this->dt, DOWN);
		}
	}
	void updateMouseInput()
	{
		// If the mouse is over any of the GUI windows do not send updates to the camera
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) == false && ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) == false)
		{
			glfwGetCursorPos(this->window, &this->mouseX, &this->mouseY);

			if (this->firstMouse)
			{
				this->lastMouseX = this->mouseX;
				this->lastMouseY = this->mouseY;
				this->firstMouse = false;
			}
			// If LMB pressed then send position changes to camera
			if (glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
				this->mouseOffsetX = this->mouseX - this->lastMouseX;
				this->mouseOffsetY = this->lastMouseY - this->mouseY;
			}
			else
			{
				// Else no change
				this->mouseOffsetX = 0;
				this->mouseOffsetY = 0;
			}
			this->lastMouseX = this->mouseX;
			this->lastMouseY = this->mouseY;
			// If RMB then set light position to camera position
			if (glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
				this->pointLights[0]->setPosition(this->camera.getPosition());
			}
		}
		else
		{
			this->firstMouse = true;
		}
	}

	// Update delta time and input
	void update()
	{
		this->updateDt();
		this->updateInput();
	}
	// Render to screen
	void render()
	{
		// Create GUI frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Clear GL
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Update uniforms
		this->updateUniforms();
		// Render models
		for (auto& i : this->models)
		{
			//i->render(this->shaders[SHADER_CORE_PROGRAM]);
			i->renderPBR(this->shaders[SHADER_CORE_PROGRAM]);
		}
		
		// Render Skybox
		shaders[SHADER_SKYBOX]->use();
		renderCube();

		// Render GUI
		renderGUI();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);
		//glFlush();

		// Set default GL state
		glBindVertexArray(0);
		glUseProgram(0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Renders the GUI
	void renderGUI()
	{
		{
			// Transformations
			static float Scale = 1.0f;
			static float TranX = 0.0f;
			static float TranY = 0.0f;
			static float TranZ = 0.0f;
			static float RotX = 0.0f;
			static float RotY = 0.0f;
			static float RotZ = 0.0f;
			// Light
			static ImVec4 lightColour = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			static float Intensity = 5.0f;
			// SCENE SETTINGS WINDOW
			ImGui::Begin("Scene Settings");                       
			ImGui::Text("Object Transformations");               
			ImGui::SliderFloat("Scale", &Scale, 0.01f, 10.0f);
			ImGui::SliderFloat("Translate X", &TranX, -10.0f, 10.0f);
			ImGui::SliderFloat("Translate Y", &TranY, -10.0f, 10.0f); 
			ImGui::SliderFloat("Translate Z", &TranZ, -10.0f, 10.0f);    
			ImGui::SliderFloat("Rotate X", &RotX, -180.0f, 180.0f);    
			ImGui::SliderFloat("Rotate Y", &RotY, -180.0f, 180.0f);    
			ImGui::SliderFloat("Rotate Z", &RotZ, -180.0f, 180.0f);
			ImGui::Text("Light Settings");
			ImGui::ColorEdit3("Colour", (float*)&lightColour);
			ImGui::SliderFloat("Intensity", &Intensity, 0.0f, 50.0f);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();

			// CAMERA SETTINGS WINDOW
			static float movementSpeed = 3.0f;
			static float sensitivity = 25.0f;

			ImGui::Begin("Camera settings");
			ImGui::Text("Settings");
			ImGui::SliderFloat("Move Speed", &movementSpeed, 0.0f, 30.0f);
			ImGui::SliderFloat("Sensitivity", &sensitivity, 0.0f, 50.0f);
			ImGui::Text("Controls");
			ImGui::BeginTable("Controls", 2);
			ImGui::TableSetupColumn("Action");
			ImGui::TableSetupColumn("Key");
			ImGui::TableHeadersRow();
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted("Forwards");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted("W");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted("Backwards");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted("S");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted("Right");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted("D");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted("Left");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted("A");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted("Up");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted("SPACE");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted("Down");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted("LSHIFT");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted("Pan Camera");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted("LMB + Drag");
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted("Set Light Position");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted("RMB");
			ImGui::TableNextRow();
			ImGui::EndTable();

			// Updating model transformations with values from GUI
			this->models[0]->scale(glm::vec3(Scale));
			this->models[0]->rotate(glm::vec3(RotX, RotY, RotZ));
			this->models[0]->translate(glm::vec3(TranX, TranY, TranZ));
			// Updating light settings with values from GUI
			this->pointLights[0]->setColour(glm::vec3(lightColour.x, lightColour.y, lightColour.z));
			this->pointLights[0]->setIntensity(Intensity);
			// Updating camera settings with values from GUI
			this->camera.setMoveSpeed(movementSpeed);
			this->camera.setSens(sensitivity);
		}
		// Render GUI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

private:
	// window variables (width, height)
	GLFWwindow* window;
	const int windowWidth;
	const int windowHeight;
	int frameBufferWidth;
	int frameBufferHeight;

	// camera variables
	glm::mat4 viewMatrix;
	glm::vec3 camPosition;
	glm::vec3 worldUp;
	glm::vec3 camFront;
	glm::mat4 projectionMatrix;

	float fov;
	float nearPlane;
	float farPlane;

	// variables for storing and calculating delta time
	float dt;
	float curTime;
	float lastTime;

	// variables for updating camera rotation
	double lastMouseX;
	double lastMouseY;
	double mouseX;
	double mouseY;
	double mouseOffsetX;
	double mouseOffsetY;
	bool firstMouse;

	//IMGUI stuff
	bool show_demo_window;
	bool show_another_window;


	Camera camera;
	
	//Shaders
	std::vector<Shader*> shaders;
	//Textures
	std::vector<Texture*> textures;
	//Materials
	std::vector<Material*> materials;
	//Models
	std::vector<Model*> models;
	//Lights
	std::vector<PointLight*> pointLights;


	// Function for rendering BRDFLUT
	unsigned int quadVAO = 0;
	unsigned int quadVBO;
	void renderQuad()
	{
		if (quadVAO == 0)
		{
			float quadVertices[] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
			// setup plane VAO
			glGenVertexArrays(1, &quadVAO);
			glGenBuffers(1, &quadVBO);
			glBindVertexArray(quadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		}
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}
	// Function for rendering to Cubemap
	unsigned int cubeVAO = 0;
	unsigned int cubeVBO = 0;
	void renderCube()
	{
		if (cubeVAO == 0)
		{
			float vertices[] = {
				// back face
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
				// front face
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				// left face
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				// right face
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
				// bottom face
				-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
				 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				// top face
				-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
				 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
			};
			glGenVertexArrays(1, &cubeVAO);
			glGenBuffers(1, &cubeVBO);
			// fill buffer
			glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			// link vertex attributes
			glBindVertexArray(cubeVAO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
		// render Cube
		glFrontFace(GL_CW);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glFrontFace(GL_CCW);
	}

	void initGLFW()
	{
		if (glfwInit() == GLFW_FALSE)
		{
			std::cout << "Error: GLFW init failed" << std::endl;
			glfwTerminate();
		}
	}

	void initGLEW()
	{
		glewExperimental = GL_TRUE;

		if (glewInit() != GLEW_OK)
		{
			std::cout << "ERROR: glewInit failed" << std::endl;
			glfwTerminate();
		}
	}


	void initImGUI()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 440");
		bool show_demo_window = true;
		bool show_another_window = false;
	}


	void initWindow(const char* title, bool resizable)
	{
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
		glfwWindowHint(GLFW_RESIZABLE, resizable);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

		this->window = glfwCreateWindow(this->windowWidth, this->windowHeight, title, NULL, NULL);

		if (this->window == nullptr)
		{
			std::cout << "Error: GLFW init failed" << std::endl;
			glfwTerminate();
		}

		glfwGetFramebufferSize(this->window, &this->frameBufferWidth, &this->frameBufferHeight);
		glfwSetFramebufferSizeCallback(this->window, framebuffer_resize_callback);

		/* Make the window's context current */
		glfwMakeContextCurrent(window);
	}

	void initOpenGLOptions()
	{
		glEnable(GL_DEPTH_TEST); // for use of Z coordinate etc..
		glDepthFunc(GL_LEQUAL); // for skybox to render properly
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // seamless cubemap sampling for lower mip levels in prefilter map
		glEnable(GL_CULL_FACE); // for culling unecessary faces
		glCullFace(GL_BACK); // triangles facing away from camera
		glFrontFace(GL_CCW); // counter clockwise = forwards face
		glEnable(GL_BLEND); // for colour blending
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Fix for wierd blending issue interpreting BRDFLUT r.g channels as alpha 1

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // default fill polygon with colour

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	}

	void initMatrices()
	{
		//View Matrix
		this->viewMatrix = glm::mat4(1.0f);
		viewMatrix = glm::lookAt(this->camPosition, this->camPosition + this->camFront, this->worldUp);

		// Proj Matrix
		this->projectionMatrix = glm::mat4(1.0f);
		this->projectionMatrix = glm::perspective(glm::radians(fov), static_cast<float>(frameBufferWidth) / frameBufferHeight, this->nearPlane, this->farPlane);
	}

	void initShaders()
	{
		this->shaders.push_back(new Shader("src\\VertexCorePBR.glsl", "src\\FragmentCorePBR.glsl")); // PBR
		this->shaders.push_back(new Shader("src\\VertexCore.glsl", "src\\FragmentCore.glsl"));	// BlinnPhong
		this->shaders.push_back(new Shader("src\\CubeMapVS.glsl", "src\\CubeMapFS.glsl"));	// EquirectangularToCubemap (IBL stuff)
		this->shaders.push_back(new Shader("src\\CubeMapVS.glsl", "src\\IrradianceConvolutionFS.glsl")); // Irradiance (IBL stuff)
		this->shaders.push_back(new Shader("src\\CubeMapVS.glsl", "src\\CubeMapPrefilterFS.glsl")); // Prefiltered Map (IBL stuff)
		this->shaders.push_back(new Shader("src\\brdfLUTVS.glsl", "src\\brdfLUTFS.glsl")); // brdfLUT shaders (IBL STUFF)
		this->shaders.push_back(new Shader("src\\skyboxVS.glsl", "src\\skyboxFS.glsl")); // Skybox (optional)
	}

	void initIBL(const char* fileName)
	{
		// Set up Frame buffer
		unsigned int cubeFBO;
		unsigned int cubeRBO;
		glGenFramebuffers(1, &cubeFBO);
		glGenRenderbuffers(1, &cubeRBO);

		glBindFramebuffer(GL_FRAMEBUFFER, cubeFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, cubeRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, cubeRBO);

		int width, height, nrComponents;

		// Load HDR image from file
		stbi_set_flip_vertically_on_load(true);
		float* image = stbi_loadf(fileName, &width, &height, &nrComponents, 0);

		unsigned int hdrTexture;
		if (image)
		{
			glGenTextures(1, &hdrTexture);
			glBindTexture(GL_TEXTURE_2D, hdrTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, image);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			stbi_image_free(image);
		}
		else
		{
			std::cout << "ERROR: Failed to load texture" << fileName << std::endl;
		}

		// Create Cubemap texture
		unsigned int envCubeMap;
		glGenTextures(1, &envCubeMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Projection and view matrix for cubemap faces
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		// Render HDR to Cubemap
		this->shaders[SHADER_EQUIRECTANGULAR_TO_CUBEMAP]->use();
		this->shaders[SHADER_EQUIRECTANGULAR_TO_CUBEMAP]->set1i(0, "equirectangularMap");
		this->shaders[SHADER_EQUIRECTANGULAR_TO_CUBEMAP]->setMat4fv(captureProjection, "projection");
		this->shaders[SHADER_EQUIRECTANGULAR_TO_CUBEMAP]->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glViewport(0, 0, 512, 512);
		glBindFramebuffer(GL_FRAMEBUFFER, cubeFBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			this->shaders[SHADER_EQUIRECTANGULAR_TO_CUBEMAP]->setMat4fv(captureViews[i], "view");
			this->shaders[SHADER_EQUIRECTANGULAR_TO_CUBEMAP]->use();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderCube(); // renders a 1x1x1 cube
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// Create texture for convoluted map
		unsigned int irradianceMap;
		glGenTextures(1, &irradianceMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, cubeFBO);

		// Convolute to create irradiance cubemap
		this->shaders[SHADER_IRRADIANCE]->set1i(0, "environmentMap");
		this->shaders[SHADER_IRRADIANCE]->setMat4fv(captureProjection, "projection");
		this->shaders[SHADER_IRRADIANCE]->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
		glViewport(0, 0, 32, 32);
		glBindFramebuffer(GL_FRAMEBUFFER, cubeFBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			this->shaders[SHADER_IRRADIANCE]->setMat4fv(captureViews[i], "view");
			this->shaders[SHADER_IRRADIANCE]->use();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// Create texture for pre filtered cubemap, rescale capture FBO to pre filter scale
		unsigned int prefilterMap;
		glGenTextures(1, &prefilterMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		this->shaders[SHADER_REFLECTION]->set1i(0, "environmentMap");
		this->shaders[SHADER_REFLECTION]->setMat4fv(captureProjection, "projection");
		this->shaders[SHADER_REFLECTION]->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
		glBindFramebuffer(GL_FRAMEBUFFER, cubeFBO);
		unsigned int maxMipLevels = 5;
		// For each each face of the cube map and each mip level render prefiltered cubemap
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			unsigned int mipWidth = 128 * std::pow(0.5, mip);
			unsigned int mipHeight = 128 * std::pow(0.5, mip);
			glBindRenderbuffer(GL_RENDERBUFFER, cubeRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			this->shaders[SHADER_REFLECTION]->set1f(roughness, "roughness");
			this->shaders[SHADER_REFLECTION]->use();
			for (unsigned int i = 0; i < 6; ++i)
			{
				this->shaders[SHADER_REFLECTION]->setMat4fv(captureViews[i], "view");
				this->shaders[SHADER_REFLECTION]->use();
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				renderCube();
			}
		}

		// Create BRDFLUT texture
		unsigned int brdfLUTTexture;
		glGenTextures(1, &brdfLUTTexture);
		GLint swizzleMask[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ONE };
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteriv(brdfLUTTexture, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

		glBindFramebuffer(GL_FRAMEBUFFER, cubeFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, cubeRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);
		// Render BRDFLUT texture
		glViewport(0, 0, 512, 512);
		this->shaders[SHADER_BRDFLUT]->use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBlendFunc(GL_ONE, GL_ONE);
		renderQuad();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Reset viewport
		glfwGetFramebufferSize(this->window, &this->frameBufferWidth, &this->frameBufferHeight);
		glViewport(0, 0, frameBufferWidth, frameBufferHeight);

		/////////////////////////////////////////////////
		// BIND AND SET UNIFORMS FOR PBR SHADER /////////
		/////////////////////////////////////////////////
		
		// Bind and set irradiancemap uniform
		this->shaders[SHADER_CORE_PROGRAM]->use();
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		this->shaders[SHADER_CORE_PROGRAM]->set1iUI(8, "irradianceMap");
		// Bind and set prefiltermap uniform
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		this->shaders[SHADER_CORE_PROGRAM]->set1iUI(6, "prefilterMap");

		// Bind and set BRDFLUT uniform
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
		this->shaders[SHADER_CORE_PROGRAM]->set1iUI(5, "brdfLUT");

		//Bind texture and set uniform for skybox shader
		this->shaders[SHADER_SKYBOX]->use();
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
		this->shaders[SHADER_SKYBOX]->set1iUI(7, "environmentMap");
	}
	// Load textures
	void initTextures()
	{
		this->textures.push_back(new Texture("Assets/albedo.png"));
		this->textures.push_back(new Texture("Assets/metal.png"));
		this->textures.push_back(new Texture("Assets/rough.png"));
		this->textures.push_back(new Texture("Assets/normal.png"));
	}
	// Create a material
	void initMaterials()
	{
		this->materials.clear();
		this->materials.push_back(new Material(glm::vec3(0.03f), 0, 1, 2, 3));
	}
	// Load model with above material and textures
	void initModel(const char *filePath)
	{
		this->models.push_back(new Model(glm::vec3(0.0f, 0.0f, 0.0f), this->materials[0], this->textures[TEX_CURRENT_A_PBR], this->textures[TEX_CURRENT_M_PBR], this->textures[TEX_CURRENT_R_PBR], this->textures[TEX_CURRENT_N_PBR], filePath));
	}
	// Create lights
	void initLights()
	{
		this->initPointLights();
	}
	// Create point light and set default position
	void initPointLights()
	{
		this->pointLights.push_back(new PointLight(glm::vec3(0.0f), 5.0f));
		this->pointLights[0]->setPosition(glm::vec3(2.5f, 2.5f, 0.0f));
	}
	// Set View matrix, projection matrix and light uniforms.
	void initUniforms()
	{
		this->shaders[SHADER_CORE_PROGRAM]->setMat4fv(viewMatrix, "ViewMatrix");
		this->shaders[SHADER_CORE_PROGRAM]->setMat4fv(projectionMatrix, "ProjectionMatrix");
		for (PointLight* pl : this->pointLights)
		{
			pl->sendToShader(*this->shaders[SHADER_CORE_PROGRAM]);
		}
	}
	// Update above uniforms each frame
	void updateUniforms()
	{
		this->viewMatrix = this->camera.getViewMatix();;

		this->shaders[SHADER_CORE_PROGRAM]->setMat4fv(this->viewMatrix, "ViewMatrix");
		this->shaders[SHADER_CORE_PROGRAM]->setVec3f(this->camera.getPosition(), "cameraPos");
		for (PointLight* pl : this->pointLights)
		{
			pl->sendToShader(*this->shaders[SHADER_CORE_PROGRAM]);
		}

		glfwGetFramebufferSize(this->window, &this->frameBufferWidth, &this->frameBufferHeight);
		
		projectionMatrix = glm::perspective(glm::radians(fov), static_cast<float>(frameBufferWidth) / frameBufferHeight, nearPlane, farPlane);
		this->shaders[SHADER_CORE_PROGRAM]->setMat4fv(projectionMatrix, "ProjectionMatrix");
		this->shaders[SHADER_SKYBOX]->setMat4fv(viewMatrix, "view");
		this->shaders[SHADER_SKYBOX]->setMat4fv(projectionMatrix, "projection");
	}

public:
	//Update deltatime
	void updateDt()
	{
		this->curTime = static_cast<float>(glfwGetTime());
		this->dt = this->curTime - this->lastTime;
		this->lastTime = this->curTime;
	}

	//getters setters
	int getWindowShouldClose()
	{
		return(glfwWindowShouldClose(this->window));
	}

	void setWindowShouldClose()
	{
		glfwSetWindowShouldClose(this->window, GLFW_TRUE);
	}

};
