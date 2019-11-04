#pragma once

#include "../Highlighters/BaseHighlighter.h"

#include <QRegularExpression>
#include <QTextCharFormat>
#include <QTextDocument>


class CppHighlighter :
	public BaseHighlighter
{
	Q_OBJECT

public:
	CppHighlighter(QTextDocument* parent = nullptr);
	~CppHighlighter();
	const QVector<QPair<QChar, QChar>>* braces() const override;

protected:
	void highlightBlock(const QString& text) override;

private:
	struct HighlightingRule
	{
		QRegularExpression pattern;
		QTextCharFormat format;
	};

	const QVector<QPair<QChar, QChar>>* m_braces;
	

	QVector<HighlightingRule> highlightingRules;

	QRegularExpression commentStartExpression;
	QRegularExpression commentEndExpression;

	QTextCharFormat keywordFormat;
	QTextCharFormat classFormat;
	QTextCharFormat singleLineCommentFormat;
	QTextCharFormat multiLineCommentFormat;
	QTextCharFormat quotationFormat;
	QTextCharFormat functionFormat;
};

