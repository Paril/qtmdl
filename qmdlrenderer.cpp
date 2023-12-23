#include "glad.h"
#include "mainwindow.h"
#include "qmdlrenderer.h"
#define TCPP_IMPLEMENTATION
#include "tcppLibrary.hpp"
#include <optional>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include "settings.h"
#include "qtutils.h"
#include "quvpainter.h"

constexpr GLuint ATTRIB_POSITION = 0;
constexpr GLuint ATTRIB_TEXCOORD = 1;
constexpr GLuint ATTRIB_COLOR = 2;
constexpr GLuint ATTRIB_NORMAL = 3;
constexpr GLuint ATTRIB_COLOR_SECONDARY = 4;

struct GPUAxisData
{
    QVector3D   position;
    VertexColor color;
};

QMDLRenderer::QMDLRenderer(QWidget *parent) :
    QOpenGLWidget(parent)
{
    _camera.setBehavior(Camera::CameraBehavior::CAMERA_BEHAVIOR_ORBIT);
    _camera.lookAt({ 25.f, 0, 0 }, {}, { 0, 1, 0 });
	_camera.zoom(Camera::DEFAULT_ORBIT_OFFSET_DISTANCE, _camera.getOrbitMinZoom(), _camera.getOrbitMaxZoom());
}

class QFileStream : public tcpp::IInputStream
{
    QFile file;

public:
    inline QFileStream(const char *filename) :
        file(filename)
    {
        file.open(QFile::ReadOnly | QFile::Text);
    }

	virtual std::string ReadLine() TCPP_NOEXCEPT override
    {
        return file.readLine().toStdString();
    }

    virtual bool HasNextLine() const TCPP_NOEXCEPT override
    {
        return !file.atEnd();
    }
};

static std::string preprocessShader(const char *filename)
{
    tcpp::Lexer lexer(std::make_unique<QFileStream>(filename));
    tcpp::Preprocessor::TPreprocessorConfigInfo info;
    tcpp::Preprocessor p(lexer, info);
    return p.Process();
}

void QMDLRenderer::generateGrid(float grid_size, size_t count)
{
    size_t point_count = ((count * 4) + 4) * 2;
    std::vector<QVector3D> points;
    points.reserve(point_count);

    // corners
    float extreme = grid_size * count;
        
    points.emplace_back(-extreme, -extreme, 0);
    points.emplace_back(extreme, -extreme, 0);
        
    points.emplace_back(-extreme, extreme, 0);
    points.emplace_back(extreme, extreme, 0);
        
    points.emplace_back(-extreme, -extreme, 0);
    points.emplace_back(-extreme, extreme, 0);
        
    points.emplace_back(extreme, extreme, 0);
    points.emplace_back(extreme, -extreme, 0);

    // lines
    for (size_t i = 0; i < count; i++)
    {
        float v = grid_size * i;

        // x
        points.emplace_back(v, -extreme, 0);
        points.emplace_back(v, extreme, 0);
        points.emplace_back(-v, -extreme, 0);
        points.emplace_back(-v, extreme, 0);

        // y
        points.emplace_back(-extreme, v,  0);
        points.emplace_back(extreme, v, 0);
        points.emplace_back(-extreme, -v, 0);
        points.emplace_back(extreme, -v, 0);
    }

    glBufferData(GL_ARRAY_BUFFER, point_count * sizeof(QVector3D), points.data(), GL_STATIC_DRAW);
    _gridSize = point_count;
}

void QMDLRenderer::generateAxis()
{
    const QColor &xColor = Settings().getEditorColor(EditorColorId::OriginX);
    const QColor &yColor = Settings().getEditorColor(EditorColorId::OriginY);
    const QColor &zColor = Settings().getEditorColor(EditorColorId::OriginZ);

    GPUAxisData axisData[] = {
        { { 0.0f, 0.0f, 0.0f }, xColor },
        { { 32.0f, 0.0f, 0.0f }, xColor },

        { { 0.0f, 0.0f, 0.0f }, yColor },
        { { 0.0f, 32.0f, 0.0f }, yColor },

        { { 0.0f, 0.0f, 0.0f }, zColor },
        { { 0.0f, 0.0f, 32.0f }, zColor },
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(axisData), axisData, GL_STATIC_DRAW);
}

template<size_t w, size_t h, size_t N = w * h * 4>
static inline void createBuiltInTexture(GLuint &out, const std::initializer_list<byte> &pixels)
{
    if (pixels.size() != N)
        throw std::runtime_error("bad pixels");

    glGenTextures(1, &out);
    glBindTexture(GL_TEXTURE_2D, out);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.begin());
}

static std::optional<std::string> loadShader(const char *filename)
{
    QDir shaderPath("res/shaders");

    if (!shaderPath.exists())
    {
        shaderPath.setPath("../../../res/shaders");

        if (!shaderPath.exists())
            return std::nullopt;
    }

    QFileInfo file(shaderPath, filename);

    if (!file.exists())
        return std::nullopt;

    QFile f(file.filePath());

    if (!f.open(QFile::ReadOnly | QFile::Text))
        return std::nullopt;

    return f.readAll().toStdString();
}

void QMDLRenderer::initializeGL()
{
    _logger = std::make_unique<QOpenGLDebugLogger>(this);
    if (_logger->initialize()) {
        connect(_logger.get(), &QOpenGLDebugLogger::messageLogged, this, [](const QOpenGLDebugMessage &debugMessage) {
            qDebug() << debugMessage.message();
        });
        _logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
    }

    gladLoadGL();

    glFrontFace(GL_CW);

    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
        
    createBuiltInTexture<1, 1>(_whiteTexture, { 0xFF, 0xFF, 0xFF, 0xFF });
    createBuiltInTexture<1, 1>(_blackTexture, { 0x00, 0x00, 0x00, 0xFF });

    _modelProgram.program = createProgram(
        createShader(GL_VERTEX_SHADER, loadShader("model.vert.glsl")->c_str()),
        createShader(GL_FRAGMENT_SHADER, loadShader("model.frag.glsl")->c_str()));
    glUseProgram(_modelProgram.program);

    _modelProgram.projectionUniformLocation = glGetUniformLocation(_modelProgram.program, "u_projection");
    _modelProgram.modelviewUniformLocation = glGetUniformLocation(_modelProgram.program, "u_modelview");

    glUniform1i(glGetUniformLocation(_modelProgram.program, "u_texture"), 0);
    glUniform1i(_modelProgram.shadedUniformLocation = glGetUniformLocation(_modelProgram.program, "u_shaded"), 1);
    glUniform1i(_modelProgram.secondarycolorsUniformLocation = glGetUniformLocation(_modelProgram.program, "u_secondarycolors"), 0);

    _simpleProgram.program = createProgram(
        createShader(GL_VERTEX_SHADER, loadShader("simple.vert.glsl")->c_str()),
        createShader(GL_FRAGMENT_SHADER, loadShader("simple.frag.glsl")->c_str()));
    glUseProgram(_simpleProgram.program);

    _simpleProgram.projectionUniformLocation = glGetUniformLocation(_simpleProgram.program, "u_projection");
    _simpleProgram.modelviewUniformLocation = glGetUniformLocation(_simpleProgram.program, "u_modelview");

    glGenBuffers(1, &_gridBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _gridBuffer);
    generateGrid(8.0f, 8);

    glGenVertexArrays(1, &_gridVao);
    glBindVertexArray(_gridVao);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glDisableVertexAttribArray(ATTRIB_TEXCOORD);
    glDisableVertexAttribArray(ATTRIB_COLOR);
    glDisableVertexAttribArray(ATTRIB_COLOR_SECONDARY);
    glDisableVertexAttribArray(ATTRIB_NORMAL);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, false, sizeof(QVector3D), nullptr);
        
    glGenBuffers(1, &_axisBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _axisBuffer);
    generateAxis();

    glGenVertexArrays(1, &_axisVao);
    glBindVertexArray(_axisVao);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glDisableVertexAttribArray(ATTRIB_TEXCOORD);
    glEnableVertexAttribArray(ATTRIB_COLOR);
    glDisableVertexAttribArray(ATTRIB_COLOR_SECONDARY);
    glDisableVertexAttribArray(ATTRIB_NORMAL);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, false, sizeof(GPUAxisData), reinterpret_cast<const GLvoid *>(offsetof(GPUAxisData, position)));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, true, sizeof(GPUAxisData), reinterpret_cast<const GLvoid *>(offsetof(GPUAxisData, color)));

    glGenBuffers(1, &_buffer);
    
    glGenBuffers(1, &_smoothNormalBuffer);
    glGenBuffers(1, &_flatNormalBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, _buffer);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glEnableVertexAttribArray(ATTRIB_TEXCOORD);
    glEnableVertexAttribArray(ATTRIB_COLOR);
    glEnableVertexAttribArray(ATTRIB_COLOR_SECONDARY);
    glEnableVertexAttribArray(ATTRIB_NORMAL);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, false, sizeof(GPUVertexData), reinterpret_cast<const GLvoid *>(offsetof(GPUVertexData, position)));
    glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, false, sizeof(GPUVertexData), reinterpret_cast<const GLvoid *>(offsetof(GPUVertexData, texcoord)));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, true, sizeof(GPUVertexData), reinterpret_cast<const GLvoid *>(offsetof(GPUVertexData, color)));
    glVertexAttribPointer(ATTRIB_COLOR_SECONDARY, 4, GL_UNSIGNED_BYTE, true, sizeof(GPUVertexData), reinterpret_cast<const GLvoid *>(offsetof(GPUVertexData, color_secondary)));
    glBindBuffer(GL_ARRAY_BUFFER, _smoothNormalBuffer);
    glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, false, 0, nullptr);

    glGenBuffers(1, &_pointBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _pointBuffer);

    glGenVertexArrays(1, &_pointVao);
    glBindVertexArray(_pointVao);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glDisableVertexAttribArray(ATTRIB_TEXCOORD);
    glEnableVertexAttribArray(ATTRIB_COLOR);
    glDisableVertexAttribArray(ATTRIB_NORMAL);
    glDisableVertexAttribArray(ATTRIB_COLOR_SECONDARY);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, false, sizeof(GPUPointData), reinterpret_cast<const GLvoid *>(offsetof(GPUPointData, position)));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, true, sizeof(GPUPointData), reinterpret_cast<const GLvoid *>(offsetof(GPUPointData, color)));
}

void QMDLRenderer::resizeGL(int w, int h)
{
}

QuadRect QMDLRenderer::getQuadrantRect(QuadrantFocus quadrant)
{
    int w = width() * devicePixelRatio();
    int h = height() * devicePixelRatio();
        
    // calculate the top-left quadrant size, which is used as the basis
    int qw = (w * Settings().getHorizontalSplit()) - 1;
    int qh = (h * Settings().getVerticalSplit()) - 1;
        
    int line_width = 2 + (w & 1);
    int line_height = 2 + (h & 1);

    if (quadrant == QuadrantFocus::TopLeft)
        return { 0, 0, qw, qh };
    else if (quadrant == QuadrantFocus::Vertical)
        return { qw, 0, line_width, h };
    else if (quadrant == QuadrantFocus::Horizontal)
        return { 0, qh, w, line_height };
    else if (quadrant == QuadrantFocus::Center)
        return { qw, qh, line_width, line_height };

    // the rest of the quadrants need the other side
    // calculated.
    int oqw = w - (qw + line_width);
    int oqh = h - (qh + line_height);

    if (quadrant == QuadrantFocus::TopRight)
        return { qw + line_width, 0, oqw, qh };
    else if (quadrant == QuadrantFocus::BottomRight)
        return { qw + line_width, qh + line_height, oqw, oqh };
    else if (quadrant == QuadrantFocus::BottomLeft)
        return { 0, qh + line_height, qw, oqh };

    throw std::runtime_error("bad quadrant");
}

QuadrantFocus QMDLRenderer::getQuadrantFocus(QPoint xy)
{
    constexpr QuadrantFocus focusOrder[] = {
        QuadrantFocus::Center,
        QuadrantFocus::Horizontal,
        QuadrantFocus::Vertical,
        QuadrantFocus::TopLeft,
        QuadrantFocus::TopRight,
        QuadrantFocus::BottomRight,
        QuadrantFocus::BottomLeft
    };
    QuadRect focusRects[] = {
        getQuadrantRect(QuadrantFocus::Center),
        getQuadrantRect(QuadrantFocus::Horizontal),
        getQuadrantRect(QuadrantFocus::Vertical),
        getQuadrantRect(QuadrantFocus::TopLeft),
        getQuadrantRect(QuadrantFocus::TopRight),
        getQuadrantRect(QuadrantFocus::BottomRight),
        getQuadrantRect(QuadrantFocus::BottomLeft)
    };

    size_t i = 0;

    for (auto &quadrant : focusRects)
    {
        if (xy.x() >= quadrant.x && xy.y() >= quadrant.y &&
            xy.x() < quadrant.x + quadrant.w && xy.y() < quadrant.y + quadrant.h)
            return focusOrder[i];

        i++;
    }

    return QuadrantFocus::None;
}

void QMDLRenderer::mousePressEvent(QMouseEvent *e)
{
    _dragging = true;
    _dragButton = e->button();
    _dragPos = _downPos = mapFromGlobal(QCursor::pos()) * devicePixelRatio();
    _dragWorldPos = mouseToWorld(_dragPos);
    _dragDelta = {};
    mouseMoveEvent(e);
}

void QMDLRenderer::mouseReleaseEvent(QMouseEvent *e)
{
    if (!_dragging)
        return;
    
    if (_dragging && (_focusedQuadrant == QuadrantFocus::TopLeft || _focusedQuadrant == QuadrantFocus::TopRight ||
        _focusedQuadrant == QuadrantFocus::BottomLeft || _focusedQuadrant == QuadrantFocus::BottomRight))
    {
        Matrix4 drag = getDragMatrix();

        if (!drag.isIdentity())
        {
#if 0
            for (auto &frame : activeModel().frames)
                for (auto &vert : frame.vertices)
                {
                    vert.position = drag.map(vert.position);
                    vert.normal = drag.mapVector(vert.normal);
                }
#endif
        }
    }

    _dragging = false;

    mouseMoveEvent(e);
    MainWindow::instance().updateRenders();
}

QMatrix4x4 QMDLRenderer::getDragMatrix()
{
    QMatrix4x4 matrix;
    
    if (!_dragging || _focusedQuadrant == QuadrantFocus::None)
        return matrix;
        
    float xDelta = _dragDelta.x() / _2dZoom;
    float yDelta = _dragDelta.y() / _2dZoom;

    if (MainWindow::instance().selectedTool() == EditorTool::Move)
    {
        if (_focusedQuadrant == QuadrantFocus::TopLeft)
            matrix.translate(-yDelta, -xDelta, 0);
        else if (_focusedQuadrant == QuadrantFocus::BottomLeft)
            matrix.translate(0, -xDelta, yDelta);
        else if (_focusedQuadrant == QuadrantFocus::BottomRight)
            matrix.translate(xDelta, 0, yDelta);
    }
    else if (MainWindow::instance().selectedTool() == EditorTool::Scale)
    {
        float s = 1.0f + (_dragDelta.y() * 0.01f) / _2dZoom;
        matrix.translate(_dragWorldPos);
        matrix.scale(s, s, s);
        matrix.translate(-_dragWorldPos);
    }
    else if (MainWindow::instance().selectedTool() == EditorTool::Rotate)
    {
        QuadRect rect = getQuadrantRect(_focusedQuadrant);
        float r = 360.f * (_dragDelta.y() / (float) rect.h);
        matrix.translate(_dragWorldPos);
        if (_focusedQuadrant == QuadrantFocus::TopLeft)
            matrix.rotate(r, QVector3D(0, 0, -1));
        else if (_focusedQuadrant == QuadrantFocus::BottomLeft)
            matrix.rotate(r, QVector3D(1, 0, 0));
        else if (_focusedQuadrant == QuadrantFocus::BottomRight)
            matrix.rotate(r, QVector3D(0, 1, 0));
        matrix.translate(-_dragWorldPos);
    }

    return matrix;
}

void QMDLRenderer::focusLost()
{
    QMouseEvent ev {
        QEvent::Type::MouseButtonRelease,
        mapFromGlobal(QCursor::pos()) * devicePixelRatio(),
        QCursor::pos(),
        Qt::MouseButton::NoButton,
        {},
        {}
    };
    QMDLRenderer::mouseReleaseEvent(&ev);
}

void QMDLRenderer::mouseMoveEvent(QMouseEvent *event)
{
    auto pos = mapFromGlobal(QCursor::pos()) * devicePixelRatio();
    auto world = mouseToWorld(pos);

    MainWindow::instance().setCurrentWorldPosition(world);

    if (_dragging)
    {
        auto delta = _dragPos - pos;

        _dragDelta += delta;
        
        float xDelta = delta.x() / _2dZoom;
        float yDelta = delta.y() / _2dZoom;

        if (_focusedQuadrant == QuadrantFocus::Horizontal ||
            _focusedQuadrant == QuadrantFocus::Vertical ||
            _focusedQuadrant == QuadrantFocus::Center)
        {
            bool adjust_vert = _focusedQuadrant == QuadrantFocus::Horizontal || _focusedQuadrant == QuadrantFocus::Center;
            bool adjust_horz = _focusedQuadrant == QuadrantFocus::Vertical || _focusedQuadrant == QuadrantFocus::Center;
                
            if (adjust_horz)
                Settings().setHorizontalSplit((float) pos.x() / (width() * devicePixelRatio()));
            if (adjust_vert)
                Settings().setVerticalSplit((float) pos.y() / (height() * devicePixelRatio()));
        }
        else if (MainWindow::instance().selectedTool() == EditorTool::Pan)
        {
            if (_focusedQuadrant == QuadrantFocus::TopRight)
            {
                if (_dragButton == Qt::MouseButton::RightButton)
                    _camera.zoom(-delta.y(), _camera.getOrbitMinZoom(), _camera.getOrbitMaxZoom());
                else if (_dragButton == Qt::MouseButton::LeftButton)
                    _camera.rotate(delta.y(), delta.x(), 0);
            }
            else if (_focusedQuadrant == QuadrantFocus::TopLeft ||
                        _focusedQuadrant == QuadrantFocus::BottomLeft ||
                        _focusedQuadrant == QuadrantFocus::BottomRight)
            {
                if (_dragButton == Qt::MouseButton::RightButton)
                {
                    _2dZoom += (delta.y() * 0.01f) * _2dZoom;
                }
                else if (_dragButton == Qt::MouseButton::LeftButton)
                {
                    if (_focusedQuadrant == QuadrantFocus::TopLeft)
                        _2dOffset += QVector3D(-xDelta, yDelta, 0);
                    else if (_focusedQuadrant == QuadrantFocus::BottomLeft)
                        _2dOffset += QVector3D(-xDelta, 0, yDelta);
                    else if (_focusedQuadrant == QuadrantFocus::BottomRight)
                        _2dOffset += QVector3D(0, -xDelta, yDelta);
                }
            }
        }
        
        update();
        
        if (MainWindow::instance().selectedTool() == EditorTool::Pan)
            _dragPos = { QtUtils::wrap(pos.x(), 0, static_cast<int>(width() * devicePixelRatio()) - 1), QtUtils::wrap(pos.y(), 0, static_cast<int>(height() * devicePixelRatio()) - 1) };
        else if (_focusedQuadrant != QuadrantFocus::None)
        {
            QuadRect rect = getQuadrantRect(_focusedQuadrant);
            _dragPos = { QtUtils::wrap(pos.x(), rect.x, rect.x + rect.w - 1), QtUtils::wrap(pos.y(), rect.y, rect.y + rect.h - 1) };
        }

        if (_dragPos != pos)
            QCursor::setPos(mapToGlobal(_dragPos / devicePixelRatio()));
        
        if (_focusedQuadrant == QuadrantFocus::Center)
            setCursor(Qt::SizeAllCursor);
        else if (_focusedQuadrant == QuadrantFocus::Horizontal)
            setCursor(Qt::SizeVerCursor);
        else if (_focusedQuadrant == QuadrantFocus::Vertical)
            setCursor(Qt::SizeHorCursor);
        else if (MainWindow::instance().selectedTool() == EditorTool::Pan)
            setCursor(Qt::ClosedHandCursor);
        else
            setCursor(Qt::ArrowCursor);

        return;
    }

    _focusedQuadrant = getQuadrantFocus(pos);
        
    int w = this->width() * devicePixelRatio();
    int h = this->height() * devicePixelRatio();
        
    int quadrant_w = (w / 2) - 1;
    int quadrant_h = (h / 2) - 1;
        
    int line_width = 2 + (w & 1);
    int line_height = 2 + (h & 1);
        
    bool within_x = pos.x() >= quadrant_w && pos.x() <= quadrant_w + line_width;
    bool within_y = pos.y() >= quadrant_h && pos.y() <= quadrant_h + line_height;

    // the separators always take priority
    if (_focusedQuadrant == QuadrantFocus::Center)
        setCursor(Qt::SizeAllCursor);
    else if (_focusedQuadrant == QuadrantFocus::Horizontal)
        setCursor(Qt::SizeVerCursor);
    else if (_focusedQuadrant == QuadrantFocus::Vertical)
        setCursor(Qt::SizeHorCursor);
    else if (MainWindow::instance().selectedTool() == EditorTool::Pan)
        setCursor(Qt::OpenHandCursor);
    else
        setCursor(Qt::ArrowCursor);
}

void QMDLRenderer::leaveEvent(QEvent *event)
{
    _focusedQuadrant = QuadrantFocus::None;
}

void QMDLRenderer::clearQuadrant(QuadRect rect, QVector4D color)
{
    // flip Y to match GL orientation
    int ry = ((height() * devicePixelRatio()) - rect.y) - rect.h;

    glViewport(rect.x, ry, rect.w, rect.h);
    glScissor(rect.x, ry, rect.w, rect.h);
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void QMDLRenderer::drawModels(QuadrantFocus quadrant, bool is_2d)
{
    RenderParameters params = MainWindow::instance().getRenderParameters(is_2d);
    QMatrix4x4 projection, modelview;
    getQuadrantMatrices(quadrant, projection, modelview);

    glUseProgram(_simpleProgram.program);
    glUniformMatrix4fv(_simpleProgram.projectionUniformLocation, 1, false, projection.data());
    glUniformMatrix4fv(_simpleProgram.modelviewUniformLocation, 1, false, modelview.data());

    glDisable(GL_CULL_FACE);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindTexture(GL_TEXTURE_2D, _whiteTexture);

    if (MainWindow::instance().showGrid())
    {
        glBindVertexArray(_gridVao);
        glBindBuffer(GL_ARRAY_BUFFER, _gridBuffer);
        const QColor &gridColor = Settings().getEditorColor(EditorColorId::Grid);
        glVertexAttrib2f(ATTRIB_TEXCOORD, 1.0f, 1.0f);
        glVertexAttrib4f(ATTRIB_COLOR, gridColor.redF(), gridColor.greenF(), gridColor.blueF(), gridColor.alphaF());
        glDrawArrays(GL_LINES, 0, (GLsizei) _gridSize);
    }
    
    if (MainWindow::instance().showOrigin())
    {
        glDisable(GL_DEPTH_TEST);

        glBindVertexArray(_axisVao);
        glBindBuffer(GL_ARRAY_BUFFER, _axisBuffer);
        glDrawArrays(GL_LINES, 0, 6);

        glEnable(GL_DEPTH_TEST);
    }

    glUseProgram(_modelProgram.program);
    glUniform1i(_modelProgram.secondarycolorsUniformLocation, is_2d);
    glUniformMatrix4fv(_modelProgram.projectionUniformLocation, 1, false, projection.data());

    if (_dragging && (_focusedQuadrant == QuadrantFocus::TopLeft || _focusedQuadrant == QuadrantFocus::TopRight ||
        _focusedQuadrant == QuadrantFocus::BottomLeft || _focusedQuadrant == QuadrantFocus::BottomRight))
        modelview *= getDragMatrix();

    glUniformMatrix4fv(_modelProgram.modelviewUniformLocation, 1, false, modelview.data());

    // model
    glBindVertexArray(_vao);
    glVertexAttrib4f(ATTRIB_COLOR, 1.0f, 1.0f, 1.0f, 1.0f);

    if (params.smoothNormals)
        glBindBuffer(GL_ARRAY_BUFFER, _smoothNormalBuffer);
    else
        glBindBuffer(GL_ARRAY_BUFFER, _flatNormalBuffer);

    glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, false, 0, nullptr);

    if (!params.drawBackfaces)
        glEnable(GL_CULL_FACE);

    if (params.mode == RenderMode::Wireframe)
    {
        glBindTexture(GL_TEXTURE_2D, _whiteTexture);
        const QColor &wireframeColor = Settings().getEditorColor(EditorColorId::FaceLineUnselected3D);
        glVertexAttrib4f(ATTRIB_COLOR, wireframeColor.redF(), wireframeColor.greenF(), wireframeColor.blueF(), wireframeColor.alphaF());
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glUniform1i(_modelProgram.shadedUniformLocation, false);
    }
    else
    {
        if (params.mode == RenderMode::Flat)
            glBindTexture(GL_TEXTURE_2D, _whiteTexture);

        glUniform1i(_modelProgram.shadedUniformLocation, params.shaded);
    }

    GLint offset = 0;

    for (auto &mesh : activeModel().meshes)
    {
        GLsizei count = mesh.triangles.size() * 3;

        if (params.mode == RenderMode::Textured)
            glBindTexture(GL_TEXTURE_2D, _modelTextures[mesh.assigned_skin.value_or(activeModel().selectedSkin.value())]);

        glDrawArrays(GL_TRIANGLES, offset, count);
        offset += count;
    }

    glVertexAttrib4f(ATTRIB_COLOR, 1.0f, 1.0f, 1.0f, 1.0f);
    
    if (MainWindow::instance().vertexTicks())
    {
        // points
        glPointSize(3.0f);
    
        // this is super ugly but it works ok enough for now.
        glDepthFunc(GL_LEQUAL);
	
        Matrix4 depthProj = projection;
        if (!depthProj(3, 3))
        {
            constexpr float n = 0.1f;
            constexpr float f = 1024;
            constexpr float delta = 0.25f;
            constexpr float pz = 8.5f;
	        constexpr float epsilon = -2.0f * f * n * delta / ((f + n) * pz * (pz + delta));

    	    depthProj(3, 3) = -epsilon;
        }
        else
            depthProj.scale(1.0f, 1.0f, 0.98f);

        glUseProgram(_simpleProgram.program);
        glUniformMatrix4fv(_simpleProgram.projectionUniformLocation, 1, false, depthProj.data());

        glBindTexture(GL_TEXTURE_2D, _whiteTexture);
        glBindVertexArray(_pointVao);
        glBindBuffer(GL_ARRAY_BUFFER, _pointBuffer);
        glDrawArrays(GL_POINTS, 0, (GLsizei) _pointData.size());

        glDepthFunc(GL_LESS);
    }
}

void QMDLRenderer::getQuadrantMatrices(QuadrantFocus quadrant, Matrix4 &projection, Matrix4 &modelview)
{
    QuadRect rect = getQuadrantRect(quadrant);

    if (quadrant == QuadrantFocus::TopRight)
    {
        _camera.perspective(45, (float) rect.w / rect.h, 0.1f, 1024.f);
        projection = _camera.getProjectionMatrix();

        modelview = _camera.getViewMatrix();
        modelview.rotate(-90, { 1, 0, 0 });
        modelview.translate(-_2dOffset.y(), _2dOffset.x(), _2dOffset.z());
    }
    else
    {
        float hw = (rect.w / 2.0f);
        float hh = (rect.h / 2.0f);
        projection.ortho(-hw, hw, -hh, hh, -8192, 8192);

        modelview.setToIdentity();
        modelview.scale(_2dZoom);

        if (quadrant == QuadrantFocus::TopLeft)
        {
            modelview.translate(_2dOffset.x(), _2dOffset.y());
            modelview.rotate(-90, 0.0f, 0.0f, 1.0f);
        }
        else if (quadrant == QuadrantFocus::BottomLeft)
        {
            modelview.translate(_2dOffset.x(), _2dOffset.z());
            modelview.rotate(-90, 0.0f, 0.0f, 1.0f);
            modelview.rotate(-90, 0.0f, 1.0f, 0.0f);
        }
        else if (quadrant == QuadrantFocus::BottomRight)
        {
            modelview.translate(_2dOffset.y(), _2dOffset.z());
            modelview.rotate(-90, 0.0f, 0.0f, 1.0f);
            modelview.rotate(-90, 0.0f, 1.0f, 0.0f);
            modelview.rotate(-90, 0.0f, 0.0f, 1.0f);
        }
    }
}

void QMDLRenderer::draw2D(Orientation2D orientation, QuadrantFocus quadrant)
{
    clearQuadrant(getQuadrantRect(quadrant), { 0.4f, 0.4f, 0.4f, 1.0f });
    drawModels(quadrant, true);
}

void QMDLRenderer::draw3D(QuadrantFocus quadrant)
{
    clearQuadrant(getQuadrantRect(quadrant), { 0.4f, 0.4f, 0.4f, 1.0f });
    drawModels(quadrant, false);
}

QVector3D QMDLRenderer::mouseToWorld(QPoint pos)
{
    if (_focusedQuadrant == QuadrantFocus::None ||
        _focusedQuadrant == QuadrantFocus::Vertical ||
        _focusedQuadrant == QuadrantFocus::Horizontal ||
        _focusedQuadrant == QuadrantFocus::Center)
        return {};

    auto r = getQuadrantRect(_focusedQuadrant);
    Matrix4 projection, modelview;
    getQuadrantMatrices(_focusedQuadrant, projection, modelview);

    // put pos into local quadrant space
    pos.setX(pos.x() - r.x);
    pos.setY(pos.y() - r.y);
    // flip Y to match GL orientation
    pos.setY(r.h - pos.y());

    QVector3D position(pos);

    if (_focusedQuadrant == QuadrantFocus::TopRight)
        position.setZ(1.0f);
    else
        position.setZ(0.5f);

    return position.unproject(modelview, projection, { 0, 0, r.w, r.h });
}
    
void QMDLRenderer::paintGL()
{
    this->rebuildBuffer();

#ifdef RENDERDOC_SUPPORT
    if (_doRenderDoc && rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);
#endif

    clearQuadrant({ 0, 0, static_cast<int>(width() * devicePixelRatio()), static_cast<int>(height() * devicePixelRatio()) }, { 0, 0, 0, 255 });
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    draw2D(Orientation2D::XY, QuadrantFocus::TopLeft);
    draw2D(Orientation2D::ZY, QuadrantFocus::BottomLeft);
    draw2D(Orientation2D::XZ, QuadrantFocus::BottomRight);

    draw3D(QuadrantFocus::TopRight);

    if (_dragging && MainWindow::instance().selectedTool() == EditorTool::Select)
    {
        QuadRect selectRect {
            std::min(_dragPos.x(), _downPos.x()),
            std::min(_dragPos.y(), _downPos.y()),
            std::abs(_dragPos.x() - _downPos.x()),
            std::abs(_dragPos.y() - _downPos.y())
        };

        const QColor &selectBox = Settings().getEditorColor(EditorColorId::SelectBox);

        clearQuadrant({
            selectRect.x, selectRect.y,
            selectRect.w, 1
        }, { selectBox.redF(), selectBox.greenF(), selectBox.blueF(), selectBox.alphaF() });

        clearQuadrant({
            selectRect.x, selectRect.y + selectRect.h,
            selectRect.w, 1
        }, { selectBox.redF(), selectBox.greenF(), selectBox.blueF(), selectBox.alphaF() });

        clearQuadrant({
            selectRect.x, selectRect.y,
            1, selectRect.h
        }, { selectBox.redF(), selectBox.greenF(), selectBox.blueF(), selectBox.alphaF() });

        clearQuadrant({
            selectRect.x + selectRect.w, selectRect.y,
            1, selectRect.h
        }, { selectBox.redF(), selectBox.greenF(), selectBox.blueF(), selectBox.alphaF() });
    }
        
#ifdef RENDERDOC_SUPPORT
    if (_doRenderDoc && rdoc_api) rdoc_api->EndFrameCapture(NULL, NULL);
#endif

    _doRenderDoc = false;

    if (_animationTimer.isValid())
        update();
}

GLuint QMDLRenderer::createShader(GLenum type, const char *source)
{
	GLuint shader = glCreateShader(type);
    const char *sources[] = { source };
    GLint lengths[] = { (GLint) strlen(source) };

	glShaderSource(shader, 1, sources, lengths);
	glCompileShader(shader);

    GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status == GL_TRUE)
		return shader;

    static char infoLog[1024];
    static GLsizei length;
    glGetShaderInfoLog(shader, sizeof(infoLog), &length, infoLog);
	glDeleteShader(shader);
    throw std::runtime_error(infoLog);
}

GLuint QMDLRenderer::createProgram(GLuint vertexShader, GLuint fragmentShader)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
    
    glBindAttribLocation(program, ATTRIB_POSITION, "i_position");
    glBindAttribLocation(program, ATTRIB_TEXCOORD, "i_texcoord");
    glBindAttribLocation(program, ATTRIB_COLOR, "i_color");
    glBindAttribLocation(program, ATTRIB_NORMAL, "i_normal");
    glBindAttribLocation(program, ATTRIB_COLOR_SECONDARY, "i_color2D");

	glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_TRUE)
		return program;
        
    static char infoLog[1024];
    static GLsizei length;
    glGetProgramInfoLog(program, sizeof(infoLog), &length, infoLog);
	glDeleteProgram(program);
    throw std::runtime_error(infoLog);
}

template<typename T>
static void uploadToBuffer(GLuint buffer, bool full_upload, const std::vector<T> &data)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    if (full_upload)
        glBufferData(GL_ARRAY_BUFFER, sizeof(T) * data.size(), data.data(), GL_DYNAMIC_DRAW);
    else
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(T) * data.size(), data.data());
}

void QMDLRenderer::rebuildBuffer()
{
    auto &model = activeModel();

    size_t count = 0;

    for (auto &mesh : model.meshes)
        count += mesh.triangles.size() * 3;

    bool full_upload = _bufferData.size() < count;
    _bufferData.resize(count);
    _pointData.resize(count);
    _smoothNormalData.resize(count);
    _flatNormalData.resize(count);
    size_t n = 0;
    int cur_frame = model.selectedFrame, next_frame = model.selectedFrame;
    float frac = 0.0f;

    if (_animationTimer.isValid())
    {
        float frame_time = ((float) _animationTimer.nsecsElapsed() / 1000000000ull) * MainWindow::instance().animationFrameRate();
        int frame_offset = (int) frame_time;

        if (MainWindow::instance().animationInterpolated())
            frac = frame_time - frame_offset;

        int start = MainWindow::instance().animationStartFrame();
        int end = MainWindow::instance().animationEndFrame();

        if (end > start)
        {
            cur_frame = start + (frame_offset % (end - start));
            next_frame = start + ((frame_offset + 1) % (end - start));
        }
    }

    auto &painter = MainWindow::instance().uvEditor().getPainter();
    Matrix4 uvMatrix = painter.getDragMatrix();
    const ModelSkin *skin = model.getSelectedSkin();
    QVector2D tcScale;

    if (skin)
    {
        auto &uvEditor = MainWindow::instance().uvEditor();
        int scale = uvEditor.getZoom();
        tcScale = QVector2D{(float) skin->width * scale, (float) skin->height * scale};
    }
    
    for (auto &mesh : model.meshes)
    {
        for (auto &tri : mesh.triangles)
        {
            auto &from = mesh.frames[cur_frame];
            auto &to = mesh.frames[next_frame];
        
            auto &gv0 = mesh.vertices[tri.vertices[0]];
            auto &gv1 = mesh.vertices[tri.vertices[1]];
            auto &gv2 = mesh.vertices[tri.vertices[2]];

            auto &cv0 = from.vertices[tri.vertices[0]];
            auto &cv1 = from.vertices[tri.vertices[1]];
            auto &cv2 = from.vertices[tri.vertices[2]];

            auto &nv0 = to.vertices[tri.vertices[0]];
            auto &nv1 = to.vertices[tri.vertices[1]];
            auto &nv2 = to.vertices[tri.vertices[2]];
        
            std::array<QVector3D, 3> positions;
            std::array<QVector3D, 3> normals;
        
            if (frac == 0.0f)
            {
                positions[0] = cv0.position;
                positions[1] = cv1.position;
                positions[2] = cv2.position;

                normals[0] = cv0.normal;
                normals[1] = cv1.normal;
                normals[2] = cv2.normal;
            }
            else if (frac == 1.0f)
            {
                positions[0] = nv0.position;
                positions[1] = nv1.position;
                positions[2] = nv2.position;

                normals[0] = nv0.normal;
                normals[1] = nv1.normal;
                normals[2] = nv2.normal;
            }
            else
            {
                positions[0] = {
                    std::lerp(cv0.position[0], nv0.position[0], frac),
                    std::lerp(cv0.position[1], nv0.position[1], frac),
                    std::lerp(cv0.position[2], nv0.position[2], frac)
                };
                positions[1] = {
                    std::lerp(cv1.position[0], nv1.position[0], frac),
                    std::lerp(cv1.position[1], nv1.position[1], frac),
                    std::lerp(cv1.position[2], nv1.position[2], frac)
                };
                positions[2] = {
                    std::lerp(cv2.position[0], nv2.position[0], frac),
                    std::lerp(cv2.position[1], nv2.position[1], frac),
                    std::lerp(cv2.position[2], nv2.position[2], frac)
                };

                normals[0] = {
                    std::lerp(cv0.normal[0], nv0.normal[0], frac),
                    std::lerp(cv0.normal[1], nv0.normal[1], frac),
                    std::lerp(cv0.normal[2], nv0.normal[2], frac)
                };
                normals[1] = {
                    std::lerp(cv1.normal[0], nv1.normal[0], frac),
                    std::lerp(cv1.normal[1], nv1.normal[1], frac),
                    std::lerp(cv1.normal[2], nv1.normal[2], frac)
                };
                normals[2] = {
                    std::lerp(cv2.normal[0], nv2.normal[0], frac),
                    std::lerp(cv2.normal[1], nv2.normal[1], frac),
                    std::lerp(cv2.normal[2], nv2.normal[2], frac)
                };
            }
            
            auto &st0 = mesh.texcoords[tri.texcoords[0]];
            auto &st1 = mesh.texcoords[tri.texcoords[1]];
            auto &st2 = mesh.texcoords[tri.texcoords[2]];

            {
                auto &ov0 = _bufferData[n + 0];
                auto &ov1 = _bufferData[n + 1];
                auto &ov2 = _bufferData[n + 2];
            
                ov0.position = positions[0];
                ov1.position = positions[1];
                ov2.position = positions[2];
            
                auto &nv0 = _smoothNormalData[n + 0];
                auto &nv1 = _smoothNormalData[n + 1];
                auto &nv2 = _smoothNormalData[n + 2];

                nv0 = normals[0];
                nv1 = normals[1];
                nv2 = normals[2];
            
                _flatNormalData[n + 0] =
                _flatNormalData[n + 1] =
                _flatNormalData[n + 2] = (nv0 + nv1 + nv2) / 3;
            
                if (!uvMatrix.isIdentity())
                {
                    ov0.texcoord = uvMatrix.map((st0.pos * tcScale).toVector3D()).toVector2D() / tcScale;
                    ov1.texcoord = uvMatrix.map((st1.pos * tcScale).toVector3D()).toVector2D() / tcScale;
                    ov2.texcoord = uvMatrix.map((st2.pos * tcScale).toVector3D()).toVector2D() / tcScale;
                }
                else
                {
                    ov0.texcoord = st0.pos;
                    ov1.texcoord = st1.pos;
                    ov2.texcoord = st2.pos;
                }

                if (MainWindow::instance().getSelectMode() == SelectMode::Face)
                {
                    ov0.color = ov1.color = ov2.color = Settings().getEditorColor(tri.selectedFace ? EditorColorId::FaceSelected3D : EditorColorId::FaceUnselected3D);
                    ov0.color_secondary = ov1.color_secondary = ov2.color_secondary = Settings().getEditorColor(tri.selectedFace ? EditorColorId::FaceSelected2D : EditorColorId::FaceUnselected2D);
                }
                else
                {
                    ov0.color = ov1.color = ov2.color = Settings().getEditorColor(EditorColorId::FaceUnselected3D);
                    ov0.color_secondary = ov1.color_secondary = ov2.color_secondary = Settings().getEditorColor(EditorColorId::FaceUnselected2D);
                }
            }

            {
                auto &ov0 = _pointData[n + 0];
                auto &ov1 = _pointData[n + 1];
                auto &ov2 = _pointData[n + 2];
            
                ov0.position = positions[0];
                ov1.position = positions[1];
                ov2.position = positions[2];

                if (MainWindow::instance().getSelectMode() == SelectMode::Vertex)
                {
                    ov0.color = Settings().getEditorColor(gv0.selected ? EditorColorId::VertexTickSelected3D : EditorColorId::VertexTickUnselected3D);
                    ov1.color = Settings().getEditorColor(gv1.selected ? EditorColorId::VertexTickSelected3D : EditorColorId::VertexTickUnselected3D);
                    ov2.color = Settings().getEditorColor(gv2.selected ? EditorColorId::VertexTickSelected3D : EditorColorId::VertexTickUnselected3D);
                }
                else
                    ov0.color = ov1.color = ov2.color = Settings().getEditorColor(EditorColorId::VertexTickUnselected3D);
            }

            n += 3;
        }
    }

    uploadToBuffer(_buffer, full_upload, _bufferData);
    uploadToBuffer(_pointBuffer, full_upload, _pointData);
    uploadToBuffer(_smoothNormalBuffer, full_upload, _smoothNormalData);
    uploadToBuffer(_flatNormalBuffer, full_upload, _flatNormalData);
}

const ModelData &QMDLRenderer::activeModel()
{
    return MainWindow::instance().activeModel();
}

void QMDLRenderer::selectedSkinChanged()
{
    makeCurrent();
    MainWindow::instance().updateRenders();
}

void QMDLRenderer::modelLoaded()
{
    selectedSkinChanged();
    _2dOffset = activeModel().boundsOfAllFrames().centroid();
    // FIXME: reorganize so that this is applied in model space.
    _2dOffset = { -_2dOffset.y(), _2dOffset.x(), -_2dOffset.z() };
    _2dZoom = 1.0f;

    int32_t skin_index = 0;

    glDeleteTextures(_modelTextures.size(), _modelTextures.data());
    _modelTextures.clear();

    _modelTextures.resize(activeModel().skins.size());
    glGenTextures(_modelTextures.size(), _modelTextures.data());

    for (auto &skin : activeModel().skins)
    {
        glBindTexture(GL_TEXTURE_2D, _modelTextures[skin_index]);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, skin.width, skin.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, skin.image.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        skin_index++;
    }
}

void QMDLRenderer::captureRenderDoc(bool)
{
#ifdef RENDERDOC_SUPPORT
    if (!rdoc_api)
    {
        if (HMODULE mod = GetModuleHandleA("renderdoc.dll"))
        {
            pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
            RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
        }
    }

    this->_doRenderDoc = true;
    this->update();
#endif
}

void QMDLRenderer::resetAnimation()
{
    if (_animationTimer.isValid())
    {
        _animationTimer.restart();
        update();
    }
}

void QMDLRenderer::setAnimated(bool animating)
{
    bool isAnimating = _animationTimer.isValid();

    if (isAnimating == animating)
        return;

    if (!animating)
        _animationTimer.invalidate();
    else
    {
        _animationTimer.start();
        update();
    }
}