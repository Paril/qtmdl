#ifndef UVEDITOR_H
#define UVEDITOR_H

#include <QMainWindow>
#include <optional>

namespace Ui {
class UVEditor;
}

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

    void resetZoom();
    void modelLoaded();

private:
    void zoomIn();
    void zoomOut();
    void nextSkin();
    void prevSkin();

    Ui::UVEditor *_ui;
    int _zoom = 1;
};

#endif // UVEDITOR_H
