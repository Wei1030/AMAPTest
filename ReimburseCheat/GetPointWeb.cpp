#include "GetPointWeb.h"
#include "WebContainerAPI.h"

GetPointWeb::GetPointWeb(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Dialog);
}

GetPointWeb::~GetPointWeb()
{
}

void GetPointWeb::init()
{
	WebContainerAPI_LoadUrl(Chromium, "https://lbs.amap.com/console/show/picker",(void*)winId());
}


