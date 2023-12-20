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
#include <limits>
#include <utility>

struct BoundingBox
{
    QVector3D   mins, maxs;

    constexpr BoundingBox(float size) noexcept
    {
        float half = size * 0.5f;
        mins = { -half, -half, -half };
        maxs = { half, half, half };
    }

    constexpr BoundingBox() noexcept :
        mins(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()),
        maxs(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity())
    {
    }

    constexpr void add(const QVector3D &pt) noexcept
    {
        for (int i = 0; i < 3; i++)
        {
            mins[i] = std::min(mins[i], pt[i]);
            maxs[i] = std::max(maxs[i], pt[i]);
        }
    }

    constexpr QVector3D centroid() const noexcept
    {
        return (maxs + mins) * 0.5f;
    }

    inline bool empty() const noexcept
    {
        return std::isinf(mins[0]);
    }
};

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

    inline BoundingBox bounds() const
    {
        BoundingBox bounds;

        for (auto &vert : vertices)
            bounds.add(vert.position);

        if (bounds.empty())
            return BoundingBox(0);

        return bounds;
    }
};

// for 8-bit skins, palettes might be embedded in them.
struct SkinPaletteData
{
    std::optional<std::vector<uint8_t>> palette;
    std::vector<uint8_t>                data;
};

struct ModelSkin
{
    std::string                    name;
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

// model data contains all of the data 'saved' with
// a model. It can not be mutated directly.
struct ModelData
{
private:
    // no copy constructor, to prevent errors
    ModelData(const ModelData &) = delete;

public:
    ModelData(ModelData &&) = default;

    // model data
    // nb: a model will always have at least one frame.
    std::vector<ModelFrame>     frames;
    // these can all be empty for a valid model
    std::vector<ModelTexCoord>  texcoords;
    std::vector<ModelTriangle>  triangles;
    std::vector<ModelSkin>      skins;
    std::vector<ModelVertex>    vertices;

    // state data
    int32_t                 selectedFrame = 0;
    // an unselected skin is technically valid
    // but will only occur if the model has zero skins.
    std::optional<int32_t>  selectedSkin = std::nullopt;

    // return a fixed set of texture coordinate indices that are
    // currently considered "selected" - that is to say, they will
    // be adjusted if an operation occurs.
    // FIXME in the future this should be cached state
    const std::unordered_set<size_t> &getSelectedTextureCoordinates(UVSelectMode mode) const
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
    const std::vector<QVector2D> &transformTexcoords(const QMatrix4x4 &matrix, UVSelectMode mode) const
    {
        static std::vector<QVector2D> coordinates;
        coordinates.resize(texcoords.size());

        const ModelSkin *skin = getSelectedSkin();

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

    constexpr const ModelSkin *getSelectedSkin() const
    {
        if (!selectedSkin.has_value())
            return nullptr;
        return &skins[selectedSkin.value()];
    }

    inline BoundingBox boundsOfAllFrames() const
    {
        BoundingBox bounds;

        for (auto &frame : frames)
            for (auto &vert : frame.vertices)
                bounds.add(vert.position);

        if (bounds.empty())
            return BoundingBox(0);

        return bounds;
    }

    constexpr ModelData() :
        frames({ { "Frame 1" } })
    {
    }
};

class ModelMutator
{
    ModelData   *data;

public:
    constexpr ModelMutator(ModelData *data = nullptr) :
        data(data)
    {
    }

    constexpr bool isValid() const { return !!data; }

    constexpr void setSelectedFrame(int32_t frame)
    {
        data->selectedFrame = frame;
    }

    constexpr void setSelectedSkin(std::optional<int32_t> skin)
    {
        data->selectedSkin = skin;
    }

    constexpr void setNextSkin()
    {
        data->selectedSkin = std::min((int) data->skins.size() - 1, data->selectedSkin.value_or(0) + 1);
    }

    constexpr void setPreviousSkin()
    {
        data->selectedSkin = std::max(0, data->selectedSkin.value_or(0) - 1);
    }

    void selectRectangleVerticesUV(const QRectF &rect, Qt::KeyboardModifiers modifiers)
    {
        for (auto &tc : data->texcoords)
        {
            if (!rect.contains(tc.pos.toPointF()))
                continue;

            if (modifiers & Qt::KeyboardModifier::AltModifier)
                tc.selected = false;
            else
                tc.selected = true;
        }
    }

    void selectRectangleTrianglesUV(const QRectF &rect, Qt::KeyboardModifiers modifiers)
    {
        for (auto &tri : data->triangles)
        {
            for (auto &tci : tri.texcoords)
            {
                auto &tc = data->texcoords[tci];

                if (!rect.contains(tc.pos.toPointF()))
                    continue;

                if (modifiers & Qt::KeyboardModifier::AltModifier)
                    tri.selectedUV = false;
                else
                    tri.selectedUV = true;
            }
        }
    }

    void selectAllVerticesUV()
    {
        for (auto &v : data->texcoords)
            v.selected = true;
    }

    void selectAllTrianglesUV()
    {
        for (auto &t : data->triangles)
            t.selectedUV = true;
    }

    void selectNoneVerticesUV()
    {
        for (auto &v : data->texcoords)
            v.selected = false;
    }

    void selectNoneTrianglesUV()
    {
        for (auto &t : data->triangles)
            t.selectedUV = false;
    }

    void selectInverseVerticesUV()
    {
        for (auto &v : data->texcoords)
            v.selected = !v.selected;
    }

    void selectInverseTrianglesUV()
    {
        for (auto &t : data->triangles)
            t.selectedUV = !t.selectedUV;
    }

    void selectTouchingVerticesUV()
    {
        std::unordered_set<size_t> selected;

        for (size_t i = 0; i < data->texcoords.size(); i++)
            if (data->texcoords[i].selected)
                selected.insert(i);

        for (auto &tri : data->triangles)
        {
            if (selected.contains(tri.texcoords[0]) ||
                selected.contains(tri.texcoords[1]) ||
                selected.contains(tri.texcoords[2]))
                for (auto &tc : tri.texcoords)
                    data->texcoords[tc].selected = true;
        }
    }

    void selectTouchingTrianglesUV()
    {
        std::unordered_set<size_t> selected;

        for (auto &tri : data->triangles)
            if (tri.selectedUV)
                for (auto &tc : tri.texcoords)
                    selected.insert(tc);

        for (auto &tri : data->triangles)
        {
            if (selected.contains(tri.texcoords[0]) ||
                selected.contains(tri.texcoords[1]) ||
                selected.contains(tri.texcoords[2]))
            {
                for (auto &tc : tri.texcoords)
                    tri.selectedUV = true;
            }
        }
    }

    // sync active UV selection to 3D
    void syncSelectionUV()
    {
        for (auto &tri : data->triangles)
            tri.selectedFace = tri.selectedUV;
    }

    // sync active 3D selection to UV
    void syncSelection3D()
    {
        for (auto &tri : data->triangles)
            tri.selectedUV = tri.selectedFace;
    }
};