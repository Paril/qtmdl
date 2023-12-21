#include "mainwindow.h"
#include "quvpainter.h"
#include <QPainter>
#include <QMatrix4x4>
#include <QMouseEvent>
#include "settings.h"
#include "qtutils.h"
#include "qmdlrenderer.h"

QUVPainter::QUVPainter(QWidget *parent) :
	QWidget(parent)
{
}

/*virtual*/ void QUVPainter::paintEvent(QPaintEvent *event) /*override*/
{
    auto &model = MainWindow::instance().activeModel();
    auto skin = model.getSelectedSkin();

    if (!skin)
        return;

    auto &uvEditor = MainWindow::instance().uvEditor();
    int scale = uvEditor.getZoom();
    QVector2D tcScale{(float) skin->width * scale, (float) skin->height * scale};
    setMinimumWidth((int) tcScale.x());
    setMinimumHeight((int) tcScale.y());
    QPainter painter(this);
    painter.drawImage(QRect { 0, 0, (int) tcScale.x(), (int) tcScale.y() }, skin->image);
    
    QMatrix4x4 drag = getDragMatrix();
    const auto &verticesSelected = model.getSelectedTextureCoordinates(uvEditor.getSelectMode());
    const auto &coordinates = model.transformTexcoords(drag, uvEditor.getSelectMode());

    if (uvEditor.getLineDisplayMode() == LineDisplayMode::Simple)
    {
        painter.setPen(Settings().getEditorColor(EditorColorId::FaceLineUnselectedUV));
        painter.setBrush(QBrush(Settings().getEditorColor(EditorColorId::FaceUnselectedUV)));

        for (auto &v : model.triangles)
        {
            if (uvEditor.getSelectMode() == UVSelectMode::Face && v.selectedUV)
                continue;

            QPointF points[] = {
                (coordinates[v.texcoords[0]] * tcScale).toPointF(),
                (coordinates[v.texcoords[1]] * tcScale).toPointF(),
                (coordinates[v.texcoords[2]] * tcScale).toPointF()
            };

            painter.drawConvexPolygon(points, std::size(points));
        }

        if (uvEditor.getSelectMode() == UVSelectMode::Face)
        {
            painter.setPen(Settings().getEditorColor(EditorColorId::FaceLineSelectedUV));
            painter.setBrush(QBrush(Settings().getEditorColor(EditorColorId::FaceSelectedUV)));

            for (auto &v : model.triangles)
            {
                if (!v.selectedUV)
                    continue;

                QPointF points[] = {
                    (coordinates[v.texcoords[0]] * tcScale).toPointF(),
                    (coordinates[v.texcoords[1]] * tcScale).toPointF(),
                    (coordinates[v.texcoords[2]] * tcScale).toPointF()
                };

                painter.drawConvexPolygon(points, std::size(points));
            }
        }
    }

    if (uvEditor.getVertexDisplayMode() != VertexDisplayMode::None)
    {
        for (size_t i = 0; i < model.texcoords.size(); i++)
        {
            QVector2D pos = coordinates[i] * tcScale;

            const QColor &color = Settings().getEditorColor(verticesSelected.contains(i) ?
                EditorColorId::VertexTickSelectedUV : EditorColorId::VertexTickUnselectedUV); 

            if (uvEditor.getVertexDisplayMode() == VertexDisplayMode::Squares)
                painter.fillRect(pos.x() - 1, pos.y() - 1, 3, 3, color);
            else
                painter.fillRect(pos.x(), pos.y(), 1, 1, color);
        }
    }

    if (_dragging && uvEditor.selectedTool() == UVEditorTool::Select)
    {
        painter.setPen(Settings().getEditorColor(EditorColorId::SelectBox));
        painter.setBrush(QBrush(QColor(0, 0, 0, 0)));
        painter.drawRect(_dragWorldPos.x(), _dragWorldPos.y(), _dragPos.x() - _dragWorldPos.x(), _dragPos.y() - _dragWorldPos.y());
    }
}

void QUVPainter::mousePressEvent(QMouseEvent *e)
{
    _dragging = true;
    _dragPos = _downPos = mapFromGlobal(QCursor::pos()) * devicePixelRatio();
    _dragWorldPos = QVector2D(_dragPos);
    _dragDelta = {};
    mouseMoveEvent(e);
}

void QUVPainter::rectangleSelect(QMouseEvent *e, QRectF rect, QVector2D tcScale)
{
    auto &uvEditor = MainWindow::instance().uvEditor();
    auto &model = MainWindow::instance().activeModel();
    auto &mutator = MainWindow::instance().activeModelMutator();

    if (uvEditor.getSelectMode() == UVSelectMode::Vertex)
    {
        if (!rect.width() || !rect.height())
        {
            rect.setLeft(rect.left() - (2 / tcScale.x()));
            rect.setTop(rect.top() - (2 / tcScale.x()));
            rect.setWidth(5 / tcScale.x());
            rect.setHeight(5 / tcScale.y());
        }

        mutator.selectRectangleVerticesUV(rect, e->modifiers());
    }
    else
        mutator.selectRectangleTrianglesUV(rect, e->modifiers());

    if (uvEditor.getSyncSelection())
    {
        mutator.syncSelectionUV();
        MainWindow::instance().updateRenders();
    }
}

void QUVPainter::mouseReleaseEvent(QMouseEvent *e)
{
    if (!_dragging)
        return;

    auto &model = MainWindow::instance().activeModel();
    auto skin = model.getSelectedSkin();

    if (skin)
    {
        auto &uvEditor = MainWindow::instance().uvEditor();
        int scale = uvEditor.getZoom();
        QVector2D tcScale{(float) skin->width * scale, (float) skin->height * scale};

        if (_dragging)
        {
            auto &uvEditor = MainWindow::instance().uvEditor();

            if (uvEditor.selectedTool() == UVEditorTool::Select)
                rectangleSelect(e, { (_dragWorldPos / tcScale).toPointF(), (QVector2D(_dragPos.x(), _dragPos.y()) / tcScale).toPointF() }, tcScale);
            else
            {
                QMatrix4x4 drag = getDragMatrix();

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
        }
    }

    _dragging = false;

    MainWindow::instance().updateRenders();
}

QMatrix4x4 QUVPainter::getDragMatrix()
{
    QMatrix4x4 matrix;
    
    if (!_dragging || _dragDelta.isNull())
        return matrix;

    auto &model = MainWindow::instance().activeModel();
    auto skin = model.getSelectedSkin();

    if (!skin)
        return matrix;

    auto &uvEditor = MainWindow::instance().uvEditor();
    float zoom = (float) uvEditor.getZoom();

    if (uvEditor.selectedTool() == UVEditorTool::Move)
    {
        if (uvEditor.getModifyX())
            matrix.translate(-_dragDelta.x() / zoom, 0, 0);
        if (uvEditor.getModifyY())
            matrix.translate(0, -_dragDelta.y() / zoom, 0);
    }
    else if (uvEditor.selectedTool() == UVEditorTool::Scale)
    {
        float s = 1.0f + (_dragDelta.y() * 0.01f) / zoom;
        matrix.translate(_dragWorldPos.x() / zoom, _dragWorldPos.y() / zoom);
        if (uvEditor.getModifyX())
            matrix.scale(s, 1.0f);
        if (uvEditor.getModifyY())
            matrix.scale(1.0f, s);
        matrix.translate(-_dragWorldPos.x() / zoom, -_dragWorldPos.y() / zoom);
    }
    else if (uvEditor.selectedTool() == UVEditorTool::Rotate)
    {
        float r = 360.f * (_dragDelta.y() / (float) height());
        matrix.translate(_dragWorldPos.x() / zoom, _dragWorldPos.y() / zoom);
        matrix.rotate(r, QVector3D(0, 0, -1));
        matrix.translate(-_dragWorldPos.x() / zoom, -_dragWorldPos.y() / zoom);
    }

    return matrix;
}

void QUVPainter::focusLost()
{
    QMouseEvent ev {
        QEvent::Type::MouseButtonRelease,
        mapFromGlobal(QCursor::pos()) * devicePixelRatio(),
        QCursor::pos(),
        Qt::MouseButton::NoButton,
        {},
        {}
    };
    QUVPainter::mouseReleaseEvent(&ev);
}

void QUVPainter::mouseMoveEvent(QMouseEvent *event)
{
    auto pos = mapFromGlobal(QCursor::pos()) * devicePixelRatio();
    auto &uvEditor = MainWindow::instance().uvEditor();
    float zoom = uvEditor.getZoom();

    if (_dragging)
    {
        auto delta = _dragPos - pos;

        _dragDelta += delta;
        
        float xDelta = delta.x() / zoom;
        float yDelta = delta.y() / zoom;
        
        if (uvEditor.selectedTool() != UVEditorTool::Select)
        {
            _dragPos = { QtUtils::wrap(pos.x(), 0, static_cast<int>(width() * devicePixelRatio()) - 1), QtUtils::wrap(pos.y(), 0, static_cast<int>(height() * devicePixelRatio()) - 1) };

            if (_dragPos != pos)
                QCursor::setPos(mapToGlobal(_dragPos / devicePixelRatio()));
        }
        else
            _dragPos = pos;

        MainWindow::instance().updateRenders();
        return;
    }
}
