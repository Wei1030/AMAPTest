#include "ReimburseCheat.h"
#include <QtWidgets/QApplication>
#include <QDesktopWidget>
#include <Mask.h>

ReimburseCheat* g_pMainWnd = nullptr;
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ReimburseCheat w;
	g_pMainWnd = &w;
	//w.setWindowFlags(Qt::FramelessWindowHint);
	w.show();	
	w.init();
	w.move((QApplication::desktop()->width() - w.width()) / 2, (QApplication::desktop()->height() - w.height()) / 2);

	return a.exec();
}
