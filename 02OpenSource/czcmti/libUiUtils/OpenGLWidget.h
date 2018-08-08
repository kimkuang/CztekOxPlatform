#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QMutex>
#include <QElapsedTimer>
#include "czcmtidefs.h"
#include "UiDefs.h"
#include "thread/syncevent.h"
#include <QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    #include <QOpenGLWidget>
    #include <QOpenGLFunctions>
#else
    #include <QGLWidget>
#endif
#include <QSemaphore>
#include <QResizeEvent>
#include <QRgb>

#if (QT_VERSION > QT_VERSION_CHECK(5, 2, 1))
#define USE_QOPENGLTEXTURE // QOpenGLTexutre in QT5.2.1 only support RGBA texture
#endif
#define USE_VBO

class LIBRARY_API OpenGLWidget :
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    public QOpenGLWidget, protected QOpenGLFunctions
#else
    public QGLWidget
#endif
{
    Q_OBJECT
public:
    explicit OpenGLWidget(QWidget *parent = 0);
    ~OpenGLWidget();

    void SetSemaphoreEvent(SyncEvent *semaphoreEvent);
    void RenderFrame(const uchar* rgbBuffer, const T_FrameParam &frameParam);
    void RenderText(const QString &text, QRgb foreground, QRgb background);
    void Clear();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void resizeEvent(QResizeEvent *e);

private:
    struct T_ShaderVariables {
        struct tAttributes {
            int LocVertex;
            int LocTexCoord0;
        } Attributes;
        struct tUniforms {
            int LocModelViewProjectionMatrix;
            int LocTexture0;
        } Uniforms;
#ifdef USE_QOPENGLTEXTURE
        QOpenGLTexture *Texture;
#else
        GLuint TextureId;
#endif
        QOpenGLShaderProgram *Program;
#ifdef USE_VBO
        QOpenGLBuffer VBO;
#endif
    } m_glShader;
    T_FrameParam m_frameParam;
    bool m_frameParamChanged;
    QRectF m_dispViewport;
    const uchar *m_pTextureFrame;
    QString m_renderText;
    SyncEvent *m_semaphoreEvent;
    QMatrix4x4 m_projection;
    QColor m_foreground;
    QColor m_background;
    bool m_backgroundChanged;
    QElapsedTimer m_elapsedTimer;
    int m_frameCounter;
    enum E_RenderMode {
        RenderMode_None,
        RenderMode_Texture,
        RenderMode_Text,
    } m_renderMode;

    void setFrameParam(const T_FrameParam &frameParam);
    void calcViewport(const QSizeF &windowSize);
    bool loadShaders();
    void drawTexture();
    void setShaderStaticUniforms(const T_FrameParam &frameParam);
    void setShaderDynamicUniforms(const T_FrameParam &frameParam);
    bool initTextureParam(const T_FrameParam &frameParam);
    bool uploadTexture(const uchar *buffer, const T_FrameParam &frameParam);
    void paintTexture();
    void fpsStatistics(QPainter *painter, const QPointF &pos);
    void resetFpsStatistics();
};

#endif // OPENGLWIDGET_H
