#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QTimer>

class MainWindow : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    GLuint                          m_uiVertLoc;
    GLuint                          m_uiTexNum;
    QOpenGLTexture          *       m_pTextures;
    QOpenGLShaderProgram    *       m_pProgram;
    GLfloat                 *       m_pVertices;
    int                             m_iCount;
    int                             m_iSpinAngle;
    QTimer                          m_timer;

    //发现要把glViewPort函数放到paintGL里才起作用，所以使用下面变量：
    int                             m_iWidth;
    int                             m_iHeight;

protected:
    void        initializeGL();
    void        paintGL();
    void        resizeGL(int w, int h);
public slots:
    void        OnTimer(void);
};

#endif // MAINWINDOW_H
