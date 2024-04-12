#include "engine.h"
#include <iostream>

enum state {start, play, dead};
state screen;

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


    standingGround = make_unique<Rect>(shapeShader, vec2(width/2, 50), vec2(width, 430), groundColor);
/*    int totalGroundWidth = 0;
    vec2 groundSize;
    while (totalGroundWidth < width + 600) {
        // Populate this vector of darkBlue buildings
        groundSize.y = height/2;
        // Building width between
        groundSize.x = 600;

        standingGround.push_back(make_unique<Rect>(shapeShader,
                                               vec2(totalGroundWidth + (groundSize.x / 2.0) + 5, (groundSize.y/2.0) + -30),
                                               groundSize, groundColor));
        totalGroundWidth += groundSize.x + 5;
    }*/

    int totalBlockWidth = 0;
    vec2 blockSize;

    while (totalBlockWidth < width + 60) {
        // Populate this vector of purple buildings
        // Building height between 300-350
        blockSize.y = rand() % 301 + 50;
        // Building width between 20-60
        blockSize.x = rand() % 21 + 40;
        blocks.push_back(make_unique<Rect>(shapeShader,
                                               vec2(totalBlockWidth + (blockSize.x / 2.0) + 5,
                                                    ((blockSize.y / 2.0) + 50)),
                                               blockSize, groundColor));
        totalBlockWidth += blockSize.x + 5;
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

    // If we're in the start screen and the user presses s, change screen to play
    // Hint: The index is GLFW_KEY_S
    if ((screen == start) && keys[GLFW_KEY_S]){
        screen = play;
        switch(screen);
    }

    // Update mouse rect to follow mouse
    MouseY = height - MouseY; // make sure mouse y-axis isn't flipped


    // Get the regular position of the unicorn

    // if the user hits the up arrow the unicorn jumps
    if((screen == play) && keys[GLFW_KEY_UP]){
        if(squares[0]->getPosY() < 600){
            jump();
        }
    }

    if(squares[0]->getPosY() > 362){
        fall();
    }


    // If the user is overlapping with the top of the mountain,
    //  exit the program.
    for (const unique_ptr<Rect>& m : blocks) {
        if (m->isOverlapping(*user)) {
            glfwSetWindowShouldClose(window, true);
        }
    }
}

// Jump method to have the character jump
void Engine::jump() {
    // Loop the move position of squares
    int i  = 0;
    while(i < squares.size()){
        squares[i]->setPosY(squares[i]->getPosY() + 3);
        i++;
    }
}

// Fall method to have the character fall after jumping.
void Engine::fall(){
    // Loop the move position of squares
    int i  = 0;
    while(i < squares.size()){
        squares[i]->setPosY(squares[i]->getPosY() - 1.5);
        i++;
    }
}


void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

/*    // Update buildings
    for (int i = 0; i < standingGround.size(); ++i) {
        // Move all the red buildings to the left
        standingGround[i]->moveX(-1);
        // If a building has moved off the screen
        if (standingGround[i]->getPosX() < -(standingGround[i]->getSize().x/2)) {
            int pos = rand() % 51 + 100;
            // Set it to the right of the screen so that it passes through again
            int buildingOnLeft = (standingGround[i] == standingGround[0]) ? standingGround.size()-1 : i - 1;
            standingGround[i]->setPosX(standingGround[buildingOnLeft]->getPosX() + standingGround[buildingOnLeft]->getSize().x/2 + standingGround[i]->getSize().x/2);
        }
    }*/

    // Update buildings
    for (int i = 0; i < blocks.size(); ++i) {
        // Move all the red buildings to the left
        blocks[i]->moveX(-.5);
        // If a building has moved off the screen
        if (blocks[i]->getPosX() < -(blocks[i]->getSize().x/2)) {
            // Set it to the right of the screen so that it passes through again
            int buildingOnLeft = (blocks[i] == blocks[0]) ? blocks.size()-1 : i - 1;
            blocks[i]->setPosX(blocks[buildingOnLeft]->getPosX() + blocks[buildingOnLeft]->getSize().x/2 + blocks[i]->getSize().x/2 + 100);
        }
    }

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

    shapeShader.use();

    // Set shader to use for all shapes
    shapeShader.use();

    // Render differently depending on screen
    switch (screen) {
        case start: {
            break;
        }
        case play: {
            for(unique_ptr<Shape> &square : squares){
                square->setUniforms();
                square->draw();
            }

            standingGround->setUniforms();
            standingGround->draw();

            // Draw the ground
            for (int n=0; n < blocks.size(); n++) {
                blocks[n]->setUniforms();
                blocks[n]->draw();
            }

            break;
        }
        case dead: {
            // Display the message on the screen
            break;
        }
    }


    glfwSwapBuffers(window);
}


bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}