#include "uveditor.h"
#include "ui_uveditor.h"
#include "mainwindow.h"
#include "qmdlrenderer.h"

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
}

UVEditor::~UVEditor()
{
    delete _ui;
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
        this->_ui->uvDrawArea->update();
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
        this->_ui->uvDrawArea->update();
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