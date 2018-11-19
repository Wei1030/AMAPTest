#include "QGPSLabel.h"
#include <QPainter>

QGPSLabel::QGPSLabel(QWidget *parent)
	: QLabel(parent)
{
}

QGPSLabel::~QGPSLabel()
{
}

void QGPSLabel::paintEvent(QPaintEvent *event)
{
	static const QPointF points[4] = {
	  QPointF(17.5, 11.5),
	  QPointF(26.0, 8),
	  QPointF(22.5, 16.5),
	  QPointF(22.5, 11.5)
	};
	QPainter painter(this);
	painter.setBrush(QColor(0, 0, 0));
	painter.drawPolygon(points, 4);	

}
