#include "ShaderClass.hpp"

#include <GLFW/glfw3.h>

#include <array>
#include <chrono>
#include <ctime>
#include <iostream>
#include <string_view>
#include <thread>

constexpr std::size_t window_width{ 400 };
constexpr std::size_t window_height{ 400 };

void process_input(GLFWwindow* window);
constexpr glm::vec3 hex2vec3(std::string_view hex);

std::array<GLfloat, 12> quad_vertices{
     1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f
};

std::array<GLuint, 6> quad_indices{
    0, 1, 2,
    3, 0, 2
};

std::array<GLfloat, 27> hand_vertices{
    // seconds hand
    -0.04f, -0.04f, 0.0f,
     0.04f, -0.04f, 0.0f,
     0.0f,   0.8f,  0.0f,

     // minutes hand
     -0.04f, -0.04f, 0.0f,
      0.04f, -0.04f, 0.0f,
      0.0f,   0.6f,  0.0f,

      // hours hand
      -0.04f, -0.04f, 0.0f,
       0.04f, -0.04f, 0.0f,
       0.0f,   0.4f,  0.0f
};

std::int32_t main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    ////////////////////////////////////////////////////////////////////////////

    GLFWwindow* window{ glfwCreateWindow(window_width, window_height, "clock",
        nullptr, nullptr) };

    if (!window)
    {
        std::cerr << "Error: Unable to initialize GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Error: Unable to initiailze glad\n";
        glfwTerminate();
        return -1;
    }

    ////////////////////////////////////////////////////////////////////////////

    ShaderProgram circle_program{ "circle-vertex.glsl",
        "circle-fragment.glsl" };
    ShaderProgram triangle_program{ "triangle-vertex.glsl",
        "triangle-fragment.glsl" };

    ////////////////////////////////////////////////////////////////////////////

    GLuint VAOs[2]{}, VBOs[2]{}, EBO{};
    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, VBOs);
    glGenBuffers(1, &EBO);

    ////////////////////////////////////////////////////////////////////////////

    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, quad_vertices.size() * sizeof(GLfloat),
        quad_vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3 * sizeof(GLfloat)),
        (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quad_indices.size() * sizeof(GLuint),
        quad_indices.data(), GL_STATIC_DRAW);

    ////////////////////////////////////////////////////////////////////////////

    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, hand_vertices.size() * sizeof(GLfloat),
        hand_vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3 * sizeof(GLfloat)),
        (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    ////////////////////////////////////////////////////////////////////////////

    glm::vec3 circle_color{ hex2vec3("fbf1c7") };
    float radius{ 0.9f };
    float line_length{ 0.75f };

    glm::vec3 triangle_colors[3]{ hex2vec3("cc241d"),
                                  hex2vec3("8ec07c"),
                                  hex2vec3("fabd2f") };

    glm::mat4 model{ 1.0f };
    auto clear_color{ hex2vec3("1d2021") };

    ////////////////////////////////////////////////////////////////////////////

    glEnable(GL_MULTISAMPLE);

    while (!glfwWindowShouldClose(window))
    {
        double time_begin{ glfwGetTime() };

        process_input(window);

        glClearColor(clear_color.x, clear_color.y, clear_color.z, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ////////////////////////////////////////////////////////////////////////

        circle_program.activate_program();
        circle_program.set_mat4("model", model);
        circle_program.set_vec3("circle_color", circle_color);
        circle_program.set_float("radius", radius);
        circle_program.set_float("line_length", line_length);

        glBindVertexArray(VAOs[0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        ////////////////////////////////////////////////////////////////////////

        time_t now_time = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        tm local_tm = *std::localtime(&now_time);

        float sec_degrees = ((float)local_tm.tm_sec / 60) * 360;
        float min_degrees = ((float)local_tm.tm_min / 60) * 360 +
            ((sec_degrees / 360) * 5);
        float hour_degrees = ((float)local_tm.tm_hour / 12) * 360 +
            ((min_degrees / 360) * 29);

        ////////////////////////////////////////////////////////////////////////

        glBindVertexArray(VAOs[1]);

        triangle_program.activate_program();

        auto draw_hand =
            [&triangle_program, &triangle_colors, &model](int index,
                float rotate, int begin, int end) {
                    triangle_program.set_vec3("triangle_color",
                        triangle_colors[index]);

                    model = glm::rotate(model, rotate,
                        glm::vec3{ 0.0f, 0.0f, -1.0f });

                    triangle_program.set_mat4("model", model);
                    model = glm::mat4{ 1.0f };

                    glDrawArrays(GL_TRIANGLES, begin, end);
        };

        draw_hand(0, glm::radians(sec_degrees), 0, 3);
        draw_hand(1, glm::radians(min_degrees), 3, 6);
        draw_hand(2, glm::radians(hour_degrees), 6, 9);

        ////////////////////////////////////////////////////////////////////////

        glfwSwapBuffers(window);

        double time_end{ glfwGetTime() };

        // Draw 5 times per second
        if ((time_begin - time_end * 1000) < 200.0f)
        {
            std::this_thread::sleep_for(
                std::chrono::duration<double, std::milli>(
                    200.0f - ((time_begin - time_end) * 1000)
                    )
            );
        }

        glfwPollEvents();

        ////////////////////////////////////////////////////////////////////////
    }

    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

void process_input(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

constexpr glm::vec3 hex2vec3(std::string_view hex)
{
    auto to_int = [](char ch) {
        switch (ch) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'a': return 10;
        case 'b': return 11;
        case 'c': return 12;
        case 'd': return 13;
        case 'e': return 14;
        case 'f': return 15;
        default: return -1;
        }
    };

    float red =
        static_cast<float>((to_int(hex[0]) * 16) + to_int(hex[1])) / 255;
    float green =
        static_cast<float>((to_int(hex[2]) * 16) + to_int(hex[3])) / 255;
    float blue =
        static_cast<float>((to_int(hex[4]) * 16) + to_int(hex[5])) / 255;

    return { red, green, blue };
}