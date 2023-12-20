#pragma once

#include <QMainWindow>
#include <QOpenGLDebugLogger>
#include "modeldata.h"
#include "uveditor.h"
#include <memory>
#include <QFileInfo>
#include <QFileDialog>

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
    bool getSyncSelection() const;
    void setSyncSelection(bool value) const;
    RenderParameters getRenderParameters(bool is_2d) const;
    EditorTool selectedTool() const;
    SelectMode getSelectMode() const;
    inline const ModelData &activeModel() const
    {
        if (_activeModel)
            return *_activeModel;

        static ModelData emptyModel;
        return emptyModel;
    }
    constexpr ModelMutator &activeModelMutator()
    {
        if (!_activeModelMutator.isValid())
            throw std::runtime_error("no model to mutate");
        return _activeModelMutator;
    }
    constexpr UVEditor &uvEditor() { return *_uveditor; }
    QMDLRenderer &mdlRenderer();

    void setCurrentWorldPosition(const QVector3D &position);
    void frameCountChanged();

    void loadModel(QFileInfo path);
    void saveModel(QFileInfo path);
    void clearModel();

    void updateRenders();

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *_ui;
    std::unique_ptr<const ModelData> _activeModel;
    ModelMutator _activeModelMutator;
    UVEditor *_uveditor;

    void newClicked();
    std::unique_ptr<QFileDialog> makeFileDialog(QString title, QFileDialog::FileMode mode, QFileDialog::AcceptMode accept);
    void openClicked();
    void exportClicked();
    void frameChanged();
    void animationChanged();
    void toggleAnimation();

    static MainWindow *_instance;
};
