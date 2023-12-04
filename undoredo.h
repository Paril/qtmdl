#pragma once

#include <optional>
#include <vector>
#include "modeldata.h"
#include <QObject>

class UndoRedo : public QObject
{
    Q_OBJECT

	// current undo position. if nullopt
	// we're not undone anywhere.
	std::optional<size_t>		_position = std::nullopt;
	std::vector<ModelData>	    _models = {};

public:
	inline UndoRedo(QObject *parent = nullptr) : QObject(parent) { }

	inline bool canRedo() { return _models.size() && _position; }
	inline bool canUndo() { return _models.size() != 0 && (!_position || _position != 0); }

	inline void push(const ModelData &data)
	{
		// we're not at the tail, clip everything
		// off after the current position.
		if (_position != std::nullopt)
		{
			_models.resize(_position.value());
			_position = std::nullopt;
		}

		_models.push_back(data);

		emit undoRedoStateChanged();
	}

	inline void undo(ModelData &current)
	{
		if (!canUndo())
			return;

		// if we're currently at the tail, we have to add a copy
		// of the current state here so that we can get back
		// to the current state.
		if (!_position.has_value())
		{
			_models.push_back(current);
			_position = _models.size() - 2;
		}
		else if (_position != 0)
			_position.value()--;

		current = _models[_position.value()];

		emit undoRedoStateChanged();
	}

	inline void redo(ModelData &current)
	{
		if (!canRedo())
			return;

		_position.value()++;

		current = _models[_position.value()];

		// we reached the tail, so remove the extra copy that we made here before
		if (_position.value() == _models.size() - 1)
		{
			_position = std::nullopt;
			_models.pop_back();
		}

		emit undoRedoStateChanged();
	}

	inline void clear()
	{
		_position.reset();
		_models.clear();

		emit undoRedoStateChanged();
	}

	inline size_t byteSize()
	{
		size_t b = 0;
		
		// TODO

		return b;
	}

signals:
    void undoRedoStateChanged();
};