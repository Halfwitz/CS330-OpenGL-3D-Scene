///////////////////////////////////////////////////////////////////////////////
// scenemanager.h
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
//  MODIFIED BY: Michael Lorenz
// Added a method for drawing a complicated jar object, Jul. 17th, 2024
// Added a utility method for drawing an object with all transformation parameters,
// Jul. 23rd, 2024
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"
#include "ShapeMeshWrappers.h"

#include <string>
#include <vector>

/***********************************************************
 *  SceneManager
 *
 *  This class contains the code for preparing and rendering
 *  3D scenes, including the shader settings.
 ***********************************************************/
class SceneManager
{
public:

	// constructor
	SceneManager(ShaderManager *pShaderManager);
	// destructor
	~SceneManager();

	struct TEXTURE_INFO
	{
		std::string tag;
		uint32_t ID;
	};

	struct OBJECT_MATERIAL
	{
		float ambientStrength;
		glm::vec3 ambientColor;
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float shininess;
		std::string tag;
	};

private:
	// pointer to shader manager object
	ShaderManager* m_pShaderManager;
	// pointer to basic shapes object
	ShapeMeshes* m_basicMeshes;
	// total number of loaded textures
	int m_loadedTextures;
	// loaded textures info
	TEXTURE_INFO m_textureIDs[16];
	// defined object materials
	std::vector<OBJECT_MATERIAL> m_objectMaterials;

	// load texture images and convert to OpenGL texture data
	bool CreateGLTexture(const char* filename, std::string tag);
	// bind loaded OpenGL textures to slots in memory
	void BindGLTextures();
	// free the loaded OpenGL textures
	void DestroyGLTextures();
	// find a loaded texture by tag
	int FindTextureID(std::string tag);
	int FindTextureSlot(std::string tag);
	// find a defined material by tag
	bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);

	// set the transformation values 
	// into the transform buffer
	void SetTransformations(
		glm::vec3 scaleXYZ,
		float XrotationDegrees,
		float YrotationDegrees,
		float ZrotationDegrees,
		glm::vec3 positionXYZ);

	// set the color values into the shader
	void SetShaderColor(
		float redColorValue,
		float greenColorValue,
		float blueColorValue,
		float alphaValue);

	// set the texture data into the shader
	void SetShaderTexture(
		std::string textureTag);

	// set the UV scale for the texture mapping
	void SetTextureUVScale(
		float u, float v);

	// set the object material into the shader
	void SetShaderMaterial(
		std::string materialTag);

public:
	// The following methods are for the students to 
	// customize for their own 3D scene

	void LoadSceneTextures();
	void PrepareScene();
	void RenderScene();
	void DefineObjectMaterials();
	void SetupSceneLights();
	
	// sets the shader manager attributes to the given color, texture, UVscale, and material
	// defaults to white color, no texture, 1.0 UVscale, and no material.
	// Use UVscale to set the number of times the texture is drawn in the X and Y direction
	void SetShaderAttributes(
		glm::vec4 colorRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
		std::string texture = "none",
		glm::vec2 UVscale = glm::vec2(1.0f, 1.0f),
		std::string material = "none");

	// Defines the alias for a pointer to ShapeMeshWrappers drawing functions passed to DrawMeshTransformation()
	typedef void (*MeshDrawFunction)(ShapeMeshes*);

	// draws an object with given transformation parameters, ShapeMeshes object reference, 
	// and a drawing function from the ShapeMeshWrappers wrapper class
	void DrawMeshTransformation(
		glm::vec3 scaleXYZ, 
		glm::vec3 rotationDegreesXYZ, 
		glm::vec3 positionXYZ, 
		ShapeMeshes* meshObject, 
		MeshDrawFunction drawMeshFunction);

	// uses DrawMeshTransformation calls to draw a complicated multi-mesh objects with one XYZ position.
	void DrawJar(float x_pos, float y_pos, float z_pos);
	void DrawCup(float x_pos, float y_pos, float z_pos);
	void DrawCucumber(float x_pos, float y_pos, float z_pos);
	void DrawKnife(float x_pos, float y_pos, float z_pos);
};