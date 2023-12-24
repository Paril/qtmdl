#pragma once

#include <QString>
#include <QSettings>
#include <QColor>
#include <array>

enum class EditorColorId
{
	VertexTickUnselected2D,
	VertexTickSelected2D,
	FaceLineUnselected2D,
	FaceLineSelected2D,
	FaceUnselected2D,
	FaceSelected2D,

	VertexTickUnselected3D,
	VertexTickSelected3D,
	FaceLineUnselected3D,
	FaceLineSelected3D,
	FaceUnselected3D,
	FaceSelected3D,
	
	VertexTickUnselectedUV,
	VertexTickSelectedUV,
	FaceLineUnselectedUV,
	FaceLineSelectedUV,
	FaceUnselectedUV,
	FaceSelectedUV,

	Grid,
	OriginX,
	OriginY,
	OriginZ,
	SelectBox,

	Total
};

static constexpr const char *EditorColorNames[] = {
	"ColorVertexTickUnselected2D",
	"ColorVertexTickSelected2D",
	"ColorFaceLineUnselected2D",
	"ColorFaceLineSelected2D",
	"ColorFaceUnselected2D",
	"ColorFaceSelected2D",

	"ColorVertexTickUnselected3D",
	"ColorVertexTickSelected3D",
	"ColorFaceLineUnselected3D",
	"ColorFaceLineSelected3D",
	"ColorFaceUnselected3D",
	"ColorFaceSelected3D",

	"ColorVertexTickUnselectedUV",
	"ColorVertexTickSelectedUV",
	"ColorFaceLineUnselectedUV",
	"ColorFaceLineSelectedUV",
	"ColorFaceUnselectedUV",
	"ColorFaceSelectedUV",

	"ColorGrid",
	"ColorOriginX",
	"ColorOriginY",
	"ColorOriginZ",
	"SelectBox",
};

static constexpr std::array<QColor, std::size(EditorColorNames)> EditorColorDefaults = {
	QColor(135, 107, 87),
	QColor(255, 235, 31),
	QColor(123, 123, 123),
	QColor(255, 235, 31),
	QColor(0, 0, 0),
	QColor(255, 171, 7),

	QColor(235, 159, 39),
	QColor(255, 235, 31),
	QColor(0, 0, 0),
	QColor(255, 235, 31),
	QColor(255, 255, 255),
	QColor(255, 171, 7),

	QColor(135, 107, 87),
	QColor(255, 235, 31),
	QColor(123, 123, 123),
	QColor(255, 235, 31),
	QColor(255, 255, 255, 31),
	QColor(255, 171, 7, 63),

	QColor(235, 211, 199),
	QColor(255, 0, 0, 127),
	QColor(0, 255, 0, 127),
	QColor(0, 0, 255, 127),
	QColor(115, 151, 167),
};

static_assert(std::size(EditorColorNames) == ((size_t) EditorColorId::Total), "need to match color names");
static_assert(std::size(EditorColorDefaults) == std::size(EditorColorNames), "need to match color names");

// static read/write access to saved settings.
// not all of these are necessarily synced to disk.
struct SettingsContainer
{
	SettingsContainer();

	const QString &getModelDialogLocation();
	void setModelDialogLocation(QString str);

	const float &getHorizontalSplit();
	void setHorizontalSplit(float percent);

	const float &getVerticalSplit();
	void setVerticalSplit(float percent);

	const QColor &getEditorColor(EditorColorId id);
	void setEditorColor(EditorColorId id, QColor color);

	QByteArray getMainLayout();
	void setMainLayout(const QByteArray &data);

	QByteArray getUVLayout();
	void setUVLayout(const QByteArray &data);

private:
	QSettings _settings;
	QString _modelDialogLocation;
	float _horizontalSplit;
	float _verticalSplit;
	std::array<QColor, (size_t) EditorColorId::Total> _editorColors;
};

SettingsContainer &Settings();