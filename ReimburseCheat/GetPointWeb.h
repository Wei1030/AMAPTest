#pragma once

#include <QWidget>
#include "ui_GetPointWeb.h"

class GetPointWeb : public QWidget
{
	Q_OBJECT

public:
	GetPointWeb(QWidget *parent = Q_NULLPTR);
	~GetPointWeb();

	void init();

private:
	Ui::GetPointWeb ui;
};
