#include "engine.h"
#include <iostream>

const color skyBlue(77/255.0, 213/255.0, 240/255.0);
const color grassGreen(26/255.0, 176/255.0, 56/255.0);
const color darkGreen(27/255.0, 81/255.0, 45/255.0);
const color white(1, 1, 1);
const color brickRed(201/255.0, 20/255.0, 20/255.0);
const color darkBlue(1/255.0, 110/255.0, 214/255.0);
const color purple(119/255.0, 11/255.0, 224/255.0);
const color black(0, 0, 0);
const color magenta(1, 0, 1);
const color orange(1, 163/255.0, 22/255.0);
const color cyan (0, 1, 1);

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
    // TODO: Initialize the user to be a 10x10 white block
    //       centered at (0, 0)
    user = make_unique<Rect>(shapeShader, vec2(width, height), vec2(0, 0), black); // placeholder for compilation

    // Init grass
    grass = make_unique<Rect>(shapeShader, vec2(width/2, 50), vec2(width, height / 3), grassGreen);

    // Init mountains
    mountains.push_back(make_unique<Triangle>(shapeShader, vec2(width/4, 300), vec2(width, 400), darkGreen));
    mountains.push_back(make_unique<Triangle>(shapeShader, vec2(2*width/3, 300), vec2(width, 500), darkGreen));

    // Init buildings from closest to furthest
    int totalBuildingWidth = 0;
    vec2 buildingSize;
    while (totalBuildingWidth < width + 50) {
        // Building height between 50-100
        buildingSize.y = rand() % 51 + 50;
        // Building width between 30-50
        buildingSize.x = rand() % 21 + 30;
        buildings1.push_back(make_unique<Rect>(shapeShader,
                                               vec2(totalBuildingWidth + (buildingSize.x / 2.0) + 5,
                                                    ((buildingSize.y / 2.0) + 50)),
                                               buildingSize, brickRed));
        totalBuildingWidth += buildingSize.x + 5;
    }
    // Populate second set of buildings
    totalBuildingWidth = 0;
    while (totalBuildingWidth < width + 100) {
        // TODO: Populate this vector of darkBlue buildings
        // Building height between 100-200
        // Building width between 50-100
        totalBuildingWidth += buildingSize.x + 5;
    }
    // Populate third set of buildings
    totalBuildingWidth = 0;
    while (totalBuildingWidth < width + 200) {
        // TODO: Populate this vector of purple buildings
        // Building height between 200-400
        // Building width between 100-200
        totalBuildingWidth += buildingSize.x + 5;
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

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);

    // Update mouse rect to follow mouse
    MouseY = height - MouseY; // make sure mouse y-axis isn't flipped

    // TODO: make the user move with the mouse

    for (const unique_ptr<Rect>& r : buildings1) {
        if (r->isOverlapping(*user)) {
            r->setColor(orange);
        } else {
            r->setColor(brickRed);
        }
    }

    // TODO: Update the colors of buildings2 and buildings3.
    //  Note that darkBlue buildings turn cyan when overlapping
    //  with the user, and purple buildings turn magenta.


    // If the user is overlapping with the top of the mountain,
    //  exit the program.
    for (const unique_ptr<Triangle>& m : mountains) {
        if (m->isOverlapping(*user)) {
            glfwSetWindowShouldClose(window, true);
        }
    }
}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Update buildings
    for (int i = 0; i < buildings1.size(); ++i) {
        // Move all the red buildings to the left
        buildings1[i]->moveX(-1.5);
        // If a building has moved off the screen
        if (buildings1[i]->getPosX() < -(buildings1[i]->getSize().x/2)) {
            // Set it to the right of the screen so that it passes through again
            int buildingOnLeft = (buildings1[i] == buildings1[0]) ? buildings1.size()-1 : i - 1;
            buildings1[i]->setPosX(buildings1[buildingOnLeft]->getPosX() + buildings1[buildingOnLeft]->getSize().x/2 + buildings1[i]->getSize().x/2 + 5);
        }
    }

    // TODO: Make the other two vectors of buildings move.
    //  The larger the buildings, the slower they should move.

}

void Engine::render() {
    glClearColor(skyBlue.red,skyBlue.green, skyBlue.blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (const unique_ptr<Triangle>& m : mountains) {
        m->setUniforms();
        m->draw();
    }

    grass->setUniforms();
    grass->draw();

    // TODO: Add logic to draw the the user and the buildings.
    //  Note that the order of drawing matters because whatever
    //  is drawn last appears on top.


    glfwSwapBuffers(window);
}

bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}