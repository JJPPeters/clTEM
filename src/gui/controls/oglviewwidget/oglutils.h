//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_OGLUTILS_H
#define CLTEM_OGLUTILS_H

#include <fstream>

#include <QtOpenGL>

#include "oglmaths.h"

#include <fstream>

#include <QtOpenGL>

#include "oglmaths.h"

bool ReadFile(const char* pFileName, std::string& outFile);

void Arrow(Vector4f start, Vector4f end, GLdouble D);

void Arrow(GLdouble x1,GLdouble y1,GLdouble z1,GLdouble x2,GLdouble y2,GLdouble z2,GLdouble D);

void Cube(std::vector<Vector3f> corners, const Matrix4f& Proj, const Matrix4f& MV);

void Cube(float x1, float y1, float z1, float x2, float y2, float z2, const Matrix4f& Proj, const Matrix4f& MV);

void XyzDirection(Matrix4f ModelMatrix);

#endif //CLTEM_OGLUTILS_H
