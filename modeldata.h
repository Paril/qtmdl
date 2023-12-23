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
#include <QDataStream>

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

constexpr int32_t QIM_VERSION = 1;

template<typename T, typename S>
concept IsSyncable = requires(S &s, T t)
{
    t.sync(s);
};

template<typename T>
concept IsQDataStreamCompatible = requires(QDataStream &s, T t)
{
    s << t;
    s >> t;
};

struct QDataSync
{
    int32_t version;
    QDataStream &stream;
    bool read;

    inline QDataSync(int32_t version, QDataStream &stream, bool read) :
        version(version),
        stream(stream),
        read(read)
    {
    }

    template<typename ...T>
    inline QDataSync &sync(T&... args)
    {
        (syncInternal(args), ... );
        return *this;
    }

private:
    template<IsSyncable<QDataSync> T>
    inline void syncInternal(T &arg)
    {
        arg.sync(*this);
    }

    template<IsQDataStreamCompatible T>
    inline void syncInternal(T &arg)
    {
        if (!read)
            stream << arg;
        else
            stream >> arg;
    }

    inline void syncInternal(std::string &str)
    {
        if (!read)
        {
            stream << str.size();
            stream.writeRawData(str.data(), str.size());
        }
        else
        {
            size_t size;
            stream >> size;

            str.resize(size, '\0');
            stream.readRawData(str.data(), size);
        }
    }

    template<typename T, size_t N>
    inline void syncInternal(std::array<T, N> &array)
    {
        for (auto &v : array)
            syncInternal(v);
    }

    template<typename T>
    inline void syncInternal(std::optional<T> &opt)
    {
        if (!read)
        {
            stream << opt.has_value();

            if (opt.has_value())
                syncInternal(opt.value());
        }
        else
        {
            bool has_value;
            stream >> has_value;

            if (!has_value)
                opt = std::nullopt;
            else
            {
                T value;
                syncInternal(value);
                opt = value;
            }
        }
    }

    template<typename T>
    inline void syncInternal(std::vector<T> &vec)
    {
        if (!read)
        {
            stream << vec.size();

            for (auto &v : vec)
                syncInternal(v);
        }
        else
        {
            size_t n;
            stream >> n;
            vec.resize(n);

            for (auto &v : vec)
                syncInternal(v);
        }
    }
};

struct ModelTriangle
{
	std::array<uint32_t, 3>	vertices;
	std::array<uint32_t, 3>	texcoords;
    bool                    selectedFace = false;
    bool                    selectedUV = false;

    inline QDataSync &sync(QDataSync &stream)
    {
        stream.sync(vertices, texcoords, selectedFace, selectedUV);
        return stream;
    }
};

struct Q1GroupData
{
    int32_t     group;
    float       interval;

    inline QDataSync &sync(QDataSync &stream)
    {
        stream.sync(group, interval);
        return stream;
    }
};

struct ModelFrame
{
    std::string					  name;
    std::optional<Q1GroupData>    q1_data;

    inline QDataSync &sync(QDataSync &stream)
    {
        stream.sync(name, q1_data);
        return stream;
    }
};

// for 8-bit skins, palettes might be embedded in them.
struct SkinPaletteData
{
    std::optional<std::vector<uint8_t>> palette;
    std::vector<uint8_t>                data;

    inline QDataSync &sync(QDataSync &stream)
    {
        stream.sync(palette, data);
        return stream;
    }
};

struct ModelSkin
{
    std::string                    name;
    int32_t                        width, height;

    // if we have an 8-bit skin, the palette + raw data
    // is stored here.
    std::optional<SkinPaletteData> raw_data;
    // "cooked" 32-bit image
    QImage                         image;

    std::optional<Q1GroupData>     q1_data;

    inline QDataSync &sync(QDataSync &stream)
    {
        stream.sync(name, width, height, raw_data, image, q1_data);
        return stream;
    }
};

struct ModelVertex
{
    bool selected = false;
    // if set, we're a tag; tags cannot be a part
    // of any triangle.
    std::optional<QQuaternion> quat = std::nullopt;

    inline QDataSync &sync(QDataSync &stream)
    {
        stream.sync(selected, quat);
        return stream;
    }
};

struct ModelTexCoord
{
    QVector2D pos;
    bool      selected = false;

    inline QDataSync &sync(QDataSync &stream)
    {
        stream.sync(pos, selected);
        return stream;
    }
};

struct MeshFrameVertex
{
    QVector3D position;
    QVector3D normal;

    inline QDataSync &sync(QDataSync &stream)
    {
        stream.sync(position, normal);
        return stream;
    }
};

struct MeshFrame
{
    std::vector<MeshFrameVertex> vertices;

    inline QDataSync &sync(QDataSync &stream)
    {
        stream.sync(vertices);
        return stream;
    }

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

struct ModelMesh
{
    // these can all be empty for a valid model
    std::vector<ModelTexCoord>  texcoords;
    std::vector<ModelTriangle>  triangles;
    std::vector<ModelVertex>    vertices;
    std::vector<MeshFrame>      frames;
    // assigned skin for this model; if set, it will
    // always use this texture and not the selected skin.
    std::optional<int32_t>      assigned_skin = std::nullopt;
    std::string				    name;

    inline QDataSync &sync(QDataSync &stream)
    {
        stream.sync(texcoords, triangles, vertices, frames, assigned_skin, name);
        return stream;
    }

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
    const std::vector<QVector2D> &transformTexcoords(int32_t width, int32_t height, const QMatrix4x4 &matrix, UVSelectMode mode) const
    {
        static std::vector<QVector2D> coordinates;
        coordinates.resize(texcoords.size());

        const auto &verticesSelected = getSelectedTextureCoordinates(mode);

        QVector2D scale { (float) width, (float) height };

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
    ModelData() = default;

    // model data
    // nb: a model will always have at least one frame.
    std::vector<ModelFrame>     frames;
    // nb: a model will always have at least one mesh.
    std::vector<ModelMesh>      meshes;
    std::vector<ModelSkin>      skins;

    // state data
    // selected animation frame
    int32_t                 selectedFrame = 0;
    // an unselected skin is valid
    std::optional<int32_t>  selectedSkin = std::nullopt;
    // if true, each mesh is assigned a matching skin
    // instead of all meshes using the same current skin.
    bool                    skinPerObject = false;

    inline QDataSync &sync(QDataSync &stream)
    {
        stream.sync(frames, meshes, skins, selectedFrame, selectedSkin, skinPerObject);
        return stream;
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

        for (auto &mesh : meshes)
            for (auto &frame : mesh.frames)
                for (auto &vert : frame.vertices)
                    bounds.add(vert.position);

        if (bounds.empty())
            return BoundingBox(0);

        return bounds;
    }

    static const ModelData blankModel;
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

    void selectRectangleVerticesUV(int32_t mesh, const QRectF &rect, Qt::KeyboardModifiers modifiers)
    {
        for (auto &tc : data->meshes[mesh].texcoords)
        {
            if (!rect.contains(tc.pos.toPointF()))
                continue;

            if (modifiers & Qt::KeyboardModifier::AltModifier)
                tc.selected = false;
            else
                tc.selected = true;
        }
    }

    void selectRectangleTrianglesUV(int32_t mesh, const QRectF &rect, Qt::KeyboardModifiers modifiers)
    {
        for (auto &tri : data->meshes[mesh].triangles)
        {
            for (auto &tci : tri.texcoords)
            {
                auto &tc = data->meshes[mesh].texcoords[tci];

                if (!rect.contains(tc.pos.toPointF()))
                    continue;

                if (modifiers & Qt::KeyboardModifier::AltModifier)
                    tri.selectedUV = false;
                else
                    tri.selectedUV = true;
            }
        }
    }

    void selectAllVerticesUV(int32_t mesh)
    {
        for (auto &v : data->meshes[mesh].texcoords)
            v.selected = true;
    }

    void selectAllTrianglesUV(int32_t mesh)
    {
        for (auto &t : data->meshes[mesh].triangles)
            t.selectedUV = true;
    }

    void selectNoneVerticesUV(int32_t mesh)
    {
        for (auto &v : data->meshes[mesh].texcoords)
            v.selected = false;
    }

    void selectNoneTrianglesUV(int32_t mesh)
    {
        for (auto &t : data->meshes[mesh].triangles)
            t.selectedUV = false;
    }

    void selectInverseVerticesUV(int32_t mesh)
    {
        for (auto &v : data->meshes[mesh].texcoords)
            v.selected = !v.selected;
    }

    void selectInverseTrianglesUV(int32_t mesh)
    {
        for (auto &t : data->meshes[mesh].triangles)
            t.selectedUV = !t.selectedUV;
    }

    void selectTouchingVerticesUV(int32_t mesh)
    {
        std::unordered_set<size_t> selected;

        for (size_t i = 0; i < data->meshes[mesh].texcoords.size(); i++)
            if (data->meshes[mesh].texcoords[i].selected)
                selected.insert(i);

        for (auto &tri : data->meshes[mesh].triangles)
        {
            if (selected.contains(tri.texcoords[0]) ||
                selected.contains(tri.texcoords[1]) ||
                selected.contains(tri.texcoords[2]))
                for (auto &tc : tri.texcoords)
                    data->meshes[mesh].texcoords[tc].selected = true;
        }
    }

    void selectTouchingTrianglesUV(int32_t mesh)
    {
        std::unordered_set<size_t> selected;

        for (auto &tri : data->meshes[mesh].triangles)
            if (tri.selectedUV)
                for (auto &tc : tri.texcoords)
                    selected.insert(tc);

        for (auto &tri : data->meshes[mesh].triangles)
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
    void syncSelectionUV(int32_t mesh)
    {
        for (auto &tri : data->meshes[mesh].triangles)
            tri.selectedFace = tri.selectedUV;
    }

    // sync active 3D selection to UV
    void syncSelection3D(int32_t mesh)
    {
        for (auto &tri : data->meshes[mesh].triangles)
            tri.selectedUV = tri.selectedFace;
    }
};