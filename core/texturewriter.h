#ifndef TEXTUREWRITER_H
#define TEXTUREWRITER_H

#define OPENGL32
//#define OPENGL40
//#define OPENGL43

#ifdef OPENGL32
#include <QOpenGLFunctions_3_2_Core>
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

#ifdef OPENGL32
class TextureWriter  : protected QOpenGLFunctions_3_2_Core
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
