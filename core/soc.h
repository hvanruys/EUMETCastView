#ifndef SOC_H
#define SOC_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>

class Soc   // : protected QOpenGLFunctions
{
public:
    Soc(QOpenGLShaderProgram *prog);
    ~Soc();
    void render(QMatrix4x4 projection, QMatrix4x4 modelview);

private:
    QOpenGLShaderProgram *program;

    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer socBuf;

    void initializeSoc();

    QOpenGLFunctions *glf;

};

#endif // SOC_H
