#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <vector>
#include <array>
#include <iostream>
#include <fstream>
#include <memory>
#include <GLFW/glfw3.h>

#include "shader/shaderManager.h"
#include "font/fontRenderer.h"
#include "shapes/rect.h"
#include "shapes/shape.h"
#include "shapes/triangle.h"

using std::vector, std::unique_ptr, std::make_unique, glm::ortho, glm::mat4, glm::vec3, glm::vec4 ;

/**
 * @brief The Engine class.
 * @details The Engine class is responsible for initializing the GLFW window, loading shaders, and rendering the game state.
 */
class Engine {
    private:
        /// @brief The actual GLFW window.
        GLFWwindow* window{};

        /// @brief The width and height of the window.
        const unsigned int width = 800, height = 600; // Window dimensions

        /// @brief Keyboard state (True if pressed, false if not pressed).
        /// @details Index this array with GLFW_KEY_{key} to get the state of a key.
        bool keys[1024];

        /// @brief Responsible for loading and storing all the shaders used in the project.
        /// @details Initialized in initShaders()
        unique_ptr<ShaderManager> shaderManager;
        unique_ptr<FontRenderer> fontRenderer;

        unique_ptr<Rect> ground;
        unique_ptr<Rect> ground2;
        // the ground that the user is standing on
        unique_ptr<Rect> standingGround;
        // What the user needs to jump over.
        vector<unique_ptr<Triangle>> spikes;
        // What the user needs to avoid in the sky.
        vector<unique_ptr<Rect>> clouds;

        // Shapes
        vector<unique_ptr<Shape>> squares;

        // Shaders
        Shader shapeShader;
        Shader textShader;

        double MouseX, MouseY;

        const int SIDE_LENGTH = 5;

        float speed = -1.5;

        float startDelay = 1;

        float delayTime = 0;

    public:
        /// @brief Constructor for the Engine class.
        /// @details Initializes window and shaders.
        Engine();

        /// @brief Destructor for the Engine class.
        ~Engine();

        /// @brief Initializes the GLFW window.
        /// @return 0 if successful, -1 otherwise.
        unsigned int initWindow(bool debug = false);

        /// @brief Loads shaders from files and stores them in the shaderManager.
        /// @details Renderers are initialized here.
        void initShaders();

        /// @brief Initializes the shapes to be rendered.
        void initShapes();

        /// @brief Processes input from the user.
        /// @details (e.g. keyboard input, mouse input, etc.)
        void processInput();

        /// @brief method that simulates the character flying while the user
        /// presses the space button.
        void fly();


        /// @brief method called that simulates the character falling after flying
        void fall();

        void checkForCollisions();
        /// @brief Updates the game state.
        /// @details (e.g. collision detection, delta time, etc.)
        void update();


        /// @brief Populates squares vector with input from file.
        void readFromFile(string filepath);

        /// @brief Renders the game state.
        /// @details Displays/renders objects on the screen.
        void render();

        /* deltaTime variables */
        float deltaTime = 0.0f; // Time between current frame and last frame
        float lastFrame = 0.0f; // Time of last frame (used to calculate deltaTime)

        /// @brief Returns true if the window should close.
        /// @details (Wrapper for glfwWindowShouldClose()).
        /// @return true if the window should close
        /// @return false if the window should not close
        bool shouldClose();

        /// Projection matrix used for 2D rendering (orthographic projection).
        /// We don't have to change this matrix since the screen size never changes.
        /// OpenGL uses the projection matrix to map the 3D scene to a 2D viewport.
        /// The projection matrix transforms coordinates in the camera space into normalized device coordinates (view space to clip space).

        /// @note The projection matrix is used in the vertex shader.
        // 4th quadrant
        const mat4 PROJECTION = ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), -1.0f, 1.0f);
        // 1st quadrant
//        mat4 PROJECTION = ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

        /// @brief Debug function to check for OpenGL errors.
        GLenum glCheckError_(const char *file, int line);
        /// @brief Macro for glCheckError_ function. Used for debugging.
        #define glCheckError() glCheckError_(__FILE__, __LINE__)
};

#endif //GRAPHICS_ENGINE_H
