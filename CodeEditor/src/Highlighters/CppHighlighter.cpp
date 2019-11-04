#include "CppHighlighter.h"

//TextBlockData::TextBlockData() {}
//
//QVector<ParenthesisInfo*> TextBlockData::parentheses()
//{
//	return m_parentheses;
//}
//
//void TextBlockData::insert(ParenthesisInfo* info)
//{
//	int i = 0;
//	while (i < m_parentheses.size() &&
//		info->position > m_parentheses.at(i)->position)
//		++i;
//
//	m_parentheses.insert(i, info);
//}


CppHighlighter::CppHighlighter(QTextDocument* parent)
	:BaseHighlighter(parent)
{
	m_braces = new const QVector<QPair<QString, QString>>{
		{"(", ")"},
		{"{", "}"},
		{"[", "]"},
		{"<", ">"},
		{"\"", "\""},
		{"'", "'"}
	};


	HighlightingRule rule;

	keywordFormat.setForeground(Qt::darkBlue);
	//keywordFormat.setFontWeight(QFont::Bold);
	const QString keywordPatterns[] = 
	{
		QStringLiteral("\\bchar\\b"), QStringLiteral("\\bclass\\b"), QStringLiteral("\\bconst\\b"),
		QStringLiteral("\\bdouble\\b"), QStringLiteral("\\benum\\b"), QStringLiteral("\\bexplicit\\b"),
		QStringLiteral("\\bfriend\\b"), QStringLiteral("\\binline\\b"), QStringLiteral("\\bint\\b"),
		QStringLiteral("\\blong\\b"), QStringLiteral("\\bnamespace\\b"), QStringLiteral("\\boperator\\b"),
		QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"),
		QStringLiteral("\\bshort\\b"), QStringLiteral("\\bsignals\\b"), QStringLiteral("\\bsigned\\b"),
		QStringLiteral("\\bslots\\b"), QStringLiteral("\\bstatic\\b"), QStringLiteral("\\bstruct\\b"),
		QStringLiteral("\\btemplate\\b"), QStringLiteral("\\btypedef\\b"), QStringLiteral("\\btypename\\b"),
		QStringLiteral("\\bunion\\b"), QStringLiteral("\\bunsigned\\b"), QStringLiteral("\\bvirtual\\b"),
		QStringLiteral("\\bvoid\\b"), QStringLiteral("\\bvolatile\\b"), QStringLiteral("\\bbool\\b"),
		QStringLiteral("\\bthis\\b")
	};

	for (const QString& pattern : keywordPatterns) 
	{
		rule.pattern = QRegularExpression(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	/*classFormat.setFontWeight(QFont::Bold);*/
	classFormat.setForeground(Qt::blue);
	rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Z][A-Za-z]+\\b"));
	rule.format = classFormat;
	highlightingRules.append(rule);

	quotationFormat.setForeground(Qt::darkGreen);
	rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	functionFormat.setFontItalic(true);
	functionFormat.setForeground(Qt::darkYellow);
	rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
	rule.format = functionFormat;
	highlightingRules.append(rule);

	singleLineCommentFormat.setForeground(Qt::red);
	rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	multiLineCommentFormat.setForeground(Qt::red);

	commentStartExpression = QRegularExpression(QStringLiteral("/\\*"));
	commentEndExpression = QRegularExpression(QStringLiteral("\\*/"));
}


void CppHighlighter::highlightBlock(const QString& text)
{
	for (const HighlightingRule& rule : qAsConst(highlightingRules)) 
	{
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
		while (matchIterator.hasNext()) 
		{
			QRegularExpressionMatch match = matchIterator.next();
			setFormat(match.capturedStart(), match.capturedLength(), rule.format);
		}
	}

	setCurrentBlockState(0);

	int startIndex = 0;
	if (previousBlockState() != 1)
		startIndex = text.indexOf(commentStartExpression);

	while (startIndex >= 0) 
	{
		QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
		int endIndex = match.capturedStart();
		int commentLength = 0;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		}
		else {
			commentLength = endIndex - startIndex
				+ match.capturedLength();
		}
		setFormat(startIndex, commentLength, multiLineCommentFormat);
		startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
	}

	/*TextBlockData* data = new TextBlockData;

	unsigned int braceIndex;
	char c;
	for (int i = 0; i < text.length(); i++)
	{
		c = text[i].toLatin1();
		braceIndex = m_braces->find(c);
		if (braceIndex != std::string::npos)
		{
			ParenthesisInfo* info = new ParenthesisInfo;
			info->character = c;
			info->position = i;

			data->insert(info);
		}
	}*/


	/*for (auto chr : text)
	{
		char c = chr.toLatin1();
		braceIndex = m_braces->find(c);
		if (braceIndex != std::string::npos)
		{
			ParenthesisInfo* info = new ParenthesisInfo;
			info->character = chr.toLatin1();
			info->position = braceIndex;

			data->insert(info);
		}
	}*/

	/*int leftPos = text.indexOf('(');
	while (leftPos != -1) 
	{
		ParenthesisInfo* info = new ParenthesisInfo;
		info->character = '(';
		info->position = leftPos;

		data->insert(info);
		l
	}eftPos = text.indexOf('(', leftPos + 1);

	

	int rightPos = text.indexOf(')');
	while (rightPos != -1) 
	{
		ParenthesisInfo* info = new ParenthesisInfo;
		info->character = ')';
		info->position = rightPos;

		data->insert(info);

		rightPos = text.indexOf(')', rightPos + 1);
	}*/

	/*setCurrentBlockUserData(data);*/
}


const QVector<QPair<QString, QString>>* CppHighlighter::braces() const
{
	return m_braces;
}