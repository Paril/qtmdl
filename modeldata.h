#pragma once

#include <QVector2D>
#include <QVector3D>
#include <QImage>
#include <QMatrix4x4>
#include <array>
#include <vector>
#include <string>
#include <optional>
#include <unordered_set>
#include "editortypes.h"

struct ModelFrameVertex
{
    QVector3D   position;
    QVector3D   normal;
};

struct ModelTriangle
{
	std::array<uint32_t, 3>	vertices;
    bool                    selectedFace = false;
	std::array<uint32_t, 3>	texcoords;
    bool                    selectedUV = false;
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

    // return a fixed set of texture coordinate indices that are
    // currently considered "selected" - that is to say, they will
    // be adjusted if an operation occurs.
    // FIXME in the future this should be cached state
    const std::unordered_set<size_t> &getSelectedTextureCoordinates(UVSelectMode mode)
    {
        static std::unordered_set<size_t> verticesSelected;
        verticesSelected.clear();

        if (mode == UVSelectMode::Face)
        {
            for (auto &triangle : triangles)
            {
                if (triangle.selectedUV)
                    for (auto &tc : triangle.texcoords)
                        verticesSelected.insert(tc);
            }
        }
        else
        {
            for (size_t i = 0; i < texcoords.size(); i++)
                if (texcoords[i].selected)
                    verticesSelected.insert(i);
        }

        return verticesSelected;
    }

    // return a fixed array of texcoord positions
    // that match the given transformation. it only modifies
    // coordinates that are actually changed by the matrix.
    // FIXME in the future this should be cached state
    const std::vector<QVector2D> &transformTexcoords(const QMatrix4x4 &matrix, UVSelectMode mode)
    {
        static std::vector<QVector2D> coordinates;
        coordinates.resize(texcoords.size());

        ModelSkin *skin = getSelectedSkin();

        if (!skin)
            return coordinates;

        const auto &verticesSelected = getSelectedTextureCoordinates(mode);

        QVector2D scale { (float) skin->width, (float) skin->height };

        for (size_t i = 0; i < texcoords.size(); i++)
        {
            coordinates[i] = texcoords[i].pos;

            if (!matrix.isIdentity() && verticesSelected.contains(i))
            {
                coordinates[i] *= scale;
                coordinates[i] = matrix.map(coordinates[i].toVector3D()).toVector2D();
                coordinates[i] /= scale;
            }
        }

        return coordinates;
    }

    constexpr ModelSkin *getSelectedSkin()
    {
        if (!selectedSkin.has_value())
            return nullptr;
        return &skins[selectedSkin.value()];
    }

    constexpr ModelData() :
        frames({ { "Frame 1" } })
    {
    }
};