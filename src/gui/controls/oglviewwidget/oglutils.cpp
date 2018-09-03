//
// Created by Jon on 03/09/2018.
//

#include "oglutils.h"

#include "GL/glu.h"

#define RADPERDEG 0.0174533

bool ReadFile(const char* pFileName, std::string& outFile)
{
    std::ifstream f(pFileName);

    bool ret = false;

    if (f.is_open()) {
        std::string line;
        while (getline(f, line)) {
            outFile.append(line);
            outFile.append("\n");
        }

        f.close();

        ret = true;
    }

    return ret;
}

void Arrow(Vector4f start, Vector4f end, GLdouble D)
{
    Arrow(start.x, start.y, start.z, end.x, end.y, end.z, D);
}

void Arrow(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2, GLdouble z2, GLdouble D)
{
    double x=x2-x1;
    double y=y2-y1;
    double z=z2-z1;
    double L=sqrt(x*x+y*y+z*z);

    GLUquadricObj *quadObj;

    glPushMatrix();

    glTranslated(x1,y1,z1);

    if((x!=0.0) || (y!=0.0))
    {
        glRotated(atan2(y,x)/RADPERDEG,0.,0.,1.);
        glRotated(atan2(sqrt(x*x+y*y),z)/RADPERDEG,0.,1.,0.);
    }
    else if (z<0)
        glRotated(180,1.,0.,0.);

    glTranslatef(0,0,L-4*D);

    quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, GLU_FILL);
    gluQuadricNormals (quadObj, GLU_SMOOTH);
    gluCylinder(quadObj, 2*D, 0.0, 4*D, 32, 1);
    gluDeleteQuadric(quadObj);

    glTranslatef(0,0,-L+4*D);

    quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, GLU_FILL);
    gluQuadricNormals (quadObj, GLU_SMOOTH);
    gluCylinder(quadObj, 0.03, 0.03, L-4*D, 32, 1);
    gluDeleteQuadric(quadObj);

    glPopMatrix();
}

void Cube(std::vector<Vector3f> corners, const Matrix4f& Proj, const Matrix4f& MV)
{
    if (corners.size() != 8)
    {
        throw std::runtime_error("OpenGL: Attempt to draw cube without 8 vertices.");
        return;
    }

    glPushMatrix();

    glColor3f(1,1,1);
    glLineWidth(1.0);

    // make it a tiny bit smaller as a bodgy way of avoiding clipping errors
    for (auto coord : corners)
        coord = coord * 0.99;

    std::vector<Vector4f> pos(8);

    for (int i = 0; i < 8; ++i)
        pos[i] = Proj * MV * Vector4f(corners[i], 1.0);

    glBegin(GL_LINE_STRIP);
    glVertex4f(pos[0].x, pos[0].y, pos[0].z, pos[0].w);
    glVertex4f(pos[1].x, pos[1].y, pos[1].z, pos[1].w);
    glVertex4f(pos[3].x, pos[3].y, pos[3].z, pos[3].w);
    glVertex4f(pos[2].x, pos[2].y, pos[2].z, pos[2].w);
    glVertex4f(pos[0].x, pos[0].y, pos[0].z, pos[0].w);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex4f(pos[4].x, pos[4].y, pos[4].z, pos[4].w);
    glVertex4f(pos[5].x, pos[5].y, pos[5].z, pos[5].w);
    glVertex4f(pos[7].x, pos[7].y, pos[7].z, pos[7].w);
    glVertex4f(pos[6].x, pos[6].y, pos[6].z, pos[6].w);
    glVertex4f(pos[4].x, pos[4].y, pos[4].z, pos[4].w);
    glEnd();

    glBegin(GL_LINES);
    glVertex4f(pos[0].x, pos[0].y, pos[0].z, pos[0].w);
    glVertex4f(pos[4].x, pos[4].y, pos[4].z, pos[4].w);

    glVertex4f(pos[1].x, pos[1].y, pos[1].z, pos[1].w);
    glVertex4f(pos[5].x, pos[5].y, pos[5].z, pos[5].w);

    glVertex4f(pos[2].x, pos[2].y, pos[2].z, pos[2].w);
    glVertex4f(pos[6].x, pos[6].y, pos[6].z, pos[6].w);

    glVertex4f(pos[3].x, pos[3].y, pos[3].z, pos[3].w);
    glVertex4f(pos[7].x, pos[7].y, pos[7].z, pos[7].w);
    glEnd();

    glPopMatrix();
}

void Cube(float x1, float y1, float z1, float x2, float y2, float z2, const Matrix4f& Proj, const Matrix4f& MV)
{
    glPushMatrix();

    glColor3f(1,1,1);
    glLineWidth(1.0);

    Vector4f c1 = Proj * MV * Vector4f(x1,y1,z1,1);
    Vector4f c2 = Proj * MV * Vector4f(x2,y1,z1,1);
    Vector4f c3 = Proj * MV * Vector4f(x1,y2,z1,1);
    Vector4f c4 = Proj * MV * Vector4f(x2,y2,z1,1);
    Vector4f c5 = Proj * MV * Vector4f(x1,y2,z2,1);
    Vector4f c6 = Proj * MV * Vector4f(x2,y2,z2,1);
    Vector4f c7 = Proj * MV * Vector4f(x1,y1,z2,1);
    Vector4f c8 = Proj * MV * Vector4f(x2,y1,z2,1);

    glBegin(GL_LINES);
    glVertex4f(c1.x, c1.y, c1.z, c1.w);
    glVertex4f(c2.x, c2.y, c2.z, c2.w);

    glVertex4f(c3.x, c3.y, c3.z, c3.w);
    glVertex4f(c4.x, c4.y, c4.z, c4.w);

    glVertex4f(c5.x, c5.y, c5.z, c5.w);
    glVertex4f(c6.x, c6.y, c6.z, c6.w);

    glVertex4f(c7.x, c7.y, c7.z, c7.w);
    glVertex4f(c8.x, c8.y, c8.z, c8.w);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex4f(c1.x, c1.y, c1.z, c1.w);
    glVertex4f(c3.x, c3.y, c3.z, c3.w);
    glVertex4f(c5.x, c5.y, c5.z, c5.w);
    glVertex4f(c7.x, c7.y, c7.z, c7.w);
    glVertex4f(c1.x, c1.y, c1.z, c1.w);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex4f(c2.x, c2.y, c2.z, c2.w);
    glVertex4f(c4.x, c4.y, c4.z, c4.w);
    glVertex4f(c6.x, c6.y, c6.z, c6.w);
    glVertex4f(c8.x, c8.y, c8.z, c8.w);
    glVertex4f(c2.x, c2.y, c2.z, c2.w);
    glEnd();

    glPopMatrix();
}

void XyzDirection(Matrix4f ModelMatrix)
{
    glPushMatrix();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

//    glMatrixMode( GL_PROJECTION );
//    glLoadIdentity();

//    glRotatef(rot.x, 0, 0, 1);
//    glRotatef(rot.y, -1, 0, 0);
//    glRotatef(rot.z, 0, 1, 0);

    float length = 1;

    //reset translation?
    ModelMatrix.m[0][3] = 0;
    ModelMatrix.m[1][3] = 0;
    ModelMatrix.m[2][3] = 0;

    Vector4f origin(0.0, 0.0, 0.0, 1.0);
    Vector4f xdir(1.0, 0.0, 0.0, 1.0);
    Vector4f ydir(0.0, 1.0, 0.0, 1.0);
    Vector4f zdir(0.0, 0.0, 1.0, 1.0);

    xdir = ModelMatrix * xdir;// * length;
    ydir = ModelMatrix * ydir;// * length;
    zdir = ModelMatrix * zdir;// * length;

    const float scale = 0.8;
    glScalef(scale, scale, scale);


    glColor3f(1,0,0);
    Arrow(origin, xdir, 0.07);

    glColor3f(0,1,0);
    Arrow(origin, ydir, 0.07);

    glColor3f(0,0,1);
    Arrow(origin, zdir, 0.07);

    // this should not be neede but push/pop matrix seem to have no effect?
    glScalef(1/scale, 1/scale, 1/scale);
    glPopMatrix();
}

