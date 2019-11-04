#pragma once

#include <QSyntaxHighlighter>

class BaseHighlighter 
	: public QSyntaxHighlighter
{
	Q_OBJECT

protected:
	const QVector<QPair<QString, QString>>* m_braces;

public:
	BaseHighlighter(QTextDocument*parent);
	~BaseHighlighter();

	const QVector<QPair<QString, QString>>* braces() const;
};
