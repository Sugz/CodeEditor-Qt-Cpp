#include "BaseHighlighter.h"

BaseHighlighter::BaseHighlighter(QTextDocument*parent)
	: QSyntaxHighlighter(parent)
{
}

BaseHighlighter::~BaseHighlighter()
{

}

const QVector<QPair<QString, QString>>* BaseHighlighter::braces() const
{
	return m_braces;
}
