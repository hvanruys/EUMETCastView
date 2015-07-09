#ifndef TEXTUREWRITER_H
#define TEXTUREWRITER_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>


class TextureWriter  : protected QOpenGLFunctions
{
public:
    TextureWriter(QOpenGLShaderProgram *prog);
    void render();
    void setupBuffers(QVector<GLfloat> positions, QVector<GLfloat> texpositions, QVector<unsigned char> rainbow);
    ~TextureWriter();
private:
    GLuint vertexPosition;
    GLuint vertexTexCoord;
    GLuint vertexTexValue;
    GLuint texturelineId;

    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer positionsBuf;
    QOpenGLBuffer texcoordBuf;
    int nbrVerticesinBuffer;

};

#endif // TEXTUREWRITER_H
