#include "mainwindow.h"
#include <WinUser.h>
#include <wingdi.h>

#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")

MainWindow::MainWindow(QWidget *parent)
    : QOpenGLWidget(parent), m_iSpinAngle(0)
{
    HDC h = GetDC(NULL);
    float fHrntDpi = GetDeviceCaps(h, LOGPIXELSX);//获取的是每inch的像素数
    float fVertDpi = GetDeviceCaps(h, LOGPIXELSY);
    float fAvrDpi = (fHrntDpi + fVertDpi) / 2;
    int iDim = 90 / 25.4 * fAvrDpi;//一个窗体分成3x3=9份，一个象限是30mmx30mm的正方形区域，故整个窗体大小90mm.90/25.4是窗体的英寸数
    resize(iDim, iDim);
    ReleaseDC(NULL, h);
    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
}

MainWindow::~MainWindow()
{
    m_timer.stop();
    m_pTextures->release();
    delete m_pTextures;
    delete m_pProgram;
    delete [] m_pVertices;

}

void MainWindow::initializeGL()
{
    initializeOpenGLFunctions();
    m_iCount = 0;
    m_uiTexNum = 0;
    QImage img(QString("MirrorEarth.bmp"));
    //1 给顶点赋值
    int iAngleStep = 10, iR = 1;
    m_pVertices = new GLfloat[180 / iAngleStep * 360 / iAngleStep * 6 * 3];
    for(int iLongitude = 0; iLongitude < 360; iLongitude += iAngleStep)
    {
        for(int iLatitude = 0; iLatitude < 180; iLatitude += iAngleStep)
        {
            m_pVertices[m_iCount++] = iLongitude;
            m_pVertices[m_iCount++] = iLatitude;
            m_pVertices[m_iCount++] = iR;

            m_pVertices[m_iCount++] = iLongitude;
            m_pVertices[m_iCount++] = iLatitude + iAngleStep;
            m_pVertices[m_iCount++] = iR;

            m_pVertices[m_iCount++] = iLongitude + iAngleStep;
            m_pVertices[m_iCount++] = iLatitude + iAngleStep;
            m_pVertices[m_iCount++] = iR;

            m_pVertices[m_iCount++] = iLongitude + iAngleStep;
            m_pVertices[m_iCount++] = iLatitude + iAngleStep;
            m_pVertices[m_iCount++] = iR;

            m_pVertices[m_iCount++] = iLongitude + iAngleStep;
            m_pVertices[m_iCount++] = iLatitude;
            m_pVertices[m_iCount++] = iR;

            m_pVertices[m_iCount++] = iLongitude;
            m_pVertices[m_iCount++] = iLatitude;
            m_pVertices[m_iCount++] = iR;
        }
    }

    //2 建立GLSL shader
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =
                        "#version 330\n"
                        "in vec3 pos;\n"
                        "out vec2 texCoord;\n"
                        "uniform mat4 mat4MVP;\n"
                        "void main()\n"
                        "{\n"
                        "    float DEG2RAD = 3.1415926 / 180.0;\n"
                        "    float x = pos.z * sin(pos.y * DEG2RAD) * cos(pos.x * DEG2RAD);\n"
                        "    float y = pos.z * sin(pos.y * DEG2RAD) * sin(pos.x * DEG2RAD);\n"
                        "    float z = pos.z * cos(pos.y * DEG2RAD);\n"
                        "    gl_Position = mat4MVP * vec4(x, y, z, 1.0);\n"
                        "    texCoord = vec2(pos.x / 360.0, 1.0 - pos.y / 180.0);\n"//纹理的Y方向是从下向上的，而pos.y的正方向是从上向下，所以是1.0 - pos.y / 180.0
                        "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =
                        "#version 330\n"
                        "out vec4 color;\n"
                        "in vec2 texCoord;\n"
                        "uniform sampler2D Tex\n;"
                        "void main()\n"
                        "{\n"
                        "    color = texture(Tex, texCoord);\n"//注意，texCoord的值域在0-1之间
                        "}\n";
    fshader->compileSourceCode(fsrc);

    //3 链接GLSL程序
    m_pProgram = new QOpenGLShaderProgram;
    m_pProgram->addShader(vshader);
    m_pProgram->addShader(fshader);
    m_pProgram->link();
    m_pProgram->bind();

    m_uiVertLoc = m_pProgram->attributeLocation("pos");
    m_pProgram->enableAttributeArray(m_uiVertLoc);
    m_pProgram->setAttributeArray(m_uiVertLoc, m_pVertices, 3, 0);


    //mirror 根据 https://www.cnblogs.com/zhsuiy/p/5235291.html
    m_pTextures = new QOpenGLTexture(img.mirrored());
    m_pTextures->setMinificationFilter(QOpenGLTexture::Nearest);
    m_pTextures->setMagnificationFilter(QOpenGLTexture::Linear);
    m_pTextures->setWrapMode(QOpenGLTexture::Repeat);
    m_pProgram->setUniformValue("Tex", m_uiTexNum);

    //qDebug()<<m_iCount;

    glEnable(GL_DEPTH_TEST);
    glClearColor(0,0,0,1);

    m_timer.start(100);
}

void MainWindow::paintGL()
{
    //QMatrix4x4在声明时被默认为单位矩阵
    QMatrix4x4 m1, m2, m3, m;

    m1.ortho(-1.0f, +1.0f, -1.0f, 1.0f, -50.0f, 50.0f);//right//generate projection matrix
    m3.rotate(m_iSpinAngle, 0.0f, 0.0f, -1.0f);//right, generate model matrices
    //qDebug()<<m2;
    //m3.translate(0,-0.707,0.0);//right, generate model matrices


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_pTextures->bind(m_uiTexNum);

    //1 发现要把glViewPort函数放到paintGL里才起作用
    glViewport(m_iWidth/3, m_iHeight * 2 / 3, m_iWidth/3, m_iHeight/3);
    m2.lookAt(QVector3D(0,-2,0), QVector3D(0,0,0), QVector3D(0,0,10));//generate view matrix, right
    m = m1 * m2 * m3;
    m_pProgram->setUniformValue("mat4MVP", m);
    glDrawArrays(GL_TRIANGLES, 0, m_iCount / 3);

    //2 发现要把glViewPort函数放到paintGL里才起作用
    glViewport(m_iWidth/3, 0, m_iWidth/3, m_iHeight/3);
    m2.setToIdentity();
    m2.lookAt(QVector3D(0,2,0), QVector3D(0,0,0), QVector3D(0,0,-10));//generate view matrix, right
    m = m1 * m2 * m3;
    m_pProgram->setUniformValue("mat4MVP", m);
    glDrawArrays(GL_TRIANGLES, 0, m_iCount / 3);

    //3 发现要把glViewPort函数放到paintGL里才起作用
    glViewport(0, m_iHeight/3, m_iWidth/3, m_iHeight/3);
    m2.setToIdentity();
    m2.lookAt(QVector3D(-2,0,0), QVector3D(0,0,0), QVector3D(0,-10,0));//generate view matrix, right
    m = m1 * m2 * m3;
    m_pProgram->setUniformValue("mat4MVP", m);
    glDrawArrays(GL_TRIANGLES, 0, m_iCount / 3);

    //4 发现要把glViewPort函数放到paintGL里才起作用
    glViewport(m_iWidth * 2 /3, m_iHeight/3, m_iWidth/3, m_iHeight/3);
    m2.setToIdentity();
    m2.lookAt(QVector3D(2,0,0), QVector3D(0,0,0), QVector3D(0,-10,0));//generate view matrix, right
    m = m1 * m2 * m3;
    m_pProgram->setUniformValue("mat4MVP", m);
    glDrawArrays(GL_TRIANGLES, 0, m_iCount / 3);

    m_pTextures->release();
}

void MainWindow::resizeGL(int w, int h)
{
    //发现要把glViewPort函数放到paintGL里才起作用
    m_iWidth = w;
    m_iHeight = h;
}

void MainWindow::OnTimer(void)
{
    m_iSpinAngle += 5;
    if(360 == m_iSpinAngle)
    {
        m_iSpinAngle = 0;
    }

    update();
}
