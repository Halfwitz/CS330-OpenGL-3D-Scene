#ifndef SHAPEMESHWRAPPERS_H
#define SHAPEMESHWRAPPERS_H

#include "ShapeMeshes.h"
///////////////////////////////////////////////////////////////////////////////
// shapemeshwrappers.h
// ============
// call the default functions of equivelent shapemeshes.h functions
// All in .h file due to simplicity. Seperate logic into .cpp if additional
// complexity is required. 
//
//  AUTHOR: Michael Lorenz - SNHU Student / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Jul. 23rd, 2024
///////////////////////////////////////////////////////////////////////////////
class ShapeMeshWrappers
{
public:

   static void DrawBoxMeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawBoxMesh();
   }
   static void DrawConeMeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawConeMesh();
   }
   static void DrawCylinderMeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawCylinderMesh();
   }
   static void DrawHollowCylinderMeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawCylinderMesh(false, false);
   }
   static void DrawNoSideCylinderMeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawCylinderMesh(true, true, false);
   }
   static void DrawPlaneMeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawPlaneMesh();
   }
   static void DrawPrismMeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawPrismMesh();
   }
   static void DrawPyramid3MeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawPyramid3Mesh();
   }
   static void DrawPyramid4MeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawPyramid4Mesh();
   }
   static void DrawSphereMeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawSphereMesh();
   }
   static void DrawHalfSphereMeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawHalfSphereMesh();
   }
   static void DrawTaperedCylinderMeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawTaperedCylinderMesh();
   }
   static void DrawTorusMeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawTorusMesh();
   }
   static void DrawHalfTorusMeshWrapper(ShapeMeshes* meshObject) {
      meshObject->DrawHalfTorusMesh();
   }
   static void DrawNone(ShapeMeshes* meshObject) {}
};

#endif // SHAPEMESHWRAPPERS_H
