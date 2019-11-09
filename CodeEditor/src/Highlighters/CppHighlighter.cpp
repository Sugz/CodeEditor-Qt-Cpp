#include "CppHighlighter.h"

//TODO: for enum, since it start with a Capital letter, it's treated like a class
// so check case like Qt::Test and QTextCursor::MoveOperation::Left (only Test and Left should be yellow)

QVector<BracketsInfo*> TextBlockData::brackets()
{
	return m_brackets;
}


void TextBlockData::append(BracketsInfo* info)
{
	m_brackets.append(info);
}


CppHighlighter::CppHighlighter(QTextDocument* parent)
	:BaseHighlighter(parent)
{
	m_braces = new const QVector<QPair<QChar, QChar>>{
		{'(', ')'},
		{'{', '}'},
		{'[', ']'},
		{'<', '>'},
	};


	HighlightingRule rule;

	keywordFormat.setForeground(QColor(70, 156, 208));
	const QString keywordPatterns[] =
	{
		R"(\basm\b)", R"(\bauto\b)", R"(\bbool\b)", R"(\bcatch\b)", R"(\bchar\b)", R"(\bclass\b)", R"(\bconst\b)",
		R"(\bconst_cast\b)", R"(\bdelete\b)", R"(\bdynamic_cast\b)", R"(\bdouble\b)", R"(\benum\b)", R"(\bexplicit\b)",
		R"(\bexport\b)", R"(\bextern\b)", R"(\bfalse\b)", R"(\bfloat\b)", R"(\bfriend\b)", R"(\binline\b)", R"(\bint\b)",
		R"(\blong\b)", R"(\bmutable\b)", R"(\bnamespace\b)", R"(\bnew\b)", R"(\bnullptr\b)", R"(\boperator\b)",
		R"(\bprivate\b)", R"(\bprotected\b)", R"(\bpublic\b)", R"(\bregister\b)", R"(\breinterpret_cast\b)", R"(\bshort\b)",
		R"(\bsignals\b)", R"(\bsigned\b)", R"(\bsizeof\b)", R"(\bslots\b)", R"(\bstatic\b)", R"(\bstatic_cast\b)",
		R"(\bstruct\b)", R"(\btemplate\b)", R"(\bthis\b)", R"(\bthrow\b)", R"(\btrue\b)", R"(\btry\b)", R"(\btypedef\b)",
		R"(\btypeid\b)", R"(\btypename\b)", R"(\bunion\b)", R"(\bunsigned\b)", R"(\busing\b)", R"(\bvirtual\b)", 
		R"(\bvoid\b)", R"(\bvolatile\b)", R"(\bwchar_t\b)",
	};

	for (const QString& pattern : keywordPatterns) 
	{
		rule.pattern = QRegularExpression(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	controlFlowFormat.setForeground(QColor(216, 160, 223));
	const QString controlFlowPatterns[] =
	{
		R"(\bif\b)", R"(\belse\b)", R"(\bdo\b)", R"(\bwhile\b)", R"(\breturn\b)", R"(\bbreak\b)", 
		R"(\bcontinue\b)", R"(\bfor\b)", R"(\bswitch\b)", R"(\bcase\b)", R"(\bdefault\b)", R"(\bgoto\b)"
	};

	for (const QString& pattern : controlFlowPatterns)
	{
		rule.pattern = QRegularExpression(pattern);
		rule.format = controlFlowFormat;
		highlightingRules.append(rule);
	}

	classFormat.setForeground(QColor(78, 201, 176));
	rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Z][A-Za-z]+(?=\\()?\\b"));
	rule.format = classFormat;
	highlightingRules.append(rule);

	functionFormat.setForeground(QColor(220, 220, 170));
	rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
	rule.format = functionFormat;
	highlightingRules.append(rule);

	classFunctionFormat.setForeground(QColor(220, 220, 170));
	rule.pattern = QRegularExpression(R"((?<=::)\w+)");
	rule.format = classFunctionFormat;
	highlightingRules.append(rule);
	
	preProcessorFormat.setForeground(Qt::darkGray);
	rule.pattern = QRegularExpression(QStringLiteral("#.*"));
	rule.format = preProcessorFormat;
	highlightingRules.append(rule);
	
	includeFormat.setForeground(QColor(214, 157, 133));
	/*rule.pattern = QRegularExpression(R"((?=)<[a-zA-Z0-9*._]+>)");*/
	rule.pattern = QRegularExpression(R"((?<=#include)(?:\s*)<[a-zA-Z0-9*._]+>)");
	rule.format = includeFormat;
	highlightingRules.append(rule);

	numbersFormat.setForeground(QColor(156, 220, 254));
	rule.pattern = QRegularExpression(R"(-?\b(0b|0x){0,1}[\d.']+\b)");
	rule.format = numbersFormat;
	highlightingRules.append(rule);
	
	quotationFormat.setForeground(QColor(214, 157, 133));
	rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
	rule.format = quotationFormat;
	highlightingRules.append(rule);
	
	singleLineCommentFormat.setForeground(QColor(78, 163, 58));
	rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);
	
	multiLineCommentFormat.setForeground(QColor(78, 163, 58));
	
	commentStartPattern = QRegularExpression(R"(/\*)");
	commentEndPattern = QRegularExpression(R"(\*/)");
	
	//includeFormat.setForeground(QColor(210, 134, 84));
	//rule.pattern = QRegularExpression(R"(#include\s+([<"][a-zA-Z0-9*._]+[">]))");
	//rule.format = includeFormat;
	//highlightingRules.append(rule);
	//
	//functionFormat.setForeground(QColor(219, 203, 121));
	//rule.pattern = QRegularExpression(R"(\b([A-Za-z0-9_]+(?:\s+|::))*([A-Za-z0-9_]+)(?=\())");
	//rule.format = functionFormat;
	//highlightingRules.append(rule);
	//
	//defTypeFormat.setForeground(QColor(78, 201, 176));
	//rule.pattern = QRegularExpression(R"(\b([A-Za-z0-9_]+)\s+[A-Za-z]{1}[A-Za-z0-9_]+\s*[;=])");
	//rule.format = defTypeFormat;
	//highlightingRules.append(rule);
	//
	//preProcessorFormat.setForeground(Qt::darkGray);
	//rule.pattern = QRegularExpression(R"(#[a-zA-Z_]+)");
	//rule.format = preProcessorFormat;
	//highlightingRules.append(rule);
	//
	//numbersFormat.setForeground(QColor(156, 220, 254));
	//rule.pattern = QRegularExpression(R"(\b(0b|0x){0,1}[\d.']+\b)");
	//rule.format = numbersFormat;
	//highlightingRules.append(rule);
	//
	//stringFormat.setForeground(QColor(210, 134, 84));
	//rule.pattern = QRegularExpression(R"("[^\n"]*")");
	//rule.format = stringFormat;
	//highlightingRules.append(rule);
	//
	//commentFormat.setForeground(QColor(78, 163, 58));
	//rule.pattern = QRegularExpression(R"(//[^\n]*)");
	//rule.format = commentFormat;
	//highlightingRules.append(rule);
	//
	//commentStartPattern = QRegularExpression(R"(/\*)");
	//commentEndPattern = QRegularExpression(R"(\*/)");
}


CppHighlighter::~CppHighlighter()
{
	delete m_braces;
	m_braces = nullptr;
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
		startIndex = text.indexOf(commentStartPattern);

	while (startIndex >= 0) 
	{
		QRegularExpressionMatch match = commentEndPattern.match(text, startIndex);
		int endIndex = match.capturedStart();
		int commentLength = 0;
		if (endIndex == -1) 
		{
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		}
		else 
		{
			commentLength = endIndex - startIndex
				+ match.capturedLength();
		}
		setFormat(startIndex, commentLength, multiLineCommentFormat);
		startIndex = text.indexOf(commentStartPattern, startIndex + commentLength);
	}


	// for each textblock, store its indentation level
	TextBlockData* data = new TextBlockData;





	/*int indentCounter = 0;
	while (text.size() > indentCounter)
	{
		if (text.at(indentCounter).toLatin1() != '\t')
			break;

		++indentCounter;
	}

	if (indentCounter > 1)
		data->indentLevel = indentCounter;*/

	setCurrentBlockUserData(data);
}


const QVector<QPair<QChar, QChar>>* CppHighlighter::braces() const
{
	return m_braces;
}