#include <string>

#include <QDebug>
#include <QMainWindow>
#include <QString>
#include <QFile>

#include "glpanel.h"

std::string textContent(QString path) {
    QFile file(path);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&file);
    return in.readAll().toStdString();
}

GLPanel::GLPanel(QWidget *parent) :
    QOpenGLWidget(parent)
{
}

GLPanel::~GLPanel()
{
}


void GLPanel::initializeGL()
{
    initializeOpenGLFunctions();

    GLfloat points[] = {
                        // first triangle
                        +1.0f, +1.0f, +0.0f, +1.0f, +1.0f, // top-right
                        +1.0f, -1.0f, +0.0f, +1.0f, +0.0f, // bottom-right
                        -1.0f, -1.0f, +0.0f, +0.0f, +0.0f, // bottom-left
                        // second triangle
                        -1.0f, -1.0f, +0.0f, +0.0f, +0.0f, // bottom-left
                        -1.0f, +1.0f, +0.0f, +0.0f, +1.0f, // top-left
                        +1.0f, +1.0f, +0.0f, +1.0f, +1.0f // top-right
    };

    // VBA & VAO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // texture
    glEnable(GL_TEXTURE_2D);
    // 1. read the image data
    QImage img(":/images/lizard.jpg");
    img = img.convertToFormat(QImage::Format_RGB888).mirrored(false, true);
    // 2. generate texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
    glGenerateMipmap(GL_TEXTURE_2D);

    // shader and shader program
    GLuint vert_shader, frag_shader;

    std::string vertex_shader_str = textContent(":/shaders/vertex.shader");
    const char *vertex_shader_code = vertex_shader_str.data();

    std::string fragment_shader_str = textContent(":/shaders/fragment.shader");
    const char *fragment_shader_code = fragment_shader_str.data();

    vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vertex_shader_code, NULL);
    glCompileShader(vert_shader);

    frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &fragment_shader_code, NULL);
    glCompileShader(frag_shader);

    shaderProg = glCreateProgram();
    glAttachShader(shaderProg, frag_shader);
    glAttachShader(shaderProg, vert_shader);
    glLinkProgram(shaderProg);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(0.1, 0.1, 0.2, 1.0);

    ((QMainWindow*)this->parent())->resize(img.width(), img.height());
}

void GLPanel::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(shaderProg);
    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glFlush();
}

void GLPanel::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}
