#pragma once

#include <QLabel>

class QPowerLable : public QLabel
{
	Q_OBJECT

public:
	QPowerLable(QWidget *parent);
	~QPowerLable();
	void SetPowerValue(const QString & str);
	void paintEvent(QPaintEvent *event);

private:
	int m_value;
};
