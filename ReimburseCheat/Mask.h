#pragma once

#include <QWidget>
#include "ui_Mask.h"

class Mask : public QWidget
{
	Q_OBJECT

public:
	Mask(QWidget *parent = Q_NULLPTR);
	~Mask();

public:
	Ui::Mask ui;
private:
	//QPixmap m_pixmap;
};
