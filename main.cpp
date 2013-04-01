// -----------------------------------------------------------------------------
//
// Contributors: 
//             1) Tommy Hinks
//
// -----------------------------------------------------------------------------

#define GLEW_STATIC // TODO: should be in project settings!

#include <GL/glew.h>
#include <GL/glfw.h>
#include "VertexAttribSize.hpp"
#include "VertexAttribType.hpp"
#include "GLTypeEnum.hpp"
#include "Plane.hpp"
#include "Frustum.hpp"
#include "Viewport.hpp"
#include "Camera.hpp"
#include "PerspCamera.hpp"
#include "OrthoCamera.hpp"
#include "Material.hpp"
#include "Light.hpp"
#include "Color.hpp"
#include "Model.hpp"
#include "Types.hpp"
#include "ObjRead.hpp"
#include "ObjWrite.hpp"

#include <nDjinn.hpp>
#include <thx.hpp>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <memory>

using std::unique_ptr;
using std::vector;
using std::string;

// -----------------------------------------------------------------------------

struct Geometry {
  unique_ptr<ndj::ArrayBuffer> positionVbo;
  unique_ptr<ndj::ArrayBuffer> textureVbo;
  unique_ptr<ndj::ElementArrayBuffer> indexVbo;
  vec3f min;
  vec3f max;
};

struct Shader {
  unique_ptr<ndj::ShaderProgram> program;
  unique_ptr<ndj::VertexArray> vertexArray;
};

class Timer {
public:
  explicit Timer()
    : _start(glfwGetTime()) {
  }

  double 
  elapsed() const {
    return glfwGetTime() - _start;
  }

  void
  reset() {
    _start = glfwGetTime();
  }

private:
  double _start;
};

// -----------------------------------------------------------------------------

// Some ugly, yet convienent, global stuff.
vec2i winSize(600, 600);

bool lmbPressed = false;
bool mmbPressed = false;
bool rmbPressed = false;
int dragStartX = 0;
int dragStartY = 0;
int dragLastX = 0;
int dragLastY = 0;
int wheelLastPos = 0;

std::string const shaderPath = "D:/GitHub/mesh-slicer/shaders";
//std::string const meshPath = "D:/GitHub/mesh_slicer/data/dragon.obj";
//std::string const meshPath = "D:/GitHub/mesh_slicer/data/perfexion.obj";
std::string const meshPath = "D:/GitHub/mesh-slicer/data/teapot.obj";

// -----------------------------------------------------------------------------

GLuint const MESH_MODEL_UNIFORM_BLOCK_BINDING = 1;
GLuint const MESH_MATERIAL_UNIFORM_BLOCK_BINDING = 2;
GLuint const SCENE_LIGHT_UNIFORM_BLOCK_BINDING = 3;
GLuint const SCENE_CAMERA_UNIFORM_BLOCK_BINDING = 4;
GLuint const SLICE_PLANE_MODEL_UNIFORM_BLOCK_BINDING = 5;
GLuint const SLICE_PLANE_COLOR_UNIFORM_BLOCK_BINDING = 6;

GLuint const SLICE_CAMERA_UNIFORM_BLOCK_BINDING = 7;
GLuint const SLICE_LINE_COLOR_UNIFORM_BLOCK_BINDING = 8;

GLuint const CONTOUR_PLANE_MODEL_UNIFORM_BLOCK_BINDING = 9;
GLuint const CONTOUR_PLANE_CAMERA_UNIFORM_BLOCK_BINDING = 10;

vec4f const SCENE_CLEAR_COLOR(0.2f, 0.2f, 0.2f, 1.f);
Viewport sceneViewport;
PerspCamera sceneCamera;
Light sceneLight;


Material const MESH_MATERIAL(vec4f(.2f, .2f, .2f, 1.f),
                             vec4f(.2f, .2f, .2f, 1.f),
                             vec4f(.4f, .5f, .8f, 1.f),
                             vec4f(.8f, .5f, .4f, 1.f),
                             vec4f(.4f, .4f, .4f, 1.f),
                             vec4f(.4f, .4f, .4f, 1.f));
vector<vec3f> meshVertices;
vector<vec3ui> meshIndices;

Model meshModel;
Geometry meshGeometry;
Shader meshShader;

Plane const SLICE_PLANE;
Color const SLICE_PLANE_COLOR(vec4f(.1f, .4f, .1f, .5f));
Model slicePlaneModel;
Geometry slicePlaneGeometry;
Shader slicePlaneShader;

unique_ptr<ndj::UniformBuffer> sceneCameraUbo;
unique_ptr<ndj::UniformBuffer> sceneLightUbo;
unique_ptr<ndj::UniformBuffer> meshMaterialUbo;
unique_ptr<ndj::UniformBuffer> meshModelUbo;
unique_ptr<ndj::UniformBuffer> slicePlaneColorUbo;
unique_ptr<ndj::UniformBuffer> slicePlaneModelUbo;

// -----------------------------------------------------------------------------

enum SliceMode { GPU, CPU };
SliceMode sliceMode = GPU;
Color const SLICE_LINE_COLOR(vec4f(1.f, 1.f, 0.f, 1.f));
GLfloat const SLICE_LINE_WIDTH(2.f);
vec4f const SLICE_CLEAR_COLOR(0.1f, 0.4f, 0.1f, 0.1f);
OrthoCamera sliceCamera;
Shader sliceMeshShader;
Shader sliceLinesShader;
Geometry sliceLinesGeometry;
unique_ptr<ndj::Texture2D> sliceTexture;
unique_ptr<ndj::Framebuffer> sliceFbo;

unique_ptr<ndj::UniformBuffer> sliceCameraUbo;
unique_ptr<ndj::UniformBuffer> sliceLineColorUbo;

// -----------------------------------------------------------------------------

GLuint const CONTOUR_PLANE_TEX_UNIT = 0;
OrthoCamera const CONTOUR_PLANE_CAMERA;
Viewport contourPlaneViewport;
Model contourPlaneModel;
Geometry contourPlaneGeometry;
Shader contourPlaneShader;

unique_ptr<ndj::Sampler> contourPlaneSampler;
unique_ptr<ndj::UniformBuffer> contourPlaneCameraUbo;
unique_ptr<ndj::UniformBuffer> contourPlaneModelUbo;

// -----------------------------------------------------------------------------

void
initGLEW() {
  using std::cerr;
  using std::abort;
  using std::exception;

  try {
    glewExperimental = GL_TRUE;
    GLenum const glewErr = glewInit();
    if (GLEW_OK != glewErr) {
      // glewInit failed, something is seriously wrong.
      cerr << "GLEW init error: " << glewGetErrorString(glewErr) << "\n";
      abort();
    }
    ndj::checkError("glewInit");
  }
  catch (exception const& ex) {
    std::cout << "Warning: " << ex.what() << std::endl;
  }
}

void
initGL() {
  using std::cout;
  using ndj::getString;
  using ndj::getIntegerv;
  using ndj::clearColor;
  using ndj::clearDepth;
  using ndj::depthRange;
  using ndj::disable;
  using ndj::enable;

  clearColor(0.2f, 0.2f, 0.2f, 1.f);
  clearDepth(1.);
  depthRange(0., 1.);
  ndj::enable(GL_MULTISAMPLE);
  disable(GL_CULL_FACE);
  //disable(GL_NORMALIZE);

  cout 
    << "GLEW_VERSION: " << glewGetString(GLEW_VERSION) << "\n"
    << "GL_VERSION: " << getString(GL_VERSION) << "\n"
    << "GL_VENDOR: " << getString(GL_VENDOR) << "\n"
    << "GL_RENDERER: " << getString(GL_RENDERER) << "\n"
    << "GL_SHADING_LANGUAGE_VERSION: " 
      << getString(GL_SHADING_LANGUAGE_VERSION) << "\n";
  GLint maxVertexAttribs = 0;
  getIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
  cout << "GL_MAX_VERTEX_ATTRIBS: " << maxVertexAttribs << "\n";
  vec2i maxViewportDims;
  getIntegerv(GL_MAX_VIEWPORT_DIMS, &maxViewportDims[0]);
  cout << "GL_MAX_VIEWPORT_DIMS: " << maxViewportDims << "\n";
}

// -----------------------------------------------------------------------------

//! DOCS
void GLFWCALL 
windowSizeCallback(const int w, const int h) {
  using std::cout;

  winSize[0] = w;
  winSize[1] = h;
  cout << "Resize: " << winSize << "\n";

  // Update scene viewport. Covers entire window.
  sceneViewport.x = 0;
  sceneViewport.y = 0;
  sceneViewport.width = static_cast<GLsizei>(winSize[0]);
  sceneViewport.height = static_cast<GLsizei>(winSize[1]);
  cout << "Scene viewport: " 
    << vec4i(sceneViewport.x, 
             sceneViewport.y, 
             sceneViewport.width, 
             sceneViewport.height) 
    << "\n";

  // Update contour plane viewport. Always in lower left corner.
  GLint const contourPlaneViewportDim = 
    std::max(
      static_cast<GLint>((1./3)*sceneViewport.width), 
      static_cast<GLint>((1./3)*sceneViewport.height));
  contourPlaneViewport.x = 
    sceneViewport.width - contourPlaneViewportDim;
  contourPlaneViewport.y = 0;
  contourPlaneViewport.width = contourPlaneViewportDim; 
  contourPlaneViewport.height = contourPlaneViewportDim;
  cout << "Contour plane viewport: " 
    << vec4i(contourPlaneViewport.x, 
             contourPlaneViewport.y, 
             contourPlaneViewport.width, 
             contourPlaneViewport.height) 
    << "\n";

  // Update scene camera. Take new scene viewport aspect ratio into account.
  Frustum newFrustum(sceneCamera.frustum());
  GLfloat const widthToHeightRatio = 
    static_cast<GLfloat>(sceneViewport.width)/sceneViewport.height;
  newFrustum.left = -.5f*widthToHeightRatio;
  newFrustum.right = .5f*widthToHeightRatio;
  sceneCamera.setProjection(newFrustum);

  sliceTexture.reset(
    new ndj::Texture2D(
      GL_TEXTURE_2D,
      2*contourPlaneViewportDim,
      2*contourPlaneViewportDim));
  cout << "Slice texture size: " << 
    vec2i(sliceTexture->width(), sliceTexture->height()) << "\n";
  sliceFbo.reset(new ndj::Framebuffer(GL_FRAMEBUFFER));
  sliceFbo->attachTexture2D(GL_COLOR_ATTACHMENT0, sliceTexture->handle());
}

//! DOCS
void GLFWCALL 
keyCallback(const int k, const int action) {
  using std::cout;

  switch (action) {
  case GLFW_PRESS:
    switch (k) {
    case 'F':
      //cam.frame(sceneMin, sceneMax);
      break;
    case 'S':
      switch (sliceMode) {
      case GPU:
        sliceMode = CPU;
        cout << "Slice mode: CPU\n";
        break;
      case CPU:
        sliceMode = GPU;
        cout << "Slice mode: GPU\n";
        break;
      }
      break;
    case GLFW_KEY_ESC:
      break;
    case GLFW_KEY_UP:
      break;
    case GLFW_KEY_DOWN:
      break;
    case GLFW_KEY_LEFT:
      break;
    case GLFW_KEY_RIGHT:
      break;
    default:
      break;
    }
    break;
  case GLFW_RELEASE:
    break;
  default:
    break;
  }
}

//! DOCS
void GLFWCALL 
mousePosCallback(const int mx, const int my) {
  int const dx = mx - dragLastX;
  int const dy = my - dragLastY;
  if (lmbPressed) {
    //GLint vp[4];
    //ndj::Viewport::getViewport(vp);
    vec3f const dr = 
      vec3f(static_cast<GLfloat>(dy), static_cast<GLfloat>(dx), 0.f);
    meshModel.setRotation(meshModel.rotation() - dr);
    meshModel.setNormalMatrix(sceneCamera.uniformData().viewMatrix);
  }
  else if (mmbPressed) {
    // TODO: implement camera panning!
  }
  else if (rmbPressed) {
    GLfloat const minMeshDimension = 
      thx::min(meshGeometry.max[0] - meshGeometry.min[0], 
               meshGeometry.max[1] - meshGeometry.min[1], 
               meshGeometry.max[2] - meshGeometry.min[2]);
    vec3f const dp = vec3f(0.f, 0.f, (2.f*minMeshDimension*dy)/winSize[0]);
    sceneCamera.setView(sceneCamera.position() - dp, vec3f(0.f));
  }

  dragLastX = mx;
  dragLastY = my;
}

//! DOCS
void GLFWCALL
mouseButtonCallback(const int button, const int state) {
  glfwGetMousePos(&dragStartX, &dragStartY);
  dragLastX = dragStartX;
  dragLastY = dragStartY;

  switch (button) {
  case GLFW_MOUSE_BUTTON_LEFT:
    switch (state) {
    case GLFW_PRESS:
      lmbPressed = true;
      break;
    case GLFW_RELEASE:
      lmbPressed = false;
      break;
    default:
      break;
    }
    break;
  case GLFW_MOUSE_BUTTON_MIDDLE:
    switch (state) {
    case GLFW_PRESS:
      mmbPressed = true;
      break;
    case GLFW_RELEASE:
      mmbPressed = false;
      break;
    default:
      break;
    }
    break;
  case GLFW_MOUSE_BUTTON_RIGHT:
    switch (state) {
    case GLFW_PRESS:
      rmbPressed = true;
      break;
    case GLFW_RELEASE:
      rmbPressed = false;
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

//! DOCS
void GLFWCALL
mouseWheelCallback(const int position) {
  GLfloat const minMeshDimension = 
    thx::min(meshGeometry.max[0] - meshGeometry.min[0], 
             meshGeometry.max[1] - meshGeometry.min[1], 
             meshGeometry.max[2] - meshGeometry.min[2]);
  vec3f const dt(0.f, 0.f, .05f*minMeshDimension);
  if (position > wheelLastPos) {
    slicePlaneModel.setTranslation(
      slicePlaneModel.translation() + dt);
  }
  else {
    slicePlaneModel.setTranslation(
      slicePlaneModel.translation() - dt);
  }
  sliceCamera.setView(SLICE_PLANE, slicePlaneModel);

  wheelLastPos = position;
}

// -----------------------------------------------------------------------------

//! Read shader source from file.
bool
readShaderFile(const std::string& fileName, std::string& src) {
  src.clear();
  FILE *file = fopen(fileName.c_str(), "rb");
  if (0 != file) {
    std::fseek(file, 0, SEEK_END);
    const long size = std::ftell(file); // [bytes].
    src.resize(size + 1);
    std::fseek(file, 0, SEEK_SET);
    std::fread(&src[0], size, 1, file);
    std::fclose(file);
    src[size] = '\0'; // Null-termination
    return true;
  }
  return false;
}

// -----------------------------------------------------------------------------

//! DOCS
template<class T>
void
bindUniformBuffer(ndj::ShaderProgram const& shaderProgram,
                  std::string const& uniformBlockName,
                  GLuint const uniformBlockBinding,
                  std::unique_ptr<ndj::UniformBuffer>& ubo,
                  T const& data) {
  ndj::ShaderProgram::UniformBlock const& uniformBlock = 
    shaderProgram.activeUniformBlock(uniformBlockName);
  if (uniformBlock.dataSize() != sizeof(T)) {
    NDJINN_THROW("invalid uniform block size");
  }

  uniformBlock.bind(uniformBlockBinding);
  ubo.reset(
    new ndj::UniformBuffer(
      sizeof(T), 
      &data, 
      GL_DYNAMIC_DRAW));
  ubo->bindBase(uniformBlockBinding);
}

//! DOCS
template <typename T>
void
vertexAttribPointer(GLint const attribLocation, 
                    GLboolean const normalize = GL_FALSE, 
                    GLsizei const stride = 0) {
  ndj::vertexAttribPointer(
    attribLocation,
    VertexAttribSize<T>::VALUE,
    VertexAttribType<T>::VALUE,
    normalize, 
    stride, 
    0); // Read from currently bound VBO.
}

// Read shaders from file and link shader program.
void
initShader(ndj::ShaderProgram& shaderProgram, 
           const std::string& vertexShaderFileName = "",
           const std::string& geometryShaderFileName = "",
           const std::string& fragmentShaderFileName = "") {
  using std::string;
  using std::cout;

  ndj::Shader vertexShader(GL_VERTEX_SHADER);
  ndj::Shader geometryShader(GL_GEOMETRY_SHADER);
  ndj::Shader fragmentShader(GL_FRAGMENT_SHADER);

  string vertexShaderSource;
  if (readShaderFile(vertexShaderFileName, vertexShaderSource)) {
    vertexShader.setSource(vertexShaderSource);
    vertexShader.compile();
    cout << "Compiled vertex shader: '" << vertexShaderFileName << "'\n";
    shaderProgram.attachShader(vertexShader);
  }

  string geometryShaderSource;
  if (readShaderFile(geometryShaderFileName, geometryShaderSource)) {
    geometryShader.setSource(geometryShaderSource);
    geometryShader.compile();
    cout << "Compiled geometry shader: '" << geometryShaderFileName << "'\n";
    shaderProgram.attachShader(geometryShader);
  }

  string fragmentShaderSource;
  if (readShaderFile(fragmentShaderFileName, fragmentShaderSource)) {
    fragmentShader.setSource(fragmentShaderSource);
    fragmentShader.compile();
    cout << "Compiled fragment shader: '" << fragmentShaderFileName << "'\n";
    shaderProgram.attachShader(fragmentShader);
  }
  
  shaderProgram.link();
  cout << "Linked shader program\n";

  if (!vertexShaderFileName.empty()) {
    shaderProgram.detachShader(vertexShader);
  }
  if (!geometryShaderFileName.empty()) {
    shaderProgram.detachShader(geometryShader);
  }
  if (!fragmentShaderFileName.empty()) {
    shaderProgram.detachShader(fragmentShader);
  }
}

//! DOCS
void 
initMeshShader() {
  using ndj::ShaderProgram;
  using ndj::Bindor;
  using ndj::ArrayBuffer;
  using ndj::ElementArrayBuffer;
  using ndj::VertexArray;
  using ndj::VertexAttribArrayEnabler;

  meshShader.program.reset(new ShaderProgram);
  initShader(*meshShader.program,
             shaderPath + "/mesh.vs",
             shaderPath + "/mesh.gs",
             shaderPath + "/mesh.fs");

  // Set up uniform buffers.

  bindUniformBuffer(
    *meshShader.program, 
    "Model", 
    MESH_MODEL_UNIFORM_BLOCK_BINDING, 
    meshModelUbo, 
    meshModel.uniformData());
  bindUniformBuffer(
    *meshShader.program, 
    "Material", 
    MESH_MATERIAL_UNIFORM_BLOCK_BINDING, 
    meshMaterialUbo, 
    MESH_MATERIAL.uniformData());
  bindUniformBuffer(
    *meshShader.program, 
    "Light", 
    SCENE_LIGHT_UNIFORM_BLOCK_BINDING, 
    sceneLightUbo, 
    sceneLight.uniformData());
  bindUniformBuffer(
    *meshShader.program, 
    "Camera", 
    SCENE_CAMERA_UNIFORM_BLOCK_BINDING, 
    sceneCameraUbo, 
    sceneCamera.uniformData());

  // Set up vertex attributes.

  meshShader.vertexArray.reset(new VertexArray);
  meshShader.vertexArray->bind();

  GLint const positionAttribLocation = 
    meshShader.program->activeAttrib("position").location;
  Bindor<ArrayBuffer> const positionBindor(*meshGeometry.positionVbo);
  VertexAttribArrayEnabler const positionVertexAttribEnabler(
    positionAttribLocation);
  vertexAttribPointer<vec3f>(positionAttribLocation);
  Bindor<ElementArrayBuffer> const indexBindor(*meshGeometry.indexVbo);
  meshShader.vertexArray->release();
}

//! DOCS
void 
initSlicePlaneShader() {
  using ndj::ShaderProgram;
  using ndj::Bindor;
  using ndj::ArrayBuffer;
  using ndj::ElementArrayBuffer;
  using ndj::VertexArray;
  using ndj::VertexAttribArrayEnabler;

  slicePlaneShader.program.reset(new ShaderProgram);
  initShader(*slicePlaneShader.program, 
             shaderPath + "/slicePlane.vs",
             "",
             shaderPath + "/slicePlane.fs");

  // Set up uniform buffers.

  bindUniformBuffer(
    *slicePlaneShader.program, 
    "Camera", 
    SCENE_CAMERA_UNIFORM_BLOCK_BINDING, 
    sceneCameraUbo, 
    sceneCamera.uniformData());
  bindUniformBuffer(
    *slicePlaneShader.program, 
    "Model", 
    SLICE_PLANE_MODEL_UNIFORM_BLOCK_BINDING, 
    slicePlaneModelUbo, 
    slicePlaneModel.uniformData());
  bindUniformBuffer(
    *slicePlaneShader.program, 
    "Color", 
    SLICE_PLANE_COLOR_UNIFORM_BLOCK_BINDING, 
    slicePlaneColorUbo, 
    SLICE_PLANE_COLOR.uniformData());

  // Set up vertex attributes.

  slicePlaneShader.vertexArray.reset(new VertexArray);
  slicePlaneShader.vertexArray->bind();
   
  GLint const positionAttribLocation = 
    slicePlaneShader.program->activeAttrib("position").location;
  Bindor<ArrayBuffer> const positionBindor(
    *slicePlaneGeometry.positionVbo);
  VertexAttribArrayEnabler const positionVertexAttribEnabler(
    positionAttribLocation);
  vertexAttribPointer<vec3f>(positionAttribLocation);
  Bindor<ElementArrayBuffer> const indexBindor(*slicePlaneGeometry.indexVbo);

  slicePlaneShader.vertexArray->release();
}

void
initContourPlaneShader() {
  using ndj::ShaderProgram;
  using ndj::Bindor;
  using ndj::ArrayBuffer;
  using ndj::ElementArrayBuffer;
  using ndj::VertexArray;
  using ndj::VertexAttribArrayEnabler;

  contourPlaneShader.program.reset(new ShaderProgram);
  initShader(*contourPlaneShader.program, 
             shaderPath + "/contourPlane.vs",
             "",
             shaderPath + "/contourPlane.fs");

  // Set up uniform buffers.

  bindUniformBuffer(
    *contourPlaneShader.program, 
    "Camera", 
    CONTOUR_PLANE_CAMERA_UNIFORM_BLOCK_BINDING, 
    contourPlaneCameraUbo, 
    CONTOUR_PLANE_CAMERA.uniformData());
  bindUniformBuffer(
    *contourPlaneShader.program, 
    "Model", 
    CONTOUR_PLANE_MODEL_UNIFORM_BLOCK_BINDING, 
    contourPlaneModelUbo,
    contourPlaneModel.uniformData());

  // Use texture unit 0.
  contourPlaneShader.program->uniform1<GLint>(
    contourPlaneShader.program->activeUniform("texUnit"), 
    CONTOUR_PLANE_TEX_UNIT);

  // Set up vertex attributes.

  contourPlaneShader.vertexArray.reset(new VertexArray);
  contourPlaneShader.vertexArray->bind();
   
  GLint const positionAttribLocation = 
    contourPlaneShader.program->activeAttrib("position").location;
  Bindor<ArrayBuffer> const positionBindor(
    *contourPlaneGeometry.positionVbo);
  VertexAttribArrayEnabler const positionVertexAttribEnabler(
    positionAttribLocation);
  vertexAttribPointer<vec3f>(positionAttribLocation);

  GLint const textureAttribLocation = 
    contourPlaneShader.program->activeAttrib("texture").location;
  Bindor<ArrayBuffer> const textureBindor(*contourPlaneGeometry.textureVbo);
  VertexAttribArrayEnabler const textureVertexAttribEnabler(
    textureAttribLocation);
  vertexAttribPointer<vec2f>(textureAttribLocation);
  Bindor<ElementArrayBuffer> const indexBindor(*contourPlaneGeometry.indexVbo);
  contourPlaneShader.vertexArray->release();
}

void
initSliceMeshShader() {
  using ndj::ShaderProgram;
  using ndj::Bindor;
  using ndj::ArrayBuffer;
  using ndj::ElementArrayBuffer;
  using ndj::VertexArray;
  using ndj::VertexAttribArrayEnabler;

  sliceMeshShader.program.reset(new ShaderProgram);
  initShader(*sliceMeshShader.program, 
             shaderPath + "/sliceMesh.vs",
             shaderPath + "/sliceMesh.gs",
             shaderPath + "/sliceMesh.fs");

  // Set up uniform buffers.

  bindUniformBuffer(
    *sliceMeshShader.program, 
    "Camera", 
    SLICE_CAMERA_UNIFORM_BLOCK_BINDING, 
    sliceCameraUbo, 
    sliceCamera.uniformData());
  bindUniformBuffer(
    *sliceMeshShader.program, 
    "Model", 
    MESH_MODEL_UNIFORM_BLOCK_BINDING, 
    meshModelUbo, 
    meshModel.uniformData());
  bindUniformBuffer(
    *sliceMeshShader.program, 
    "Color", 
    SLICE_LINE_COLOR_UNIFORM_BLOCK_BINDING, 
    sliceLineColorUbo, 
    SLICE_LINE_COLOR.uniformData());

  // Set up vertex attributes.

  sliceMeshShader.vertexArray.reset(new VertexArray);
  sliceMeshShader.vertexArray->bind();

  GLint const positionAttribLocation = 
    sliceMeshShader.program->activeAttrib("position").location;
  Bindor<ArrayBuffer> const positionBindor(*meshGeometry.positionVbo);
  VertexAttribArrayEnabler const positionVertexAttribEnabler(
    positionAttribLocation);
  vertexAttribPointer<vec3f>(positionAttribLocation);
  Bindor<ElementArrayBuffer> const indexBindor(*meshGeometry.indexVbo);
  sliceMeshShader.vertexArray->release();
}

//! DOCS
void
initSliceLinesShader() {
  using ndj::ShaderProgram;
  using ndj::Bindor;
  using ndj::ArrayBuffer;
  using ndj::ElementArrayBuffer;
  using ndj::VertexArray;
  using ndj::VertexAttribArrayEnabler;

  sliceLinesShader.program.reset(new ShaderProgram);
  initShader(*sliceLinesShader.program, 
             shaderPath + "/sliceLines.vs",
             "",
             shaderPath + "/sliceLines.fs");

  // Set up uniform buffers.

  bindUniformBuffer(
    *sliceLinesShader.program, 
    "Camera", 
    SLICE_CAMERA_UNIFORM_BLOCK_BINDING, 
    sliceCameraUbo, 
    sliceCamera.uniformData());
  bindUniformBuffer(
    *sliceLinesShader.program, 
    "Color", 
    SLICE_LINE_COLOR_UNIFORM_BLOCK_BINDING, 
    sliceLineColorUbo, 
    SLICE_LINE_COLOR.uniformData());

  // Set up vertex attributes.

  sliceLinesShader.vertexArray.reset(new VertexArray);
  sliceLinesShader.vertexArray->bind();

  GLint const positionAttribLocation = 
    sliceLinesShader.program->activeAttrib("position").location;
  Bindor<ArrayBuffer> const positionBindor(*sliceLinesGeometry.positionVbo);
  VertexAttribArrayEnabler const positionVertexAttribEnabler(
    positionAttribLocation);
  vertexAttribPointer<vec3f>(positionAttribLocation);
  Bindor<ElementArrayBuffer> const indexBindor(*sliceLinesGeometry.indexVbo);
  sliceLinesShader.vertexArray->release();
}

// -----------------------------------------------------------------------------

void
initSceneCamera() {
  GLfloat const meshRadius = .5f*thx::mag(meshGeometry.max - meshGeometry.min);
  Frustum newFrustum = sceneCamera.frustum();
  newFrustum.near = 1.f;//0.5f*meshRadius;
  newFrustum.far = 1000.f;//;
  sceneCamera.setView(vec3f(0.f, 0.f, 2.f*meshRadius), vec3f(0.f, 0.f, 0.f));
  sceneCamera.setProjection(newFrustum);
  sceneLight.setPosition(vec4f(sceneCamera.position(), 1.f));
}

void
initSliceCamera() {
  GLfloat const SLICE_CAMERA_NEAR = -.5f;
  GLfloat const SLICE_CAMERA_FAR = .5f;

  //vec3f const meshCenter = .5f*(meshGeometry.max + meshGeometry.min);
  //slicePlaneModel.setTranslation(-meshCenter);

  sliceCamera.setView(SLICE_PLANE, slicePlaneModel);
  GLfloat const meshRadius = .5f*thx::mag(meshGeometry.max - meshGeometry.min);
  sliceCamera.setProjection(Frustum(
    -meshRadius, meshRadius,
    -meshRadius, meshRadius,
    SLICE_CAMERA_NEAR, SLICE_CAMERA_FAR));
}

void
initMeshGeometry(std::string const& fileName) {
  using std::string;
  using std::vector;
  using std::min;
  using std::max;
  using std::size_t;

  obj::read(fileName, meshVertices, meshIndices);

  meshGeometry.min[0] = std::numeric_limits<GLfloat>::max();
  meshGeometry.min[1] = std::numeric_limits<GLfloat>::max();
  meshGeometry.min[2] = std::numeric_limits<GLfloat>::max();
  meshGeometry.max[0] = -std::numeric_limits<GLfloat>::max();
  meshGeometry.max[1] = -std::numeric_limits<GLfloat>::max();
  meshGeometry.max[2] = -std::numeric_limits<GLfloat>::max();

  for (size_t i = 0; i < meshVertices.size(); ++i) {
    for (size_t j = 0; j < 3; ++j) {
      meshGeometry.min[j] = 
        min<GLfloat>(meshGeometry.min[j], meshVertices[i][j]);
      meshGeometry.max[j] = 
        max<GLfloat>(meshGeometry.max[j], meshVertices[i][j]);
    }
  }

  vec3f const meshCenter = .5f*(meshGeometry.max + meshGeometry.min);
  meshModel.setTranslation(-meshCenter);

  std::cout 
    << "Mesh vertex count: " << meshVertices.size() << "\n"
    << "Mesh triangle count: " << meshIndices.size() << "\n"
    << "Mesh min: " << meshGeometry.min << "\n"
    << "Mesh max: " << meshGeometry.max << "\n"
    << "Mesh center: " << meshCenter << "\n";

  meshGeometry.positionVbo.reset(
    new ndj::ArrayBuffer(
      meshVertices.size()*sizeof(vec3f), 
      &meshVertices[0]));
  meshGeometry.indexVbo.reset(
    new ndj::ElementArrayBuffer(
      meshIndices.size()*sizeof(vec3ui), 
      &meshIndices[0]));
}

void
initContourPlaneGeometry() {
  Frustum const& f = CONTOUR_PLANE_CAMERA.frustum();
  GLfloat const xMin = f.left;
  GLfloat const xMax = f.right;
  GLfloat const yMin = f.bottom;
  GLfloat const yMax = f.top;
  GLfloat const zMid = .5f*(f.near + f.far);

  static vec3f const CONTOUR_PLANE_VERTICES[4] = { 
    vec3f(xMax, yMin,  zMid), 
    vec3f(xMax, yMax,  zMid), 
    vec3f(xMin, yMax,  zMid), 
    vec3f(xMin, yMin,  zMid), 
  };

  static vec2f const CONTOUR_PLANE_TEX_COORDS[4] = { 
    vec2f(1.f, 0.f), 
    vec2f(1.f, 1.f), 
    vec2f(0.f, 1.f), 
    vec2f(0.f, 0.f), 
  };

  static const vec3ui CONTOUR_PLANE_INDICES[2] = {
    vec3ui(0, 1, 2),
    vec3ui(2, 3, 0)
  };

  contourPlaneGeometry.positionVbo.reset(
    new ndj::ArrayBuffer(
      4*sizeof(vec3f), CONTOUR_PLANE_VERTICES));
  contourPlaneGeometry.textureVbo.reset(
    new ndj::ArrayBuffer(
      4*sizeof(vec2f), CONTOUR_PLANE_TEX_COORDS));
  contourPlaneGeometry.indexVbo.reset(
    new ndj::ElementArrayBuffer(
      2*sizeof(vec3ui), CONTOUR_PLANE_INDICES));
  contourPlaneGeometry.min = vec3f(xMin, yMin, zMid);
  contourPlaneGeometry.max = vec3f(xMax, yMax, zMid);

  contourPlaneModel.setNormalMatrix(
    CONTOUR_PLANE_CAMERA.uniformData().viewMatrix);
}

void
initSlicePlaneGeometry() {
  Frustum const& f = sliceCamera.frustum();
  GLfloat const xMin = f.left;
  GLfloat const xMax = f.right;
  GLfloat const yMin = f.bottom;
  GLfloat const yMax = f.top;
  GLfloat const zMid = .5f*(f.near + f.far);

  static vec3f const SLICE_PLANE_VERTICES[4] = { 
    vec3f(xMax, yMin,  zMid), 
    vec3f(xMax, yMax,  zMid), 
    vec3f(xMin, yMax,  zMid), 
    vec3f(xMin, yMin,  zMid), 
  };

  static const vec3ui SLICE_PLANE_INDICES[2] = {
    vec3ui(0, 1, 2),
    vec3ui(2, 3, 0)
  };


  slicePlaneGeometry.positionVbo.reset(
    new ndj::ArrayBuffer(
      4*sizeof(vec3f), SLICE_PLANE_VERTICES));
  slicePlaneGeometry.indexVbo.reset(
    new ndj::ElementArrayBuffer(
      2*sizeof(vec3ui), SLICE_PLANE_INDICES));
  slicePlaneGeometry.min = vec3f(xMin, yMin, zMid);
  slicePlaneGeometry.max = vec3f(xMax, yMax, zMid);
}

void
initSliceLinesGeometry() {
  sliceLinesGeometry.positionVbo.reset(new ndj::ArrayBuffer);
  sliceLinesGeometry.indexVbo.reset(new ndj::ElementArrayBuffer);
  //sliceLinesGeometry.min = vec3f(xMin, yMin, zMid);
  //sliceLinesGeometry.max = vec3f(xMax, yMax, zMid);
}

void
initContourPlaneSampler() {
  contourPlaneSampler.reset(new ndj::Sampler);
  contourPlaneSampler->parameter<GLint>(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  contourPlaneSampler->parameter<GLint>(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

// -----------------------------------------------------------------------------

void
renderTriangles(Shader const& shader,
                GLuint const start,
                GLuint const end,
                GLsizei const count,
                GLenum const type) 
{
  using ndj::Bindor;
  using ndj::ShaderProgram;
  using ndj::VertexArray;

  Bindor<ShaderProgram> const spBindor(*shader.program);
  Bindor<VertexArray> const vaBindor(*shader.vertexArray);
  ndj::drawRangeElements(
    GL_TRIANGLES, 
    start,
    end,
    count,
    type,
    0); // Read indices from currently bound element array.
}

void 
renderLines(Shader const& shader,
            GLuint const start,
            GLuint const end,
            GLsizei const count,
            GLenum const type) 
{
  using ndj::Bindor;
  using ndj::ShaderProgram;
  using ndj::VertexArray;

  Bindor<ShaderProgram> const spBindor(*shader.program);
  Bindor<VertexArray> const vaBindor(*shader.vertexArray);
  ndj::drawRangeElements(
    GL_LINES, 
    start,
    end,
    count,
    type,
    0); // Read indices from currently bound element array.
}

void 
renderMesh() {
  using ndj::elementCount;
  using ndj::Enabler;

  meshModelUbo->data(
    sizeof(Model::UniformData), &meshModel.uniformData());
  
  Enabler depthTestEnabler(GL_DEPTH_TEST);
  renderTriangles(
    meshShader, 
    0, 
    elementCount<vec3f, GLuint>(*meshGeometry.positionVbo), 
    3*elementCount<vec3ui, GLsizei>(*meshGeometry.indexVbo),
    GLTypeEnum<vec3ui::value_type>::value);
}

void 
renderSlicePlane() {
  using ndj::elementCount;
  using ndj::Enabler;  

  slicePlaneModelUbo->data(
    sizeof(Model::UniformData), &slicePlaneModel.uniformData());

  Enabler const blendEnabler(GL_BLEND);
  ndj::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  Enabler const depthTestEnabler(GL_DEPTH_TEST);
  
  renderTriangles(
    slicePlaneShader, 
    0, 
    elementCount<vec3f, GLuint>(*slicePlaneGeometry.positionVbo),
    3*elementCount<vec3ui, GLuint>(*slicePlaneGeometry.indexVbo),
    GLTypeEnum<vec3ui::value_type>::value);
}

void
renderContourPlane() {
  using ndj::elementCount;
  using ndj::blendFunc;
  using ndj::activeTexture;
  using ndj::Bindor;
  using ndj::Texture;
  using ndj::Sampler;
  using ndj::Enabler;

  ndj::viewport(contourPlaneViewport.x, 
                contourPlaneViewport.y, 
                contourPlaneViewport.width, 
                contourPlaneViewport.height);

  Enabler blendEnabler(GL_BLEND);
  blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  activeTexture(GL_TEXTURE0 + CONTOUR_PLANE_TEX_UNIT);
  Bindor<Texture> const texBindor(*sliceTexture);
  Bindor<Sampler> const samplerBindor(
    *contourPlaneSampler, CONTOUR_PLANE_TEX_UNIT);

  renderTriangles(
    contourPlaneShader,
    0, 
    elementCount<vec3f, GLuint>(*contourPlaneGeometry.positionVbo),
    3*elementCount<vec3ui, GLsizei>(*contourPlaneGeometry.indexVbo),
    GLTypeEnum<vec3ui::value_type>::value);
}

//! DOCS
void
renderScene() {
  ndj::viewport(sceneViewport.x, 
                sceneViewport.y, 
                sceneViewport.width, 
                sceneViewport.height);

  sceneCameraUbo->data(
    sizeof(PerspCamera::UniformData), &sceneCamera.uniformData());
  sceneLightUbo->data(
    sizeof(Light::UniformData), &sceneLight.uniformData());

  renderMesh();
  renderSlicePlane();
}

//! DOCS
void 
render() {
  ndj::clearColor(SCENE_CLEAR_COLOR[0], 
                  SCENE_CLEAR_COLOR[1],
                  SCENE_CLEAR_COLOR[2],
                  SCENE_CLEAR_COLOR[3]);
  ndj::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  renderScene();
  renderContourPlane();
}

// -----------------------------------------------------------------------------

void
sliceMeshGpu() {
  using ndj::Enabler;
  using ndj::elementCount;

  sliceCameraUbo->data(
    sizeof(OrthoCamera::UniformData), &sliceCamera.uniformData());
  sliceLineColorUbo->data(
    sizeof(Color::UniformData), &SLICE_LINE_COLOR.uniformData());
  meshModelUbo->data(
    sizeof(Model::UniformData), &meshModel.uniformData());
  
  Enabler depthTestEnabler(GL_DEPTH_TEST);
  ndj::lineWidth(SLICE_LINE_WIDTH);
  renderTriangles(
    sliceMeshShader, 
    0, 
    elementCount<vec3f, GLuint>(*meshGeometry.positionVbo), 
    3*elementCount<vec3ui, GLsizei>(*meshGeometry.indexVbo),
    GLTypeEnum<vec3ui::value_type>::value);
}

void 
sliceMeshCpu() {
  using ndj::elementCount;
  using std::size_t;

  vec3f const VIEW_PLANE_NORMAL(0.f, 0.f, 1.f);
  GLfloat const VIEW_PLANE_DISTANCE = 0.f;

  auto const pointPlaneDistance = 
    [](vec3f const& point,
       vec3f const& planeNormal,
       GLfloat const planeDistance) 
  {
    return thx::dot(planeNormal, point) + planeDistance;
  };

  auto const edgePlaneIntersection = 
    [](vec3f const& p1, vec3f const& p2, 
       GLfloat const d1, GLfloat const d2) -> vec3f
  {
    GLfloat const t = d1/(d1 - d2);
    return p1 + t*(p2 - p1);
  };

  // Emulate GLSL!
  vec3f gl_Position;
  vector<vec3f> lineVertices; // Clip space!
  vector<vec2ui> lineIndices;
  auto const EmitVertex = 
    [&]() 
  {
    lineVertices.push_back(gl_Position);    
  };

  auto const EndPrimitive =
    [&]()
  {
    lineIndices.push_back(
      vec2ui(lineVertices.size() - 2, lineVertices.size() - 1));
  };


  mat4f const viewModelMatrix = 
    sliceCamera.uniformData().viewMatrix*meshModel.uniformData().modelMatrix;
  //mat4f const projectionMatrix = 
  //  sliceCamera.uniformData().projectionMatrix;

  size_t const triangleCount = meshIndices.size();
  for (size_t i = 0; i < triangleCount; ++i) {
    vec3f const esx0 = viewModelMatrix*meshVertices[meshIndices[i][0]];
    vec3f const esx1 = viewModelMatrix*meshVertices[meshIndices[i][1]];
    vec3f const esx2 = viewModelMatrix*meshVertices[meshIndices[i][2]];

    GLfloat const d0 = 
      pointPlaneDistance(esx0, VIEW_PLANE_NORMAL, VIEW_PLANE_DISTANCE);
    GLfloat const d1 = 
      pointPlaneDistance(esx1, VIEW_PLANE_NORMAL, VIEW_PLANE_DISTANCE);
    GLfloat const d2 = 
      pointPlaneDistance(esx2, VIEW_PLANE_NORMAL, VIEW_PLANE_DISTANCE);

    // All triangle vertices on the positive side of the plane.
    if (d0 > 0.f && d1 > 0.f && d2 > 0.f) {
      continue;//return;
    }

    // All triangle vertices on the negative side of the plane.
    if (d0 < 0.f && d1 < 0.f && d2 < 0.f) {
      continue;//return;
    }

    // All triangle vertices on the plane. Cannot output a line segment.
    if (d0 == 0.f && d1 == 0.f && d2 == 0.f) {
      continue;//return;
    }

    // Edge0 lies in plane. Output edge0 as a line.
    if (d0 == 0.f && d1 == 0.f) {
      gl_Position = /*projectionMatrix*/esx0;
      EmitVertex();
      gl_Position = /*projectionMatrix*/esx1;
      EmitVertex();
      EndPrimitive();
      continue;//return;
    }

    // Edge1 lies in plane. Output edge1 as a line.
    if (d1 == 0.f && d2 == 0.f) {
      gl_Position = /*projectionMatrix*/esx1;
      EmitVertex();
      gl_Position = /*projectionMatrix*/esx2;
      EmitVertex();
      EndPrimitive();
      continue;//return;
    }

    // Edge2 lies in plane. Output edge2 as a line.
    if (d2 == 0.f && d0 == 0.f) {
      gl_Position = /*projectionMatrix*/esx2;
      EmitVertex();
      gl_Position = /*projectionMatrix*/esx0;
      EmitVertex();
      EndPrimitive();
      continue;//return;
    }

    // Vertex0 lies in the plane.
    if (d0 == 0.f) {
      gl_Position = /*projectionMatrix*/esx0;
      EmitVertex();
      EmitVertex();
      EndPrimitive();
      continue;//return;
    }

    // Vertex1 lies in the plane.
    if (d1 == 0.f) {
      gl_Position = /*projectionMatrix*/esx1;
      EmitVertex();
      EmitVertex();
      EndPrimitive();
      continue;//return;
    }

    // Vertex2 lies in the plane.
    if (d2 == 0.f) {
      gl_Position = /*projectionMatrix*/esx2;
      EmitVertex();
      EmitVertex();
      EndPrimitive();
      continue;//return;
    }

    int lineIndex = 0;
    vec3f edgeIntersections[2];

    // Edge0 intersects plane.
    if (thx::signum(d0) != thx::signum(d1)) {
      edgeIntersections[lineIndex] = edgePlaneIntersection(esx0, esx1, d0, d1);
      ++lineIndex;
    }

    // Edge1 intersects plane.
    if (thx::signum(d1) != thx::signum(d2)) {
      edgeIntersections[lineIndex] = edgePlaneIntersection(esx1, esx2, d1, d2);
      ++lineIndex;
    }

    // Edge2 intersects plane.
    if (thx::signum(d2) != thx::signum(d0)) {
      edgeIntersections[lineIndex] = edgePlaneIntersection(esx2, esx0, d2, d0);
      ++lineIndex;
    }

    gl_Position = /*projectionMatrix*/edgeIntersections[0];
    EmitVertex();
    gl_Position = /*projectionMatrix*/edgeIntersections[1];
    EmitVertex();
    EndPrimitive();
  }

  sliceLinesGeometry.positionVbo->data(
    lineVertices.size()*sizeof(vec3f),
    &lineVertices[0]);
  sliceLinesGeometry.indexVbo->data(
    lineIndices.size()*sizeof(vec2ui),
    &lineIndices[0]);

  sliceCameraUbo->data(
    sizeof(OrthoCamera::UniformData), &sliceCamera.uniformData());
  sliceLineColorUbo->data(
    sizeof(Color::UniformData), &SLICE_LINE_COLOR.uniformData());

  ndj::lineWidth(SLICE_LINE_WIDTH);

  renderLines(
    sliceLinesShader,
    0,
    elementCount<vec3f, GLuint>(*sliceLinesGeometry.positionVbo),
    2*elementCount<vec2ui, GLsizei>(*sliceLinesGeometry.indexVbo),
    GLTypeEnum<vec2ui::value_type>::value);
}

void
sliceMesh() {
  using ndj::elementCount;
  using ndj::Enabler;
  using ndj::Bindor;
  using ndj::Framebuffer;

  Bindor<Framebuffer> const fboBindor(*sliceFbo);

  ndj::viewport(0, 0, sliceTexture->width(), sliceTexture->height());
  ndj::clearColor(SLICE_CLEAR_COLOR[0], 
                  SLICE_CLEAR_COLOR[1], 
                  SLICE_CLEAR_COLOR[2], 
                  SLICE_CLEAR_COLOR[3]);
  ndj::clear(GL_COLOR_BUFFER_BIT);

  switch (sliceMode) {
  case GPU:
    sliceMeshGpu();
    break;
  case CPU:
    sliceMeshCpu();
    break;
  }
}

// -----------------------------------------------------------------------------

int
main(int argc, char *argv[])
{
  using std::cerr;
  using std::cout;
  using std::string;
  using ndj::getString;
  using ndj::getIntegerv;

  try {
    int glfwMajor = 0;
    int glfwMinor = 0;
    int glfwRev = 0;
    glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRev);
    cout << "GLFW version: " 
      << glfwMajor << "." << glfwMinor << "." << glfwRev << "\n";
    if (!glfwInit()) {
      cerr << "GLFW init error.\n";
      return EXIT_FAILURE;
    }

    // Open an OpenGL window. Creates an OpenGL context.
    glfwSetWindowTitle("Mesh Slicer");
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4); // 4 x AA
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 4);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE/*GLFW_OPENGL_COMPAT_PROFILE*/);
    GLFWvidmode vm;
    glfwGetDesktopMode(&vm);
    int const ALPHA_BITS = 0;
    int const DEPTH_BITS = 0;
    int const STENCIL_BITS = 0;
    if (GL_FALSE == glfwOpenWindow(
           winSize[0], 
           winSize[1], 
           vm.RedBits,
           vm.GreenBits,
           vm.BlueBits,
           ALPHA_BITS,
           DEPTH_BITS, 
           STENCIL_BITS, 
           GLFW_WINDOW)) {
        glfwTerminate();
        cerr << "GLFW Open Window error.\n";
        return EXIT_FAILURE;
    }
    std::cout << "Red bits: " << vm.RedBits << std::endl;
    std::cout << "Green bits: " << vm.GreenBits << std::endl;
    std::cout << "Blue bits: " << vm.BlueBits << std::endl;

    initGLEW();
    initGL();
    
    glfwEnable(GLFW_KEY_REPEAT);
    glfwSwapInterval(0);    // Disable vsync.

    // Set callback functions.
    glfwSetWindowSizeCallback(windowSizeCallback);
    glfwSetKeyCallback(keyCallback);
    glfwSetMousePosCallback(mousePosCallback);
    glfwSetMouseButtonCallback(mouseButtonCallback);
    glfwSetMouseWheelCallback(mouseWheelCallback);

    initMeshGeometry(meshPath);
    initSceneCamera();
    initSliceCamera();
    initSlicePlaneGeometry();
    initContourPlaneGeometry();
    initSliceLinesGeometry();

    initContourPlaneSampler();

    initMeshShader();
    initSlicePlaneShader();
    initContourPlaneShader();
    initSliceMeshShader();
    initSliceLinesShader();

    double const FPS_INTERVAL = 5.0;
    Timer fpsTimer;
    size_t frameCount = 0;

    // Main loop, check if ESC key was pressed or window was closed.
    while (!glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED)) {
      // OpenGL rendering goes here...
      sliceMesh();
      render();  
      glfwSwapBuffers();
      ++frameCount;
      if (fpsTimer.elapsed() > FPS_INTERVAL) {
        cout << "FPS: " << frameCount / fpsTimer.elapsed() << "\n";
        fpsTimer.reset();
        frameCount = 0;
      }
    }
    
    glfwTerminate(); // Close window and terminate GLFW.
    return EXIT_SUCCESS;
  }
  catch (std::exception const& ex) {
    cerr << "Exception: " << ex.what() << "\n";
    glfwTerminate(); // Close window and terminate GLFW.
    abort();
  }
  catch (...) {
    cerr << "Unknown exception\n";
    glfwTerminate(); // Close window and terminate GLFW.
    abort();
  }
}



#if 0
  static const vec3f cubeMeshVertices[8] = { 
    vec3f(-1.f, -1.f,  1.f), 
    vec3f( 1.f, -1.f,  1.f), 
    vec3f( 1.f,  1.f,  1.f), 
    vec3f(-1.f,  1.f,  1.f), 
    vec3f(-1.f, -1.f, -1.f), 
    vec3f( 1.f, -1.f, -1.f), 
    vec3f( 1.f,  1.f, -1.f), 
    vec3f(-1.f,  1.f, -1.f) 
  };

  static const vec3ui cubeMeshIndices[12] = {
    vec3ui(5, 2, 1), // X+ 
    vec3ui(5, 6, 2),
    vec3ui(2, 6, 7), // Y+
    vec3ui(2, 7, 3),
    vec3ui(1, 2, 3), // Z+
    vec3ui(1, 3, 0), 
    vec3ui(3, 4, 0), // X-
    vec3ui(3, 7, 4),
    vec3ui(0, 5, 1), // Y-
    vec3ui(0, 4, 5),
    vec3ui(6, 4, 7), // Z-
    vec3ui(6, 5, 4)
  };

  meshGeometry.positionVbo->bufferData(
    8*sizeof(vec3f), cubeMeshVertices);
  meshGeometry.indexVbo->bufferData(
    12*sizeof(vec3ui), cubeMeshIndices);
#endif

#if 0
  std::vector<vec3f> positions;
  std::vector<vec3ui> indices;
  for (int i = 0; i <= 1783; ++i) {
    std::stringstream ss;
    ss << "D:/code/demo/mesh_slicer/data/perfexion/object";
    if (i < 1000) {
      ss << 0;
    }
    if (i < 100) {
      ss << 0;
    }
    if (i < 10) {
      ss << 0;
    }
    ss << i << ".obj";
    std::cout << ss.str() << std::endl;


    std::vector<vec3f> pos;
    std::vector<vec3ui> idx;
    obj::read(ss.str(), pos, idx);

    std::size_t offset = positions.size();

    for (std::size_t i = 0; i < pos.size(); ++i) {
      positions.push_back(pos[i]);
    }
    
    for (std::size_t i = 0; i < idx.size(); ++i) {
      indices.push_back(vec3ui(idx[i][0] + offset, idx[i][1] + offset, idx[i][2] + offset));
    }
  }

  obj::write("D:/code/demo/mesh_slicer/data/perfexion.obj", positions, indices);
#endif
