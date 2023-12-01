#pragma once

#include <QObject>
#include <QAction>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QActionGroup>
#include <initializer_list>

// some shared utils I use in a few places
namespace QtUtils
{
	template<typename F>
	static void setupMenuRadioButtons(QObject *owner, std::initializer_list<QAction *> actions, F func)
	{
		auto groupTool = new QActionGroup(owner);

		for (auto &action : actions)
		{
			QObject::connect(action, &QAction::triggered, func);
			groupTool->addAction(action);
		}
	}

	template<typename F>
	static void setupGroupedButtons(QObject *owner, std::initializer_list<QAbstractButton *> buttons, F func)
	{
		auto groupTool = new QButtonGroup(owner);

		for (auto &button : buttons)
		{
			QObject::connect(button, &QAbstractButton::toggled, func);
			groupTool->addButton(button);
		}
	}
};