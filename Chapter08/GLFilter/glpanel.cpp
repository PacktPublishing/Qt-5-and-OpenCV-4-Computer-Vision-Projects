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
#ifdef USE_OPENCV
    img = cv::imread("./images/lizard.jpg");
    cv::Mat tmp;
    cv::flip(img, tmp, 0);
    cvtColor(tmp, img, cv::COLOR_BGR2RGB);
#else
    img = QImage(":/images/lizard.jpg");
    img = img.convertToFormat(QImage::Format_RGB888).mirrored(false, true);
#endif

    // 2. generate texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

#ifdef USE_OPENCV
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        img.cols, img.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data);
#else
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
#endif
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

    // scale ration
    glUseProgram(shaderProg);
    int pixel_scale_loc = glGetUniformLocation(shaderProg, "pixelScale");
#ifdef USE_OPENCV
    glUniform2f(pixel_scale_loc, 1.0f / img.cols, 1.0f / img.rows);
#else
    glUniform2f(pixel_scale_loc, 1.0f / img.width(), 1.0f / img.height());
#endif

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(0.1, 0.1, 0.2, 1.0);

#ifdef USE_OPENCV
    ((QMainWindow*)this->parent())->resize(img.cols, img.rows);
#else
    ((QMainWindow*)this->parent())->resize(img.width(), img.height());
#endif
}

void GLPanel::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(shaderProg);
    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glFlush();
    saveOutputImage("./output.jpg");
}

void GLPanel::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

void GLPanel::saveOutputImage(QString path)
{
#ifdef USE_OPENCV
    cv::Mat output(img.rows, img.cols, CV_8UC3);
    glReadPixels(
        0, 0, img.cols, img.rows, GL_RGB, GL_UNSIGNED_BYTE, output.data);
    cv::Mat tmp;
    cv::flip(output, tmp, 0);
    cvtColor(tmp, output, cv::COLOR_RGB2BGR);
    cv::imwrite(path.toStdString(), output);
#else
    QImage output(img.width(), img.height(), QImage::Format_RGB888);
    glReadPixels(
        0, 0, img.width(), img.height(), GL_RGB, GL_UNSIGNED_BYTE, output.bits());
    output = output.mirrored(false, true);
    output.save(path);
#endif
}
