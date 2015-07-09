#include "skybox.h"
#include "SOIL.h"
#include "options.h"

extern Options opts;

SkyBox::SkyBox(QOpenGLShaderProgram *prog)
{
    initializeOpenGLFunctions();

    program = prog;

    GLfloat skyboxVertices[] = {
            // Positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };


    QVector<QString> faces;
    faces.push_back(opts.skyboxright);
    faces.push_back(opts.skyboxleft);
    faces.push_back(opts.skyboxup);
    faces.push_back(opts.skyboxdown);
    faces.push_back(opts.skyboxback);
    faces.push_back(opts.skyboxfront);
    cubemapTexture = loadCubemap(faces);

    qDebug() << QString("ID cubemapTexture = %1").arg(cubemapTexture);

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);
}

SkyBox::~SkyBox()
{

}

void SkyBox::render(QMatrix4x4 projection, QMatrix4x4 rot)
{

    program->bind();
    // Draw skybox first
    glDepthMask(GL_FALSE);// Remember to turn depth writing off

    //QMatrix4x4 view = modelview.
    //glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
    //glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
    //glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    program->setUniformValue("view", rot);
    //glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    program->setUniformValue("projection", projection);
    // skybox cube
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    //glUniform1i(glGetUniformLocation(shader.Program, "skybox"), 0);
    program->setUniformValue("skybox", 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    program->release();


}

GLuint SkyBox::loadCubemap(QVector<QString> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glActiveTexture(GL_TEXTURE0);

//    int width,height;
//    unsigned char* image;

//    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
//    for(GLuint i = 0; i < faces.size(); i++)
//    {
//        image = SOIL_load_image(faces.at(i).toLatin1(), &width, &height, 0, SOIL_LOAD_RGB);
//        glTexImage2D(
//            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
//            GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
//        );
//    }


    /**
        Loads 6 images from memory into an OpenGL cubemap texture.
        \param x_pos_buffer the image data in RAM to upload as the +x cube face
        \param x_pos_buffer_length the size of the above buffer
        \param x_neg_buffer the image data in RAM to upload as the +x cube face
        \param x_neg_buffer_length the size of the above buffer
        \param y_pos_buffer the image data in RAM to upload as the +x cube face
        \param y_pos_buffer_length the size of the above buffer
        \param y_neg_buffer the image data in RAM to upload as the +x cube face
        \param y_neg_buffer_length the size of the above buffer
        \param z_pos_buffer the image data in RAM to upload as the +x cube face
        \param z_pos_buffer_length the size of the above buffer
        \param z_neg_buffer the image data in RAM to upload as the +x cube face
        \param z_neg_buffer_length the size of the above buffer
        \param force_channels 0-image format, 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
        \param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
        \param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
        \return 0-failed, otherwise returns the OpenGL texture handle
    **/


//    QImage x_pos_buffer(faces.at(0));
//    QImage x_neg_buffer(faces.at(1));
//    QImage y_pos_buffer(faces.at(2));
//    QImage y_neg_buffer(faces.at(3));
//    QImage z_neg_buffer(faces.at(4));
//    QImage z_pos_buffer(faces.at(5));


//    GLuint tex_cube = SOIL_load_OGL_cubemap_from_memory
//            (
//                x_pos_buffer.bits(),
//                x_pos_buffer.byteCount(),
//                x_neg_buffer.bits(),
//                x_neg_buffer.byteCount(),

//                y_pos_buffer.bits(),
//                y_pos_buffer.byteCount(),
//                y_neg_buffer.bits(),
//                y_neg_buffer.byteCount(),

//                z_pos_buffer.bits(),
//                z_pos_buffer.byteCount(),
//                z_neg_buffer.bits(),
//                z_neg_buffer.byteCount(),
//                4,
//                0,
//                SOIL_FLAG_MIPMAPS
//            );



//            const unsigned char *const x_pos_buffer,
//            int x_pos_buffer_length,
//            const unsigned char *const x_neg_buffer,
//            int x_neg_buffer_length,
//            const unsigned char *const y_pos_buffer,
//            int y_pos_buffer_length,
//            const unsigned char *const y_neg_buffer,
//            int y_neg_buffer_length,
//            const unsigned char *const z_pos_buffer,
//            int z_pos_buffer_length,
//            const unsigned char *const z_neg_buffer,
//            int z_neg_buffer_length,

//            int force_channels,
//            unsigned int reuse_texture_ID,
//            unsigned int flags
//        );


    GLuint tex_cube = SOIL_load_OGL_cubemap
        (
            faces.at(0).toLatin1(),
            faces.at(1).toLatin1(),
            faces.at(2).toLatin1(),
            faces.at(3).toLatin1(),
            faces.at(4).toLatin1(),
            faces.at(5).toLatin1(),
            SOIL_LOAD_RGB,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_MIPMAPS
        );


    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return tex_cube;
}
