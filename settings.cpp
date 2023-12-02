#include "settings.h"
#include <QString>
#include <QSettings>

SettingsContainer::SettingsContainer() :
	_settings(QSettings::IniFormat, QSettings::UserScope, "Altered Softworks", "QTMDL"),
	_modelDialogLocation(_settings.value("ModelDialogLocation", "").toString()),
	_horizontalSplit(_settings.value("HorizontalSplit", 0.5f).toFloat()),
	_verticalSplit(_settings.value("VerticalSplit", 0.5f).toFloat())
{
	for (size_t i = 0; i < std::size(EditorColorDefaults); i++)
		_editorColors[i] = _settings.value(EditorColorNames[i], EditorColorDefaults[i]).value<QColor>();
}

const QString &SettingsContainer::getModelDialogLocation()
{
	return _modelDialogLocation;
}

void SettingsContainer::setModelDialogLocation(QString str)
{
	_modelDialogLocation = str;
	_settings.setValue("ModelDialogLocation", str);
}

const float &SettingsContainer::getHorizontalSplit()
{
	return _horizontalSplit;
}

void SettingsContainer::setHorizontalSplit(float percent)
{
	_horizontalSplit = percent;
	_settings.setValue("HorizontalSplit", _horizontalSplit);
}

const float &SettingsContainer::getVerticalSplit()
{
	return _verticalSplit;
}

void SettingsContainer::setVerticalSplit(float percent)
{
	_verticalSplit = percent;
	_settings.setValue("VerticalSplit", _verticalSplit);
}

const QColor &SettingsContainer::getEditorColor(EditorColorId id)
{
	return _editorColors[(size_t) id];
}

void SettingsContainer::setEditorColor(EditorColorId id, QColor color)
{
	_editorColors[(size_t) id] = color;
	_settings.setValue(EditorColorNames[(size_t) id], color);
}

SettingsContainer &Settings()
{
	static SettingsContainer settings;
	return settings;
}