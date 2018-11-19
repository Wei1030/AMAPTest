#pragma once

#include <QtWidgets/QWidget>
#include "ui_ReimburseCheat.h"
#include "WebContainerAPI.h"
#include "GetPointWeb.h"

class Mask;

class ReimburseCheat : public QWidget
{
	Q_OBJECT

public:
	ReimburseCheat(QWidget *parent = Q_NULLPTR);
	~ReimburseCheat();

	static int WEBCONTAINERAPI_CB fnWebStartCb(unsigned int error, WebEngineType type, void* usr);

	static int WEBCONTAINERAPI_CB OnSearchFinish(const void* data_in, unsigned int size_in, void* reserve1, void* reserve2);
	int init();

protected:
	void OnDistanceChanged(const QString & str);
	void OnElapsedChanged(const QString & str);
	void OnAverageChanged(const QString & str);

	void OnPowerChanged(const QString & str);

	void moveEvent(QMoveEvent *event);
	void timerEvent(QTimerEvent *event);

	static bool parseResult(const QByteArray& request, QVariantMap& data);

	void StartSearch();

	void capPng();

	void showGetPointWeb();

protected slots:
	void onSearchFinish(bool bOk, QVariantMap data);
	void onWebStart();

private:
	Ui::ReimburseCheatClass ui;
	Mask* m_pMask;

	void* m_hWnd;
};
