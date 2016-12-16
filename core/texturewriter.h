#ifndef TEXTUREWRITER_H
#define TEXTUREWRITER_H

//#define OPENGL31
#define OPENGL40
//#define OPENGL43

#ifdef OPENGL31
#include <QOpenGLFunctions_3_1>
#endif
#ifdef OPENGL40
#include <QOpenGLFunctions_4_0_Core>
#endif
#ifdef OPENGL43
#include <QOpenGLFunctions_4_3_Core>
#endif



#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>


#ifdef OPENGL31
class TextureWriter  : protected QOpenGLFunctions_3_1
#endif
#ifdef OPENGL40
class TextureWriter  : protected QOpenGLFunctions_4_0_Core
#endif
#ifdef OPENGL43
class TextureWriter  : protected QOpenGLFunctions_4_3_Core
#endif
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
