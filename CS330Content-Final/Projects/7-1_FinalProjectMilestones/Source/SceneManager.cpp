///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
//  MODIFIED BY: Michael Lorenz
// Added a method for drawing a complicated jar object, Jul. 17th, 2024
// Added a utility method for drawing an object with all transformation parameters,
// Jul. 23rd, 2024
// Added utility method for setting texture, color, material shader attributes
// Aug. 4th, 2024
// Completed 3D scene with necessary objects, lighting, textures, and materials.
// Aug. 16th 2024
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}


/*******************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for       ***/
/*** preparing and rendering their own 3D replicated scenes.     ***/
/*** Please refer to the code in the OpenGL sample project       ***/
/*** for assistance.                                             ***/
/********************************************************************/

/***********************************************************
*  LoadSceneTextures() 
* 
*  This method is used for preparing the 3D scene by loading 
*  the shapes, textures in memory to support the 3D scene 
*  rendering 
***********************************************************/ 
void SceneManager::LoadSceneTextures() 
{
	bool bReturn = false;
	// load each texture into memory with CreateGLTexture()
	bReturn = CreateGLTexture("../../Utilities/textures/tile.jpg", "backdrop"); // Used for backdrop wall
	bReturn = CreateGLTexture("../../Utilities/textures/counter.jpg", "counter"); // Used for counter surface
	bReturn = CreateGLTexture("../../Utilities/textures/knife_handle.jpg", "wood"); // Used for knife handle and backdrop ledge
	bReturn = CreateGLTexture("../../Utilities/textures/metal.jpg", "metal"); // Used for knife and cup metal
	bReturn = CreateGLTexture("../../Utilities/textures/marble.jpg", "marble"); // Used for coaster
	bReturn = CreateGLTexture("../../Utilities/textures/drywall.jpg", "plastic"); // Used for cutting board
	bReturn = CreateGLTexture("../../Utilities/textures/cucumber_outer.jpeg", "cucumber_outer"); // Used for cucumber sides
	bReturn = CreateGLTexture("../../Utilities/textures/cucumber_inner.jpg", "cucumber_inner"); // Used for cucumber interior
	bReturn = CreateGLTexture("../../Utilities/textures/glass10.png", "glass10"); // Used for glass jar
	bReturn = CreateGLTexture("../../Utilities/textures/glass13.png", "glass13");

	// bind loaded textures to texture slots
	BindGLTextures();
}

/***********************************************************
*  DefineObjectMaterials()
*
*  This method configures various types of materials which
*  can be set for each object with SetShaderMaterial(tag).
***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	/*Create glass material(used for complex glass jar)*/
	OBJECT_MATERIAL glassMaterial;
	// set attributes
	glassMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	glassMaterial.ambientStrength = 0.15f;
	glassMaterial.diffuseColor = glm::vec3(0.32f, 0.32f, 0.3f);
	glassMaterial.specularColor = glm::vec3(0.6f, 0.6f, 0.6f);
	glassMaterial.shininess = 75.0f;
	glassMaterial.tag = "glass"; // used to identify material
	// load "glass" as a material
	m_objectMaterials.push_back(glassMaterial);

	/* Create wood material (used for wood countertop/knife handle) */
	OBJECT_MATERIAL woodMaterial;
	// set attributes
	woodMaterial.ambientColor = glm::vec3(0.25f, 0.22f, 0.2f);
	woodMaterial.ambientStrength = 0.2f;
	woodMaterial.diffuseColor = glm::vec3(0.25f, 0.2f, 0.15f);
	woodMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	woodMaterial.shininess = 5.0f;
	woodMaterial.tag = "wood"; // used to identify material
	// load "wood" as a material
	m_objectMaterials.push_back(woodMaterial);

	/*Create plastic material(used for cup and cutting board)*/
	OBJECT_MATERIAL plasticMaterial;
	// set attributes
	plasticMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.23f);
	plasticMaterial.ambientStrength = 0.15f;
	plasticMaterial.diffuseColor = glm::vec3(0.25f, 0.255f, 0.28f);
	plasticMaterial.specularColor = glm::vec3(0.32f, 0.32f, 0.3f);
	plasticMaterial.shininess = 7.0f;
	plasticMaterial.tag = "plastic"; // used to identify material
	// load "plastic" as a material
	m_objectMaterials.push_back(plasticMaterial);

	/*Create polished stone material(used for countertop, background wall and coaster)*/
	OBJECT_MATERIAL stoneMaterial;
	// set attributes
	stoneMaterial.ambientColor = glm::vec3(0.39f, 0.37f, 0.35f);
	stoneMaterial.ambientStrength = 0.25f;
	stoneMaterial.diffuseColor = glm::vec3(0.4f, 0.37f, 0.35f);
	stoneMaterial.specularColor = glm::vec3(0.27f, 0.3f, 0.33f);
	stoneMaterial.shininess = 2.0f;
	stoneMaterial.tag = "stone"; // used to identify material
	// load "stone" as a material
	m_objectMaterials.push_back(stoneMaterial);

	/*Create metal material(used for cup lip/straw and knife )*/
	OBJECT_MATERIAL metalMaterial;
	// set attributes
	metalMaterial.ambientColor = glm::vec3(0.23f, 0.23f, 0.21f);
	metalMaterial.ambientStrength = 0.4f;
	metalMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.25f);
	metalMaterial.specularColor = glm::vec3(0.45f, 0.45f, 0.45f);
	metalMaterial.shininess = 25.0f;
	metalMaterial.tag = "metal"; // used to identify material
	// load "metal" as a material
	m_objectMaterials.push_back(metalMaterial);

	/*Create organic material(used for cucumber)*/
	OBJECT_MATERIAL organicMaterial;
	// set attributes
	organicMaterial.ambientColor = glm::vec3(0.25f, 0.28f, 0.25f);
	organicMaterial.ambientStrength = 0.15f;
	organicMaterial.diffuseColor = glm::vec3(0.3f, 0.34f, 0.3f);
	organicMaterial.specularColor = glm::vec3(0.35f, 0.35f, 0.3f);
	organicMaterial.shininess = 12.0f;
	organicMaterial.tag = "organic"; // used to identify material
	// load "organic" as a material
	m_objectMaterials.push_back(organicMaterial);
}

/***********************************************************
*  SetupSceneLights()
*
*  This method is used for setting the attributes of each
*  custom light source in the scene (position, ambient color,
*  diffuse color, specular color, focal strength, specular
*  intensity).
*  - ensure number of scene lights equals number of lights
*    in fragment shader code.
***********************************************************/
void SceneManager::SetupSceneLights()
{
	// enable custom lighting in the scene
	m_pShaderManager->setBoolValue("bUseLighting", true);

	// Set up ceiling light [0] source // light left/front of objects
	m_pShaderManager->setVec3Value("lightSources[0].position", -6.7f, 5.5f, 1.0f); // position
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.03f, 0.01f, 0.01f); // ambient light color
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.32f, 0.32f, 0.3f); // diffuse light color
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 0.4f, 0.4f, 0.39f); // specular light color
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 45.0f); // strength of emitted focal beam
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.05f); // strength of emitted specular light

	// Set up ceiling light [1] source // light right/front of objects
	m_pShaderManager->setVec3Value("lightSources[1].position", 8.0f, 6.5f, 0.5f); //  above and in front of scene
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.03f, 0.02f, 0.01f); 
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.3f, 0.3f, 0.3f); 
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.4f, 0.4f, 0.4f); 
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 70.0f); 
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.20f); 

	// Set up light source [2] // focus on ambient lighting
	m_pShaderManager->setVec3Value("lightSources[2].position", 0.0f, 15.0f, 0.0f);  
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.3f, 0.25f, 0.25f);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.0f, 0.0f, 0.0f); 
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", 0.0f, 0.0f, 0.0f); 
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 0.01f); 
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.01f); 
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{

	LoadSceneTextures();		  // Use custom textures for scene
	DefineObjectMaterials(); // define the materials to create proper object lighting
	SetupSceneLights();	   // create the lights for the scene (up to 4 lights)
	
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadPyramid3Mesh();
	m_basicMeshes->LoadPyramid4Mesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadPrismMesh();

}

/***********************************************************
*  SetShaderAttributes()
*
*  This method is used for setting the shader's color,
	texturing, and material attributes for the mesh.
	- Use "none" for no texture or material.
	- UVscale sets number of texture tiles in X&Y directions.
	- Parameters default to white color, "none" texture,
	  (1.0f, 1.0f) UVscale, and "none" material. Omit trailing
	  parameters for default values.
***********************************************************/
void SceneManager::SetShaderAttributes(
	glm::vec4 colorRGBA,		 // RGBA color
	std::string texture,	   // texture name (default = "none) (from LoadSceneTextures())
	glm::vec2 UVscale,	  // number of times texture drawn in X&Y direction (default = (1.0f, 1.0f))
	std::string material) // name of material (default = "none") (from DefineObjectMaterials())
{
	// set RGBA color
	SetShaderColor(colorRGBA.x, colorRGBA.y, colorRGBA.z, colorRGBA.w);
	// Set texture if provided
	if (texture != "none")
	{
		SetShaderTexture(texture);
	}
	// Set UV scale
	SetTextureUVScale(UVscale.x, UVscale.y);
	// Set material if provided
	if (material != "none")
	{
		SetShaderMaterial(material);
	}
}

/***********************************************************
 *  DrawMeshTransformation()
 *
 *  This method is used for drawing a basic 3D shape given parameter
 *  vectors for color, scale, rotation, position. Must pass the ShapeMeshes object,
 *  and must use the drawing functions in the ShapeMeshWrappers.h wrapper class. This
 *  allows the use of all default parameter arguments from ShapeMeshes class.
 *  don't use ShapeMeshes::DrawCylinderMesh use instead-> ShapeMeshWrapper::DrawCylinderMeshWrapper
 ***********************************************************/
void SceneManager::DrawMeshTransformation(
	glm::vec3 scaleXYZ,					// shape XYZ scale
	glm::vec3 rotationDegreesXYZ,	  // shape XYZ rotation
	glm::vec3 positionXYZ,			 // XYZ position of shape
	ShapeMeshes* meshObject,		// pointer to ShapeMesh object
	MeshDrawFunction drawMeshFunction) // pointer to function used to draw specific shape (see notes)
{
	// Set transform buffer using passed in arguments
	SetTransformations(
		scaleXYZ, // Scale
		rotationDegreesXYZ.x, // X axis rotation
		rotationDegreesXYZ.y, // Y axis rotation
		rotationDegreesXYZ.z, // Z axis rotation
		positionXYZ); // XYZ position
	
	// Draws the object with the function pointer passed, must be from ShapeMeshWrapper class.
	drawMeshFunction(meshObject);
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	//***********************DRAW OBJECTS*****************************//
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						       ***/
	/******************************************************************/

	// BACKGROUND PLANE:
	SetShaderAttributes(
		glm::vec4(.3, .3, .3, 1),  // shader RGBA color
		"backdrop",  				  // memory-loaded texture name
		glm::vec2(3.0, 0.7f),	 // UV scale
		"stone");						// Material tag
	DrawMeshTransformation(
		glm::vec3(22.5f, 1.0f, 3.5f),	// shape XYZ scale
		glm::vec3(90.0f, 0.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(0.0f, 3.5f, -10.0f),		// XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawPlaneMeshWrapper);	// pointer to function used to draw specific shape

	// BACKGROUND LEDGE
	SetShaderAttributes(
		glm::vec4(.3, .3, .3, 1),  // shader RGBA color
		"wood",						  // memory-loaded texture name
		glm::vec2(6.0, 0.4f),    // UV scale
		"wood");						// Material tag
	DrawMeshTransformation(
		glm::vec3(45.0f, 1.0f, 3.5f),	 // shape XYZ scale
		glm::vec3(0.0f, 0.0f, 0.0f),	   // shape XYZ rotation
		glm::vec3(0.0f, 7.0f, -9.5f),	  // XYZ position of shape
		m_basicMeshes,						 // pointer to ShapeMesh object
		ShapeMeshWrappers::DrawBoxMeshWrapper);	// pointer to function used to draw specific shape

	// COUNTERTOP
	SetShaderAttributes(
		glm::vec4(0.3, 0.3, 0.3, 1),  // shader RGBA color
		"counter",						  // memory-loaded texture name
		glm::vec2(4.0, 2.0f),    // UV scale
		"stone");						// Material tag
	DrawMeshTransformation(
		glm::vec3(45.0f, 2.0f, 13.0f),	 // shape XYZ scale
		glm::vec3(0.0f, 0.0f, 0.0f),	   // shape XYZ rotation
		glm::vec3(0.0f, -1.0f, -3.5f),	  // XYZ position of shape
		m_basicMeshes,						 // pointer to ShapeMesh object
		ShapeMeshWrappers::DrawBoxMeshWrapper);	// pointer to function used to draw specific shape

	 // GLASS JAR (complex object):
	DrawJar(6.0f, 0.0f, -6.2f);

	// CUP WITH STRAW (complex object):
	DrawCup(-3.0f, 0.0f, -4.2f);

	// MARBLE COASTER UNDER CUP
	SetShaderAttributes(
		glm::vec4(0.8, 0.8, 0.8, 1),  // shader RGBA color
		"marble",						  // memory-loaded texture name
		glm::vec2(2.0f, 0.5f),    // UV scale
		"stone");						// Material tag
	DrawMeshTransformation(
		glm::vec3(1.6f, 0.3f, 1.6f),	 // shape XYZ scale
		glm::vec3(0.0f, 0.0f, 0.0f),	   // shape XYZ rotation
		glm::vec3(-3.0f, 0.0f, -4.0f),	  // XYZ position of shape
		m_basicMeshes,						 // pointer to ShapeMesh object
		ShapeMeshWrappers::DrawCylinderMeshWrapper);	// pointer to function used to draw specific shape

	// CUTTING BOARD
	SetShaderAttributes(
		glm::vec4(0.8, 0.8, 0.8, 1),  // shader RGBA color
		"plastic",						  // memory-loaded texture name
		glm::vec2(4.0f, 2.0f),    // UV scale
		"plastic");						// Material tag
	DrawMeshTransformation(
		glm::vec3(8.0f, 0.3f, 5.5f),		// XYZ scale
		glm::vec3(0.0f, -20.0f, 0.0f),   // XYZ rotation
 		glm::vec3(3.0f, 0.15f, -1.5f),  // XYZ position
		m_basicMeshes,
		ShapeMeshWrappers::DrawBoxMeshWrapper); // mesh shape function

	// CUCUMBER
	DrawCucumber(3.8f, 0.3f, -3.0f);

	// KNIFE
	DrawKnife(0.6f, 0.25f, -0.7f);
}

/***********************************************************
 *  DrawJar()
 *
 *  This method is used by RenderScene() to draw and transform
 *  a complex 3D object of multiple basic 3D shapes. Created to
 *  reduce complexity of the RenderScene() method.
 ***********************************************************/
void SceneManager::DrawJar(float x_pos, float y_pos, float z_pos)
{
	// Position of the entire object.
	float jarX_pos = x_pos;
	float jarY_pos = y_pos;
	float jarZ_pos = z_pos;

	glm::vec4 baseShaderColorRGBA = glm::vec4(0.7, 0.7, 0.9, 0.8);
	std::string baseTexture = "glass13";
	std::string lidTexture = "glass10";
	glm::vec2 baseUVscale = glm::vec2(1.0f, 0.35f);
	std::string baseMaterial = "glass";

	/********************[DRAW GLASS JAR (complex)]************************/
	  // set initial color, texture, UVscale
	SetShaderAttributes(
		baseShaderColorRGBA, // shader RGBA color
		baseTexture,		  // memory-loaded texture name
		baseUVscale,		 // X and Y UVscale
		baseMaterial);		// Material Tag

	   /********Transform each individual shape from bottom to top******/
	// ROUNDED BASE BOTTOM SPHERE:
	DrawMeshTransformation(
		glm::vec3(2.0f, 0.3f, 2.0f),		// shape XYZ scale
		glm::vec3(0.0f, 0.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(jarX_pos + 0.0f, jarY_pos + 0.15f, jarZ_pos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawSphereMeshWrapper);	// pointer to function used to draw specific shape
	// CYLINDER BASE:
	DrawMeshTransformation(
		glm::vec3(2.0f, 4.05f, 2.0f),		// shape XYZ scale
		glm::vec3(0.0f, 0.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(jarX_pos + 0.0f, jarY_pos + 0.15f, jarZ_pos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawCylinderMeshWrapper);	// pointer to function used to draw specific shape
	//ROUNDED BASE TOP SPHERE :
	SetTextureUVScale(1.0, 0.25); // adjust scale to align shape textures
	DrawMeshTransformation(
		glm::vec3(2.02f, 0.7f, 2.02f),		// shape XYZ scale
		glm::vec3(0.0f, -10.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(jarX_pos + 0.0f, jarY_pos + 4.2f, jarZ_pos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawSphereMeshWrapper);	// pointer to function used to draw specific shape
	//ROUNDED NECK CYLINDER:
	SetTextureUVScale(0.8f, 0.1f); // adjust scale to align shape textures
	DrawMeshTransformation(
		glm::vec3(1.6f, 0.80f, 1.6f),		// shape XYZ scale
		glm::vec3(0.0f, 8.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(jarX_pos + 0.0f, jarY_pos + 4.4f, jarZ_pos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawCylinderMeshWrapper);	// pointer to function used to draw specific shape
	// NECK TORUS LARGE:
	SetTextureUVScale(1.5f, 0.3f);
	DrawMeshTransformation(
		glm::vec3(1.48f, 1.48f, 0.65f),		// shape XYZ scale
		glm::vec3(90.0f, 0.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(jarX_pos + 0.0f, jarY_pos + 5.10f, jarZ_pos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawTorusMeshWrapper);	// pointer to function used to draw specific shape

	SetShaderTexture(lidTexture);
	SetTextureUVScale(2.0f, 1.0f); // adjust scale to align shape textures
	// LID TORUS LARGE:
	DrawMeshTransformation(
		glm::vec3(1.5f, 1.5f, 0.5f),		// shape XYZ scale
		glm::vec3(90.0f, 0.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(jarX_pos + 0.0f, jarY_pos + 5.25f, jarZ_pos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawTorusMeshWrapper);	// pointer to function used to draw specific shape
	//LID SPHERE LARGE:
	DrawMeshTransformation(
		glm::vec3(1.6f, 0.16f, 1.6f),		// shape XYZ scale
		glm::vec3(0.0f, 0.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(jarX_pos + 0.0f, jarY_pos + 5.25f, jarZ_pos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawSphereMeshWrapper);	// pointer to function used to draw specific shape
	// LID CYLINDER:
	SetTextureUVScale(2.0f, 1.0f);
	DrawMeshTransformation(
		glm::vec3(0.9f, 0.5f, 0.9f),		// shape XYZ scale
		glm::vec3(0.0f, 0.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(jarX_pos + 0.0f, jarY_pos + 5.2f, jarZ_pos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawCylinderMeshWrapper);	// pointer to function used to draw specific shape
	SetTextureUVScale(1.5f, 1.0f);
	// LID TORUS SMALL:
	DrawMeshTransformation(
		glm::vec3(1.1f, 1.1f, 0.5f),		// shape XYZ scale
		glm::vec3(90.0f, 0.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(jarX_pos + 0.0f, jarY_pos + 5.7f, jarZ_pos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawTorusMeshWrapper);	// pointer to function used to draw specific shape
	// LID SPHERE TOP:
	DrawMeshTransformation(
		glm::vec3(1.1f, 0.2f, 1.1f),		// shape XYZ scale
		glm::vec3(0.0f, 0.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(jarX_pos + 0.0f, jarY_pos + 5.65f, jarZ_pos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawSphereMeshWrapper);	// pointer to function used to draw specific shape
}

/*********Complex Object Functions***************************
* The following functions are used by RenderScene() to draw	*
* and transform a complex 3D object of multiple 3D shapes	*
* and/or textures/materials.											*
* Created to reduce complexity of the RenderScene() method.	*
************************************************************/

/***********************************************************
 *  DrawCup()
 *
 *  Draws a plastic/metal cup object
 ***********************************************************/
void SceneManager::DrawCup(float x_pos, float y_pos, float z_pos)
{
	// Position of the entire object.
	float cupX_pos = x_pos;
	float cupY_pos = y_pos;
	float cupZ_pos = z_pos;

	glm::vec4 baseShaderColorRGBA = glm::vec4(0.6, 0.1, 0.1, 1.0);
	std::string baseTexture = "none";
	std::string metalTexture = "metal";
	glm::vec2 baseUVscale = glm::vec2(3.0f, 0.8f);
	std::string baseMaterial = "plastic";
	std::string metalMaterial = "metal";

	/********************[DRAW CUP (complex)]************************/
	  // set initial color, texture, UVscale
	SetShaderAttributes(
		baseShaderColorRGBA, // shader RGBA color
		baseTexture,		  // memory-loaded texture name
		baseUVscale,		 // X and Y UVscale
		baseMaterial);		// Material Tag
	/********Transform each individual shape from bottom to top******/
	// BASE TAPERED CYLINDER:
	DrawMeshTransformation(
		glm::vec3(1.5f, 6.49f, 1.5f),		// shape XYZ scale
		glm::vec3(0.0f, 0.0f, 180.0f),		// shape XYZ rotation
		glm::vec3(cupX_pos + 0.0f, cupY_pos + 4.5f, cupZ_pos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawTaperedCylinderMeshWrapper);	// pointer to function used to draw specific shape

	// TOP METALLIC LIP
	SetShaderAttributes( // metal texture and material
		baseShaderColorRGBA, // shader RGBA color
		metalTexture,		  // memory-loaded texture name
		baseUVscale,		 // X and Y UVscale
		metalMaterial);   // Material Tag
	DrawMeshTransformation(
		glm::vec3(1.5f, 1.0f, 1.5f),		// shape XYZ scale
		glm::vec3(0.0f, 0.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(cupX_pos + 0.0f, cupY_pos + 4.5f, cupZ_pos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawHollowCylinderMeshWrapper);	// pointer to function used to draw specific shape

	// METALLIC STRAW
	SetTextureUVScale(1.0f, 5.0f);
	DrawMeshTransformation(
		glm::vec3(0.2f, 7.5f, 0.2f),		// shape XYZ scale
		glm::vec3(14.6f, 0.0f, 10.5f),		// shape XYZ rotation
		glm::vec3(cupX_pos + 0.35f, cupY_pos + 0.0f, cupZ_pos + -0.35f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawHollowCylinderMeshWrapper);	// pointer to function used to draw specific shape
}

/***********************************************************
 *  DrawCucumber()
 *
 *  Draws a sliced cucumber object
 ***********************************************************/
void SceneManager::DrawCucumber(float x_pos, float y_pos, float z_pos)
{
	// Position of the entire object.
	float xPos = x_pos;
	float yPos = y_pos;
	float zPos = z_pos;

	glm::vec4 baseShaderColorRGBA = glm::vec4(0.2, 0.5, 0.2, 1.0);
	std::string outerTexture = "cucumber_outer";
	std::string innerTexture = "cucumber_inner";
	glm::vec2 baseUVscale = glm::vec2(1.0f, 1.0f);
	std::string baseMaterial = "organic";

	/********************[DRAW CUCUMBER (complex)]************************/
	 // set initial color, texture, UVscale
	SetShaderAttributes(
		baseShaderColorRGBA, // shader RGBA color
		outerTexture,		  // memory-loaded texture name
		baseUVscale,		 // X and Y UVscale
		baseMaterial);		// Material Tag

   /********Transform each individual shape******/
	// Long Cucumber slice
	DrawMeshTransformation( // cylinder
		glm::vec3(0.7f, 2.8f, 0.7f),		// shape XYZ scale
		glm::vec3(0.0f, -25.0f, 90.0f),		// shape XYZ rotation
		glm::vec3(xPos + 0.0f, yPos + 0.7f, zPos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawHollowCylinderMeshWrapper);	// pointer to function used to draw specific shape
	SetShaderTexture(innerTexture);
	m_basicMeshes->DrawCylinderMesh(false, true, false); // draw bottom only
	SetShaderTexture(outerTexture);

	DrawMeshTransformation( // sphere end
		glm::vec3(1.0f, 0.7f, 0.7f),		// shape XYZ scale
		glm::vec3(0.0f, -25.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(xPos + -2.538f, yPos + 0.7f, zPos + -1.183f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawSphereMeshWrapper);	// pointer to function used to draw specific shape


	
	// 5 small cucumber slices
	DrawMeshTransformation( // cylinder
		glm::vec3(0.7f, 0.15f, 0.7f),		// shape XYZ scale
		glm::vec3(0.0f, 0.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(xPos + 0.9f, yPos + 0.0f, zPos + 0.2f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawHollowCylinderMeshWrapper);	// pointer to function used to draw specific shape
	SetShaderTexture(innerTexture);
	m_basicMeshes->DrawCylinderMesh(true, true, false);

	SetShaderTexture(outerTexture);
	DrawMeshTransformation( // cylinder
		glm::vec3(0.7f, 0.15f, 0.7f),		// shape XYZ scale
		glm::vec3(-3.5f, 0.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(xPos + 1.35f, yPos + 0.02f, zPos + 2.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawHollowCylinderMeshWrapper);	// pointer to function used to draw specific shape
	SetShaderTexture(innerTexture);
	m_basicMeshes->DrawCylinderMesh(true, true, false);

	SetShaderTexture(outerTexture);
	DrawMeshTransformation( // cylinder
		glm::vec3(0.75f, 0.17f, 0.7f),		// shape XYZ scale
		glm::vec3(0.0f, -5.0f, 0.0f),		// shape XYZ rotation
		glm::vec3(xPos + 1.3f, yPos + 0.15f, zPos + 0.9f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawHollowCylinderMeshWrapper);	// pointer to function used to draw specific shape
	SetShaderTexture(innerTexture);
	m_basicMeshes->DrawCylinderMesh(true, true, false);

	SetShaderTexture(outerTexture);
	DrawMeshTransformation( // cylinderq
		glm::vec3(0.7f, 0.13f, 0.65f),		// shape XYZ scale
		glm::vec3(0.0f, -1.0f, -1.5f),		// shape XYZ rotation
		glm::vec3(xPos + 1.2f, yPos + 0.3f, zPos + 0.7f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawHollowCylinderMeshWrapper);	// pointer to function used to draw specific shape
	SetShaderTexture(innerTexture);
	m_basicMeshes->DrawCylinderMesh(true, true, false);

	SetShaderTexture(outerTexture);
	DrawMeshTransformation( // cylinder
		glm::vec3(0.7f, 0.2f, 0.65f),		// shape XYZ scale
		glm::vec3(0.0f, -1.0f, -3.0f),		// shape XYZ rotation
		glm::vec3(xPos + 0.7f, yPos + 0.45f, zPos + 0.4f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawHollowCylinderMeshWrapper);	// pointer to function used to draw specific shape
	SetShaderTexture(innerTexture);
	m_basicMeshes->DrawCylinderMesh(true, true, false);
}

/***********************************************************
 *  DrawKnife()
 *
 *  Draws a knife with wooden handle.
 ***********************************************************/
void SceneManager::DrawKnife(float x_pos, float y_pos, float z_pos)
{
	// Position of the entire object.
	float xPos = x_pos;
	float yPos = y_pos;
	float zPos = z_pos;

	glm::vec4 baseShaderColorRGBA = glm::vec4(0.3, 0.3, 0.2, 1.0);
	std::string handleTexture = "wood";
	std::string bladeTexture = "metal";
	glm::vec2 baseUVscale = glm::vec2(1.0f, 1.0f);
	std::string handleMaterial = "wood";
	std::string bladeMaterial = "metal";

	/********************[DRAW KNIFE (complex)]************************/
	 // set initial color, texture, UVscale
	SetShaderAttributes(
		baseShaderColorRGBA, // shader RGBA color
		bladeTexture,		  // memory-loaded texture name
		baseUVscale,		 // X and Y UVscale
		bladeMaterial);		// Material Tag

	/********Transform each individual shape******/
	// Handle Base Wood Tapered Cylinder
	SetShaderAttributes(
		baseShaderColorRGBA, // shader RGBA color
		handleTexture,		  // memory-loaded texture name
		baseUVscale,		 // X and Y UVscale
		handleMaterial);	// Material Tag
	DrawMeshTransformation(
		glm::vec3(0.45f, 2.9f, 0.35f),		// shape XYZ scale
		glm::vec3(90.0f, 178.0f, 80.0f),		// shape XYZ rotation
		glm::vec3(xPos + -3.0f, yPos + 0.35f, zPos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawTaperedCylinderMeshWrapper);	// pointer to function used to draw specific shape

	// Handle End Metal Cylinder
	SetShaderAttributes(
		baseShaderColorRGBA, // shader RGBA color
		bladeTexture,		  // memory-loaded texture name
		baseUVscale,		 // X and Y UVscale
		bladeMaterial);	// Material Tag
	DrawMeshTransformation(
		glm::vec3(0.451f, 0.1f, 0.351f),		// shape XYZ scale
		glm::vec3(90.0f, 178.0f, 80.0f),		// shape XYZ rotation
		glm::vec3(xPos + -3.01f, yPos + 0.35f, zPos + 0.0f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawCylinderMeshWrapper);	// pointer to function used to draw specific shape
	
	// Blade Beginning Metal cylinder
	DrawMeshTransformation(
		glm::vec3(0.27f, 0.35f, 0.2f),		// shape XYZ scale
		glm::vec3(90.0f, 178.0f, 80.0f),		// shape XYZ rotation
		glm::vec3(xPos + -0.4f, yPos + 0.27f, zPos + 0.45f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawCylinderMeshWrapper);	// pointer to function used to draw specific shape

	// Blade Base Metal Rectangle
	DrawMeshTransformation(
		glm::vec3(0.65f, 4.5f, 0.02f),		// shape XYZ scale
		glm::vec3(88.0f, 178.0f, 80.0f),		// shape XYZ rotation
		glm::vec3(xPos + -0.05f, yPos + 0.27f, zPos + 0.1f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawCylinderMeshWrapper);	// pointer to function used to draw specific shape

	// Blade Tip Metal Pyramid
	SetTextureUVScale(0.3f, 0.3f);
	DrawMeshTransformation(
		glm::vec3(1.43f, 1.25f, 0.02f),		// shape XYZ scale
		glm::vec3(88.0f, 178.0f, 105.0f),		// shape XYZ rotation
		glm::vec3(xPos + 4.65f, yPos + 0.125f, zPos + 0.675f), // XYZ position of shape
		m_basicMeshes,							// pointer to ShapeMesh object
		ShapeMeshWrappers::DrawPyramid4MeshWrapper);	// pointer to function used to draw specific shape

}

/***************CHALLENGES*************************
* - different primitive shape meshes have different scaling, origins, and rotation
* - tapered cylinder used to make shapes is too narrow on the end
* - repetitive, long code for making so many shapes. needs to be cleaned up / consolidated / refactored
* - producing an orthographic view to view objects within the clipping plane 
* - unusual shape of complex non-primitive shapes :: used multiple shapes merged together (glass jar and knife)
* - even with all the prior work in the milestones, completing the final scene took a very long time getting
    every object shape, texture, material, and lighting looking just right.
**************************************************/

