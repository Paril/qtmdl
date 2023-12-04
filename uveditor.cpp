#include "uveditor.h"
#include "ui_uveditor.h"
#include "mainwindow.h"
#include "qmdlrenderer.h"
#include "qtutils.h"
#include <unordered_set>

UVEditor::UVEditor(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::UVEditor)
{
    _ui->setupUi(this);

    QObject::connect(this->_ui->actionAlways_On_Top, &QAction::toggled, [this] () {
        this->setWindowFlag(Qt::WindowStaysOnTopHint, this->_ui->actionAlways_On_Top->isChecked());
        this->hide();
        this->show();
    });
    
    QObject::connect(this->_ui->actionZoom_In, &QAction::triggered, this, &UVEditor::zoomIn);
    QObject::connect(this->_ui->actionZoom_Out, &QAction::triggered, this, &UVEditor::zoomOut);
    QObject::connect(this->_ui->toolButton_17, &QToolButton::clicked, this, &UVEditor::zoomIn);
    QObject::connect(this->_ui->toolButton_16, &QToolButton::clicked, this, &UVEditor::zoomOut);

    QObject::connect(this->_ui->toolButton_19, &QToolButton::clicked, this, &UVEditor::nextSkin);
    QObject::connect(this->_ui->toolButton_18, &QToolButton::clicked, this, &UVEditor::prevSkin);

    QtUtils::setupMenuRadioButtons(this, {
		this->_ui->actionNone, this->_ui->actionSimple
	}, [this] () { this->_ui->uvDrawArea->update(); });

	QtUtils::setupMenuRadioButtons(this, {
		this->_ui->actionNone_2, this->_ui->actionPixels, this->_ui->actionSquares
	}, [this] () { this->_ui->uvDrawArea->update(); });

	QtUtils::setupGroupedButtons(this, {
		this->_ui->toolButton_7, this->_ui->toolButton_8, this->_ui->toolButton_9,
		this->_ui->toolButton_10
	});

	QtUtils::setupGroupedButtons(this, {
		this->_ui->toolButton_5, this->_ui->toolButton_4
	}, [this] () { this->_ui->uvDrawArea->update(); });
    
    QObject::connect(this->_ui->actionSelect_All, &QAction::triggered, this, &UVEditor::selectAll);
    QObject::connect(this->_ui->actionSelect_None, &QAction::triggered, this, &UVEditor::selectNone);
    QObject::connect(this->_ui->actionSelect_Inverse, &QAction::triggered, this, &UVEditor::selectInverse);
    QObject::connect(this->_ui->actionSelect_Connected, &QAction::triggered, this, &UVEditor::selectConnected);
    QObject::connect(this->_ui->actionSelect_Touching, &QAction::triggered, this, &UVEditor::selectTouching);
    
	QObject::connect(this->_ui->actionUndo, &QAction::triggered, [this] () { MainWindow::instance().undoRedo().undo(MainWindow::instance().activeModel()); MainWindow::instance().updateRenders(); });
	QObject::connect(this->_ui->actionRedo, &QAction::triggered, [this] () { MainWindow::instance().undoRedo().redo(MainWindow::instance().activeModel()); MainWindow::instance().updateRenders(); });

	QObject::connect(&MainWindow::instance().undoRedo(), &UndoRedo::undoRedoStateChanged, [this] () {
		this->_ui->actionUndo->setEnabled(MainWindow::instance().undoRedo().canUndo());
		this->_ui->actionRedo->setEnabled(MainWindow::instance().undoRedo().canRedo());
	});
}

UVEditor::~UVEditor()
{
    delete _ui;
}

void UVEditor::selectAll()
{
    auto &model = MainWindow::instance().activeModel();

    if (getSelectMode() == UVSelectMode::Vertex)
        for (auto &v : model.texcoords)
            v.selected = true;
    else
        for (auto &t : model.triangles)
            t.selectedUV = true;

    this->_ui->uvDrawArea->update();
}

void UVEditor::selectNone()
{
    auto &model = MainWindow::instance().activeModel();

    if (getSelectMode() == UVSelectMode::Vertex)
        for (auto &v : model.texcoords)
            v.selected = false;
    else
        for (auto &t : model.triangles)
            t.selectedUV = false;

    this->_ui->uvDrawArea->update();
}

void UVEditor::selectInverse()
{
    auto &model = MainWindow::instance().activeModel();

    if (getSelectMode() == UVSelectMode::Vertex)
        for (auto &v : model.texcoords)
            v.selected = !v.selected;
    else
        for (auto &t : model.triangles)
            t.selectedUV = !t.selectedUV;

    this->_ui->uvDrawArea->update();
}

static size_t countNumSelected(UVSelectMode mode, const ModelData &model)
{
    size_t num_selected = 0;

    if (mode == UVSelectMode::Vertex)
    {
        for (size_t i = 0; i < model.texcoords.size(); i++)
            if (model.texcoords[i].selected)
                num_selected++;
    }
    else
    {
        for (auto &tri : model.triangles)
            if (tri.selectedUV)
                num_selected++;
    }

    return num_selected;
}

// FIXME: optimize
void UVEditor::selectConnected()
{
    auto &model = MainWindow::instance().activeModel();
    size_t numSelected = countNumSelected(getSelectMode(), model);

    while (true)
    {
        selectTouching();

        size_t numSelectedNew = countNumSelected(getSelectMode(), model);
        
        if (numSelectedNew == numSelected)
            break;

        numSelected = numSelectedNew;
    }
}

void UVEditor::selectTouching()
{
    auto &model = MainWindow::instance().activeModel();
    std::unordered_set<size_t> selected;

    if (getSelectMode() == UVSelectMode::Vertex)
    {
        for (size_t i = 0; i < model.texcoords.size(); i++)
            if (model.texcoords[i].selected)
                selected.insert(i);

        for (auto &tri : model.triangles)
        {
            if (selected.contains(tri.texcoords[0]) ||
                selected.contains(tri.texcoords[1]) ||
                selected.contains(tri.texcoords[2]))
                for (auto &tc : tri.texcoords)
                    model.texcoords[tc].selected = true;
        }
    }
    else
    {
        for (auto &tri : model.triangles)
            if (tri.selectedUV)
                for (auto &tc : tri.texcoords)
                    selected.insert(tc);

        for (auto &tri : model.triangles)
        {
            if (selected.contains(tri.texcoords[0]) ||
                selected.contains(tri.texcoords[1]) ||
                selected.contains(tri.texcoords[2]))
                for (auto &tc : tri.texcoords)
                    tri.selectedUV = true;
        }
    }

    this->_ui->uvDrawArea->update();
}

LineDisplayMode UVEditor::getLineDisplayMode() const
{
    if (this->_ui->actionNone->isChecked())
        return LineDisplayMode::None;
    else
        return LineDisplayMode::Simple;
}

VertexDisplayMode UVEditor::getVertexDisplayMode() const
{
    if (this->_ui->actionNone_2->isChecked())
        return VertexDisplayMode::None;
    else if (this->_ui->actionPixels->isChecked())
        return VertexDisplayMode::Pixels;
    else
        return VertexDisplayMode::Squares;
}

UVEditorTool UVEditor::selectedTool() const
{
    if (this->_ui->toolButton_7->isChecked())
        return UVEditorTool::Select;
    else if (this->_ui->toolButton_8->isChecked())
        return UVEditorTool::Move;
    else if (this->_ui->toolButton_9->isChecked())
        return UVEditorTool::Rotate;
    else if (this->_ui->toolButton_10->isChecked())
        return UVEditorTool::Scale;

    throw std::runtime_error("bad tool");
}

UVSelectMode UVEditor::getSelectMode() const
{
    if (this->_ui->toolButton_5->isChecked())
        return UVSelectMode::Vertex;
    else if (this->_ui->toolButton_4->isChecked())
        return UVSelectMode::Face;

    throw std::runtime_error("bad tool");
}

QUVPainter &UVEditor::getPainter()
{
    return *this->_ui->uvDrawArea;
}

bool UVEditor::getModifyX() const
{
    return this->_ui->toolButton->isChecked();
}

bool UVEditor::getModifyY() const
{
    return this->_ui->toolButton_2->isChecked();
}

void UVEditor::show()
{
    this->setWindowFlag(Qt::WindowStaysOnTopHint, this->_ui->actionAlways_On_Top->isChecked());
    QMainWindow::show();
}

void UVEditor::zoomIn()
{
    _zoom++;
    this->_ui->uvDrawArea->update();
}

void UVEditor::zoomOut()
{
    if (_zoom == 1)
        return;

    _zoom--;
    this->_ui->uvDrawArea->update();
}

void UVEditor::resetZoom()
{
    _zoom = 1;
    this->_ui->uvDrawArea->update();
}

void UVEditor::nextSkin()
{
    auto &model = MainWindow::instance().activeModel();

    if (!model.skins.empty())
    {
        model.selectedSkin = std::min((int) model.skins.size() - 1, model.selectedSkin.value_or(0) + 1);
	    this->_ui->label_5->setText(QString::asprintf("%i", model.selectedSkin.value_or(0)));
        MainWindow::instance().mdlRenderer().selectedSkinChanged();
    }
}

void UVEditor::prevSkin()
{
    auto &model = MainWindow::instance().activeModel();

    if (!model.skins.empty())
    {
        model.selectedSkin = std::max(0, model.selectedSkin.value_or(0) - 1);
	    this->_ui->label_5->setText(QString::asprintf("%i", model.selectedSkin.value_or(0)));
        MainWindow::instance().mdlRenderer().selectedSkinChanged();
    }
}

void UVEditor::modelLoaded()
{
    auto &model = MainWindow::instance().activeModel();

    if (model.skins.empty())
        model.selectedSkin = std::nullopt;
    else
        model.selectedSkin = 0;
    
	this->_ui->label_5->setText(QString::asprintf("%i", model.selectedSkin.value_or(0)));
    resetZoom();
}
