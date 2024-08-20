///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
//  MODIFIED BY: Michael Lorenz - SNHU Student
// Added additional functionality for camera controls and scene interactivity,
// Jul. 23rd, 2024
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f; 
	float gLastFrame = 0.0f;

	// the following variable is false when orthographic projection
	// is off and true when it is on
	bool bOrthographicProjection = false;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager *pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	// default camera view parameters
	g_pCamera->Position = glm::vec3(2.0f, 5.5f, 9.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.9f, -4.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 60;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}

	// set created window as main GLFW window for openGL
	glfwMakeContextCurrent(window);

	// this callback is used to receive window sizing events
	//glfwSetFramebufferSizeCallback(window, &ViewManager::Window_Resize_Callback);

	// tell GLFW to capture all mouse events
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// this callback is used to receive mouse moving events calling Mouse_Position_Callback()
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	// this callback is used to receive mouse wheel scrolling events within the window in Mouse_Scroll_Wheel_Callback()
	glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Wheel_Callback);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 *  Processes mouse movement on camera.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	// adjust to allow for more sensitive camera movement
	float mouseSensitivity = 2.50f;

	// Record first move event to calculate subsequent mouse movement offsets
	if (gFirstMouse)
	{
		gLastX = xMousePos;
		gLastY = yMousePos;
		gFirstMouse = false;
	}

	// calculate X & Y offset values used to move 3D camera
	float xOffset = xMousePos - gLastX;
	float yOffset = gLastY - yMousePos; //reversed because y increases from bottom to top

	// set current positions into last position variables
	gLastX = xMousePos;
	gLastY = yMousePos;

	// move 3D camera using calculated offsets
	g_pCamera->ProcessMouseMovement(xOffset*mouseSensitivity, yOffset*mouseSensitivity);
}

/***********************************************************
 *  Mouse_Scroll_Wheel_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse scroll wheel is used. Controls camera movement
 *  speed.
 ***********************************************************/
void ViewManager::Mouse_Scroll_Wheel_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
	// Change movement speed of camera accordingly to yOffset (vertical scroll)
	// Scroll down = decrease speed, Scroll up = increase speed
	g_pCamera->ProcessMouseScroll(-yOffset);
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue. Includes camera
 *  view and movement controls.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// if the camera object is null, then exit this method
	if (NULL == g_pCamera)
	{
		return;
	}

	/*process camera moving forward and backward*/
	// Move Forward:
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
	}
	// Move Backward:
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
	}

	/* process camera panning left and right*/
	// Pan Left:
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
	}
	// Pan Right:
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
	}

	/*process camera panning upward and downward*/
	// Pan Up:
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(UP, gDeltaTime);
	}
	// Pan Down:
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);
	}

	/* process camera zooming in and out*/
	// Zoom in:
	if (glfwGetKey(m_pWindow, GLFW_KEY_UP) == GLFW_PRESS)
	{
		if (g_pCamera->Zoom >= 10) { // Maximum zoom = minimum of 10
			g_pCamera->Zoom -= 0.01;
		}
	}
	// Zoom out:
	if (glfwGetKey(m_pWindow, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		if (g_pCamera->Zoom <= 160) { // Maximum field of view of 160
			g_pCamera->Zoom += 0.01;
		}
	}

	/*Change view of the scene*/
	// Orthographic (2D) view
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
	{
		// changes projection matrix in PrepareSceneView()
		bOrthographicProjection = true;
		// Set camera settings for a front view, perpendicular to the horizontal plane
		g_pCamera->Position = glm::vec3(0.0f, 2.0f, 10.0f); // Reset camera position along z axis in front of object
		g_pCamera->Front = glm::vec3(0.0f, 0.0f, -1.0f); // Look at origin
		g_pCamera->Up = glm::vec3(0.0f, 5.0f, 0.0f); // correct camera orientation
	}
	// Perspective (3D) view
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
	{
		// changes projection matrix in PrepareSceneView()
		bOrthographicProjection = false;
	}
}

/***********************************************************
 *  Window_Resize_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the framebuffer for the active GLFW window is resized
 ***********************************************************/
void ViewManager::Window_Resize_Callback(GLFWwindow* window, double width, double height)
{
	//glViewport(0, 0, width, height);
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// process any keyboard events that may be waiting in the 
	// event queue
	ProcessKeyboardEvents();

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();

	// define the current projection matrix if bOrthographicProjection is enabled (modified in ProcessKeyboardEvents()
	if (bOrthographicProjection) // Orthographic(2D) View
	{
		// apply an orthographic matrix transformation to the projection matrix
		projection = glm::ortho(-9.0f, 9.0f, -7.0f, 7.0f, -1.0f, 30.0f);
	}
	else // Perspective(3D) view
	{
		projection = glm::perspective(glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}

	// if the shader manager object is valid
	if (NULL != m_pShaderManager)
	{
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		// set the view position of the camera into the shader for proper rendering
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}