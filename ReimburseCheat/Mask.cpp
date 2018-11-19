#include "Mask.h"
#include <QPalette>

Mask::Mask(QWidget *parent)
	: QWidget(parent)
	//, m_pixmap(QCoreApplication::applicationDirPath() + "/exmple.png")
{
	ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool /*| Qt::WindowStaysOnTopHint*/);
	setAttribute(Qt::WA_TranslucentBackground);

	//QPixmap p1(QCoreApplication::applicationDirPath() + "/exmple.png");
	//QPixmap p2(QCoreApplication::applicationDirPath() + "/exmple1.jpeg");

	//p1.copy(217, 981, 329 - 217, 1003 - 981).save("c:\\1.png");
	//p2.copy(130, 523, 197 - 130, 536 - 523).scaled(QSize(56, 12), Qt::KeepAspectRatio, Qt::SmoothTransformation).save("c:\\2.jpeg");
	//ui.label_4->setPixmap(m_pixmap.copy(569,5,65,29).scaled(QSize(32,25),Qt::KeepAspectRatio,Qt::SmoothTransformation));

	//m_pixmap.copy(0, 804, 750, 1334 - 804).save("c:\\1.png");
	//m_pixmap.copy(569, 5, 65, 29).scaled(QSize(32, 25), Qt::KeepAspectRatio, Qt::SmoothTransformation).save("c:\\2.png");
}

Mask::~Mask()
{
}
