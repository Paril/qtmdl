#include "modelloader.h"
#include <QDir>

using byte = uint8_t;

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
	std::array<float, 3> scale;	// multiply byte verts by this
	std::array<float, 3> translate;	// then add this
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

	for (auto &frame : data.frames)
		frame.vertices.resize(header.num_xyz);

	data.vertices.resize(header.num_xyz);

	file.seek(header.ofs_frames);

	for (auto &frame : data.frames)
	{
		daliasframe_t frame_header;
		stream >> frame_header.scale[0] >> frame_header.scale[1] >> frame_header.scale[2];
		stream >> frame_header.translate[0] >> frame_header.translate[1] >> frame_header.translate[2];
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
			v.lightnormalindex = 0;

			stream << v.v[0] << v.v[1] << v.v[2];
			stream << v.lightnormalindex;
		}
	}
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

	for (auto &frame : data.frames)
		frame.vertices.resize(header.num_xyz);

	data.vertices.resize(header.num_xyz);

	file.seek(header.ofs_frames);

	for (auto &frame : data.frames)
	{
		qint64 pos = file.pos();
		daliasframef_t frame_header;
		stream.readRawData(frame_header.name, sizeof(frame_header.name));
		frame_header.name[sizeof(frame_header.name) - 1] = '\0';

		frame.name = frame_header.name;

		for (auto &vert : frame.vertices)
		{
			dtrivertxf_t v;
			stream >> v.position[0] >> v.position[1] >> v.position[2];
			stream >> v.normal[0] >> v.normal[1] >> v.normal[2];

			vert.position = v.position;
			vert.normal = v.normal;
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
		char skin_path[MD2_MAX_SKINNAME];
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
	
	return std::make_unique<ModelData>(std::move(data));
}

/*static*/ std::unique_ptr<ModelData> ModelLoader::load(QFileInfo file, QMimeType type)
{
	std::unique_ptr<ModelData> model;

	if (type.name() == "x-qtmdl/md2")
	    model = LoadMD2(file.filePath());
	else if (type.name() == "x-qtmdl/md2f")
	    model = LoadMD2F(file.filePath());
	else
		throw std::runtime_error("invalid file type");

	// post-load operations
	ModelMutator mutator{model.get()};
	if (!model->skins.empty())
		mutator.setSelectedSkin(0);

	return model;
}

/*static*/ void ModelLoader::save(const ModelData &model, QFileInfo file, QMimeType type)
{
	if (type.name() == "x-qtmdl/md2")
		SaveMD2(model, file.filePath());
	else
		throw std::runtime_error("invalid file type");
}