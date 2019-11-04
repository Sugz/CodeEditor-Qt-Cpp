#pragma once

#include <QSyntaxHighlighter>

class BaseHighlighter 
	: public QSyntaxHighlighter
{
	Q_OBJECT

protected:
	const QVector<QPair<QChar, QChar>>* m_braces = 0;

public:
	BaseHighlighter(QTextDocument*parent);
	~BaseHighlighter();

	virtual const QVector<QPair<QChar, QChar>>* braces() const = 0;
};
