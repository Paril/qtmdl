#include "mainwindow.h"
#include "quvpainter.h"
#include <QPainter>
#include "settings.h"

QUVPainter::QUVPainter(QWidget *parent) :
	QWidget(parent)
{
}

/*virtual*/ void QUVPainter::paintEvent(QPaintEvent *event) /*override*/
{
    ModelData &model = MainWindow::instance().activeModel();
    const std::optional<int> &skinId = model.selectedSkin;

    if (!skinId.has_value())
        return;

    int scale = MainWindow::instance().uvEditor().getZoom();

    const auto &skin = model.skins[*skinId];
    setMinimumWidth(skin.width * scale);
    setMinimumHeight(skin.height * scale);
    QPainter painter(this);
    painter.drawImage(QRect { 0, 0, skin.width * scale, skin.height * scale }, skin.image);
    
    QVector2D tcScale{(float) skin.width * scale, (float) skin.height * scale};

    painter.setPen(Settings().getEditorColor(EditorColorId::FaceLineUnselected2D));

    if (MainWindow::instance().uvEditor().getLineDisplayMode() == LineDisplayMode::Simple)
    {
        for (auto &v : model.triangles)
        {
            auto tc0 = model.texcoords[v.texcoords[0]].pos * tcScale;
            auto tc1 = model.texcoords[v.texcoords[1]].pos * tcScale;
            auto tc2 = model.texcoords[v.texcoords[2]].pos * tcScale;
        
            painter.drawLine(tc0.x(), tc0.y(), tc1.x(), tc1.y());
            painter.drawLine(tc1.x(), tc1.y(), tc2.x(), tc2.y());
            painter.drawLine(tc2.x(), tc2.y(), tc0.x(), tc0.y());
        }
    }

    if (MainWindow::instance().uvEditor().getVertexDisplayMode() != VertexDisplayMode::None)
    {
        for (auto &tc : model.texcoords)
        {
            QVector2D pos = tc.pos * tcScale;
            const QColor &color = Settings().getEditorColor(tc.selected ? EditorColorId::VertexTickSelected2D : EditorColorId::VertexTickUnselected2D); 

            if (MainWindow::instance().uvEditor().getVertexDisplayMode() == VertexDisplayMode::Squares)
                painter.fillRect(pos[0] - 1, pos[1] - 1, 3, 3, color);
            else
                painter.fillRect(pos[0], pos[1], 1, 1, color);
        }
    }
}