#pragma once

#include <QLabel>

class QGPSLabel : public QLabel
{
	Q_OBJECT

public:
	QGPSLabel(QWidget *parent);
	~QGPSLabel();

	void paintEvent(QPaintEvent *event);
};
