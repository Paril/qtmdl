#pragma once

#include <QVector2D>
#include <QVector3D>
#include <array>
#include <vector>
#include <string>
#include <optional>
#include <QImage>

struct ModelFrameVertex
{
    QVector3D   position;
    QVector3D   normal;
};

struct ModelTriangle
{
	std::array<uint32_t, 3>	vertices;
	std::array<uint32_t, 3>	texcoords;
    bool                    selected = false;
};

struct ModelFrame
{
    std::string					  name;
    std::vector<ModelFrameVertex> vertices;
};

// for 8-bit skins, palettes might be embedded in them.
struct SkinPaletteData
{
    std::optional<std::vector<uint8_t>> palette;
    std::vector<uint8_t>                data;
};

struct ModelSkin
{
    int                            width, height;

    // if we have an 8-bit skin, the palette + raw data
    // is stored here.
    std::optional<SkinPaletteData> raw_data;
    // "cooked" 32-bit image
    QImage                         image;
};

struct ModelVertex
{
    bool selected = false;
};

struct ModelTexCoord
{
    QVector2D pos;
    bool      selected = false;
};

// model data contains all of the data that is
// mutable and stored in undo/redo, as well as
// passed to scripts.
struct ModelData
{
    // model data
    // nb: a model will always have at least one frame.
    std::vector<ModelFrame>     frames;
    // these can all be empty for a valid model
    std::vector<ModelTexCoord>  texcoords;
    std::vector<ModelTriangle>  triangles;
    std::vector<ModelSkin>      skins;
    std::vector<ModelVertex>    vertices;

    // state data
    int                 selectedFrame = 0;
    // an unselected skin is technically valid
    // but will only occur if the model has zero skins.
    std::optional<int>  selectedSkin = std::nullopt;

    constexpr ModelData() :
        frames({ { "Frame 1" } })
    {
    }
};