#pragma once

#include "modeldata.h"
#include <QFileInfo>
#include <QMimeType>

struct ModelLoader
{
	static std::unique_ptr<ModelData> load(QFileInfo file, QMimeType mime);
	static void save(const ModelData &model, QFileInfo file, QMimeType type);
};