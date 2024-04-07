#include "engine.h"
#include <iostream>

const color background(204/255.0, 230/255.0, 255/255.0);
const color mountainColorLighter(179/255.0, 218/255.0, 255/255.0); //
const color mountainColorDarker(128/255.0, 193/255.0, 255/255.0);
const color groundColor (0/255.0, 20/255.0, 77/255.0);
const color groundColor2 (0/255.0, 92/255.0, 179/255.0);


Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();
}

Engine::~Engine() {}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(width, height, "engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag",  nullptr, "shape");

    // Set uniforms that never change
    shapeShader.use();
    shapeShader.setMatrix4("projection", this->PROJECTION);
}

void Engine::initShapes() {
    user = make_unique<Rect>(shapeShader, vec2(width/2, height/2), vec2(100, 50), color{1, 0, 0, 1}); // placeholder for compilation

    // Ground color
    ground = make_unique<Rect>(shapeShader, vec2(width/2, 20), vec2(width, height / 3), groundColor);

    // Ground color
    ground2 = make_unique<Rect>(shapeShader, vec2(width/2, 50), vec2(width, height / 3), groundColor2);

    int totalMountainWidth = 0;
    vec2 mountainSize;
    while (totalMountainWidth < width + 100) {
        mountainSize.y = rand() % 101 + 200;
        mountainSize.x = rand() % 101 + 500;
        mountains.push_back(make_unique<Triangle>(shapeShader,
                                               vec2(totalMountainWidth + (mountainSize.x / 2.0) + 5,
                                                    ((mountainSize.y / 2.0) + 50)),
                                                  mountainSize, mountainColorLighter));
        totalMountainWidth += mountainSize.x + 5;
    }

    totalMountainWidth = 0;
    while (totalMountainWidth < width + 200) {
        mountainSize.y = rand() % 101 + 300;
        mountainSize.x = rand() % 101 + 600;
        mountains2.push_back(make_unique<Triangle>(shapeShader,
                                                  vec2(totalMountainWidth + (mountainSize.x / 2.0) + 5,
                                                       ((mountainSize.y / 2.0) + 50)),
                                                   mountainSize, mountainColorDarker));
        totalMountainWidth += mountainSize.x + 5;
    }
}

void Engine::processInput() {
    glfwPollEvents();

    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    float speed = 200.0f * deltaTime;
    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);

    // Update mouse rect to follow mouse
    MouseY = height - MouseY; // make sure mouse y-axis isn't flipped



    // If the user is overlapping with the top of the mountain,
    //  exit the program.
    for (const unique_ptr<Triangle>& m : mountains) {
        if (m->isOverlapping(*user)) {
            glfwSetWindowShouldClose(window, true);
        }
    }
}

void Engine::generateLand() {

}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Have the mountains move
    for (int i = 0; i < mountains.size(); ++i) {
        mountains[i]->moveX(-.5);
        // If a mountain has moved off the screen
        if (mountains[i]->getPosX() < -(mountains[i]->getSize().x/2)) {
            // Set it to the right of the screen so that it passes through again
            int mountainOnLeft = (mountains[i] == mountains[0]) ? mountains.size()-1 : i - 1;
            mountains[i]->setPosX(mountains[mountainOnLeft]->getPosX() + mountains[mountainOnLeft]->getSize().x/2 + mountains[i]->getSize().x/2 + 5);
        }
    }

    for (int i = 0; i < mountains2.size(); ++i) {
        mountains2[i]->moveX(-.5/2);
        // If a mountain has moved off the screen
        if (mountains2[i]->getPosX() < -(mountains2[i]->getSize().x/2)) {
            // Set it to the right of the screen so that it passes through again
            int mountainOnLeft = (mountains2[i] == mountains2[0]) ? mountains2.size()-1 : i - 1;
            mountains2[i]->setPosX(mountains2[mountainOnLeft]->getPosX() + mountains2[mountainOnLeft]->getSize().x/2 + mountains2[i]->getSize().x/2 + 5);
        }
    }

    // Need to make standing ground randomly generate

}

void Engine::render() {
    glClearColor(background.red,background.green, background.blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (const unique_ptr<Triangle>& m2 : mountains2) {
        m2->setUniforms();
        m2->draw();
    }

    for (const unique_ptr<Triangle>& m : mountains) {
        m->setUniforms();
        m->draw();
    }

    ground2->setUniforms();
    ground2->draw();

    ground->setUniforms();
    ground->draw();

    for (const unique_ptr<Rect>& g : standingGround) {
        g->setUniforms();
        g->draw();
    }

    user->setUniforms();
    user->draw();

    glfwSwapBuffers(window);
}

bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}