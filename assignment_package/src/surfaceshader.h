#pragma once

#include "shaderprogram.h"


class SurfaceShader: public ShaderProgram
{
public:
//GLuint vertShader; // A handle for the vertex shader stored in this shader program
//GLuint fragShader; // A handle for the fragment shader stored in this shader program
//GLuint prog;       // A handle for the linked shader program stored in this class

int attrPos; // A handle for the "in" vec4 representing vertex position in the vertex shader
int attrNor; // A handle for the "in" vec4 representing vertex normal in the vertex shader
int attrCol; // A handle for the "in" vec4 representing vertex color in the vertex shader
int attrPosOffset; // A handle for a vec3 used only in the instanced rendering shader
int attrUV;
int attrAnim; // handle to tell fragment shader this part of texture is animatable
int attrTra2Opq; // handle to tell fragment to draw faces between transparent blocks with alpha=1 opacity instead

int unifModel; // A handle for the "uniform" mat4 representing model matrix in the vertex shader
int unifModelInvTr; // A handle for the "uniform" mat4 representing inverse transpose of the model matrix in the vertex shader
int unifViewProj; // A handle for the "uniform" mat4 representing combined projection and view matrices in the vertex shader
int unifViewProjInv; // A handle for the "uniform" mat4 representing inverse ViewProj

int unifColor; // A handle for the "uniform" vec4 representing color of geometry in the vertex shader
int unifDimensions;
int unifEye;
int unifFBSampler; // A handle for a supplmental texture from intermediate FrameBuffer

//int unifCamPos; // a handle to a "uniform" vec3 representing the camera's position in world space.


public:
SurfaceShader(OpenGLContext* context);
virtual ~SurfaceShader();
virtual void setupMemberVars() override;

// Pass the given model matrix to this shader on the GPU
void setModelMatrix(const glm::mat4 &model);
// Pass the given Projection * View matrix to this shader on the GPU
void setViewProjMatrix(const glm::mat4 &vp);
void setViewProjInvMatrix(const glm::mat4 &vp);
// Pass the given color to this shader on the GPU
void setGeometryColor(glm::vec4 color);

// For Sky specifically
void setDimensions(glm::ivec2 dims);
// Pass the camera position to this shader on the GPU
void setEye(glm::vec3 eye);


// Draw the given object to our screen using this ShaderProgram's shaders
virtual void draw(Drawable &d, int textureSlot) override;
void drawInterleaved(Drawable &d, int textureSlot);
// Draw the given object to our screen multiple times using instanced rendering
void drawInstanced(InstancedDrawable &d);
};
