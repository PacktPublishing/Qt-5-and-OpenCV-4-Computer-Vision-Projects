#include <string>

#include <QDebug>
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

    qDebug() << "GL Version:" << QString((const char*)glGetString(GL_VERSION));

    GLfloat points[] = {+0.0f, +0.5f, +0.0f,
                        +0.5f, -0.5f, +0.0f,
                        -0.5f, -0.5f, +0.0f };

#ifndef USE_QTGL_API
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // sizeof(points) = 9 * sizeof(GLfloat)
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

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

#else

    vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    Q_ASSERT(vbo.create());
    Q_ASSERT(vbo.bind());
    vbo.allocate(sizeof(points));
    vbo.write(0, points, sizeof(points));

    const int vPosition = 0;
    vao.create();
    vao.bind();
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    shaderProg.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertex.shader");
    shaderProg.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragment.shader");
    Q_ASSERT(shaderProg.link());
    Q_ASSERT(shaderProg.bind());

#endif

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(0.1, 0.1, 0.2, 1.0);
}

void GLPanel::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

#ifndef USE_QTGL_API
    glUseProgram(shaderProg);
    glBindVertexArray(vao);
#else
    shaderProg.bind();
    vao.bind();
#endif
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glFlush();
}

void GLPanel::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}
