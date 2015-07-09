#ifndef OCTAHEDRON_H
#define OCTAHEDRON_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>

class Octahedron : protected QOpenGLFunctions
{
public:
    Octahedron(QOpenGLShaderProgram *prog);
    void render(QMatrix4x4 projection, QMatrix4x4 modelview, QColor rendercolor);

    ~Octahedron();
private:
    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer posBuf;
    int vertexPosition;

};

#endif // OCTAHEDRON_H
