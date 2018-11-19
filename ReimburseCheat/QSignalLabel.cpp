#include "QSignalLabel.h"
#include <QPainter>

QSignalLabel::QSignalLabel(QWidget *parent)
	: QLabel(parent)
	, m_signals(4)
{
}

QSignalLabel::~QSignalLabel()
{
}

void QSignalLabel::SetSignals(const QString & str)
{
	m_signals = str.toInt();
	repaint();
}

void QSignalLabel::paintEvent(QPaintEvent *event)
{
	QPainter paint(this);
	
	int xStart = 9;
	int iStart = 0;

	paint.setBrush(QColor(0, 0, 0));

	for (iStart = 0; iStart < m_signals; iStart++)
	{
		paint.drawEllipse(QPoint(xStart, 12), 2, 2);
		xStart += 7;
	}
	paint.setBrush(Qt::NoBrush);

	paint.setPen(QColor(0, 0, 0));

	for (;iStart<5;iStart++)
	{
		paint.drawEllipse(QPoint(xStart, 12), 2, 2);
		xStart += 7;
	}	
}