#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

int main() {

    // init glfw and GL context
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // 3.3 or 4.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow *window = NULL;
    window = glfwCreateWindow(640, 480, "Hello OpenGL", NULL, NULL);
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    GLenum ret = glewInit();
    if ( ret != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(ret));
    }

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

    while (!glfwWindowShouldClose(window)) {
        // wipe the drawing surface clear
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_prog);
        glBindVertexArray(vao);
        // draw points 0-3 from the currently bound VAO with current in-use shader
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // update other events like input handling
        glfwPollEvents();
        // put the stuff we've been drawing onto the display
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
