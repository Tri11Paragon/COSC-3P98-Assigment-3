#define GLAD_GL_IMPLEMENTATION
#include <config.h>
#ifdef EXTRAS
    //#include <modes/basic.h>
    #include <modes/high_perf.h>
#else
    #include <modes/basic.h>
#endif

#include <camera.h>
#include <blt/std/logging.h>

int WINDOW_WIDTH = 1440;
int WINDOW_HEIGHT = 720;
long lastFrameTime = 0;
long delta = 16000000;

camera cam;
particle_system* fountain;

long getDelta() {
    return delta;
}

void window_resize_i(int width, int height) {
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    window_resize(width, height);
    BLT_TRACE("Window Resized (width: %d, height: %d)!", WINDOW_WIDTH, WINDOW_HEIGHT);
}

void render_i(){
    // the window size callback was getting the size wrong. this does not.
    int local_width = glutGet(GLUT_WINDOW_WIDTH);
    int local_height = glutGet(GLUT_WINDOW_HEIGHT);
    if (local_width != WINDOW_WIDTH || local_height != WINDOW_HEIGHT)
        window_resize_i(local_width, local_height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    cam.update(WINDOW_WIDTH, WINDOW_HEIGHT, (float)((double)delta / 1000000000.0));
    updateView();
    
    render();

#ifdef EXTRAS
    std::stringstream str;
    str << WINDOW_TITLE;
    str << " | Particle Count: ";
    str << particle_count;
    str << " | FPS: ";
    str << 1000000000.0 / (double)getDelta();
    glutSetWindowTitle(str.str().c_str());
#endif
    
    glutSwapBuffers();
    cam.inputUpdate();
    auto curTime = getCurrentTimeNanoseconds();
    delta = curTime - lastFrameTime;
    lastFrameTime = curTime;
}

int main(int argc, char** argv) {
    // change the first bool to disable color.
    auto logging_properties = blt::logging::LOG_PROPERTIES{true, true, false, "./"};
    logging_properties.m_logFullPath = false;
    
    blt::logging::init(logging_properties);
    
    // BLT logging functions are designed to operate one call per line of text. Thus use formatting for all uses
    // (\n is implicitly added, if the last character in the format string is \n, it will be ignored!)
    // (\n\n will for instance insert one extra line between the current line and the next, not two!)
    // alternatively blt::logging::tlog <---> blt::logging::flog provides a std::cout like interface but lacks filename and line number!
    BLT_DEBUG("Beginning initialization of '%s'", WINDOW_TITLE.c_str());
    BLT_WARN("If your terminal does not support ANSI color codes please use one that does.");
    BLT_WARN("(or edit the source to disable BLT colored logging, there is an option for this!)");
    
    // print help
    BLT_INFO("How to use this program:");
    BLT_INFO("\tw/a/s/d to move around");
    BLT_INFO("\tEscape will hide your mouse cursor and enable camera rotation");
    BLT_INFO("\t\tThis doesn't work very well... because GLUT.");
    BLT_INFO("\tThe arrow keys can be used to move the camera look direction around");
    BLT_INFO("\tx will randomize the speed of currently spawned particles");
    BLT_INFO("\t[ shifts operational mode to the left (Continuous -> Single");
    BLT_INFO("\t] shifts operational mode to the right (Continuous -> Manual");
    BLT_INFO("\tf spawns a single particle (manual mode) or will continuously fire particles if held (manual mode) at the rate of the particle system's PPS");
    BLT_INFO("\tr will reset the world by clearing the internal maps of particles");
    BLT_INFO("\tc toggles friction (enabled by default)");
    BLT_INFO("\t; toggles the spray mode");
    // this is because I made the particle system before reading to that part of the document / deciding to do this.
    BLT_INFO("\t\tSpray mode is added in addition to the default particle fountain spread!");
    BLT_INFO("\tt randomizes the spawned particle texture");
    
    // create the display
    glutInit(&argc, argv);
#ifdef EXTRAS
    glutInitContextVersion(4, 6);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
#endif
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutSetOption(GLUT_MULTISAMPLE, 8);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE | GLUT_DEPTH);
    glutCreateWindow(WINDOW_TITLE.c_str());
    BLT_DEBUG("Window successfully created!");
    
    int version = gladLoadGL(glutGetProcAddress);
    if (version == 0) {
        BLT_FATAL("Failed to initialize OpenGL context");
        return -1;
    }
    
    BLT_DEBUG("Loaded OpenGL (GLAD) %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    
    // setup callbacks
    // ------------------------
    
    // keyboard
    glutKeyboardFunc([](unsigned char key, int x, int y) -> void {
        cam.keyPress(key, x, y);
    });
    glutKeyboardUpFunc([](unsigned char key, int x, int y) -> void {
        cam.keyRelease(key, x, y);
        // s key is used, use x instead
        if (key == 'x')
            fountain->randomizeSpeed(10);
        // glut will repeat after some time for some reason. this is a bug in glut not the code
        if (key == 'f')
            fountain->singleFire();
        if (key == '[')
            fountain->changeFiringMode(fountain->getFiringMode() - 1);
        if (key == ']')
            fountain->changeFiringMode(fountain->getFiringMode() + 1);
        if (key == 'r')
            fountain->reset();
        if (key == 'c')
            fountain->setFriction(!fountain->getFriction());
        if (key == ';')
            fountain->toggleSpray();
        if (key == 't')
            fountain->toggleTexRandomizer();
        if (key == 'p')
            beginExecution();
    });
    glutSpecialFunc([](int k, int x, int y) -> void {
        cam.specialPress(k);
    });
    glutSpecialUpFunc([](int k, int x, int y) -> void {
        cam.specialRelease(k);
    });
    
    // mouse
    glutMouseFunc([](int button, int state, int x, int y) -> void {
    
    });
    glutMotionFunc([](int x, int y) -> void {
        cam.mouseMotion(x, y);
    });
    glutPassiveMotionFunc([](int x, int y) -> void {
        cam.mousePassiveMotion(x, y);
    });
    glutCloseFunc([]() -> void {
        BLT_DEBUG("Program has exited, cleaning up resources");
        
        cleanup();
        
        BLT_DEBUG("Cleanup complete, have a good day!");
    });
    
    // display
    glutIdleFunc(render_i);
    glutDisplayFunc(render_i);
    
    BLT_DEBUG("Callbacks installed!");

    glClearColor(80 / 255.0, 182 / 255.0, 230 / 255.0, 1.0);
    window_resize_i(WINDOW_WIDTH, WINDOW_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    BLT_DEBUG("GL setup complete!");
    
    init();
    
    fountain = new particle_system({0, 1, 0}, {0, 1, 0}, 4.5, 5000);
    
    BLT_DEBUG("Resource initialization complete!");
    
    glutMainLoop();

    return 0;
}