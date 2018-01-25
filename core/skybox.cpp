#include "skybox.h"
#include "options.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA

#include "stb_image.h"

extern Options opts;

SkyBox::SkyBox(QOpenGLShaderProgram *prog)
{
//    initializeOpenGLFunctions();

//    program = prog;

//    GLfloat skyboxVertices[] = {
//            // Positions
//            -1.0f,  1.0f, -1.0f,
//            -1.0f, -1.0f, -1.0f,
//             1.0f, -1.0f, -1.0f,
//             1.0f, -1.0f, -1.0f,
//             1.0f,  1.0f, -1.0f,
//            -1.0f,  1.0f, -1.0f,

//            -1.0f, -1.0f,  1.0f,
//            -1.0f, -1.0f, -1.0f,
//            -1.0f,  1.0f, -1.0f,
//            -1.0f,  1.0f, -1.0f,
//            -1.0f,  1.0f,  1.0f,
//            -1.0f, -1.0f,  1.0f,

//             1.0f, -1.0f, -1.0f,
//             1.0f, -1.0f,  1.0f,
//             1.0f,  1.0f,  1.0f,
//             1.0f,  1.0f,  1.0f,
//             1.0f,  1.0f, -1.0f,
//             1.0f, -1.0f, -1.0f,

//            -1.0f, -1.0f,  1.0f,
//            -1.0f,  1.0f,  1.0f,
//             1.0f,  1.0f,  1.0f,
//             1.0f,  1.0f,  1.0f,
//             1.0f, -1.0f,  1.0f,
//            -1.0f, -1.0f,  1.0f,

//            -1.0f,  1.0f, -1.0f,
//             1.0f,  1.0f, -1.0f,
//             1.0f,  1.0f,  1.0f,
//             1.0f,  1.0f,  1.0f,
//            -1.0f,  1.0f,  1.0f,
//            -1.0f,  1.0f, -1.0f,

//            -1.0f, -1.0f, -1.0f,
//            -1.0f, -1.0f,  1.0f,
//             1.0f, -1.0f, -1.0f,
//             1.0f, -1.0f, -1.0f,
//            -1.0f, -1.0f,  1.0f,
//             1.0f, -1.0f,  1.0f
//        };


//    QVector<QString> faces;
//    faces.push_back(opts.skyboxright);
//    faces.push_back(opts.skyboxleft);
//    faces.push_back(opts.skyboxup);
//    faces.push_back(opts.skyboxdown);
//    faces.push_back(opts.skyboxback);
//    faces.push_back(opts.skyboxfront);

//    loadCubemap(faces);

//    qDebug() << QString("ID cubemapTexture = %1").arg(cubemapTexture);

//    glGenVertexArrays(1, &skyboxVAO);
//    glGenBuffers(1, &skyboxVBO);
//    glBindVertexArray(skyboxVAO);
//    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
//    glBindVertexArray(0);
}

SkyBox::~SkyBox()
{

}

void SkyBox::render(QMatrix4x4 projection, QMatrix4x4 rot)
{

//    program->bind();
//    // Draw skybox first
//    glDepthMask(GL_FALSE);// Remember to turn depth writing off

//    //QMatrix4x4 view = modelview.
//    //glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
//    //glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
//    //glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
//    program->setUniformValue("view", rot);
//    //glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
//    program->setUniformValue("projection", projection);
//    // skybox cube
//    glBindVertexArray(skyboxVAO);
//    glActiveTexture(GL_TEXTURE0);
//    //glUniform1i(glGetUniformLocation(shader.Program, "skybox"), 0);
//    program->setUniformValue("skybox", 0);
//    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
//    glDrawArrays(GL_TRIANGLES, 0, 36);
//    glBindVertexArray(0);
//    glDepthMask(GL_TRUE);
//    program->release();


}

void SkyBox::loadCubemap(QVector<QString> faces)
{


    int w, h, n;

    // right
    unsigned char *x = stbi_load(faces.at(0).toLatin1(), &w, &h, &n, 4);
    if(x) qDebug() << "SkyBox::loadCubemap x loaded" ; else qDebug() << "---> x not loaded !";
    // left
    unsigned char *_x = stbi_load(faces.at(1).toLatin1(), &w, &h, &n, 4);
    if(_x) qDebug() << "SkyBox::loadCubemap _x loaded" ; else qDebug() << "---> _x not loaded !";
    // up
    unsigned char *y = stbi_load(faces.at(2).toLatin1(), &w, &h, &n, 4);
    if(y) qDebug() << "SkyBox::loadCubemap y loaded" ; else qDebug() << "---> y not loaded !";
    // down
    unsigned char *_y = stbi_load(faces.at(3).toLatin1(), &w, &h, &n, 4);
    if(_y) qDebug() << "SkyBox::loadCubemap _y loaded" ; else qDebug() << "---> _y not loaded !";
    // back
    unsigned char *z = stbi_load(faces.at(4).toLatin1(), &w, &h, &n, 4);
    if(z) qDebug() << "SkyBox::loadCubemap z loaded" ; else qDebug() << "---> z not loaded !";
    // front
    unsigned char *_z = stbi_load(faces.at(5).toLatin1(), &w, &h, &n, 4);
    if(_z) qDebug() << "SkyBox::loadCubemap _z loaded" ; else qDebug() << "---> _z not loaded !";


    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*) x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*) y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*) z);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*) _x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*) _y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*) _z);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

    if (x) free(x);
    if (_x) free(_x);
    if (y) free(y);
    if (_y) free(_y);
    if (z) free(z);
    if (_z) free(_z);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


}

