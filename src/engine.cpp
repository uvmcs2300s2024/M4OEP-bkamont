#include "engine.h"
#include <iostream>

using namespace std;

const color background(204/255.0, 230/255.0, 255/255.0);
const color mountainColorLighter(179/255.0, 218/255.0, 255/255.0); //
const color mountainColorDarker(128/255.0, 193/255.0, 255/255.0);
const color groundColor (0/255.0, 20/255.0, 77/255.0);
const color groundColor2 (0/255.0, 92/255.0, 179/255.0);
const color black (0/255.0, 0/255.0, 0/255.0);


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
    user = make_unique<Rect>(shapeShader, vec2(100 , height/2), vec2(100, 50), black); // placeholder for compilation
    // Ground color
    ground = make_unique<Rect>(shapeShader, vec2(width/2, 20), vec2(width, height / 3), groundColor);

    // Ground color
    ground2 = make_unique<Rect>(shapeShader, vec2(width/2, 50), vec2(width, height / 3), groundColor2);

    int totalGroundWidth = 0;
    vec2 groundSize;
    while (totalGroundWidth < width + 400) {
        groundSize.y = rand() % 20 + 21;
        groundSize.x = rand() % 15 + 30;
        standingGround.push_back(make_unique<Rect>(shapeShader,
                                                  vec2(totalGroundWidth + (groundSize.x / 2.0) + 100,
                                                       ((groundSize.y / 2.0) + 100)),
                                                  groundSize, mountainColorLighter));
        totalGroundWidth += groundSize.x + 5;
    }

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

    readFromFile("../res/art/scene.txt");
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

    int y = 0;

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);

    // Update mouse rect to follow mouse
    MouseY = height - MouseY; // make sure mouse y-axis isn't flipped


    // Get the regular position of the unicorn
    float regPos = squares[0]->getPosY();

    // if the user hits the up arrow the unicorn jumps
    if(keys[GLFW_KEY_UP]){
        for(int i = 0; i < squares.size(); i++){
            if(squares[0]->getPosY() < 500){
                jump();
            }
        }
    }

    if(keys[GLFW_KEY_DOWN]){
        while(squares[0]->getPosY() > regPos){
            fall();
        }
    }

    // If the user is overlapping with the top of the mountain,
    //  exit the program.
    for (const unique_ptr<Triangle>& m : mountains) {
        if (m->isOverlapping(*user)) {
            glfwSetWindowShouldClose(window, true);
        }
    }
}

void Engine::jump() {
    // Loop the move position of squares
    int i  = 0;
    while(i < squares.size()){
        squares[i]->setPosY(squares[i]->getPosY() + 2);
        i++;
    }
}

void Engine::fall(){
    // Loop the move position of squares
    int i  = 0;
    while(i < squares.size()){
        squares[i]->setPosY(squares[i]->getPosY() - 2);
        i++;
    }
}


void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // TODO need to fix something with the mountains. They randomly cause the program to shut down after a while.
    /*
    // Have the mountains move
    for (int i = 0; i < mountains.size(); ++i) {
        mountains[i]->moveX(-.5);
        // If a mountain has moved off the screen
        if (mountains[i]->getPosX() < -(mountains[i]->getSize().x/2)) {
            int s = rand() % 30;
            // Set it to the right of the screen so that it passes through again
            int mountainOnLeft = (mountains[i] == mountains[0]) ? mountains.size()-1 : i - 1;
            mountains[i]->setPosX(mountains[mountainOnLeft]->getPosX() + mountains[mountainOnLeft]->getSize().x/s + mountains[i]->getSize().x/s + s);
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
     */

}

void Engine::readFromFile(std::string filepath) {
    ifstream ins(filepath);

    if (!ins) {
        cout << "Error opening file" << endl;
    }
    ins >> std::noskipws;
    int xCoord = 0, yCoord = height;
    char letter;
    bool draw;
    color c;
    while (ins >> letter) {
        draw = true;
        switch(letter) {
            case ' ': draw = false; xCoord += SIDE_LENGTH; break;
            case 'r': c = color(1, 0, 0); break;
            case 'g': c = color(40/255.0, 193/255.0, 249/255.0); break;
            case 'b': c = color(0, 0, 1); break;
            case 'y': c = color(1, 1, 0); break;
            case 'm': c = color(1, 0, 1); break;
            case 'c': c = color(0, 1, 1); break;
            case 'w': c = color(1, 1, 1); break;
            case 'k': c = color(0, 0, 0); break;
            default: // newline
                draw = false;
                xCoord = width/9;
                yCoord -= SIDE_LENGTH;
        }
        if (draw) {
            squares.push_back(make_unique<Rect>(shapeShader, vec2(xCoord + SIDE_LENGTH/2, yCoord + SIDE_LENGTH/2), vec2(SIDE_LENGTH, SIDE_LENGTH), c));
            xCoord += SIDE_LENGTH;
        }
    }
    ins.close();
}

void Engine::render() {
    glClearColor(background.red,background.green, background.blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (const unique_ptr<Triangle>& m2 : mountains2) {
        m2->setUniforms();
        m2->draw();
    }
    /*

    for (const unique_ptr<Triangle>& m : mountains) {
        m->setUniforms();
        m->draw();
    }*/

    ground2->setUniforms();
    ground2->draw();

    ground->setUniforms();
    ground->draw();

    shapeShader.use();

    for(unique_ptr<Shape> &square : squares){
        square->setUniforms();
        square->draw();
    }

    glfwSwapBuffers(window);
}


bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}