#ifndef SKYBOX_H
#define SKYBOX_H
// Minimum OpenGL version = 3.2
// for glGenVertexArrays glBindVertexArray
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLShaderProgram>

class SkyBox : protected  QOpenGLFunctions_3_2_Core
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
