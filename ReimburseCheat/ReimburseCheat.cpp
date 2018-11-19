#include "ReimburseCheat.h"
#include "Mask.h"
#include <QProcess>
#include <QMetaObject>

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDateTime>
#include <QScreen>
#include <QMessageBox>

#pragma comment(lib,"WebContainerAPI.lib")

extern ReimburseCheat* g_pMainWnd;
//	border-image: url(:/ReimburseCheat/Resources/hour.jpeg);
int WEBCONTAINERAPI_CB ReimburseCheat::fnWebStartCb(unsigned int error, WebEngineType type, void* usr)
{
	ReimburseCheat* pThis = (ReimburseCheat*)usr;
	if (error)
	{
		WebContainerInfo wi = { sizeof(WebContainerInfo) };
		wi.iEngineType = Chromium;

		WebContainerAPI_Start(&wi);
		return -1;
	}

	//WebContainerAPI_LoadUrl(Chromium, "https://lbs.amap.com/console/show/picker", pThis->m_hWnd1);

	QString url = QCoreApplication::applicationDirPath() + "/test.html";
	std::string strUrl = url.toLocal8Bit();

	WebContainerAPI_LoadUrl(Chromium, strUrl.c_str(), pThis->m_hWnd);

	QMetaObject::invokeMethod(g_pMainWnd, "onWebStart", Qt::QueuedConnection);
	return 0;
}

int WEBCONTAINERAPI_CB ReimburseCheat::OnSearchFinish(const void* data_in, unsigned int size_in, void* reserve1, void* reserve2)
{
	QVariantMap v;
	bool bOk = ReimburseCheat::parseResult((const char*)data_in, v);

	QMetaObject::invokeMethod(g_pMainWnd, "onSearchFinish", Qt::QueuedConnection,
		Q_ARG(bool, bOk),
		Q_ARG(QVariantMap, v));
	return 0;
}

int ReimburseCheat::init()
{
	m_pMask = new Mask;	
	m_pMask->show();	

	connect(ui.edSig, &QLineEdit::textEdited, m_pMask->ui.label, &QSignalLabel::SetSignals);
	connect(ui.edTime, &QLineEdit::textEdited, m_pMask->ui.label_3, &QSignalLabel::setText);
	connect(ui.edPower, &QLineEdit::textEdited, m_pMask->ui.label_6, &QPowerLable::SetPowerValue);

	ui.pushButton->setFocus();

	m_hWnd = (void*)ui.widget_3->winId();

	ui.widget_3->stackUnder(ui.widget_4);

	WebContainerAPI_RegCbForJsCall("MSG_SEARCH_FINISH", &ReimburseCheat::OnSearchFinish);

	WebContainerAPI_Init();

	WebContainerAPI_SetWebStartCb(&ReimburseCheat::fnWebStartCb, (void*)this);

	WebContainerInfo wi = { sizeof(WebContainerInfo) };
	wi.iEngineType = Chromium;

	WebContainerAPI_Start(&wi);

	return 0;
}

ReimburseCheat::ReimburseCheat(QWidget *parent)
	: QWidget(parent)
	, m_pMask(nullptr)
{
	ui.setupUi(this);
	
	connect(ui.edStart, &QLineEdit::textChanged, ui.lbStart, &QLabel::setText);
	connect(ui.edEnd, &QLineEdit::textChanged, ui.lbEnd, &QLabel::setText);
	connect(ui.edScore, &QLineEdit::textChanged, ui.lbScore, &QLabel::setText);
	connect(ui.edDistance, &QLineEdit::textChanged, ui.lbDistances, &QLabel::setText);
	connect(ui.edElapsed, &QLineEdit::textChanged, ui.lbElapsed, &QLabel::setText);
	connect(ui.edAverage, &QLineEdit::textChanged, ui.lbAverage, &QLabel::setText);
	connect(ui.edTop, &QLineEdit::textChanged, ui.lbTop, &QLabel::setText);
	connect(ui.edTip1, &QLineEdit::textChanged, ui.lbTip1, &QLabel::setText);
	connect(ui.edTip2, &QLineEdit::textChanged, ui.lbTip2, &QLabel::setText);

	connect(ui.edDistance, &QLineEdit::textEdited, this, &ReimburseCheat::OnDistanceChanged);
	connect(ui.edElapsed, &QLineEdit::textEdited, this, &ReimburseCheat::OnElapsedChanged);
	connect(ui.edAverage, &QLineEdit::textEdited, this, &ReimburseCheat::OnAverageChanged);

	connect(ui.edPower, &QLineEdit::textEdited, this, &ReimburseCheat::OnPowerChanged);

	connect(ui.pushButton, &QPushButton::clicked, this, &ReimburseCheat::StartSearch);
	connect(ui.pushButton_2, &QPushButton::clicked, this, &ReimburseCheat::capPng);
	connect(ui.pushButton_3, &QPushButton::clicked, this, &ReimburseCheat::showGetPointWeb);
	ui.pushButton_3->setDisabled(true);

	QProcess::startDetached(QCoreApplication::applicationDirPath() + "\\WebContainer.exe");

	startTimer(500);
}

ReimburseCheat::~ReimburseCheat()
{
	WebContainerAPI_Stop(Chromium);
	WebContainerAPI_Uninit();
}

void ReimburseCheat::OnDistanceChanged(const QString & str)
{	
	double distance = str.toDouble();
	if (0 == distance)
	{
		return;
	}
	
	unsigned int average = ui.edAverage->text().toUInt();
	if (0 == average)
	{
		double elapsed = ui.edElapsed->text().toDouble();
		if ( 0 == elapsed)
		{
			return;
		}
		elapsed /= 60;
		ui.edAverage->setText(QString::number((unsigned int)(distance / elapsed)));
		return;
	}
	
	unsigned int Elapsed = distance / average * 60;
	unsigned int hour = Elapsed / 60;
	unsigned int minute = Elapsed % 60;
	QString strElapsed;
	if (hour)
	{
		strElapsed = QString("%1:%2").arg(QString::number(hour)).arg(QString::number(minute));
		ui.label_4->setStyleSheet("border-image: url(:/ReimburseCheat/Resources/hour.jpeg)");
	}
	else
	{
		strElapsed = QString::number(minute);
		ui.label_4->setStyleSheet("border-image: url(:/ReimburseCheat/Resources/minute.jpeg)");
	}
	ui.edElapsed->setText(strElapsed);
}

void ReimburseCheat::OnElapsedChanged(const QString & str)
{
	QStringList s = str.split(":");
	double elapsed = 0.0;
	int c = s.size();
	if (1 == c)
	{
		elapsed = str.toDouble();
		ui.label_4->setStyleSheet("border-image: url(:/ReimburseCheat/Resources/minute.jpeg)");
	}
	else
	{
		int h = s[0].toInt();
		int m = 0;
		if (c >= 2)
		{
			m = s[1].toInt();
		}
		elapsed = h * 60 + m;
		ui.label_4->setStyleSheet("border-image: url(:/ReimburseCheat/Resources/hour.jpeg)");
	}
	
	if (0.0 == elapsed)
	{
		return;
	}
	elapsed /= 60;

	double distance = ui.edDistance->text().toDouble();
	if (0 == distance)
	{
		unsigned int average = ui.edAverage->text().toUInt();
		if (0 == average)
		{
			return;
		}

		ui.edDistance->setText(QString::number((double)(elapsed*average),'f',1));
		return;
	}
	
	ui.edAverage->setText(QString::number((unsigned int)(distance / elapsed)));
}

void ReimburseCheat::OnAverageChanged(const QString & str)
{
	unsigned int average = str.toUInt();
	if (0 == average)
	{
		return;
	}

	double distance = ui.edDistance->text().toDouble();
	if (0 == distance)
	{
		double elapsed = ui.edElapsed->text().toDouble();
		if (0 == elapsed)
		{
			return;
		}
		elapsed /= 60;

		ui.edDistance->setText(QString::number((double)(elapsed*average), 'f', 1));
		return;
	}
	unsigned int Elapsed = distance / average * 60;
	unsigned int hour = Elapsed / 60;
	unsigned int minute = Elapsed % 60;
	QString strElapsed;
	if (hour)
	{
		strElapsed = QString("%1:%2").arg(QString::number(hour)).arg(QString::number(minute));
		ui.label_4->setStyleSheet("border-image: url(:/ReimburseCheat/Resources/hour.jpeg)");
	}
	else
	{
		strElapsed = QString::number(minute);
		ui.label_4->setStyleSheet("border-image: url(:/ReimburseCheat/Resources/minute.jpeg)");
	}
	ui.edElapsed->setText(strElapsed);
}

void ReimburseCheat::OnPowerChanged(const QString & str)
{
	if (m_pMask)
	{
		m_pMask->ui.label_5->setText(str + "%");
	}	
}

void ReimburseCheat::moveEvent(QMoveEvent *event)
{
	if (m_pMask)
	{
		m_pMask->move(ui.widget_3->mapToGlobal(ui.widget_3->pos()));
	}	
}

void ReimburseCheat::timerEvent(QTimerEvent *event)
{
	if (isActiveWindow())
	{
		if (m_pMask)
		{
			m_pMask->raise();
		}
	}	
}

bool ReimburseCheat::parseResult(const QByteArray& request, QVariantMap& data)
{
	QJsonParseError json_error;
	QJsonDocument json_doc = QJsonDocument::fromJson(request, &json_error);
	if (json_error.error != QJsonParseError::NoError)
	{
		return false;
	}

	if (!json_doc.isObject())
	{
		return false;
	}

	QJsonObject json_object = json_doc.object();
	if (!json_object.contains("ok"))
	{
		return false;
	}

	int bOk = json_object["ok"].toInt();

	data = json_object.toVariantMap();	

	return bOk? true:false;
}

void ReimburseCheat::StartSearch()
{
	QStringList strSLngLat = ui.sLngLat->text().split(',', QString::SkipEmptyParts);
	QStringList strELngLat = ui.eLngLat->text().split(',', QString::SkipEmptyParts);

	if (strSLngLat.size() != 2 
		|| strELngLat.size() != 2)
	{
		QMessageBox::information(this, u8"´íÎó", QString(u8"¹æ»®Ê§°Ü£¬Çë¼ì²é¸ñÊ½!"));
		return;
	}

	QVariantMap v;
	v["sLng"] = strSLngLat[0].toDouble();
	v["sLat"] = strSLngLat[1].toDouble();
	v["eLng"] = strELngLat[0].toDouble();
	v["eLat"] = strELngLat[1].toDouble();

	QJsonDocument d(QJsonObject::fromVariantMap(v));
	std::string args = d.toJson();

	WebContainerAPI_CallJs(Chromium, m_hWnd, "searchAndDraw", args.data(), args.length()+1);
}

void ReimburseCheat::capPng()
{
	activateWindow();
	raise();
	if (m_pMask)
	{
		m_pMask->raise();
	}
	QScreen *screen = QGuiApplication::primaryScreen();

	QString filePathName = QCoreApplication::applicationDirPath() + "\\";
	filePathName +=	QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss-zzz");
	filePathName += ".png";

	QPoint pStart = ui.widget_3->mapToGlobal(ui.widget_3->pos());
	if (!screen->grabWindow(0, pStart.x(),pStart.y(),375,672).save(filePathName, "png"))
	{
		QMessageBox::information(this, u8"´íÎó", QString(u8"½ØÍ¼Ê§°Ü"));
		return;
	}
	QMessageBox::information(this, "", QString(u8"½ØÍ¼³É¹¦"));
}

void ReimburseCheat::showGetPointWeb()
{
	GetPointWeb* p = new GetPointWeb;
	p->init();
	p->show();	
}

void ReimburseCheat::onSearchFinish(bool bOk, QVariantMap data)
{
	double dis = data["distance"].toDouble() / 1000;
	double el = data["time"].toDouble() / 60 / 60;
	int av = dis / el;

	unsigned int Elapsed = el * 60;
	unsigned int hour = Elapsed / 60;
	unsigned int minute = Elapsed % 60;
	QString strElapsed;
	if (hour)
	{
		strElapsed = QString("%1:%2").arg(QString::number(hour)).arg(QString::number(minute));
		ui.label_4->setStyleSheet("border-image: url(:/ReimburseCheat/Resources/hour.jpeg)");
	}
	else
	{
		strElapsed = QString::number(minute);
		ui.label_4->setStyleSheet("border-image: url(:/ReimburseCheat/Resources/minute.jpeg)");
	}

	ui.edDistance->setText(QString::number(dis, 'f', 1));
	ui.edElapsed->setText(strElapsed);
	ui.edAverage->setText(QString::number(av));
}

void ReimburseCheat::onWebStart()
{
	ui.pushButton_3->setDisabled(false);
}
