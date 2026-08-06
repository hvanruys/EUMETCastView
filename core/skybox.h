#ifndef SKYBOX_H
#define SKYBOX_H

// The only drawing class that calls glGenVertexArrays and glBindVertexArray
// itself rather than through QOpenGLVertexArrayObject, so it is the only one
// that needs a versioned functions class - the rest get by on plain
// QOpenGLFunctions, which has no VAO entry points.
//
// It must follow the same OPENGL* define as Globe. This used to be pinned to
// QOpenGLFunctions_3_0 whatever Globe was built with, and that is a
// compatibility class: on the core profile the context now asks for, its
// initializeOpenGLFunctions() fails and every call below it silently does
// nothing. Moving Globe to core without moving this would have traded a globe
// that refuses to start for one that starts with no sky in it.
#include <QOpenGLShaderProgram>

#ifdef OPENGL30
#include <QOpenGLFunctions_3_0>
#endif
#ifdef OPENGL33
#include <QOpenGLFunctions_3_3_Core>
#endif
#ifdef OPENGL40
#include <QOpenGLFunctions_4_0_Core>
#endif
#ifdef OPENGL43
#include <QOpenGLFunctions_4_3_Core>
#endif

#ifdef OPENGL30
class SkyBox  : protected QOpenGLFunctions_3_0
#endif
#ifdef OPENGL33
class SkyBox  : protected QOpenGLFunctions_3_3_Core
#endif
#ifdef OPENGL40
class SkyBox  : protected QOpenGLFunctions_4_0_Core
#endif
#ifdef OPENGL43
class SkyBox  : protected QOpenGLFunctions_4_3_Core
#endif
{

public:
    SkyBox(QOpenGLShaderProgram *prog);
    ~SkyBox();
    void render(QMatrix4x4 projection, QMatrix4x4 rot);

private:

    QOpenGLShaderProgram *program;
    GLuint uniformViewMatrix;
    void loadCubemap(QVector<QString> faces);
    GLuint skyboxVAO, skyboxVBO;
    GLuint cubemapTexture;

};


#endif // SKYBOX_H
