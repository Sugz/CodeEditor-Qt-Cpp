#pragma once

#include "../Highlighters/BaseHighlighter.h"

#include <QRegularExpression>
#include <QTextCharFormat>
#include <QTextDocument>


struct BracketsInfo
{
	int startPosition;
	int endPosition;
};

class TextBlockData : 
	public QTextBlockUserData
{
public:
	QVector<BracketsInfo*> brackets();
	void append(BracketsInfo* info);

private:
	QVector<BracketsInfo*> m_brackets;

};



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

	// defines the braces for c++
	const QVector<QPair<QChar, QChar>>* m_braces;
	QVector<int> startBrackets();
	//
	

	QVector<HighlightingRule> highlightingRules;

	QRegularExpression commentStartPattern;
	QRegularExpression commentEndPattern;

	QTextCharFormat keywordFormat;
	QTextCharFormat controlFlowFormat;
	QTextCharFormat functionFormat;
	QTextCharFormat classFunctionFormat;
	QTextCharFormat classFormat;
	QTextCharFormat preProcessorFormat;
	QTextCharFormat includeFormat;
	QTextCharFormat singleLineCommentFormat;
	QTextCharFormat multiLineCommentFormat;
	QTextCharFormat quotationFormat;
	QTextCharFormat numbersFormat;
	

	/*QTextCharFormat keywordFormat;
	QTextCharFormat includeFormat;
	QTextCharFormat functionFormat;
	QTextCharFormat defTypeFormat;
	QTextCharFormat preProcessorFormat;
	QTextCharFormat numbersFormat;
	QTextCharFormat stringFormat;
	QTextCharFormat commentFormat;*/
};

