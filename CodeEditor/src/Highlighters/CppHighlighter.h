#pragma once

#include "../Highlighters/BaseHighlighter.h"

#include <QRegularExpression>
#include <QTextCharFormat>
#include <QTextDocument>


//struct ParenthesisInfo
//{
//	char character;
//	int position;
//};
//
//
//class TextBlockData : public QTextBlockUserData
//{
//public:
//	TextBlockData();
//
//	QVector<ParenthesisInfo*> parentheses();
//	void insert(ParenthesisInfo* info);
//
//private:
//	QVector<ParenthesisInfo*> m_parentheses;
//};


class CppHighlighter :
	public BaseHighlighter
{
	Q_OBJECT

public:
	CppHighlighter(QTextDocument* parent = nullptr);
	const QVector<QPair<QString, QString>>* braces() const;

protected:
	void highlightBlock(const QString& text) override;

private:
	struct HighlightingRule
	{
		QRegularExpression pattern;
		QTextCharFormat format;
	};

	const QVector<QPair<QString, QString>>* m_braces;
	

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

