#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "qtutils.h"
#include <qopenglcontext.h>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
    _ui(new Ui::MainWindow),
	settings(QSettings::IniFormat, QSettings::UserScope, "Altered Softworks", "QTMDL")
{
	_instance = this;

    _ui->setupUi(this);
	
    QObject::connect(this->_ui->actionNew, &QAction::triggered, this, &MainWindow::newClicked);
    QObject::connect(this->_ui->actionOpen, &QAction::triggered, this, &MainWindow::openClicked);
	QObject::connect(this->_ui->actionCapture_RenderDoc_Frame, &QAction::triggered, this->_ui->openGLWidget, &QMDLRenderer::captureRenderDoc);

	QObject::connect(this->_ui->horizontalSlider, &QSlider::valueChanged, this, &MainWindow::frameChanged);
	QObject::connect(this->_ui->toolButton_18, &QToolButton::clicked, [this] () { this->_ui->horizontalSlider->triggerAction(QAbstractSlider::SliderAction::SliderSingleStepSub); });
	QObject::connect(this->_ui->toolButton_19, &QToolButton::clicked, [this] () { this->_ui->horizontalSlider->triggerAction(QAbstractSlider::SliderAction::SliderSingleStepAdd); });

	QObject::connect(this->_ui->actionShow_Grid, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));
	QObject::connect(this->_ui->actionShow_Origin, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));
	QObject::connect(this->_ui->actionVertice_Ticks, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));

	QtUtils::setupMenuRadioButtons(this, {
		this->_ui->actionWireframe, this->_ui->actionFlat, this->_ui->actionTextured
	}, [this] () { this->_ui->openGLWidget->update(); });
	
	QObject::connect(this->_ui->actionDraw_Backfaces, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));
	QObject::connect(this->_ui->actionPer_Vertex_Normals, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));
	QObject::connect(this->_ui->actionShading, &QAction::toggled, this->_ui->openGLWidget, qOverload<>(&QWidget::update));

	QtUtils::setupMenuRadioButtons(this, {
		this->_ui->actionWireframe_2, this->_ui->actionFlat_2, this->_ui->actionTextured_2
	}, [this] () { this->_ui->openGLWidget->update(); });
	
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
	}, [this] () { });

	QObject::connect(qApp, &QGuiApplication::applicationStateChanged, this, [this] (Qt::ApplicationState state) {
		if (state != Qt::ApplicationState::ApplicationActive)
			this->_ui->openGLWidget->focusLost();
	});
	
	QObject::connect(this->_ui->actionUndo, &QAction::triggered, [this] () { this->undoRedo.undo(this->_activeModel); this->_ui->openGLWidget->update(); });
	QObject::connect(this->_ui->actionRedo, &QAction::triggered, [this] () { this->undoRedo.redo(this->_activeModel); this->_ui->openGLWidget->update(); });

	QObject::connect(this->_ui->actionSkins, &QAction::triggered, [this] () {
		if (this->_uveditor.isVisible())
			this->_uveditor.activateWindow();
		else
			this->_uveditor.show();
	});
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

#include <QFileDialog>

constexpr QVector3D anorms[] = {
	{-0.525731f, 0.000000f, 0.850651f}, 
	{-0.442863f, 0.238856f, 0.864188f}, 
	{-0.295242f, 0.000000f, 0.955423f}, 
	{-0.309017f, 0.500000f, 0.809017f}, 
	{-0.162460f, 0.262866f, 0.951056f}, 
	{0.000000f, 0.000000f, 1.000000f}, 
	{0.000000f, 0.850651f, 0.525731f}, 
	{-0.147621f, 0.716567f, 0.681718f}, 
	{0.147621f, 0.716567f, 0.681718f}, 
	{0.000000f, 0.525731f, 0.850651f}, 
	{0.309017f, 0.500000f, 0.809017f}, 
	{0.525731f, 0.000000f, 0.850651f}, 
	{0.295242f, 0.000000f, 0.955423f}, 
	{0.442863f, 0.238856f, 0.864188f}, 
	{0.162460f, 0.262866f, 0.951056f}, 
	{-0.681718f, 0.147621f, 0.716567f}, 
	{-0.809017f, 0.309017f, 0.500000f}, 
	{-0.587785f, 0.425325f, 0.688191f}, 
	{-0.850651f, 0.525731f, 0.000000f}, 
	{-0.864188f, 0.442863f, 0.238856f}, 
	{-0.716567f, 0.681718f, 0.147621f}, 
	{-0.688191f, 0.587785f, 0.425325f}, 
	{-0.500000f, 0.809017f, 0.309017f}, 
	{-0.238856f, 0.864188f, 0.442863f}, 
	{-0.425325f, 0.688191f, 0.587785f}, 
	{-0.716567f, 0.681718f, -0.147621f}, 
	{-0.500000f, 0.809017f, -0.309017f}, 
	{-0.525731f, 0.850651f, 0.000000f}, 
	{0.000000f, 0.850651f, -0.525731f}, 
	{-0.238856f, 0.864188f, -0.442863f}, 
	{0.000000f, 0.955423f, -0.295242f}, 
	{-0.262866f, 0.951056f, -0.162460f}, 
	{0.000000f, 1.000000f, 0.000000f}, 
	{0.000000f, 0.955423f, 0.295242f}, 
	{-0.262866f, 0.951056f, 0.162460f}, 
	{0.238856f, 0.864188f, 0.442863f}, 
	{0.262866f, 0.951056f, 0.162460f}, 
	{0.500000f, 0.809017f, 0.309017f}, 
	{0.238856f, 0.864188f, -0.442863f}, 
	{0.262866f, 0.951056f, -0.162460f}, 
	{0.500000f, 0.809017f, -0.309017f}, 
	{0.850651f, 0.525731f, 0.000000f}, 
	{0.716567f, 0.681718f, 0.147621f}, 
	{0.716567f, 0.681718f, -0.147621f}, 
	{0.525731f, 0.850651f, 0.000000f}, 
	{0.425325f, 0.688191f, 0.587785f}, 
	{0.864188f, 0.442863f, 0.238856f}, 
	{0.688191f, 0.587785f, 0.425325f}, 
	{0.809017f, 0.309017f, 0.500000f}, 
	{0.681718f, 0.147621f, 0.716567f}, 
	{0.587785f, 0.425325f, 0.688191f}, 
	{0.955423f, 0.295242f, 0.000000f}, 
	{1.000000f, 0.000000f, 0.000000f}, 
	{0.951056f, 0.162460f, 0.262866f}, 
	{0.850651f, -0.525731f, 0.000000f}, 
	{0.955423f, -0.295242f, 0.000000f}, 
	{0.864188f, -0.442863f, 0.238856f}, 
	{0.951056f, -0.162460f, 0.262866f}, 
	{0.809017f, -0.309017f, 0.500000f}, 
	{0.681718f, -0.147621f, 0.716567f}, 
	{0.850651f, 0.000000f, 0.525731f}, 
	{0.864188f, 0.442863f, -0.238856f}, 
	{0.809017f, 0.309017f, -0.500000f}, 
	{0.951056f, 0.162460f, -0.262866f}, 
	{0.525731f, 0.000000f, -0.850651f}, 
	{0.681718f, 0.147621f, -0.716567f}, 
	{0.681718f, -0.147621f, -0.716567f}, 
	{0.850651f, 0.000000f, -0.525731f}, 
	{0.809017f, -0.309017f, -0.500000f}, 
	{0.864188f, -0.442863f, -0.238856f}, 
	{0.951056f, -0.162460f, -0.262866f}, 
	{0.147621f, 0.716567f, -0.681718f}, 
	{0.309017f, 0.500000f, -0.809017f}, 
	{0.425325f, 0.688191f, -0.587785f}, 
	{0.442863f, 0.238856f, -0.864188f}, 
	{0.587785f, 0.425325f, -0.688191f}, 
	{0.688191f, 0.587785f, -0.425325f}, 
	{-0.147621f, 0.716567f, -0.681718f}, 
	{-0.309017f, 0.500000f, -0.809017f}, 
	{0.000000f, 0.525731f, -0.850651f}, 
	{-0.525731f, 0.000000f, -0.850651f}, 
	{-0.442863f, 0.238856f, -0.864188f}, 
	{-0.295242f, 0.000000f, -0.955423f}, 
	{-0.162460f, 0.262866f, -0.951056f}, 
	{0.000000f, 0.000000f, -1.000000f}, 
	{0.295242f, 0.000000f, -0.955423f}, 
	{0.162460f, 0.262866f, -0.951056f}, 
	{-0.442863f, -0.238856f, -0.864188f}, 
	{-0.309017f, -0.500000f, -0.809017f}, 
	{-0.162460f, -0.262866f, -0.951056f}, 
	{0.000000f, -0.850651f, -0.525731f}, 
	{-0.147621f, -0.716567f, -0.681718f}, 
	{0.147621f, -0.716567f, -0.681718f}, 
	{0.000000f, -0.525731f, -0.850651f}, 
	{0.309017f, -0.500000f, -0.809017f}, 
	{0.442863f, -0.238856f, -0.864188f}, 
	{0.162460f, -0.262866f, -0.951056f}, 
	{0.238856f, -0.864188f, -0.442863f}, 
	{0.500000f, -0.809017f, -0.309017f}, 
	{0.425325f, -0.688191f, -0.587785f}, 
	{0.716567f, -0.681718f, -0.147621f}, 
	{0.688191f, -0.587785f, -0.425325f}, 
	{0.587785f, -0.425325f, -0.688191f}, 
	{0.000000f, -0.955423f, -0.295242f}, 
	{0.000000f, -1.000000f, 0.000000f}, 
	{0.262866f, -0.951056f, -0.162460f}, 
	{0.000000f, -0.850651f, 0.525731f}, 
	{0.000000f, -0.955423f, 0.295242f}, 
	{0.238856f, -0.864188f, 0.442863f}, 
	{0.262866f, -0.951056f, 0.162460f}, 
	{0.500000f, -0.809017f, 0.309017f}, 
	{0.716567f, -0.681718f, 0.147621f}, 
	{0.525731f, -0.850651f, 0.000000f}, 
	{-0.238856f, -0.864188f, -0.442863f}, 
	{-0.500000f, -0.809017f, -0.309017f}, 
	{-0.262866f, -0.951056f, -0.162460f}, 
	{-0.850651f, -0.525731f, 0.000000f}, 
	{-0.716567f, -0.681718f, -0.147621f}, 
	{-0.716567f, -0.681718f, 0.147621f}, 
	{-0.525731f, -0.850651f, 0.000000f}, 
	{-0.500000f, -0.809017f, 0.309017f}, 
	{-0.238856f, -0.864188f, 0.442863f}, 
	{-0.262866f, -0.951056f, 0.162460f}, 
	{-0.864188f, -0.442863f, 0.238856f}, 
	{-0.809017f, -0.309017f, 0.500000f}, 
	{-0.688191f, -0.587785f, 0.425325f}, 
	{-0.681718f, -0.147621f, 0.716567f}, 
	{-0.442863f, -0.238856f, 0.864188f}, 
	{-0.587785f, -0.425325f, 0.688191f}, 
	{-0.309017f, -0.500000f, 0.809017f}, 
	{-0.147621f, -0.716567f, 0.681718f}, 
	{-0.425325f, -0.688191f, 0.587785f}, 
	{-0.162460f, -0.262866f, 0.951056f}, 
	{0.442863f, -0.238856f, 0.864188f}, 
	{0.162460f, -0.262866f, 0.951056f}, 
	{0.309017f, -0.500000f, 0.809017f}, 
	{0.147621f, -0.716567f, 0.681718f}, 
	{0.000000f, -0.525731f, 0.850651f}, 
	{0.425325f, -0.688191f, 0.587785f}, 
	{0.587785f, -0.425325f, 0.688191f}, 
	{0.688191f, -0.587785f, 0.425325f}, 
	{-0.955423f, 0.295242f, 0.000000f}, 
	{-0.951056f, 0.162460f, 0.262866f}, 
	{-1.000000f, 0.000000f, 0.000000f}, 
	{-0.850651f, 0.000000f, 0.525731f}, 
	{-0.955423f, -0.295242f, 0.000000f}, 
	{-0.951056f, -0.162460f, 0.262866f}, 
	{-0.864188f, 0.442863f, -0.238856f}, 
	{-0.951056f, 0.162460f, -0.262866f}, 
	{-0.809017f, 0.309017f, -0.500000f}, 
	{-0.864188f, -0.442863f, -0.238856f}, 
	{-0.951056f, -0.162460f, -0.262866f}, 
	{-0.809017f, -0.309017f, -0.500000f}, 
	{-0.681718f, 0.147621f, -0.716567f}, 
	{-0.681718f, -0.147621f, -0.716567f}, 
	{-0.850651f, 0.000000f, -0.525731f}, 
	{-0.688191f, 0.587785f, -0.425325f}, 
	{-0.587785f, 0.425325f, -0.688191f}, 
	{-0.425325f, 0.688191f, -0.587785f}, 
	{-0.425325f, -0.688191f, -0.587785f}, 
	{-0.587785f, -0.425325f, -0.688191f}, 
	{-0.688191f, -0.587785f, -0.425325f}
};

/*
========================================================================

.MD2 triangle model file format

========================================================================
*/

#define IDALIASHEADER		(('2'<<24)+('P'<<16)+('D'<<8)+'I')
#define ALIAS_VERSION	8

#define	MAX_TRIANGLES	4096
#define MAX_VERTS		2048
#define MAX_FRAMES		512
#define MAX_MD2SKINS	32
#define	MAX_SKINNAME	64

struct dstvert_t
{
	short	s;
	short	t;
};

struct dtriangle_t
{
	std::array<short, 3> index_xyz;
	std::array<short, 3> index_st;
};

struct dtrivertx_t
{
	std::array<byte, 3> v;			// scaled byte to fit in frame mins/maxs
	byte	            lightnormalindex;
};

struct daliasframe_t
{
	std::array<float, 3> scale;	// multiply byte verts by this
	std::array<float, 3> translate;	// then add this
	char		         name[16];	// frame name from grabbing
	//dtrivertx_t	verts[1];	// variable sized
};

struct dmdl_t
{
	int			ident;
	int			version;

	int			skinwidth;
	int			skinheight;
	int			framesize;		// byte size of each frame

	int			num_skins;
	int			num_xyz;
	int			num_st;			// greater than num_xyz for seams
	int			num_tris;
	int			num_glcmds;		// dwords in strip/fan command list
	int			num_frames;

	int			ofs_skins;		// each skin is a MAX_SKINNAME string
	int			ofs_st;			// byte offset from start for stverts
	int			ofs_tris;		// offset for dtriangles
	int			ofs_frames;		// offset for first frame
	int			ofs_glcmds;	
	int			ofs_end;		// end of file
};

struct pcx_t
{
    char    manufacturer;
    char    version;
    char    encoding;
    char    bits_per_pixel;
    unsigned short  xmin,ymin,xmax,ymax;
    unsigned short  hres,vres;
    unsigned char   palette[48];
    char    reserved;
    char    color_planes;
    unsigned short  bytes_per_line;
    unsigned short  palette_type;
    char    filler[58];
};

/*
==============
LoadPCX
==============
*/
bool LoadPCX (QDataStream &skinStream, ModelSkin &skin)
{
    pcx_t   pcx;
    int     x, y;
    uint8_t dataByte, runLength;
    byte    *pix, *raw_pix;

    //
    // parse the PCX file
    //
	skinStream >> pcx.manufacturer >> pcx.version >> pcx.encoding >> pcx.bits_per_pixel;
	skinStream >> pcx.xmin >> pcx.ymin >> pcx.xmax >> pcx.ymax;
	skinStream >> pcx.hres >> pcx.vres;
	skinStream.skipRawData(sizeof(pcx_t::palette));
	skinStream >> pcx.reserved >> pcx.color_planes;
	skinStream >> pcx.bytes_per_line >> pcx.palette_type;
	skinStream.skipRawData(sizeof(pcx_t::filler));

    if (pcx.manufacturer != 0x0a
        || pcx.version != 5
        || pcx.encoding != 1
        || pcx.bits_per_pixel != 8)
    {
		return false;
    }

	skin.width = pcx.xmax + 1;
	skin.height = pcx.ymax + 1;

	SkinPaletteData &raw_data = skin.raw_data.emplace(SkinPaletteData{});
	raw_data.data.resize(skin.width * skin.height);
	std::vector<uint8_t> &palette = raw_data.palette.emplace(std::vector<uint8_t>(static_cast<size_t>(768)));

	skin.image = QImage(skin.width, skin.height, QImage::Format_ARGB32);

	raw_pix = raw_data.data.data();
	pix = skin.image.bits();

	skinStream.device()->seek(skinStream.device()->size() - 768);

	skinStream.readRawData(reinterpret_cast<char *>(palette.data()), 768);

	skinStream.device()->seek(sizeof(pcx_t));

    for (y=0 ; y<=pcx.ymax ; y++, pix += (pcx.xmax + 1) * 4, raw_pix += (pcx.xmax + 1))
    {
        for (x=0; x<=pcx.xmax ; )
        {
            skinStream >> dataByte;

            if((dataByte & 0xC0) == 0xC0)
            {
                runLength = dataByte & 0x3F;
                skinStream >> dataByte;
            }
            else
                runLength = 1;

            while(runLength-- > 0)
			{
                pix[(x * 4) + 2] = palette[(dataByte * 3) + 0];
                pix[(x * 4) + 1] = palette[(dataByte * 3) + 1];
                pix[(x * 4) + 0] = palette[(dataByte * 3) + 2];
				pix[(x * 4) + 3] = 0xFF;
				raw_pix[x] = dataByte;
				x++;
			}
        }
    }

	return true;
}

ModelData LoadMD2(QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::ExistingOnly))
        throw std::runtime_error("bad");

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
	stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

	dmdl_t header;
	stream.readRawData(reinterpret_cast<char *>(&header), sizeof(header));

	ModelData data;

	data.frames.resize(header.num_frames);

	for (auto &frame : data.frames)
		frame.vertices.resize(header.num_xyz);

	data.vertices.resize(header.num_xyz);

	file.seek(header.ofs_frames);

	for (auto &frame : data.frames)
	{
		qint64 pos = file.pos();
		daliasframe_t frame_header;
		stream >> frame_header.scale[0] >> frame_header.scale[1] >> frame_header.scale[2];
		pos = file.pos();
		stream >> frame_header.translate[0] >> frame_header.translate[1] >> frame_header.translate[2];
		pos = file.pos();
		stream.readRawData(frame_header.name, sizeof(frame_header.name));
		frame_header.name[sizeof(frame_header.name) - 1] = '\0';

		frame.name = frame_header.name;

		for (auto &vert : frame.vertices)
		{
			dtrivertx_t v;
			stream >> v.v[0] >> v.v[1] >> v.v[2];
			stream >> v.lightnormalindex;

			vert.position = {
				(v.v[0] * frame_header.scale[0]) + frame_header.translate[0],
				(v.v[1] * frame_header.scale[1]) + frame_header.translate[1],
				(v.v[2] * frame_header.scale[2]) + frame_header.translate[2]
			};
			vert.normal = anorms[v.lightnormalindex];
		}
	}

	file.seek(header.ofs_st);
	data.texcoords.resize(header.num_st);

	for (auto &st : data.texcoords)
	{
		dstvert_t v;
		stream >> v.s >> v.t;
		st.pos = { (float) v.s / header.skinwidth, (float) v.t / header.skinheight };
	}

	file.seek(header.ofs_tris);
	data.triangles.resize(header.num_tris);

	for (auto &tri : data.triangles)
	{
		dtriangle_t t;
		stream >> t.index_xyz[0] >> t.index_xyz[1] >> t.index_xyz[2];
		stream >> t.index_st[0] >> t.index_st[1] >> t.index_st[2];
		
		std::copy(t.index_xyz.begin(), t.index_xyz.end(), tri.vertices.begin());
		std::copy(t.index_st.begin(), t.index_st.end(), tri.texcoords.begin());
	}

	data.skins.resize(header.num_skins);
	
	file.seek(header.ofs_skins);

	QDir model_dir = QFileInfo(filename).dir();
	QString model_file = model_dir.absolutePath();

	for (auto &skin : data.skins)
	{
		char skin_path[64];
		stream.readRawData(skin_path, sizeof(skin_path));
		skin_path[sizeof(skin_path) - 1] = '\0';

		// try to find the matching PCX file
		QDir skin_dir = model_dir;
		QFileInfo skin_file;

		while (!skin_dir.isRoot())
		{
			skin_file = QFileInfo(skin_dir.filePath(skin_path));

			if (skin_file.exists())
				break;

			skin_dir.cdUp();
		}

		if (!skin_file.exists())
			continue;

		QFile sf = QFile(skin_file.filePath());

		if (!sf.open(QIODevice::ReadOnly | QIODevice::ExistingOnly))
			continue;
		
		QDataStream skinStream(&sf);
		skinStream.setByteOrder(QDataStream::LittleEndian);
		skinStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

		LoadPCX(skinStream, skin);
	}

	return data;
}

void MainWindow::loadModel(QString path)
{
    _activeModel = LoadMD2(path);
	this->_uveditor.modelLoaded();
	this->_ui->openGLWidget->modelLoaded();
	frameCountChanged();
}

void MainWindow::clearModel()
{
	_activeModel = {};
	this->_uveditor.modelLoaded();
	this->_ui->openGLWidget->modelLoaded();
	frameCountChanged();
}

void MainWindow::newClicked()
{
	// TODO: are you sure?
	clearModel();
}

void MainWindow::openClicked()
{
	// TODO: are you sure?

    QFileDialog dlg(this, "Load MD2", "", "*.md2");
    if (dlg.exec() == QFileDialog::Accepted)
		loadModel(dlg.selectedFiles()[0]);
}

void MainWindow::frameCountChanged()
{
	int maxFrames = (int) (_activeModel.frames.size() - 1);

	this->_ui->horizontalSlider->setMaximum(maxFrames);
	this->_ui->spinBox_2->setMaximum(maxFrames);
	this->_ui->spinBox_3->setMaximum(maxFrames);

	frameChanged();
}

void MainWindow::frameChanged()
{
	_activeModel.selectedFrame = this->_ui->horizontalSlider->value();

	this->_ui->openGLWidget->update();

	this->_ui->label_5->setText(QString::asprintf("%i", _activeModel.selectedFrame));

	if (_activeModel.frames.size())
		this->_ui->label_frameName->setText(QString::fromStdString(_activeModel.frames[_activeModel.selectedFrame].name));
}
