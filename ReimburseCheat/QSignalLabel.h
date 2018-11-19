#pragma once

#include <QLabel>

class QSignalLabel : public QLabel
{
	Q_OBJECT

public:
	QSignalLabel(QWidget *parent);
	~QSignalLabel();
	void SetSignals(const QString & str);
	void paintEvent(QPaintEvent *event);

private:
	int m_signals;
};
