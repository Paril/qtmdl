#pragma once

#include <QMainWindow>
#include <QOpenGLDebugLogger>
#include "modeldata.h"
#include "undoredo.h"
#include "uveditor.h"
#include <memory>
#include <QFileInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QMDLRenderer;

enum class SelectMode
{
    Vertex,
    Face
};

enum class RenderMode
{
    Wireframe,
    Flat,
    Textured
};

struct RenderParameters
{
    RenderMode  mode;
    bool        drawBackfaces;
    bool        smoothNormals;
    bool        shaded;
};

enum class EditorTool
{
    Pan,
    Select,
    Move,
    Rotate,
    Scale,
    CreateVertex,
    CreateFace
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static MainWindow &instance() { return *_instance; }

    bool showGrid() const;
    bool showOrigin() const;
    bool vertexTicks() const;
    int animationFrameRate() const;
    bool animationInterpolated() const;
    int animationStartFrame() const;
    int animationEndFrame() const;
    RenderParameters getRenderParameters(bool is_2d) const;
    EditorTool selectedTool() const;
    constexpr ModelData &activeModel() { return _activeModel; }
    constexpr const ModelData &activeModel() const { return _activeModel; }
    constexpr UVEditor &uvEditor() { return *_uveditor; }
    QMDLRenderer &mdlRenderer();
    constexpr UndoRedo &undoRedo() { return *_undoRedo; }

    void setCurrentWorldPosition(const QVector3D &position);
    void frameCountChanged();

    void loadModel(QFileInfo path);
    void clearModel();

    void updateRenders();

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *_ui;
    ModelData _activeModel;
    UVEditor *_uveditor;
    UndoRedo *_undoRedo;

    void newClicked();
    void openClicked();
    void frameChanged();
    void animationChanged();
    void toggleAnimation();

    static MainWindow *_instance;
};
