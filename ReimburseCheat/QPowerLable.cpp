#include "QPowerLable.h"
#include <QPainter>

QPowerLable::QPowerLable(QWidget *parent)
	: QLabel(parent)
	, m_value(18)
{
}

QPowerLable::~QPowerLable()
{
}

void QPowerLable::SetPowerValue(const QString & str)
{
	m_value = 22 * str.toInt() / 100;
	repaint();
}

void QPowerLable::paintEvent(QPaintEvent *event)
{
	QPainter paint(this);
	paint.setPen(QColor(0, 0, 0));
	paint.drawRoundedRect(0, 7, 22, 9, 2, 2);
	paint.setBrush(QColor(0, 0, 0));
	paint.drawRect(23, 11, 1, 2);
 	paint.drawRect(0, 7, m_value, 9);
}
