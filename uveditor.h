#ifndef UVEDITOR_H
#define UVEDITOR_H

#include <QMainWindow>
#include <optional>
#include "editortypes.h"

namespace Ui {
class UVEditor;
}

class QUVPainter;

enum class LineDisplayMode
{
    None,
    Simple
};

enum class VertexDisplayMode
{
    None,
    Pixels,
    Squares
};

enum class UVEditorTool
{
    Select,
    Move,
    Rotate,
    Scale
};

class UVEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit UVEditor(QWidget *parent = nullptr);
    ~UVEditor();

    void show();
    constexpr int getZoom() const { return _zoom; }
    LineDisplayMode getLineDisplayMode() const;
    VertexDisplayMode getVertexDisplayMode() const;
    UVEditorTool selectedTool() const;
    UVSelectMode getSelectMode() const;
    QUVPainter &getPainter();
    bool getModifyX() const;
    bool getModifyY() const;
    bool getSyncSelection() const;
    void setSyncSelection(bool value) const;

    void resetZoom();
    void modelLoaded();
    
    void selectAll();
    void selectNone();
    void selectInverse();
    void selectConnected();
    void selectTouching();

private:
    void zoomIn();
    void zoomOut();
    void nextSkin();
    void prevSkin();

    Ui::UVEditor *_ui;
    int _zoom = 1;
};

#endif // UVEDITOR_H
