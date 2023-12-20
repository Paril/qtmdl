#pragma once

#define RENDERDOC_SUPPORT

#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QMatrix4x4>
#include <QElapsedTimer>
#include "camera.h"
#ifdef RENDERDOC_SUPPORT
#include "renderdoc_app.h"
#endif
#include "modeldata.h"

struct VertexColor
{
    std::array<uint8_t, 4>  data;

    constexpr VertexColor() = default;
    inline VertexColor(const QColor &c) :
        data({ (uint8_t) c.red(), (uint8_t) c.green(), (uint8_t) c.blue(), (uint8_t) c.alpha() })
    {
    }
};

struct GPUVertexData
{
    QVector3D position;
    QVector2D texcoord;
    VertexColor color;
};

struct GPUPointData
{
    QVector3D   position;
    VertexColor color;
};

enum class QuadrantFocus
{
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Horizontal,
    Vertical,
    Center,

    Quadrants = 4,
    None = -1
};

enum class Orientation2D
{
    XY,
    ZY,
    XZ
};

struct QuadRect { int x, y, w, h; };

struct ProgramShared
{
    GLuint program;
    GLint projectionUniformLocation, modelviewUniformLocation, shadedUniformLocation;
};

class QMDLRenderer : public QOpenGLWidget
{
#ifdef RENDERDOC_SUPPORT
    RENDERDOC_API_1_1_2 *rdoc_api = NULL;
#endif

public:
    QMDLRenderer(QWidget *parent = nullptr);

    void captureRenderDoc(bool);
    void resetAnimation();
    void setAnimated(bool animating);
    void focusLost();
    void modelLoaded();
    void selectedSkinChanged();

protected:
    void initializeGL() override;

    void resizeGL(int w, int h) override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

    void paintGL() override;

private:
    void generateGrid(float grid_size, size_t count);
    void generateAxis();

    QuadRect getQuadrantRect(QuadrantFocus quadrant);
    QuadrantFocus getQuadrantFocus(QPoint xy);
    void getQuadrantMatrices(QuadrantFocus quadrant, Matrix4 &projection, Matrix4 &modelview);
    void clearQuadrant(QuadRect rect, QVector4D color);
    void drawModels(QuadrantFocus quadrant, bool is_2d);
    void draw2D(Orientation2D orientation, QuadrantFocus quadrant);
    void draw3D(QuadrantFocus quadrant);
    QMatrix4x4 getDragMatrix();
    QVector3D mouseToWorld(QPoint pos);
    
    std::unique_ptr<QOpenGLDebugLogger> _logger;
    QElapsedTimer _animationTimer;
    QuadrantFocus _focusedQuadrant = QuadrantFocus::None;
    bool _dragging = false;
    QVector3D _dragWorldPos;
    QPoint _dragPos, _downPos, _dragDelta;
    Qt::MouseButton _dragButton;
    ProgramShared _modelProgram, _simpleProgram;

    GLuint _buffer, _pointBuffer, _smoothNormalBuffer, _flatNormalBuffer, _axisBuffer, _gridBuffer;
    GLuint _whiteTexture, _blackTexture, _modelTexture;
    size_t _gridSize;
    std::vector<GPUVertexData> _bufferData;
    std::vector<GPUPointData> _pointData;
    std::vector<QVector3D> _smoothNormalData, _flatNormalData;
    GLuint _vao, _pointVao, _axisVao, _gridVao;
    float _2dZoom = 1.0f;
    QVector3D _2dOffset = {};
    Camera _camera;
#ifdef RENDERDOC_SUPPORT
    bool _doRenderDoc;
#endif
    const ModelData &activeModel();

	GLuint createShader(GLenum type, const char *source);
	GLuint createProgram(GLuint vertexShader, GLuint fragmentShader);
    void rebuildBuffer();
};
