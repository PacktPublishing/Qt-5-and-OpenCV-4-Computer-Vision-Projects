#include <stdio.h>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"

#include <GL/glew.h>

struct DrawData
{
    GLuint vao;
    GLuint shader_prog;
};

void draw(void* userdata)
{
    DrawData* data = static_cast<DrawData*>(userdata);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(data->shader_prog);
    glBindVertexArray(data->vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

int main() {
    cv::namedWindow("OpenGL", cv::WINDOW_OPENGL);
    cv::resizeWindow("OpenGL", 640, 480);

    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    GLenum ret = glewInit();
    if (ret != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(ret));
    }

    // opencv don't always choose the latest version of OpenGL,
    // and we have no chance to specify one,
    // so here it doesn't always use the core profile.
    printf("Using OpenGL: %s.\n", (const char*)glGetString(GL_VERSION));

    // vao and vbo
    GLuint vao, vbo;
    GLfloat points[] = {+0.0f, +0.5f, +0.0f,
                        +0.5f, -0.5f, +0.0f,
                        -0.5f, -0.5f, +0.0f };
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), points, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // shader and shader program
    GLuint vert_shader, frag_shader;
    GLuint shader_prog;
    const char *vertex_shader_code = "#version 330\n"
        "in vec3 vp;"
        "void main () {"
        "  gl_Position = vec4(vp, 1.0);"
        "}";

    const char *fragment_shader_code = "#version 330\n"
        "out vec4 frag_colour;"
        "void main () {"
        "  frag_colour = vec4(0.5, 1.0, 0.5, 1.0);"
        "}";

    vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vertex_shader_code, NULL);
    glCompileShader(vert_shader);

    frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &fragment_shader_code, NULL);
    glCompileShader(frag_shader);

    shader_prog = glCreateProgram();
    glAttachShader(shader_prog, frag_shader);
    glAttachShader(shader_prog, vert_shader);
    glLinkProgram(shader_prog);

    DrawData data;
    data.vao = vao;
    data.shader_prog =shader_prog;

    cv::setOpenGlDrawCallback("OpenGL", draw, &data);

    while (true) {
        cv::updateWindow("OpenGL");
        char key = (char)cv::waitKey(50);
        if (key == 27)
            break;
    }

    cv::setOpenGlDrawCallback("OpenGL", 0, 0);
    cv::destroyAllWindows();

    return 0;
}
