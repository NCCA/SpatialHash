#include <QMouseEvent>
#include <QGuiApplication>
#include <QElapsedTimer>
#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Transformation.h>
#include <ngl/SimpleVAO.h>
#include <ngl/VAOFactory.h>
#include <ngl/Random.h>
#include <iostream>
#include <ngl/NGLStream.h>

NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  setTitle("Spatial Hash Demo");
  m_hash = std::make_unique<SpatialHash>(1.0f);
}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::generatePoints(size_t _num)
{
  m_points.clear();
  m_points.resize(_num);
  for(auto &p : m_points)
  {
    p=ngl::Random::getRandomPoint(20,20,20);
  }
}

void NGLScene::resizeGL(int _w , int _h)
{
  m_projection = ngl::perspective(45.0f, static_cast<float>(_w) / _h, 0.1f, 200.0f);

  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
  m_text->setScreenSize(width(), height());
}

constexpr auto SphereShader="SphereShader";
void NGLScene::initializeGL()
{
  // we must call that first before any other GL commands to load and link the
  // gl commands from the lib, if that is not done program will crash
  ngl::NGLInit::initialize();
  glClearColor(0.7f, 0.7f, 0.7f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  ngl::ShaderLib::loadShader(SphereShader,"shaders/SphereShaderVertex.glsl","shaders/SphereShaderFragment.glsl");
  m_view = ngl::lookAt({60.0f, 60.0f, 60.0f}, {0,0,0}, {0,1,0});

  ngl::VAOPrimitives::createSphere("sphere",0.5f,20.0f);
  ngl::ShaderLib::use(SphereShader);
  generatePoints(m_numPoints);
  m_colours=ngl::generateDistinctColours(m_numPoints);
  m_vao = ngl::VAOFactory::createVAO(ngl::simpleVAO, GL_LINES);
  m_text = std::make_unique<ngl::Text>("fonts/Arial.ttf", 18);
  m_text->setScreenSize(1024, 720);
  startTimer(10);
}

void NGLScene::updateHash()
{
  m_hash->clear();
  for(auto p : m_points)
  {
    m_hash->insert(p);
  }
}

void NGLScene::paintGL()
{
  glViewport(0, 0, m_win.width, m_win.height);
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  ngl::ShaderLib::use(SphereShader);
  // Rotation based on the mouse position for our global transform
  auto rotX = ngl::Mat4::rotateX(m_win.spinXFace);
  auto rotY = ngl::Mat4::rotateY(m_win.spinYFace);
  // multiply the rotations
  m_mouseGlobalTX = rotX * rotY;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  ngl::Transformation tx;
  size_t i=0;
  if(m_showAll)
  {
    for(auto s : m_points)
    {
      tx.setPosition(s);
      tx.setScale(1.2f,1.2f,1.2f);
      ngl::ShaderLib::setUniform("MVP",m_projection*m_view*m_mouseGlobalTX*tx.getMatrix());
      ngl::ShaderLib::setUniform("colour",m_colours[i++]);
      ngl::VAOPrimitives::draw("sphere");
    }
  }
  auto spheres=m_hash->query(m_hashPos,m_radius);
  std::vector<ngl::Vec3> data;
  for(auto s : spheres)
  {
    tx.setPosition(s);
    data.push_back(m_hashPos);
    data.push_back(s);
    ngl::ShaderLib::setUniform("MVP",m_projection*m_view*m_mouseGlobalTX*tx.getMatrix());
    ngl::ShaderLib::setUniform("colour",1.0f,0.0f,0.0f);
    ngl::VAOPrimitives::draw("cube");
  }


  ngl::ShaderLib::use("nglColourShader");

  ngl::Mat4 MVP;
  MVP = m_projection * m_view * m_mouseGlobalTX;
  ngl::ShaderLib::setUniform("MVP", MVP);
  m_vao->bind();
  m_vao->setData(ngl::SimpleVAO::VertexData(data.size() * sizeof(ngl::Vec3), data[0].m_x));
  // We must do this each time as we change the data.
  m_vao->setVertexAttributePointer(0, 3, GL_FLOAT, 0, 0);
  m_vao->setNumIndices(data.size());
  m_vao->draw();
  m_vao->unbind();

  m_text->setColour(1.0f, 1.0f, 1.0f);
  std::string text = fmt::format("Search Radius {} ", m_radius);
  m_text->renderText(10, 700, text);
  text = fmt::format("Position [{},{},{}] ", m_hashPos.m_x, m_hashPos.m_y, m_hashPos.m_z);
  m_text->renderText(10, 680, text);
  
  text = fmt::format("Num Points {} Hash Size {} Found {} ",m_points.size(),m_hash->size(), spheres.size());
  m_text->renderText(10, 660, text);
  



}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  case Qt::Key_Space :
      m_win.spinXFace=0;
      m_win.spinYFace=0;
      m_modelPos.set(ngl::Vec3::zero());

  break;

  case Qt::Key_Plus :
  case Qt::Key_Equal :
    m_radius+=1.0f;
    std::clamp(m_radius,2.0f,20.0f);
    break;
  case Qt::Key_Minus :
    m_radius-=1.0f;
    std::clamp(m_radius,2.0f,20.0f);
    break;


  case Qt::Key_Left : m_hashPos.m_x-=1.0f; break;
  case Qt::Key_Right : m_hashPos.m_x+=1.0f; break;
  case Qt::Key_Up : m_hashPos.m_y+=1.0f; break;
  case Qt::Key_Down : m_hashPos.m_y-=1.0f; break;
  case Qt::Key_I : m_hashPos.m_z+=1.0f; break;
  case Qt::Key_O : m_hashPos.m_z-=1.0f; break;
  case Qt::Key_A : m_showAll^=true; break;
  case Qt::Key_1 : 
    m_numPoints-=100; 
    std::clamp(m_numPoints,size_t(100),size_t(100000));
    generatePoints(m_numPoints); 
  break;
  case Qt::Key_2 : 
    m_numPoints+=100; 
    std::clamp(m_numPoints,size_t(100),size_t(100000));
    generatePoints(m_numPoints); 
  break;
case Qt::Key_3 : 
    m_numPoints-=100; 
    std::clamp(m_numPoints,size_t(100),size_t(100000));
    generatePoints(m_numPoints); 
  break;
  case Qt::Key_4 : 
    m_numPoints+=1000; 
    std::clamp(m_numPoints,size_t(100),size_t(100000));
    generatePoints(m_numPoints); 
  break;
  default : break;
  }
  // finally update the GLWindow and re-draw

    update();
}


void NGLScene::timerEvent(QTimerEvent *)
{
  QElapsedTimer timer;
  timer.start();
  updateHash();
  qDebug() << "Hash took" << timer.elapsed() << "milliseconds";

  update();
}