#ifndef SKYBOX_H
#define SKYBOX_H
// Minimum OpenGL version = 3.0
// for glGenVertexArrays glBindVertexArray
#include <QOpenGLFunctions_3_0>
#include <QOpenGLShaderProgram>

//#ifdef OPENGL21
//#include <QOpenGLFunctions_2_1>
//#endif
//#ifdef OPENGL32
//#include <QOpenGLFunctions_3_2_Core>
//#endif
//#ifdef OPENGL40
//#include <QOpenGLFunctions_4_0_Core>
//#endif
//#ifdef OPENGL43
//#include <QOpenGLFunctions_4_3_Core>
//#endif

//#ifdef OPENGL21
//class SkyBox  : protected QOpenGLFunctions_2_1_CoreBackend
//#endif
//#ifdef OPENGL32
//class SkyBox  : protected QOpenGLFunctions_3_2_Core
//#endif
//#ifdef OPENGL40
//class SkyBox  : protected QOpenGLFunctions_4_0_Core
//#endif
//#ifdef OPENGL43
//class SkyBox  : protected QOpenGLFunctions_4_3_Core
//#endif
class SkyBox  : protected QOpenGLFunctions_3_0
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
