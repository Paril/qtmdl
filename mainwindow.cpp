#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "qtutils.h"
#include <qopenglcontext.h>
#include "settings.h"
#include "quvpainter.h"
#include "modelloader.h"
#include <QMessageBox>
#include <QMimeDatabase>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
    _ui(new Ui::MainWindow)
{
	_instance = this;

    _ui->setupUi(this);

	_uveditor = new UVEditor();
	
    QObject::connect(this->_ui->actionNew, &QAction::triggered, this, &MainWindow::newClicked);
    QObject::connect(this->_ui->actionOpen, &QAction::triggered, this, &MainWindow::openClicked);
    QObject::connect(this->_ui->actionExport, &QAction::triggered, this, &MainWindow::exportClicked);
	QObject::connect(this->_ui->actionCapture_RenderDoc_Frame, &QAction::triggered, this->_ui->openGLWidget, &QMDLRenderer::captureRenderDoc);

	QObject::connect(this->_ui->horizontalSlider, &QSlider::valueChanged, this, &MainWindow::frameChanged);
	QObject::connect(this->_ui->toolButton_18, &QToolButton::clicked, [this] () { this->_ui->horizontalSlider->triggerAction(QAbstractSlider::SliderAction::SliderSingleStepSub); });
	QObject::connect(this->_ui->toolButton_19, &QToolButton::clicked, [this] () { this->_ui->horizontalSlider->triggerAction(QAbstractSlider::SliderAction::SliderSingleStepAdd); });

	QObject::connect(this->_ui->actionShow_Grid, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));
	QObject::connect(this->_ui->actionShow_Origin, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));
	QObject::connect(this->_ui->actionVertice_Ticks, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));

	QtUtils::setupMenuRadioButtons(this, {
		this->_ui->actionWireframe, this->_ui->actionFlat, this->_ui->actionTextured
	}, [this] () { this->updateRenders(); });
	
	QObject::connect(this->_ui->actionDraw_Backfaces, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));
	QObject::connect(this->_ui->actionPer_Vertex_Normals, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));
	QObject::connect(this->_ui->actionShading, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));

	QtUtils::setupMenuRadioButtons(this, {
		this->_ui->actionWireframe_2, this->_ui->actionFlat_2, this->_ui->actionTextured_2
	}, [this] () { this->updateRenders(); });
	
	QObject::connect(this->_ui->actionDraw_Backfaces_2, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));
	QObject::connect(this->_ui->actionPer_Vertex_Normals_2, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));
	QObject::connect(this->_ui->actionShading_2, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));
	
	QObject::connect(this->_ui->spinBox, &QSpinBox::valueChanged, this, &MainWindow::animationChanged);
	QObject::connect(this->_ui->spinBox_2, &QSpinBox::valueChanged, this, &MainWindow::animationChanged);
	QObject::connect(this->_ui->spinBox_3, &QSpinBox::valueChanged, this, &MainWindow::animationChanged);
	QObject::connect(this->_ui->toolButton_14, &QToolButton::clicked, this, &MainWindow::toggleAnimation);
	QObject::connect(this->_ui->toolButton_15, &QToolButton::clicked, this, &MainWindow::toggleAnimation);

	QtUtils::setupGroupedButtons(this, {
		this->_ui->toolButton_7, this->_ui->toolButton_8, this->_ui->toolButton_9,
		this->_ui->toolButton_10, this->_ui->toolButton_6, this->_ui->toolButton_20, this->_ui->toolButton_21
	});

	QObject::connect(qApp, &QGuiApplication::applicationStateChanged, this, [this] (Qt::ApplicationState state) {
		if (state != Qt::ApplicationState::ApplicationActive)
		{
			this->_ui->openGLWidget->focusLost();
			this->uvEditor().getPainter().focusLost();
		}
	});
	
	QObject::connect(this->_ui->actionSkins, &QAction::triggered, [this] () {
		if (this->_uveditor->isVisible())
			this->_uveditor->activateWindow();
		else
			this->_uveditor->show();
	});

	QObject::connect(this->_ui->actionSync_Skin_Selection, &QAction::toggled, [this] (bool value) {
        this->uvEditor().setSyncSelection(value);
        MainWindow::instance().updateRenders();
    });
	
	QtUtils::setupGroupedButtons(this, {
		this->_ui->toolButton_5, this->_ui->toolButton_4
	}, [this] () { this->updateRenders(); });

#if 0
	QObject::connect(this->_ui->actionUndo, &QAction::triggered, [this] () { this->undoRedo().undo(this->_activeModel); this->updateRenders(); });
	QObject::connect(this->_ui->actionRedo, &QAction::triggered, [this] () { this->undoRedo().redo(this->_activeModel); this->updateRenders(); });

	QObject::connect(_undoRedo, &UndoRedo::undoRedoStateChanged, [this] () {
		this->_ui->actionUndo->setEnabled(this->undoRedo().canUndo());
		this->_ui->actionRedo->setEnabled(this->undoRedo().canRedo());
	});
#endif
}

MainWindow::~MainWindow()
{
	delete _ui;
}

/*static*/ MainWindow *MainWindow::_instance = nullptr;

bool MainWindow::showGrid() const
{
	return this->_ui->actionShow_Grid->isChecked();
}

bool MainWindow::showOrigin() const
{
	return this->_ui->actionShow_Origin->isChecked();
}

bool MainWindow::vertexTicks() const
{
	return this->_ui->actionVertice_Ticks->isChecked();
}

int MainWindow::animationFrameRate() const
{
	return this->_ui->spinBox->value();
}

bool MainWindow::animationInterpolated() const
{
	return this->_ui->toolButton_15->isChecked();
}

int MainWindow::animationStartFrame() const
{
	return this->_ui->spinBox_2->value();
}

int MainWindow::animationEndFrame() const
{
	return this->_ui->spinBox_3->value();
}

bool MainWindow::getSyncSelection() const
{
	return this->_ui->actionSync_Skin_Selection->isChecked();
}

void MainWindow::setSyncSelection(bool value)
{
	this->_ui->actionSync_Skin_Selection->setChecked(value);
	activeModelMutator().syncSelection3D(0);
}

RenderParameters MainWindow::getRenderParameters(bool is_2d) const
{
	RenderMode mode;

	if ((is_2d ? this->_ui->actionWireframe : this->_ui->actionWireframe_2)->isChecked())
		mode = RenderMode::Wireframe;
	else if ((is_2d ? this->_ui->actionFlat : this->_ui->actionFlat_2)->isChecked())
		mode = RenderMode::Flat;
	else
		mode = RenderMode::Textured;

	return {
		mode,
		(is_2d ? this->_ui->actionDraw_Backfaces : this->_ui->actionDraw_Backfaces_2)->isChecked(),
		(is_2d ? this->_ui->actionPer_Vertex_Normals : this->_ui->actionPer_Vertex_Normals_2)->isChecked(),
		(is_2d ? this->_ui->actionShading : this->_ui->actionShading_2)->isChecked(),
	};
}

EditorTool MainWindow::selectedTool() const
{
	if (this->_ui->toolButton_6->isChecked())
		return EditorTool::Pan;
	else if (this->_ui->toolButton_7->isChecked())
		return EditorTool::Select;
	else if (this->_ui->toolButton_8->isChecked())
		return EditorTool::Move;
	else if (this->_ui->toolButton_9->isChecked())
		return EditorTool::Rotate;
	else if (this->_ui->toolButton_10->isChecked())
		return EditorTool::Scale;
	else if (this->_ui->toolButton_20->isChecked())
		return EditorTool::CreateVertex;
	else if (this->_ui->toolButton_21->isChecked())
		return EditorTool::CreateFace;

	throw std::runtime_error("wat");
}

SelectMode MainWindow::getSelectMode() const
{
    if (this->_ui->toolButton_5->isChecked())
        return SelectMode::Vertex;
    else if (this->_ui->toolButton_4->isChecked())
        return SelectMode::Face;

    throw std::runtime_error("bad tool");
}

QMDLRenderer &MainWindow::mdlRenderer()
{
	return *_ui->openGLWidget;
}

void MainWindow::setCurrentWorldPosition(const QVector3D &position)
{
	this->_ui->doubleSpinBox->setValue(position.x());
	this->_ui->doubleSpinBox_2->setValue(position.y());
	this->_ui->doubleSpinBox_3->setValue(position.z());
}

void MainWindow::animationChanged()
{
	this->_ui->openGLWidget->resetAnimation();
}

void MainWindow::toggleAnimation()
{
	this->_ui->openGLWidget->setAnimated(this->_ui->toolButton_14->isChecked());
}

void MainWindow::loadModel(QFileInfo path)
{
	try
	{
		static QMimeDatabase db;
		std::unique_ptr<ModelData> model = ModelLoader::load(path, db.mimeTypeForFile(path.filePath()));
		_activeModelMutator = ModelMutator { model.get() };
		_activeModel.reset(model.release());
	}
	catch(std::runtime_error e)
	{
		QMessageBox::warning(this, tr("QTMDL"), tr("Unknown file type."));
		return;
	}

	this->_uveditor->modelLoaded();
	this->_ui->openGLWidget->modelLoaded();
	this->updateRenders();
	frameCountChanged();
	Settings().setModelDialogLocation(path.dir().path());
}

void MainWindow::saveModel(QFileInfo path)
{
	static QMimeDatabase db;
	ModelLoader::save(activeModel(), path, db.mimeTypeForFile(path.filePath()));
	Settings().setModelDialogLocation(path.dir().path());
}

void MainWindow::clearModel()
{
	_activeModel.reset();
	this->_uveditor->modelLoaded();
	this->_ui->openGLWidget->modelLoaded();
	this->updateRenders();
	frameCountChanged();
}

void MainWindow::updateRenders()
{
	this->_ui->openGLWidget->update();

	if (_uveditor->isVisible())
		_uveditor->getPainter().update();
}

/*virtual*/ void MainWindow::closeEvent(QCloseEvent *event) /*override*/
{
	delete _uveditor;
}

void MainWindow::newClicked()
{
	// TODO: are you sure?
	clearModel();
}

std::unique_ptr<QFileDialog> MainWindow::makeFileDialog(QString title, QFileDialog::FileMode mode, QFileDialog::AcceptMode accept)
{
	// TODO: are you sure?
    QFileDialog *dlg = new QFileDialog(this, title, Settings().getModelDialogLocation());

	dlg->setFileMode(mode);
	dlg->setAcceptMode(accept);

	dlg->setMimeTypeFilters({
		"x-qtmdl/mdl",
		"x-qtmdl/md2",
		"x-qtmdl/md2f",
		"x-qtmdl/md3",
		"x-qtmdl/qim",
		"application/octet-stream"
	});

	// compose filter for all supported types
	// https://stackoverflow.com/a/46534037
	QMimeDatabase mimeDB;
	QStringList allSupportedFormats;
	for(const QString& mimeTypeFilter : dlg->mimeTypeFilters())
	{
		QMimeType mimeType = mimeDB.mimeTypeForName(mimeTypeFilter);
		if (mimeType.isValid())
			allSupportedFormats.append(mimeType.globPatterns());
	}
	QString allSupportedFormatsFilter = QString("All supported formats (%1)").arg(allSupportedFormats.join(' '));
	
	QStringList nameFilters = dlg->nameFilters();
	nameFilters.prepend(allSupportedFormatsFilter);
	dlg->setNameFilters(nameFilters);
	return std::unique_ptr<QFileDialog>(dlg);
}

void MainWindow::openClicked()
{
	// TODO: are you sure?
    std::unique_ptr<QFileDialog> dlg = makeFileDialog("Load Model", QFileDialog::ExistingFile, QFileDialog::AcceptOpen);

    if (dlg->exec() == QFileDialog::Accepted)
		loadModel(QFileInfo(dlg->selectedFiles()[0]));
}

void MainWindow::exportClicked()
{
	// TODO: are you sure?
    std::unique_ptr<QFileDialog> dlg = makeFileDialog("Export Model", QFileDialog::FileMode::AnyFile, QFileDialog::AcceptSave);

    if (dlg->exec() == QFileDialog::Accepted)
	{
		// TODO: pass selected mime filter
		saveModel(QFileInfo(dlg->selectedFiles()[0]));
	}
}

void MainWindow::frameCountChanged()
{
	int maxFrames = (int) (activeModel().frames.size() - 1);

	this->_ui->horizontalSlider->setMaximum(maxFrames);
	this->_ui->spinBox_2->setMaximum(maxFrames);
	this->_ui->spinBox_3->setMaximum(maxFrames);

	frameChanged();
}

void MainWindow::frameChanged()
{
	_activeModelMutator.setSelectedFrame(this->_ui->horizontalSlider->value());

	updateRenders();

	this->_ui->label_5->setText(QString::asprintf("%i", activeModel().selectedFrame));

	if (activeModel().frames.size())
		this->_ui->label_frameName->setText(QString::fromStdString(activeModel().frames[activeModel().selectedFrame].name));
}
