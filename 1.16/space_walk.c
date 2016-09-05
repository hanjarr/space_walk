// space_walk
// version 1.16
// Max Gefvert and Hannes Järrendahl
// Should work on Linux and Mac, if version in shaders is changed and different makefile is used.

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include "MicroGlutmac.h"
// Linking hint for Lightweight IDE
// uses framework Cocoa
#endif
#include "GL_utilities.h"
#include "loadobj.h"
#include "LoadTGA.c"
#include "VectorUtils3.h"
#include <math.h>

// GLOBALS

// Model init
Model *sphere;
Model *skybox;
Model *r2d2;

// Projection matrix
#define near 1.0
#define far 500.0
#define right 0.89
#define left -0.89
#define top 0.5
#define bottom -0.5

GLfloat projectionMatrix[] = {
    2.0f*near/(right-left), 0.0f, (right+left)/(right-left), 0.0f,
    0.0f, 2.0f*near/(top-bottom), (top+bottom)/(top-bottom), 0.0f,
    0.0f, 0.0f, -(far + near)/(far - near), -2*far*near/(far - near),
    0.0f, 0.0f, -1.0f, 0.0f };

// Shader referens
GLuint program, skyProgram;

// Struct for planets: planet positions, radius and which texture (not implemented)
typedef struct{
    vec3 pos;
    float rad;
    int tex;
} planet;

// Number of planets and which one is closest
int nbrPlanets = 6;
int closestPlanet = 0;


// To different planet setups
planet planetArray[] = {
    {{0.0, 0.0, 0.0}, 8.0, 1},
    {{40.0, 10.0, 0.0}, 8.0, 1},
    {{80.0, 0.0, 0.0}, 8.0, 1},
    {{110.0, 10.0, 0.0}, 5.0, 1},
    {{70.0, 100.0, 0.0}, 20.0, 1},
    {{110.0, 10.0, 40.0}, 5.0, 1}
};

planet planetArray2[] = {
    {{0.0, 0.0, 0.0}, 8.0, 1},
    {{40.0, 0.0, 0.0}, 8.0, 1},
    {{40.0, 40.0, 0.0}, 8.0, 1},
    {{0.0, 40.0, 0.0}, 8.0, 1},
};


// Struct for moons
typedef struct{
    vec3 pos;
    float rad;
    int tex;
} moon;

moon moonArray[] = {
    {{100.0, 130.0, 0.0}, 4.0, 1}
};

// Distance between camera and R2
GLfloat camDist = 15.0;

// Elapsed time
GLfloat t = 0;

// Rotation angle around axis
GLfloat alpha = 0;

// Rotation angle in space
GLfloat theta = 0.03;

// Angles changing camera direction from l.
int ver = 0;
int hor = 0;
GLfloat tilt = 35;

// Initial speed when jumping
GLfloat jumpForce = 0.3;

// "Gravitation constant"
GLfloat G = 0.001;

// Bool to check if R2 is in the air (space)
bool isAir = false;

// Jump speed vector
vec3 speed = {0.0,0.0,0.0};

// Camera position
vec3 p = {0.0,0.0,0.0};

// Looking at (R2 position)
vec3 l = {0.0,0.0,0.0};
vec3 l2 = {0.0,0.0,0.0};

// Up vector
vec3 v = {0.0,1.0,0.0};
vec3 v2 = {0.0,1.0,0.0};


// Transformation matrices
mat4 axisRot, spaceRot, cameraRot, trans, scale, model2world, world2view;

// Texture references
GLuint tex, tex2, tex3, tex4, tex5, tex6, tex7, tex8, tex9;

// Int for changing between directional and positional light
GLint isDirectional[] = {1,0};


void init(void)
{
    
    dumpInfo();
    
    // GL inits
    glClearColor(1.0,1.0,1.0,0);
    glEnable(GL_DEPTH_TEST);
    //printError("GL inits");
    
    // Models
    sphere = LoadModelPlus("sphere.obj");
    r2d2 = LoadModelPlus("R2D2.obj");
    
    // Skybox
    skybox = LoadModelPlus("skyboxmod.obj");
    
    // Textures
    LoadTGATextureSimple("earth.tga", &tex);
    glUniform1i(glGetUniformLocation(program, "tex"),0);
    LoadTGATextureSimple("asteroid.tga", &tex2);
    LoadTGATextureSimple("venus.tga", &tex3);
    LoadTGATextureSimple("planet1.tga", &tex4);
    LoadTGATextureSimple("jupiter2.tga", &tex5);
    LoadTGATextureSimple("planet3.tga", &tex6);
    LoadTGATextureSimple("R23.tga", &tex7);
    
    LoadTGATextureSimple("moon.tga", &tex9);
    glUniform1i(glGetUniformLocation(program, "tex9"),0);
    
    glUseProgram(skyProgram);
    LoadTGATextureSimple("space3.tga", &tex8);
    glUniform1i(glGetUniformLocation(skyProgram, "tex8"),0);
    
    // Load and compile shader
    skyProgram = loadShaders("skybox.vert", "skybox.frag");
    glUseProgram(skyProgram);
    
    program = loadShaders("space_walk.vert", "space_walk.frag");
    glUseProgram(program);
    
    //printError("init shader");
    
    // End of upload of geometry
    //printError("init arrays");
    
    // Init rotation around planet
    spaceRot = IdentityMatrix();
    
    //Init camera position
    l.y = planetArray[closestPlanet].rad;
    p.y = planetArray[closestPlanet].rad;
    l2.y = planetArray[closestPlanet].rad;
    p.z = camDist;
}


// Moving forward along planet surface
void forward(vec3 planetPos)
{
    // Rotate l theta radians around planet
    vec3 r2Pos = VectorSub(l,planetPos);
    vec3 dir = Normalize(VectorSub(l,p));
    vec3 rotVec = CrossProduct(r2Pos, dir);
    mat4 rotMat = ArbRotate(rotVec, theta);
    r2Pos = MultVec3(rotMat, r2Pos);
    l =  VectorAdd(planetPos, r2Pos);
    
    // Set new up vector
    v = Normalize(r2Pos);
    
    // Set p orthogonal behind l
    p = VectorSub(l, ScalarMult(Normalize(CrossProduct(v, CrossProduct(dir, v))), camDist));
    
    // Update rotation of r2
    spaceRot = Mult(rotMat, spaceRot);
}

void backward(vec3 planetPos)
{
    // rotate l theta radians around planet
    vec3 r2Pos = VectorSub(l,planetPos);
    vec3 dir = Normalize(VectorSub(l,p));
    vec3 rotVec = CrossProduct(dir, r2Pos);
    mat4 rotMat = ArbRotate(rotVec, theta);
    r2Pos = MultVec3(rotMat, r2Pos);
    l =  VectorAdd(planetPos, r2Pos);
    
    // Set new up vector
    v = Normalize(r2Pos);
    
    // Set p orthogonal behind (plus one up)...
    p = VectorSub(l, ScalarMult(Normalize(CrossProduct(v, CrossProduct(dir, v))), camDist));
    
    // Update rotation of r2
    spaceRot = Mult(rotMat, spaceRot);
}


// Jump along normal
void jump()
{
    isAir = true;
    speed = ScalarMult(v, jumpForce);
}

void inAir()
{
    if(isAir)
    {
        GLfloat closestDist = Norm(VectorSub(planetArray[closestPlanet].pos,l)) - planetArray[closestPlanet].rad;
        GLfloat r2Dist;
        vec3 r2Pos, dir;
        
        // Calculate force contribution on R2 from every planet
        for (int i = 0; i < nbrPlanets; i++)
        {
            r2Pos = VectorSub(planetArray[i].pos, l);
            r2Dist = Norm(r2Pos);
            speed = VectorAdd(speed, ScalarMult(Normalize(r2Pos), G*planetArray[i].rad*planetArray[i].rad*planetArray[i].rad/r2Dist/r2Dist));
            
            // Update closest planet
            if (r2Dist-planetArray[i].rad <= closestDist)
            {
                closestDist = r2Dist-planetArray[i].rad;
                closestPlanet = i;
            }
        }
        
        // Update vector between closest planet and R2
        r2Pos = VectorSub(planetArray[closestPlanet].pos, l);
        
        // Check if R2 has landed or not. If not, update speed
        if(Norm(VectorAdd(VectorSub(l,planetArray[closestPlanet].pos), speed)) > planetArray[closestPlanet].rad)
        {
            l = VectorAdd(l,speed);
            p = VectorAdd(p,speed);
            r2Pos = VectorSub(planetArray[closestPlanet].pos, l);
            
            dir = VectorSub(p, l);
            GLfloat rotAmount = DotProduct(Normalize(r2Pos), v);
            
            // If R2 is not positioned towards the closest planet, rotate him to align
            if (rotAmount > -0.999999)
            {
                mat4 rotMat = ArbRotate(CrossProduct(r2Pos, v), fmin(0.07, 10*(rotAmount+1.0)));
                v = Normalize(MultVec3(rotMat, v));
                p = VectorAdd(l, MultVec3(rotMat, dir));
                spaceRot = Mult(rotMat, spaceRot);
            }
        }
        
        // Calculate landingspot and position R2 on the surface
        else
        {
            vec3 surfacePos = VectorSub(r2Pos, ScalarMult(Normalize(r2Pos), planetArray[closestPlanet].rad));
            GLfloat cosAngle = DotProduct(Normalize(speed), Normalize(surfacePos));
            vec3 landing = ScalarMult(Normalize(speed), Norm(surfacePos)/cosAngle);
            l = VectorAdd(l, landing);
            p = VectorAdd(p, landing);
            r2Pos = VectorSub(planetArray[closestPlanet].pos, l);
            
            dir = VectorSub(p, l);
            isAir = false;
            speed = SetVector(0.0,0.0,0.0);
            
            mat4 rotMat = ArbRotate(CrossProduct(r2Pos, v), acos(DotProduct(v, Normalize(ScalarMult(r2Pos, -1.0)))));
            v = Normalize(MultVec3(rotMat, v));
            p = VectorAdd(l, MultVec3(rotMat, dir));
            spaceRot = Mult(rotMat, spaceRot);
        }
        
    }
}

void changeCamera(int direction)
{
    if(direction == 1)
        if(ver < tilt)
            ver++;
    
    if(direction == 2)
        if(ver > -tilt)
            ver--;
    
    if(direction == 3)
        if(hor < tilt)
            hor++;
    
    if(direction == 4)
        if(hor > -tilt)
            hor--;
}

void movement()
{
    if(keyIsDown('r'))
    {
        closestPlanet = 0;
        spaceRot = IdentityMatrix();
        axisRot = IdentityMatrix();
        alpha = 0;
        l = SetVector(0.0, planetArray[closestPlanet].rad, 0.0);
        p = SetVector(0.0, planetArray[closestPlanet].rad, camDist);
        l2 = l;
        v = SetVector(0.0, 1.0, 0.0);
        v2 = v;
        speed = SetVector(0.0,0.0,0.0);
    }
    
    if(keyIsDown('w') && !isAir)
    {
        forward(planetArray[closestPlanet].pos);
    }
    
    if(keyIsDown('s') && !isAir)
    {
        backward(planetArray[closestPlanet].pos);
    }
    
    if(keyIsDown('a'))
    {
        vec3 dir = VectorSub(p,l);
        axisRot = ArbRotate(v,0.1);
        p = VectorAdd(l, MultVec3(axisRot, dir));
        alpha = alpha - 0.1;
    }
    
    if(keyIsDown('d'))
    {
        vec3 dir = VectorSub(p,l);
        axisRot = ArbRotate(v,-0.1);
        p = VectorAdd(l, MultVec3(axisRot, dir));
        alpha = alpha + 0.1;
    }
    
    if(keyIsDown('h') && !isAir)
    {
        jump();
    }
    
    if(keyIsDown('i'))
    {
        changeCamera(1);
    }
    
    if(keyIsDown('k'))
    {
        changeCamera(2);
    }
    
    if(keyIsDown('j'))
    {
        changeCamera(3);
    }
    
    if(keyIsDown('l'))
    {
        changeCamera(4);
    }
    
    if(!keyIsDown('i') && !keyIsDown('k') && ver != 0)
    {
        if(ver > 0)
            ver--;
        else
            ver++;
    }
    
    if(!keyIsDown('j') && !keyIsDown('l') && hor != 0)
    {
        if(hor > 0)
            hor--;
        else
            hor++;
    }
}


void display(void)
{
    //printError("pre display");
    
    
    movement();
    inAir();
    
    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(program);
    
    glUniform3f(glGetUniformLocation(program, "camPos"), p.x, p.y, p.z);
    glUniform3f(glGetUniformLocation(program, "lightPos"), l.x + v.x, l.y + v.y, l.z + v.z);
    glUniform1iv(glGetUniformLocation(program, "isDirectional"), 2, isDirectional);
    
    glUniformMatrix4fv(glGetUniformLocation(program, "projectionMatrix"), 1, GL_TRUE, projectionMatrix);
    
    
    //printError("display");
    
    // Disable face culling and z-buffering
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    glUseProgram(skyProgram);
    
    world2view = lookAtv(p,l,v);
    world2view.m[3] = 0;
    world2view.m[7] = 0;
    world2view.m[11] = 0;
    model2world = T(0,0,0);
    
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "world2view"), 1, GL_TRUE, world2view.m);
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "model2world"), 1, GL_TRUE, model2world.m);
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projectionMatrix"), 1, GL_TRUE, projectionMatrix);
    
    glBindTexture(GL_TEXTURE_2D, tex8);
    DrawModel(skybox, skyProgram, "in_Position", NULL,"in_TexCoord");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    
    // Change camera accorting to hor and ver angle.
    vec3 dir = VectorSub(l, p);
    cameraRot = Mult(ArbRotate(CrossProduct(dir, v),0.01*ver), ArbRotate(v, 0.01*hor));
    dir = MultVec3(cameraRot, dir);
    v2 = MultVec3(cameraRot, v);
    l2 = VectorAdd(p, dir);
    
    world2view =  lookAtv(p, l2, v2);
    
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, "world2view"), 1, GL_TRUE, world2view.m);
    
    //Display planets
    for (int i = 0; i<nbrPlanets; i++)
    {
        glBindTexture(GL_TEXTURE_2D, i+1);
        scale =  S(planetArray[i].rad, planetArray[i].rad, planetArray[i].rad);
        trans = T(planetArray[i].pos.x, planetArray[i].pos.y, planetArray[i].pos.z);
        model2world = Mult(trans,scale);
        glUniformMatrix4fv(glGetUniformLocation(program, "model2world"), 1, GL_TRUE, model2world.m);
        DrawModel(sphere, program, "in_Position", "in_Normal",NULL);
    }
    
    //Display moon
    vec3 moonPos = VectorSub(moonArray[0].pos, planetArray[4].pos);
    mat4 moonRot = ArbRotate(planetArray[4].pos, t/3000);
    axisRot = ArbRotate(CrossProduct(planetArray[4].pos, moonArray[0].pos),t/3000);
    moonPos = VectorAdd(MultVec3(axisRot, moonPos), planetArray[4].pos);
    scale = S(moonArray[0].rad, moonArray[0].rad, moonArray[0].rad);
    trans = T(moonPos.x, moonPos.y, moonPos.z);
    model2world = Mult(trans,Mult(scale,moonRot));
    glUniformMatrix4fv(glGetUniformLocation(program, "model2world"), 1, GL_TRUE, model2world.m);
    
    glBindTexture(GL_TEXTURE_2D, tex9);
    DrawModel(sphere, program, "in_Position", "in_Normal",NULL);
    
    
    // Update model2world matrix for R2
    scale =  S(0.01,0.01,0.01);
    axisRot = ArbRotate(v,-alpha-M_PI);
    trans = T(l.x - 0.06*v.x, l.y - 0.06*v.y, l.z - 0.06*v.z);
    model2world = Mult(trans, Mult(scale, Mult(axisRot, spaceRot)));
    glUniformMatrix4fv(glGetUniformLocation(program, "model2world"), 1, GL_TRUE, model2world.m);
    
    // R2 texture and model
    glBindTexture(GL_TEXTURE_2D, tex7);
    DrawModel(r2d2, program, "in_Position", "in_Normal", NULL);
    
    glutSwapBuffers();
}

void OnTimer(int value)
{
    t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);
    glutPostRedisplay();
    glutTimerFunc(20, &OnTimer, value);
}

int main(int argc, char *argv[])
{
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInit(&argc, argv);
    glutInitContextVersion(3, 2);
    glutInitWindowSize(1920, 1080);
    glutCreateWindow ("space_walk");
    init ();
    initKeymapManager();
    glutDisplayFunc(display);
    glutTimerFunc(20, &OnTimer, 0);
    glutMainLoop();
}
