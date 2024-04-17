#include "engine.h"
#include <iostream>

enum state {start,directions, play, dead};
state screen;

float currentTime;
float elapsedTime;

float passedTime;
float elapsedPassedTime;

using namespace std;

const color background(204/255.0, 230/255.0, 255/255.0);
const color mountainColorLighter(179/255.0, 218/255.0, 255/255.0); //
const color mountainColorDarker(128/255.0, 193/255.0, 255/255.0);
const color groundColor (0/255.0, 20/255.0, 77/255.0);
const color groundColor2 (0/255.0, 92/255.0, 179/255.0);
const color black (0/255.0, 0/255.0, 0/255.0);
const color white (255/255.0, 255/255.0, 255/255.0);


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

// TODO ask about the spikes spawning only when the user hits start.
void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag",  nullptr, "shape");

    // Configure text shader and renderer
    textShader = shaderManager->loadShader("../res/shaders/text.vert", "../res/shaders/text.frag", nullptr, "text");
    fontRenderer = make_unique<FontRenderer>(shaderManager->getShader("text"), "../res/fonts/MxPlus_IBM_BIOS.ttf", 24);

    // Set uniforms
    textShader.setVector2f("vertex", vec4(100, 100, .5, .5));
    shapeShader.use();
    shapeShader.setMatrix4("projection", this->PROJECTION);

}

void Engine::initShapes() {
    // Ground color
    ground = make_unique<Rect>(shapeShader, vec2(width/2, 20), vec2(width, height / 3), groundColor);

    // Ground color
    ground2 = make_unique<Rect>(shapeShader, vec2(width/2, 50), vec2(width, height / 3), groundColor2);


    standingGround = make_unique<Rect>(shapeShader, vec2(width/2, 50), vec2(width, 430), groundColor);

    // To generate spikes
    int totalSpikeWidth = 0;
    vec2 spikeSize;

    while (totalSpikeWidth < width + 70) {
        // Spike height
        spikeSize.y = rand() % 301 + 60;
        // Spike width
        spikeSize.x = rand() % 31 + 40;

        spikes.push_back(make_unique<Triangle>(shapeShader,
                                               vec2(totalSpikeWidth + (spikeSize.x / 2.0) + 100,((spikeSize.y / 2.0) + 50)),
                                               spikeSize, groundColor));
        totalSpikeWidth += spikeSize.x + 115;
    }

    // Clouds
    int totalBlockWidth = 0;
    vec2 blockSize;
    while (totalBlockWidth < width + 100) {
        // block height
        blockSize.y = rand() % 51 + 50;
        // block width
        blockSize.x = rand() % 101 + 100;
        int height = rand() % 11 + 10;
        clouds.push_back(make_unique<Rect>(shapeShader,
                                               vec2(totalBlockWidth + (blockSize.x / 2.0) + 300,((blockSize.y / 2.0) + 487 + height)),
                                               blockSize, white));
        totalBlockWidth += blockSize.x + 300;
    }


    readFromFile("../res/art/scene.txt");
}

void Engine::processInput() {\
    float startTime = 0;
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

    // If we're in the start screen and the user presses s, change screen to play
    if ((screen == start) && keys[GLFW_KEY_C]){
        screen = directions;
    }

    // If we are in the directions screen and the user presses s, switch to play
    if ((screen == directions) && keys[GLFW_KEY_S]){
        screen = play;
    }

    // Update mouse rect to follow mouse
    MouseY = height - MouseY; // make sure mouse y-axis isn't flipped

    // if the screen is in play and the user hits space, have the unicorn fly
    if((screen == play) && keys[GLFW_KEY_SPACE]){
        // set delay time to 1 so that the things start moving when the user hits space.
        delayTime = 1;
        if(squares[0]->getPosY()){
            fly();
        }
        // If the user hits the top of the window they die.
        if(squares[0]->getPosY() > 600){
            screen = dead;
        }
    }

    // If the screen is in play and the user has hit space,
    // the character will drop until it hits the ground.
    if((screen == play) && (squares[0]->getPosY() > 362)){
        fall();
    }


    // If the user overlaps with the blocks, end the game
    for (const unique_ptr<Triangle>& b : spikes) {
        for(const unique_ptr<Shape>& s: squares){
            if(b->isOverlapping(*s) && (screen == play)){
                screen = dead;
                switch(screen);
            }
        }
    }

    // If the user overlaps with the clouds, end the game
    for (const unique_ptr<Rect>& c : clouds) {
        for(const unique_ptr<Shape>& s: squares){
            if(c->isOverlapping(*s) && (screen == play)){
                screen = dead;
                switch(screen);
            }
        }
    }

    // If the user is in the directions screen and the right key is pressed,
    // set the speed to -6
    if((screen == directions) && (keys[GLFW_KEY_RIGHT])){
        speed = -6;
    }


    // If the user is in the directions screen and the up key is pressed,
    // set the speed to -3
    if((screen == directions) && (keys[GLFW_KEY_UP])){
        speed = -3;
    }


    // If the user is in the directions screen and the down key is pressed,
    // set the speed to -1
    if((screen == directions) && (keys[GLFW_KEY_DOWN])){
        speed = -1;
    }
}

// fly method to have the character fly
void Engine::fly() {
    // Loop the move position of squares
    int i  = 0;
    while(i < squares.size()){
        squares[i]->setPosY(squares[i]->getPosY() + 3);
        i++;
    }
}

// Fall method to have the character fall when the user is not hitting space
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

    if(screen == play){
        // until the player hits space, the spikes and clouds won't move.
        if(delayTime >= startDelay){
            // Update spikes
            for (int i = 0; i < spikes.size(); ++i) {
                // move spikes to the left
                spikes[i]->moveX(speed);
                // If a spike has moved off the screen
                if (spikes[i]->getPosX() < -(spikes[i]->getSize().x/2)) {
                    // Set it to the right of the screen so that it passes through again
                    int spikesOnLeft = (spikes[i] == spikes[0]) ? spikes.size()-1 : i - 1;
                    int num = rand() % 41 + 40;
                    spikes[i]->setPosX(spikes[spikesOnLeft]->getPosX() + spikes[spikesOnLeft]->getSize().x/2 + spikes[i]->getSize().x/2 + num);
                }
            }

            // Update clouds
            for (int i = 0; i < clouds.size(); ++i) {
                // Move clouds to the left
                clouds[i]->moveX(speed);
                // If a cloud has moved off the screen
                if (clouds[i]->getPosX() < -(clouds[i]->getSize().x/2)) {
                    // Set it to the right of the screen so that it passes through again
                    int cloudsOnLeft = (clouds[i] == clouds[0]) ? clouds.size()-1 : i - 1;
                    clouds[i]->setPosX(clouds[cloudsOnLeft]->getPosX() + clouds[cloudsOnLeft]->getSize().x/2 + clouds[i]->getSize().x/2 + 100);
                }
            }
        }
    }
}

// Draws the unicorn
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

    // Set shader to use for all shapes
    shapeShader.use();

    // Render differently depending on screen
    switch (screen) {
        case start: {
            // Display the message on the screen
            string message = "Hit space to fly, but don't touch the top.";
            this->fontRenderer->renderText(message, width/2 - (8.5 * message.length()), height - 100, 0.7, vec3{0, 0, 0});
            message = "If you stop hitting space, you will fall";
            this->fontRenderer->renderText(message, width/2 - (9 * message.length()), height - 150, 0.7, vec3{0, 0, 0});
            message = "But watch out for the spikes and clouds,";
            this->fontRenderer->renderText(message, width/2 - (8.5 * message.length()), height - 200, 0.7, vec3{0, 0, 0});
            message = "touch one, and you die!";
            this->fontRenderer->renderText(message, width/2 - (8.5 * message.length()), height - 250, 0.7, vec3{0, 0, 0});
            message = "Press C to continue!";
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height/2 - 60, 1, vec3{0, 0, 0});
            break;
        }
        case directions: {
            string message;
            message = "Hit nothing to play the default speed!";
            this->fontRenderer->renderText(message, width/2 - (8.5 * message.length()), height - 100, 0.7, vec3{0, 0, 0});
            message = "Hit the up error to go fast!";
            this->fontRenderer->renderText(message, width/2 - (8.5 * message.length()), height - 150, 0.7, vec3{0, 0, 0});
            message = "Hit the down error to go slow!";
            this->fontRenderer->renderText(message, width/2 - (8.5 * message.length()), height - 200, 0.7, vec3{0, 0, 0});
            message = "Hit the right error to go supersonic!";
            this->fontRenderer->renderText(message, width/2 - (8.5 * message.length()), height - 250, 0.7, vec3{0, 0, 0});
            message = "Press S to start!";
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height/2 - 60, 1, vec3{0, 0, 0});
            break;
        }
        case play: {
            for(unique_ptr<Shape>& square : squares){
                square->setUniforms();
                square->draw();
            }

            standingGround->setUniforms();
            standingGround->draw();

            ground->setUniforms();
            ground->draw();

            // Draw the clouds
            for (int n=0; n < clouds.size(); n++) {
                clouds[n]->setUniforms();
                clouds[n]->draw();
            }

            // Draw the ground
            for (int n=0; n < spikes.size(); n++) {
                spikes[n]->setUniforms();
                spikes[n]->draw();
            }

            ground2->setUniforms();
            ground2->draw();

            float startTime = 0;

            // Calculate the time that the user did not hit space to play.
            if (delayTime != 1){
                passedTime = glfwGetTime();
                elapsedPassedTime = passedTime - startTime;
                string message = "Hit space to start!";
                this->fontRenderer->renderText(message, 50 , 100,.8, vec3{0, 0, 0});
            }

            //  Subtract the time the user didn't hit space to start from the current time from the window
            currentTime = glfwGetTime() - elapsedPassedTime;
            if(delayTime == 1){
                elapsedTime = currentTime - startTime;
                string time = to_string(elapsedTime);
                string message = "Time elapsed: " + time;
                this->fontRenderer->renderText(message, 50 , 100,.8, vec3{0, 0, 0});
            }
            // Set elapsed time equal to the calculated elapsed times
            elapsedTime = elapsedTime;

            break;
        }
        case dead: {
            // Display the message on the screen
            string message;
            message = "You Died!";
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height - 100, 1, vec3{0, 0, 0});
            message = ":(";
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height - 150, 1, vec3{0, 0, 0});
            string fTime = to_string(elapsedTime);
            message = "You survived for:";
            this->fontRenderer->renderText(message, width/2 - (12* message.length()), height - 200, 1, vec3{0, 0, 0});
            message = fTime + " seconds";
            this->fontRenderer->renderText(message, width/2 - (13* message.length()), height - 250, 1, vec3{0, 0, 0});
            message = "Press esc to exit.";
            this->fontRenderer->renderText(message, width/2 - (9.5* message.length()), height/2 - 60, .8, vec3{0, 0, 0});
            break;
        }
    }

    glfwSwapBuffers(window);
}

bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}