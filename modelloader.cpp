#include "modelloader.h"
#include <QDir>

using byte = uint8_t;

constexpr int32_t QIM_MAGIC = (('M'<<24)+('I'<<16)+('T'<<8)+'Q');

static inline ModelData constructBlankModel()
{
	ModelData model;
	model.frames.emplace_back("Frame 1");
	model.meshes.emplace_back();
	model.meshes[0].frames.emplace_back();
	return model;
}

/*static*/ const ModelData ModelData::blankModel = constructBlankModel();

void SaveQIM(const ModelData &model, QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        throw std::runtime_error("bad");

    QDataStream stream(&file);
	stream.setVersion(QDataStream::Version::Qt_6_5);

	stream << QIM_MAGIC;
	stream << QIM_VERSION;

	QDataSync sync(QIM_VERSION, stream, false);

	const_cast<ModelData &>(model).sync(sync);
}

std::unique_ptr<ModelData> LoadQIM(QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::ExistingOnly))
        throw std::runtime_error("bad");
	
    QDataStream stream(&file);
	stream.setVersion(QDataStream::Version::Qt_6_5);

	int32_t v;

	stream >> v;
	stream >> v;

	QDataSync sync(v, stream, true);

	ModelData data;
	data.sync(sync);

	return std::make_unique<ModelData>(std::move(data));
}

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

static uint8_t CompressNormal(const QVector3D &v)
{
	float bestdot = 0;
	const QVector3D *bestnorm = nullptr;

	for (auto &norm : anorms)
	{
		float dot = QVector3D::dotProduct(v, norm);

		if (bestnorm && dot <= bestdot)
			continue;

		bestdot = dot;
		bestnorm = &norm;
	}

	if (!bestnorm)
		throw std::runtime_error("fatal");

	return bestnorm - anorms;
}

/*
==============
LoadPCX
==============
*/
struct pcx_t
{
    int8_t   manufacturer;
    int8_t   version;
    int8_t   encoding;
    int8_t   bits_per_pixel;
    uint16_t xmin,ymin,xmax,ymax;
    uint16_t hres,vres;
    uint8_t  palette[48];
    int8_t   reserved;
    int8_t   color_planes;
    uint16_t bytes_per_line;
    uint16_t palette_type;
    int8_t   filler[58];
};

bool LoadPCX (QDataStream &skinStream, ModelSkin &skin)
{
    pcx_t   pcx;
    int     x, y;
    uint8_t dataByte, runLength;
    byte    *raw_pix;

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

	raw_pix = raw_data.data.data();

	skinStream.device()->seek(skinStream.device()->size() - 768);

	skinStream.readRawData(reinterpret_cast<char *>(palette.data()), 768);

	skinStream.device()->seek(sizeof(pcx_t));

    for (y=0 ; y<=pcx.ymax ; y++, raw_pix += (pcx.xmax + 1))
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
				raw_pix[x] = dataByte;
				x++;
			}
        }
    }

	return true;
}


/*
========================================================================

.MD2 triangle model file format

========================================================================
*/
constexpr int32_t MD2_MAGIC = (('2'<<24)+('P'<<16)+('D'<<8)+'I');
constexpr int32_t MD2_VERSION = 8;

constexpr size_t MD2_MAX_SKINNAME = 64;
constexpr size_t MD2_MAX_FRAMENAME = 16;

struct dstvert_t
{
	int16_t	s;
	int16_t	t;
};

struct dtriangle_t
{
	std::array<int16_t, 3> index_xyz;
	std::array<int16_t, 3> index_st;
};

struct dtrivertx_t
{
	std::array<uint8_t, 3> v;			// scaled byte to fit in frame mins/maxs
	uint8_t				   lightnormalindex;
};

struct daliasframe_t
{
	QVector3D            scale;	// multiply byte verts by this
	QVector3D            translate;	// then add this
	char		         name[MD2_MAX_FRAMENAME];	// frame name from grabbing
};

struct dmdl_t
{
	int32_t	ident;
	int32_t	version;

	int32_t	skinwidth;
	int32_t	skinheight;
	int32_t	framesize;		// byte size of each frame

	int32_t	num_skins;
	int32_t	num_xyz;
	int32_t	num_st;			// greater than num_xyz for seams
	int32_t	num_tris;
	int32_t	num_glcmds;		// dwords in strip/fan command list
	int32_t	num_frames;

	int32_t	ofs_skins;		// each skin is a MAX_SKINNAME string
	int32_t	ofs_st;			// byte offset from start for stverts
	int32_t	ofs_tris;		// offset for dtriangles
	int32_t	ofs_frames;		// offset for first frame
	int32_t	ofs_glcmds;	
	int32_t	ofs_end;		// end of file
};

std::unique_ptr<ModelData> LoadMD2(QString filename)
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

	auto &mesh = data.meshes.emplace_back();

	data.frames.resize(header.num_frames);
	mesh.frames.resize(header.num_frames);

	for (auto &frame : mesh.frames)
		frame.vertices.resize(header.num_xyz);

	mesh.vertices.resize(header.num_xyz);

	file.seek(header.ofs_frames);

	for (size_t i = 0; i < data.frames.size(); i++)
	{
		auto &modelframe = data.frames[i];
		auto &meshframe = mesh.frames[i];

		daliasframe_t frame_header;
		stream >> frame_header.scale[0] >> frame_header.scale[1] >> frame_header.scale[2];
		stream >> frame_header.translate[0] >> frame_header.translate[1] >> frame_header.translate[2];
		stream.readRawData(frame_header.name, sizeof(frame_header.name));
		frame_header.name[sizeof(frame_header.name) - 1] = '\0';

		modelframe.name = frame_header.name;

		for (auto &vert : meshframe.vertices)
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
	mesh.texcoords.resize(header.num_st);

	for (auto &st : mesh.texcoords)
	{
		dstvert_t v;
		stream >> v.s >> v.t;
		st.pos = { (float) v.s / header.skinwidth, (float) v.t / header.skinheight };
	}

	file.seek(header.ofs_tris);
	mesh.triangles.resize(header.num_tris);

	for (auto &tri : mesh.triangles)
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
		char skin_path[MD2_MAX_SKINNAME];
		stream.readRawData(skin_path, sizeof(skin_path));
		skin_path[sizeof(skin_path) - 1] = '\0';
		skin.name = skin_path;

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

	return std::make_unique<ModelData>(std::move(data));
}

void SaveMD2(const ModelData &model, QString filename)
{
#if 0
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        throw std::runtime_error("bad");

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
	stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

	dmdl_t header;

	header.ident = MD2_MAGIC;
	header.version = MD2_VERSION;

	if (model.skins.size())
	{
		// TODO: warn on mismatches
		header.skinwidth = model.skins[0].width;
		header.skinheight = model.skins[0].height;
	}
	else
		header.skinwidth = header.skinheight = 0;

	header.framesize = sizeof(daliasframe_t) + (sizeof(dtrivertx_t) * model.vertices.size());
	header.num_skins = model.skins.size();
	header.num_xyz = model.vertices.size();
	header.num_st = model.texcoords.size();
	header.num_tris = model.triangles.size();
	header.num_glcmds = 0;
	header.num_frames = model.frames.size();

	header.ofs_skins = sizeof(dmdl_t);
	header.ofs_st = header.ofs_skins + (MD2_MAX_SKINNAME * model.skins.size());
	header.ofs_tris = header.ofs_st + (sizeof(dstvert_t) * model.texcoords.size());
	header.ofs_frames = header.ofs_tris + (sizeof(dtriangle_t) * model.triangles.size());
	header.ofs_glcmds = header.ofs_end = header.ofs_frames + (header.framesize * model.frames.size());

	stream.writeRawData(reinterpret_cast<const char *>(&header), sizeof(header));

	for (auto &skin : model.skins)
	{
		size_t l = std::min((MD2_MAX_SKINNAME - 1), skin.name.size());
		stream.writeRawData(skin.name.data(), (int) l);

		for (l; l < MD2_MAX_SKINNAME; l++)
			stream << (uint8_t) 0;
	}

	for (auto &st : model.texcoords)
	{
		dstvert_t v;
		v.s = (st.pos.x() * header.skinwidth) + 0.5f;
		v.t = (st.pos.y() * header.skinheight) + 0.5f;
		stream << v.s << v.t;
	}

	for (auto &tri : model.triangles)
	{
		dtriangle_t t;
		
		std::copy(tri.vertices.begin(), tri.vertices.end(), t.index_xyz.begin());
		std::copy(tri.texcoords.begin(), tri.texcoords.end(), t.index_st.begin());

		stream << t.index_xyz[0] << t.index_xyz[1] << t.index_xyz[2];
		stream << t.index_st[0] << t.index_st[1] << t.index_st[2];
	}

	for (auto &frame : model.frames)
	{
		BoundingBox box = frame.bounds();
		daliasframe_t frame_header;
		
		memset(frame_header.name, 0, sizeof(frame_header.name));
		size_t l = std::min((sizeof(frame_header.name) - 1), frame.name.size());
		strncpy(frame_header.name, frame.name.data(), l);

		QVector3D c = box.mins;
		QVector3D s = (box.maxs - box.mins ) / 255.f;
		frame_header.translate = { c[0], c[1], c[2] };
		frame_header.scale = { s[0], s[1], s[2] };

		stream << frame_header.scale[0] << frame_header.scale[1] << frame_header.scale[2];
		stream << frame_header.translate[0] << frame_header.translate[1] << frame_header.translate[2];
		stream.writeRawData(frame_header.name, sizeof(frame_header.name));

		for (auto &vert : frame.vertices)
		{
			dtrivertx_t v;

			v.v = {
				(uint8_t) std::clamp(rint((vert.position[0] - frame_header.translate[0]) / frame_header.scale[0]), 0.f, 255.f),
				(uint8_t) std::clamp(rint((vert.position[1] - frame_header.translate[1]) / frame_header.scale[1]), 0.f, 255.f),
				(uint8_t) std::clamp(rint((vert.position[2] - frame_header.translate[2]) / frame_header.scale[2]), 0.f, 255.f)
			};
			v.lightnormalindex = CompressNormal(vert.normal);

			stream << v.v[0] << v.v[1] << v.v[2];
			stream << v.lightnormalindex;
		}
	}
#endif
}

/*
========================================================================

.MD2F triangle model file format

========================================================================
*/
constexpr int32_t MD2F_VERSION = 9;

struct dtrivertxf_t
{
	QVector3D	position;
	QVector3D	normal;
};

struct daliasframef_t
{
	char		         name[MD2_MAX_FRAMENAME];	// frame name from grabbing
};

struct dmd2f_t
{
	int32_t	ident;
	int32_t	version;

	int32_t	skinwidth;
	int32_t	skinheight;
	int32_t	framesize;		// byte size of each frame

	int32_t	num_skins;
	int32_t	num_xyz;
	int32_t	num_st;			// greater than num_xyz for seams
	int32_t	num_tris;
	int32_t	num_frames;

	int32_t	ofs_skins;		// each skin is a MAX_SKINNAME string
	int32_t	ofs_st;			// byte offset from start for stverts
	int32_t	ofs_tris;		// offset for dtriangles
	int32_t	ofs_frames;		// offset for first frame
	int32_t	ofs_end;		// end of file
};

std::optional<QFileInfo> FindSkinFile(QDir base_dir, const char *skin_path, const std::initializer_list<const char *> &formats)
{
	// try to find the matching PCX file
	QDir skin_dir = base_dir;
	QFileInfo skin_file;

	while (!skin_dir.isRoot())
	{
		for (auto &format : formats)
		{
			skin_file = QFileInfo(skin_dir.filePath(skin_path));
			QString s = skin_file.path() + "/" + skin_file.baseName() + "." + format;
			skin_file = QFileInfo(s);

			if (skin_file.exists())
				break;
		}

		if (skin_file.exists())
			break;

		skin_dir.cdUp();
	}

	if (!skin_file.exists())
		return std::nullopt;

	return skin_file;
}

std::unique_ptr<ModelData> LoadMD2F(QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::ExistingOnly))
        throw std::runtime_error("bad");

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
	stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

	dmd2f_t header;
	stream.readRawData(reinterpret_cast<char *>(&header), sizeof(header));

	ModelData data;

	data.frames.resize(header.num_frames);

	auto &mesh = data.meshes.emplace_back();

	data.frames.resize(header.num_frames);
	mesh.frames.resize(header.num_frames);

	for (auto &frame : mesh.frames)
		frame.vertices.resize(header.num_xyz);

	mesh.vertices.resize(header.num_xyz);
	
	for (size_t i = 0; i < data.frames.size(); i++)
	{
		auto &modelframe = data.frames[i];
		auto &meshframe = mesh.frames[i];

		qint64 pos = file.pos();
		daliasframef_t frame_header;
		stream.readRawData(frame_header.name, sizeof(frame_header.name));
		frame_header.name[sizeof(frame_header.name) - 1] = '\0';

		modelframe.name = frame_header.name;

		for (auto &vert : meshframe.vertices)
		{
			dtrivertxf_t v;
			stream >> v.position[0] >> v.position[1] >> v.position[2];
			stream >> v.normal[0] >> v.normal[1] >> v.normal[2];

			vert.position = v.position;
			vert.normal = v.normal;
		}
	}

	file.seek(header.ofs_st);
	mesh.texcoords.resize(header.num_st);

	for (auto &st : mesh.texcoords)
	{
		dstvert_t v;
		stream >> v.s >> v.t;
		st.pos = { (float) v.s / header.skinwidth, (float) v.t / header.skinheight };
	}

	file.seek(header.ofs_tris);
	mesh.triangles.resize(header.num_tris);

	for (auto &tri : mesh.triangles)
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

	for (auto &skin : data.skins)
	{
		char skin_path[MD2_MAX_SKINNAME];
		stream.readRawData(skin_path, sizeof(skin_path));
		skin_path[sizeof(skin_path) - 1] = '\0';

		// try to find the matching PCX file
		if (auto skin_file = FindSkinFile(model_dir, skin_path, { "pcx", "tga", "png" }))
		{
			QFile sf = QFile(skin_file->filePath());

			if (!sf.open(QIODevice::ReadOnly | QIODevice::ExistingOnly))
				continue;
		
			QDataStream skinStream(&sf);
			skinStream.setByteOrder(QDataStream::LittleEndian);
			skinStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

			LoadPCX(skinStream, skin);
		}
	}
	
	return std::make_unique<ModelData>(std::move(data));
}

// Quake 1 MDL

#define ALIAS_VERSION	6

#define ALIAS_ONSEAM				0x0020

// must match definition in spritegn.h
enum synctype_t {ST_SYNC=0, ST_RAND };

enum aliasframetype_t { ALIAS_SINGLE=0, ALIAS_GROUP };

enum aliasskintype_t { ALIAS_SKIN_SINGLE=0, ALIAS_SKIN_GROUP };

using vec3_t = QVector3D;

struct mdl_t {
	int			ident;
	int			version;
	vec3_t		scale;
	vec3_t		scale_origin;
	float		boundingradius;
	vec3_t		eyeposition;
	int			numskins;
	int			skinwidth;
	int			skinheight;
	int			numverts;
	int			numtris;
	int			numframes;
	synctype_t	synctype;
	int			flags;
	float		size;
};

// TODO: could be shorts

struct stvert_t {
	int		onseam;
	int		s;
	int		t;
};

struct dmdltriangle_t {
	int					facesfront;
	int					vertindex[3];
};

constexpr int DT_FACES_FRONT				= 0x0010;

using trivertx_t = dtrivertx_t;

struct dmdlaliasframe_t {
	trivertx_t	bboxmin;	// lightnormal isn't used
	trivertx_t	bboxmax;	// lightnormal isn't used
	char		name[16];	// frame name from grabbing
};

struct daliasgroup_t {
	int			numframes;
	trivertx_t	bboxmin;	// lightnormal isn't used
	trivertx_t	bboxmax;	// lightnormal isn't used
};

struct daliasskingroup_t {
	int			numskins;
};

struct daliasinterval_t {
	float	interval;
};

struct daliasskininterval_t {
	float	interval;
};

struct daliasframetype_t {
	aliasframetype_t	type;
};

struct daliasskintype_t
{
	aliasskintype_t	type;
};

constexpr int32_t IDPOLYHEADER	= (('O'<<24)+('P'<<16)+('D'<<8)+'I'); // little-endian "IDPO"

/* C:\Users\Paril\Desktop\palette.lmp (2023-12-20 6:06:19 AM)
   StartOffset(h): 00000000, EndOffset(h): 000002FF, Length(h): 00000300 */

constexpr uint8_t quakePalette[768] = {
	0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x1F, 0x1F, 0x1F, 0x2F, 0x2F, 0x2F,
	0x3F, 0x3F, 0x3F, 0x4B, 0x4B, 0x4B, 0x5B, 0x5B, 0x5B, 0x6B, 0x6B, 0x6B,
	0x7B, 0x7B, 0x7B, 0x8B, 0x8B, 0x8B, 0x9B, 0x9B, 0x9B, 0xAB, 0xAB, 0xAB,
	0xBB, 0xBB, 0xBB, 0xCB, 0xCB, 0xCB, 0xDB, 0xDB, 0xDB, 0xEB, 0xEB, 0xEB,
	0x0F, 0x0B, 0x07, 0x17, 0x0F, 0x0B, 0x1F, 0x17, 0x0B, 0x27, 0x1B, 0x0F,
	0x2F, 0x23, 0x13, 0x37, 0x2B, 0x17, 0x3F, 0x2F, 0x17, 0x4B, 0x37, 0x1B,
	0x53, 0x3B, 0x1B, 0x5B, 0x43, 0x1F, 0x63, 0x4B, 0x1F, 0x6B, 0x53, 0x1F,
	0x73, 0x57, 0x1F, 0x7B, 0x5F, 0x23, 0x83, 0x67, 0x23, 0x8F, 0x6F, 0x23,
	0x0B, 0x0B, 0x0F, 0x13, 0x13, 0x1B, 0x1B, 0x1B, 0x27, 0x27, 0x27, 0x33,
	0x2F, 0x2F, 0x3F, 0x37, 0x37, 0x4B, 0x3F, 0x3F, 0x57, 0x47, 0x47, 0x67,
	0x4F, 0x4F, 0x73, 0x5B, 0x5B, 0x7F, 0x63, 0x63, 0x8B, 0x6B, 0x6B, 0x97,
	0x73, 0x73, 0xA3, 0x7B, 0x7B, 0xAF, 0x83, 0x83, 0xBB, 0x8B, 0x8B, 0xCB,
	0x00, 0x00, 0x00, 0x07, 0x07, 0x00, 0x0B, 0x0B, 0x00, 0x13, 0x13, 0x00,
	0x1B, 0x1B, 0x00, 0x23, 0x23, 0x00, 0x2B, 0x2B, 0x07, 0x2F, 0x2F, 0x07,
	0x37, 0x37, 0x07, 0x3F, 0x3F, 0x07, 0x47, 0x47, 0x07, 0x4B, 0x4B, 0x0B,
	0x53, 0x53, 0x0B, 0x5B, 0x5B, 0x0B, 0x63, 0x63, 0x0B, 0x6B, 0x6B, 0x0F,
	0x07, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x17, 0x00, 0x00, 0x1F, 0x00, 0x00,
	0x27, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x37, 0x00, 0x00, 0x3F, 0x00, 0x00,
	0x47, 0x00, 0x00, 0x4F, 0x00, 0x00, 0x57, 0x00, 0x00, 0x5F, 0x00, 0x00,
	0x67, 0x00, 0x00, 0x6F, 0x00, 0x00, 0x77, 0x00, 0x00, 0x7F, 0x00, 0x00,
	0x13, 0x13, 0x00, 0x1B, 0x1B, 0x00, 0x23, 0x23, 0x00, 0x2F, 0x2B, 0x00,
	0x37, 0x2F, 0x00, 0x43, 0x37, 0x00, 0x4B, 0x3B, 0x07, 0x57, 0x43, 0x07,
	0x5F, 0x47, 0x07, 0x6B, 0x4B, 0x0B, 0x77, 0x53, 0x0F, 0x83, 0x57, 0x13,
	0x8B, 0x5B, 0x13, 0x97, 0x5F, 0x1B, 0xA3, 0x63, 0x1F, 0xAF, 0x67, 0x23,
	0x23, 0x13, 0x07, 0x2F, 0x17, 0x0B, 0x3B, 0x1F, 0x0F, 0x4B, 0x23, 0x13,
	0x57, 0x2B, 0x17, 0x63, 0x2F, 0x1F, 0x73, 0x37, 0x23, 0x7F, 0x3B, 0x2B,
	0x8F, 0x43, 0x33, 0x9F, 0x4F, 0x33, 0xAF, 0x63, 0x2F, 0xBF, 0x77, 0x2F,
	0xCF, 0x8F, 0x2B, 0xDF, 0xAB, 0x27, 0xEF, 0xCB, 0x1F, 0xFF, 0xF3, 0x1B,
	0x0B, 0x07, 0x00, 0x1B, 0x13, 0x00, 0x2B, 0x23, 0x0F, 0x37, 0x2B, 0x13,
	0x47, 0x33, 0x1B, 0x53, 0x37, 0x23, 0x63, 0x3F, 0x2B, 0x6F, 0x47, 0x33,
	0x7F, 0x53, 0x3F, 0x8B, 0x5F, 0x47, 0x9B, 0x6B, 0x53, 0xA7, 0x7B, 0x5F,
	0xB7, 0x87, 0x6B, 0xC3, 0x93, 0x7B, 0xD3, 0xA3, 0x8B, 0xE3, 0xB3, 0x97,
	0xAB, 0x8B, 0xA3, 0x9F, 0x7F, 0x97, 0x93, 0x73, 0x87, 0x8B, 0x67, 0x7B,
	0x7F, 0x5B, 0x6F, 0x77, 0x53, 0x63, 0x6B, 0x4B, 0x57, 0x5F, 0x3F, 0x4B,
	0x57, 0x37, 0x43, 0x4B, 0x2F, 0x37, 0x43, 0x27, 0x2F, 0x37, 0x1F, 0x23,
	0x2B, 0x17, 0x1B, 0x23, 0x13, 0x13, 0x17, 0x0B, 0x0B, 0x0F, 0x07, 0x07,
	0xBB, 0x73, 0x9F, 0xAF, 0x6B, 0x8F, 0xA3, 0x5F, 0x83, 0x97, 0x57, 0x77,
	0x8B, 0x4F, 0x6B, 0x7F, 0x4B, 0x5F, 0x73, 0x43, 0x53, 0x6B, 0x3B, 0x4B,
	0x5F, 0x33, 0x3F, 0x53, 0x2B, 0x37, 0x47, 0x23, 0x2B, 0x3B, 0x1F, 0x23,
	0x2F, 0x17, 0x1B, 0x23, 0x13, 0x13, 0x17, 0x0B, 0x0B, 0x0F, 0x07, 0x07,
	0xDB, 0xC3, 0xBB, 0xCB, 0xB3, 0xA7, 0xBF, 0xA3, 0x9B, 0xAF, 0x97, 0x8B,
	0xA3, 0x87, 0x7B, 0x97, 0x7B, 0x6F, 0x87, 0x6F, 0x5F, 0x7B, 0x63, 0x53,
	0x6B, 0x57, 0x47, 0x5F, 0x4B, 0x3B, 0x53, 0x3F, 0x33, 0x43, 0x33, 0x27,
	0x37, 0x2B, 0x1F, 0x27, 0x1F, 0x17, 0x1B, 0x13, 0x0F, 0x0F, 0x0B, 0x07,
	0x6F, 0x83, 0x7B, 0x67, 0x7B, 0x6F, 0x5F, 0x73, 0x67, 0x57, 0x6B, 0x5F,
	0x4F, 0x63, 0x57, 0x47, 0x5B, 0x4F, 0x3F, 0x53, 0x47, 0x37, 0x4B, 0x3F,
	0x2F, 0x43, 0x37, 0x2B, 0x3B, 0x2F, 0x23, 0x33, 0x27, 0x1F, 0x2B, 0x1F,
	0x17, 0x23, 0x17, 0x0F, 0x1B, 0x13, 0x0B, 0x13, 0x0B, 0x07, 0x0B, 0x07,
	0xFF, 0xF3, 0x1B, 0xEF, 0xDF, 0x17, 0xDB, 0xCB, 0x13, 0xCB, 0xB7, 0x0F,
	0xBB, 0xA7, 0x0F, 0xAB, 0x97, 0x0B, 0x9B, 0x83, 0x07, 0x8B, 0x73, 0x07,
	0x7B, 0x63, 0x07, 0x6B, 0x53, 0x00, 0x5B, 0x47, 0x00, 0x4B, 0x37, 0x00,
	0x3B, 0x2B, 0x00, 0x2B, 0x1F, 0x00, 0x1B, 0x0F, 0x00, 0x0B, 0x07, 0x00,
	0x00, 0x00, 0xFF, 0x0B, 0x0B, 0xEF, 0x13, 0x13, 0xDF, 0x1B, 0x1B, 0xCF,
	0x23, 0x23, 0xBF, 0x2B, 0x2B, 0xAF, 0x2F, 0x2F, 0x9F, 0x2F, 0x2F, 0x8F,
	0x2F, 0x2F, 0x7F, 0x2F, 0x2F, 0x6F, 0x2F, 0x2F, 0x5F, 0x2B, 0x2B, 0x4F,
	0x23, 0x23, 0x3F, 0x1B, 0x1B, 0x2F, 0x13, 0x13, 0x1F, 0x0B, 0x0B, 0x0F,
	0x2B, 0x00, 0x00, 0x3B, 0x00, 0x00, 0x4B, 0x07, 0x00, 0x5F, 0x07, 0x00,
	0x6F, 0x0F, 0x00, 0x7F, 0x17, 0x07, 0x93, 0x1F, 0x07, 0xA3, 0x27, 0x0B,
	0xB7, 0x33, 0x0F, 0xC3, 0x4B, 0x1B, 0xCF, 0x63, 0x2B, 0xDB, 0x7F, 0x3B,
	0xE3, 0x97, 0x4F, 0xE7, 0xAB, 0x5F, 0xEF, 0xBF, 0x77, 0xF7, 0xD3, 0x8B,
	0xA7, 0x7B, 0x3B, 0xB7, 0x9B, 0x37, 0xC7, 0xC3, 0x37, 0xE7, 0xE3, 0x57,
	0x7F, 0xBF, 0xFF, 0xAB, 0xE7, 0xFF, 0xD7, 0xFF, 0xFF, 0x67, 0x00, 0x00,
	0x8B, 0x00, 0x00, 0xB3, 0x00, 0x00, 0xD7, 0x00, 0x00, 0xFF, 0x00, 0x00,
	0xFF, 0xF3, 0x93, 0xFF, 0xF7, 0xC7, 0xFF, 0xFF, 0xFF, 0x9F, 0x5B, 0x53
};

std::unique_ptr<ModelData> LoadMDL(QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::ExistingOnly))
        throw std::runtime_error("bad");

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
	stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

	mdl_t header;
	stream.readRawData(reinterpret_cast<char *>(&header), sizeof(header));

	ModelData data;

	auto &mesh = data.meshes.emplace_back();

	int32_t group_id = 0;

	auto parseSingleSkin = [&stream, &header](ModelSkin &skin) {
		skin.width = header.skinwidth;
		skin.height = header.skinheight;

		SkinPaletteData skindata;

		skindata.data.resize(header.skinwidth * header.skinheight);
		stream.readRawData(reinterpret_cast<char *>(skindata.data.data()), skindata.data.size());
		skindata.palette = std::vector<uint8_t>(&quakePalette[0], &quakePalette[sizeof(quakePalette)]);

		skin.raw_data = std::move(skindata);
	};

	// skins may grow with groups
	data.skins.reserve(header.numskins);

	for (int i = 0; i < header.numskins; i++)
	{
		daliasskintype_t type;
		stream >> (int32_t &) type.type;

		if (type.type == ALIAS_SKIN_SINGLE)
		{
			auto &outSkin = data.skins.emplace_back();
			parseSingleSkin(outSkin);
		}
		else
		{
			daliasskingroup_t group;
			stream >> group.numskins;

			size_t frame_start = data.skins.size();
			
			for (int f = 0; f < group.numskins; f++)
			{
				auto &outskin = data.skins.emplace_back();

				daliasskininterval_t interval;
				stream >> interval.interval;

				outskin.q1_data = { group_id, interval.interval };
			}

			group_id++;
			
			for (int f = 0; f < group.numskins; f++)
			{
				auto &outskin = data.skins[frame_start + f];
				parseSingleSkin(outskin);
			}
		}
	}

	std::vector<stvert_t> stverts;
	stverts.resize(header.numverts);
	
	mesh.texcoords.resize(header.numverts);
	mesh.vertices.resize(header.numverts);

	for (int i = 0; i < header.numverts; i++)
	{
		auto &st = stverts[i];
		stream >> st.onseam;
		stream >> st.s;
		stream >> st.t;

		auto &v = mesh.texcoords[i];
		v.pos = { (float) st.s / header.skinwidth, (float) st.t / header.skinheight };
	}

	mesh.triangles.resize(header.numtris);

	for (int i = 0; i < header.numtris; i++)
	{
		dmdltriangle_t t;
		stream >> t.facesfront;
		stream >> t.vertindex[0] >> t.vertindex[1] >> t.vertindex[2];

		auto &tri = mesh.triangles[i];
		tri.vertices = { (uint32_t) t.vertindex[0], (uint32_t) t.vertindex[1], (uint32_t) t.vertindex[2] };

		if (!t.facesfront)
		{
			for (int x = 0; x < 3; x++)
			{
				if (stverts[tri.vertices[x]].onseam)
				{
					tri.texcoords[x] = mesh.texcoords.size();
					mesh.texcoords.emplace_back(
						QVector2D { ((float) stverts[tri.vertices[x]].s / header.skinwidth) + 0.5f, (float) stverts[tri.vertices[x]].t / header.skinheight }
					);
				}
				else
					tri.texcoords[x] = tri.vertices[x];
			}
		}
		else
			tri.texcoords = tri.vertices;
	}

	// framegroups can add more, technically
	data.frames.reserve(header.numframes);

	group_id = 0;

	auto parseSingleFrame = [&stream, &header](ModelFrame &outframe, MeshFrame &meshframe) {
		dmdlaliasframe_t frame;
		stream >> frame.bboxmin.v[0] >> frame.bboxmin.v[1] >> frame.bboxmin.v[2];
		stream >> frame.bboxmin.lightnormalindex;
		stream >> frame.bboxmax.v[0] >> frame.bboxmax.v[1] >> frame.bboxmax.v[2];
		stream >> frame.bboxmax.lightnormalindex;

		stream.readRawData(frame.name, sizeof(frame.name));
		frame.name[sizeof(frame.name) - 1] = 0;

		outframe.name = frame.name;
		meshframe.vertices.resize(header.numverts);

		for (int x = 0; x < header.numverts; x++)
		{
			dtrivertx_t v;
			stream >> v.v[0] >> v.v[1] >> v.v[2];
			stream >> v.lightnormalindex;

			auto &vert = meshframe.vertices[x];

			vert.position = {
				(v.v[0] * header.scale[0]) + header.scale_origin[0],
				(v.v[1] * header.scale[1]) + header.scale_origin[1],
				(v.v[2] * header.scale[2]) + header.scale_origin[2]
			};
			vert.normal = anorms[v.lightnormalindex];
		}
	};

	for (int i = 0; i < header.numframes; i++)
	{
		aliasframetype_t type;
		stream >> (int32_t &) type;

		if (type == ALIAS_SINGLE)
		{
			auto &outframe = data.frames.emplace_back();
			auto &meshframe = mesh.frames.emplace_back();
			parseSingleFrame(outframe, meshframe);
		}
		else
		{
			daliasgroup_t group;
			stream >> group.numframes;
			stream >> group.bboxmin.v[0] >> group.bboxmin.v[1] >> group.bboxmin.v[2];
			stream >> group.bboxmin.lightnormalindex;
			stream >> group.bboxmax.v[0] >> group.bboxmax.v[1] >> group.bboxmax.v[2];
			stream >> group.bboxmax.lightnormalindex;

			size_t frame_start = data.frames.size();
			
			for (int f = 0; f < group.numframes; f++)
			{
				auto &outframe = data.frames.emplace_back();
				auto &meshframe = mesh.frames.emplace_back();

				daliasinterval_t interval;
				stream >> interval.interval;

				outframe.q1_data = { group_id, interval.interval };
			}

			group_id++;
			
			for (int f = 0; f < group.numframes; f++)
			{
				auto &outframe = data.frames[frame_start + f];
				auto &meshframe = mesh.frames[frame_start + f];
				parseSingleFrame(outframe, meshframe);
			}
		}
	}
	
	return std::make_unique<ModelData>(std::move(data));
}


/*
========================================================================

.MD3 triangle model file format

========================================================================
*/

constexpr size_t MAX_QPATH = 64;
constexpr int MD3_IDENT			= (('3'<<24)+('P'<<16)+('D'<<8)+'I');
constexpr int MD3_VERSION		= 15;

// limits
constexpr size_t MD3_MAX_LODS		= 4;
constexpr size_t MD3_MAX_TRIANGLES	= 8192;	// per surface
constexpr size_t MD3_MAX_VERTS		= 4096;	// per surface
constexpr size_t MD3_MAX_SHADERS	= 256;	// per surface
constexpr size_t MD3_MAX_FRAMES		= 1024;	// per model
constexpr size_t MD3_MAX_SURFACES	= 32;	// per model
constexpr size_t MD3_MAX_TAGS		= 16;	// per frame

// vertex scales
constexpr float MD3_XYZ_SCALE		= (1.0/64);

struct md3Frame_t {
	vec3_t		bounds[2];
	vec3_t		localOrigin;
	float		radius;
	char		name[16];
};

struct md3Tag_t {
	char		name[MAX_QPATH];	// tag name
	vec3_t		origin;
	vec3_t		axis[3];
};

/*
** md3Surface_t
**
** CHUNK			SIZE
** header			sizeof( md3Surface_t )
** shaders			sizeof( md3Shader_t ) * numShaders
** triangles[0]		sizeof( md3Triangle_t ) * numTriangles
** st				sizeof( md3St_t ) * numVerts
** XyzNormals		sizeof( md3XyzNormal_t ) * numVerts * numFrames
*/
struct md3Surface_t {
	int		ident;				// 

	char	name[MAX_QPATH];	// polyset name

	int		flags;
	int		numFrames;			// all surfaces in a model should have the same

	int		numShaders;			// all surfaces in a model should have the same
	int		numVerts;

	int		numTriangles;
	int		ofsTriangles;

	int		ofsShaders;			// offset from start of md3Surface_t
	int		ofsSt;				// texture coords are common for all frames
	int		ofsXyzNormals;		// numVerts * numFrames

	int		ofsEnd;				// next surface follows
};

struct md3Shader_t {
	char			name[MAX_QPATH];
	int				shaderIndex;	// for in-game use
};

struct md3Triangle_t {
	int			indexes[3];
};

struct md3St_t {
	float		st[2];
};

struct md3XyzNormal_t {
	short		xyz[3];
	short		normal;
};

struct md3Header_t {
	int			ident;
	int			version;

	char		name[MAX_QPATH];	// model name

	int			flags;

	int			numFrames;
	int			numTags;			
	int			numSurfaces;

	int			numSkins;

	int			ofsFrames;			// offset for first frame
	int			ofsTags;			// numFrames * numTags
	int			ofsSurfaces;		// first surface, others follow

	int			ofsEnd;				// end of file
};

constexpr double Q_PI	= 3.14159265358979323846;

std::unique_ptr<ModelData> LoadMD3(QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::ExistingOnly))
        throw std::runtime_error("bad");

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
	stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

	md3Header_t header;
	stream.readRawData(reinterpret_cast<char *>(&header), sizeof(header));

	ModelData data;

	data.frames.resize(header.numFrames);

	stream.device()->seek(header.ofsFrames);

	for (auto &frame : data.frames)
	{
		md3Frame_t inframe;
		stream.readRawData(reinterpret_cast<char *>(&inframe), sizeof(inframe));
		inframe.name[sizeof(inframe.name) - 1] = '\0';

		frame.name = inframe.name;
	}

	stream.device()->seek(header.ofsSurfaces);

	data.meshes.resize(header.numSurfaces);

	std::unordered_map<std::string, int32_t> shaders_by_name;
	QDir model_dir = QFileInfo(filename).dir();

	for (auto &mesh : data.meshes)
	{
		qint64 surface_start = stream.device()->pos();

		md3Surface_t insurface;
		stream.readRawData(reinterpret_cast<char *>(&insurface), sizeof(insurface));
		insurface.name[sizeof(insurface.name) - 1] = '\0';

		mesh.name = insurface.name;

		mesh.frames.resize(insurface.numFrames);
		mesh.texcoords.resize(insurface.numVerts);
		mesh.vertices.resize(insurface.numVerts);
		mesh.triangles.resize(insurface.numTriangles);

		stream.device()->seek(surface_start + insurface.ofsTriangles);

		for (auto &triangle : mesh.triangles)
		{
			md3Triangle_t intri;
			stream.readRawData(reinterpret_cast<char *>(&intri), sizeof(intri));

			triangle.texcoords = triangle.vertices = { (uint32_t) intri.indexes[0], (uint32_t) intri.indexes[1], (uint32_t) intri.indexes[2] };
		}

		stream.device()->seek(surface_start + insurface.ofsSt);

		for (auto &st : mesh.texcoords)
		{
			md3St_t inst;
			stream.readRawData(reinterpret_cast<char *>(&inst), sizeof(inst));

			st.pos = { inst.st[0], inst.st[1] };
		}

		stream.device()->seek(surface_start + insurface.ofsXyzNormals);

		for (auto &frame : mesh.frames)
		{
			frame.vertices.resize(insurface.numVerts);

			for (auto &vertex : frame.vertices)
			{
				md3XyzNormal_t invertex;
				stream.readRawData(reinterpret_cast<char *>(&invertex), sizeof(invertex));
				
				vertex.position[0] = invertex.xyz[0] * MD3_XYZ_SCALE;
				vertex.position[1] = invertex.xyz[1] * MD3_XYZ_SCALE;
				vertex.position[2] = invertex.xyz[2] * MD3_XYZ_SCALE;

				// decode the lat/lng normal to a 3 float normal
				float lat = ( invertex.normal >> 8 ) & 0xff;
				float lng = ( invertex.normal & 0xff );
				lat *= Q_PI/128;
				lng *= Q_PI/128;

				vertex.normal[0] = cos(lat) * sin(lng);
				vertex.normal[1] = sin(lat) * sin(lng);
				vertex.normal[2] = cos(lng);
			}
		}

		stream.device()->seek(surface_start + insurface.ofsShaders);

		md3Shader_t inshader;
		stream.readRawData(reinterpret_cast<char *>(&inshader), sizeof(inshader));
		inshader.name[sizeof(inshader.name) - 1] = '\0';

		if (auto it = shaders_by_name.find(inshader.name); it != shaders_by_name.end())
			mesh.assigned_skin = it->second;
		else
		{
			mesh.assigned_skin = (int32_t) data.skins.size();
			auto &skin = data.skins.emplace_back();
			skin.width = skin.height = 64;
			skin.name = inshader.name;
			shaders_by_name.insert_or_assign(skin.name, mesh.assigned_skin.value());

			if (auto skin_file = FindSkinFile(model_dir, inshader.name, { "tga", "png", "jpg", "jpeg", "pcx" }))
			{	
				if (!skin.image.load(skin_file->filePath()))
					skin.image.load("res/missing.png");
			}
			else
				skin.image.load("res/missing.png");

			if (skin.image.width())
			{
				skin.image.convertTo(QImage::Format_ARGB32);
				skin.width = skin.image.width();
				skin.height = skin.image.height();
			}
		}

		stream.device()->seek(surface_start + insurface.ofsEnd);
	}

	return std::make_unique<ModelData>(std::move(data));
}

/*static*/ std::unique_ptr<ModelData> ModelLoader::load(QFileInfo file, QMimeType type)
{
	std::unique_ptr<ModelData> model;

	if (type.name() == "x-qtmdl/md2")
	    model = LoadMD2(file.filePath());
	else if (type.name() == "x-qtmdl/md2f")
	    model = LoadMD2F(file.filePath());
	else if (type.name() == "x-qtmdl/mdl")
	    model = LoadMDL(file.filePath());
	else if (type.name() == "x-qtmdl/qim")
	    model = LoadQIM(file.filePath());
	else if (type.name() == "x-qtmdl/md3")
	    model = LoadMD3(file.filePath());
	else
		throw std::runtime_error("invalid file type");

	// post-load operations
	ModelMutator mutator{model.get()};
	if (!model->skins.empty())
	{
		mutator.setSelectedSkin(0);

		// load skin pixels
		// TODO: this should be moved to GL area I think
		for (auto &skin : model->skins)
		{
			// already loaded
			if (skin.image.width())
				continue;
			// nothing to load
			else if (!skin.raw_data)
				continue;

			skin.image = QImage(skin.width, skin.height, QImage::Format_ARGB32);
			byte *bits = skin.image.bits();

			for (size_t i = 0; i < skin.width * skin.height; i++, bits += 4)
			{
				bits[2] = skin.raw_data->palette->at((skin.raw_data->data[i] * 3) + 0);
				bits[1] = skin.raw_data->palette->at((skin.raw_data->data[i] * 3) + 1);
				bits[0] = skin.raw_data->palette->at((skin.raw_data->data[i] * 3) + 2);
				bits[3] = 0xFF;
			}
		}
	}

	return model;
}

/*static*/ void ModelLoader::save(const ModelData &model, QFileInfo file, QMimeType type)
{
	if (type.name() == "x-qtmdl/md2")
		SaveMD2(model, file.filePath());
	else if (type.name() == "x-qtmdl/qim")
		SaveQIM(model, file.filePath());
	else
		throw std::runtime_error("invalid file type");
}