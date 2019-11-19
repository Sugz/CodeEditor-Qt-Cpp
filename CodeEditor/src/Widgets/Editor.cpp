#include "Editor.h"
#include "LineNumberArea.h"
#include "FoldingArea.h"
#include "FoldedTextAttr.h"
#include "../Highlighters/BaseHighlighter.h"

#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QScrollBar>
#include <QApplication>
#include <QDebug>
#include <QCommonStyle>

#include <vector>
#include <iostream>
#include <iomanip>



Editor::Editor(QWidget* parent) 
	: QTextEdit(parent),
	m_lineNumberArea(new LineNumberArea(this)),
	m_foldingArea(new FoldingArea(this)),
	m_highlighter(nullptr),
	m_tabStop(4),
	m_lineHeightMultiplier(1),
	m_minVisibleLines(5)
{

	//connect(this, &Editor::textChanged, this, &Editor::updateLineNumberArea);
	//TODO if i don't need to connect textChanged & updateLineNumberArea, I can try to connect to setLineSpacing ?

	//TODO: store the first visible block and update it when scrollbarvalue change->  m_lineNumberArea->update() & paint indented lines

	//document()->documentLayout()->registerHandler(foldedHandler->type(), foldedHandler);

	setConnections();
	document()->setDocumentMargin(0);
	setStylePrivate();
}


Editor::~Editor()
{
	delete m_highlighter;
	delete m_lineNumberArea;
	
	m_highlighter = nullptr;
	m_lineNumberArea = nullptr;
	
}


void Editor::setConnections()
{
	// update the viewport margins as the width of the linenumberArea may change
	connect(document(), &QTextDocument::blockCountChanged, this, &Editor::updateViewportMargins);

	// force the left areas to repaint when scroll vertically
	connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &Editor::updateLeftAreas);

	// update the extra selections when the cursor is moved or there is an active selection
	connect(this, &Editor::cursorPositionChanged, this, &Editor::onCursorPositionChanged);
	connect(this, &Editor::selectionChanged, this, &Editor::onCursorPositionChanged);
}


void Editor::resizeEvent(QResizeEvent* e)
{
	QTextEdit::resizeEvent(e);

	setBottomMargin();

	QRect cr = contentsRect();
	m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), m_lineNumberArea->sizeHint().width(), cr.height()));
	m_foldingArea->setGeometry(QRect(cr.left() + m_lineNumberArea->sizeHint().width(), cr.top(), m_foldingArea->sizeHint().width(), cr.height()));
}


//TODO use previous technique, for empty lines, check next one instead
void Editor::paintEvent(QPaintEvent* e)
{
	QTextEdit::paintEvent(e);

	QPainter painter(viewport());
	painter.setPen(QColor(100, 100, 100));

	QTextBlock block = document()->firstBlock();

	
	while (block.isValid())
	{


		block = block.next();
	}


	/*QTextBlock block = document()->findBlockByNumber(firstVisibleBlock());
	int top = static_cast<int>(document()->documentLayout()->blockBoundingRect(block).translated(0, -verticalScrollBar()->value()).top());
	int blockHeight = static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
	int bottom = top + blockHeight;

	QVector<int> openBrackets;
	QVector<int> closeBrackets;

	

	while (block.isValid() && top <= e->rect().bottom())
	{
		if (block.isVisible() && bottom >= e->rect().top())
		{
			int blockPosition = block.position();
			int position = 0;
			QChar c;
		}

		block = block.next();
		top = bottom;
		bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
	}*/

	/*QPainter painter(viewport());
	painter.setPen(QColor(100, 100, 100));

	QTextBlock block = document()->findBlockByNumber(firstVisibleBlock());
	int top = static_cast<int>(document()->documentLayout()->blockBoundingRect(block).translated(0, -verticalScrollBar()->value()).top());
	int blockHeight = static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
	int bottom = top + blockHeight;

	//QVector<Brackets> brackets;
	QVector<int> openBrackets;
	QVector<int> closeBrackets;

	while (block.isValid() && top <= e->rect().bottom())
	{
		if (block.isVisible() && bottom >= e->rect().top())
		{
			// run over each char in the block and find brackets.
			// only keep unmacthed brackets
			int blockPosition = block.position();
			int position = 0;
			QChar c;
			while (block.contains(position + blockPosition))
			{
				c = block.text()[position];
				if (c == '{')
				{
					openBrackets.append(position + blockPosition);
				}
				else if (c == '}')
				{
					if (openBrackets.size() != 0)
						openBrackets.removeLast();
					else
						closeBrackets.append(position + blockPosition);
				}

				++position;
			}

			// draw unmatched open brackets 
			//if (openBrackets.size() != 0)
			//{
			//	int position = openBrackets[0];
			//	int counter = openBrackets.size();
			//	QTextBlock nextBlock = block.next();
			//	while (counter != 0 && position < document()->characterCount() - 1)
			//	{
			//		//++position;
			//		int relativePosition = 0;
			//		while (nextBlock.contains(position))
			//		{
			//			c = block.text()[relativePosition];
			//			if (c == '{')
			//			{
			//				++counter;
			//			}
			//			else if (c == '}')
			//			{
			//				--counter;

			//			}
			//		}
			//	}
			//	

			//	
			//}

			// draw unmacthed close brackets
			if (closeBrackets.size() != 0)
			{
				for (int bracket : closeBrackets)
				{
					QString slice = block.text().left(bracket);
					int tabsCount = slice.count('\t');
					slice = slice.remove('\t');
					int x = fontMetrics().width(slice) + tabStopWidth() * tabsCount + fontMetrics().averageCharWidth() / 2;

					QRect r1 = viewport()->geometry();
					painter.drawLine(x, 0, x, relativeBlockBoundingRect(block, r1).y());
				}
			}

			
			//int closeBracketPos = block.text().indexOf('}');
			//if (closeBracketPos != -1)
			//{
			//	QString slice = block.text().left(closeBracketPos);
			//	int tabsCount = slice.count('\t');
			//	slice = slice.remove('\t');
			//	int x = fontMetrics().width(slice) + tabStopWidth() * tabsCount + fontMetrics().averageCharWidth() / 2;
			//
			//	QRect r1 = viewport()->geometry();
			//	painter.drawLine(x, 0, x, relativeBlockBoundingRect(block, r1).y());
			//}
		}

		block = block.next();
		top = bottom;
		bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
	}*/

	/*QPainter painter(viewport());
	painter.setPen(QColor(180, 180, 180));

	QTextBlock block = document()->findBlockByNumber(firstVisibleBlock());
	int top = static_cast<int>(document()->documentLayout()->blockBoundingRect(block).translated(0, -verticalScrollBar()->value()).top());
	int blockHeight = static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
	int bottom = top + blockHeight;

	//TODO: don't draw line before finding a brace ?

	while (block.isValid() && top <= e->rect().bottom())
	{
		if (block.isVisible() && bottom >= e->rect().top())
		{
			//only draw line if there is a leats two block that require that indentation level
			TextBlockData* data = static_cast<TextBlockData*>(block.userData());
			// show indent lines when block text is empty is previous and next lines are indented
			if (block.text().size() == 0)
			{
				QTextBlock previousBlock = block.previous();
				//while(previousBlock.text().size() == 0)
				//	previousBlock = previousBlock.previous();

				QTextBlock nextBlock = block.next();
				while (nextBlock.text().size() == 0)
					nextBlock = nextBlock.next();

				TextBlockData* previousData = static_cast<TextBlockData*>(previousBlock.userData());
				TextBlockData* nextData = static_cast<TextBlockData*>(nextBlock.userData());

				for (int i = 1; i < previousData->indentLevel; i++)
				{
					if (i < nextData->indentLevel)
					{
						int x = tabStopWidth() * i + fontMetrics().averageCharWidth() / 2;
						painter.drawLine(x, top, x, bottom);
					}
				}
			}

			if (data && data->indentLevel > 0)
			{
				for (int i = 1; i < data->indentLevel; i++)
				{
					int x = tabStopWidth() * i + fontMetrics().averageCharWidth() / 2;
					painter.drawLine(x, top, x, bottom);
				}
			}
		}

		block = block.next();
		top = bottom;
		bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
	}*/
}


#pragma region Left Areas

void Editor::updateLeftAreas(int)
{
	m_lineNumberArea->update();
	m_foldingArea->update();
}


void Editor::updateViewportMargins(int)
{
	int width = m_lineNumberArea->sizeHint().width() +
		m_foldingArea->sizeHint().width();

	setViewportMargins(width, 0, 150, 0);

	/*QScrollBar* hScrollBar = horizontalScrollBar();
	hScrollBar->setMaximumWidth(viewport()->width());
	hScrollBar->move(width, 0);*/
}

#pragma endregion


#pragma region Highlight current line and braces

void Editor::onCursorPositionChanged()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (textCursor().selectionStart() == textCursor().selectionEnd())
	{
		highlightCurrentLine(extraSelections);
		if (m_highlighter && m_highlighter->braces() != 0)
			highlightBraces(extraSelections);
	}

	setExtraSelections(extraSelections);
	m_lineNumberArea->update();
}


void Editor::highlightCurrentLine(QList<QTextEdit::ExtraSelection>& extraSelections)
{
	if (!isReadOnly())
	{
		QTextEdit::ExtraSelection selection;

		selection.format.setBackground(QColor(45, 45, 50));
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}
}

//TODO: bug with }) if I click bewteen })
// not buggy anymore, but it doesn't take the correct char (here '}') because the ')' is match before in the for loop
void Editor::highlightBraces(QList<QTextEdit::ExtraSelection>& extraSelections)
{
	QChar currentChar = charUnderCursor();
	QChar prevChar = charUnderCursor(-1);
	if (currentChar.isNull() && prevChar.isNull())
		return;

	QTextEdit::ExtraSelection selection;
	selection.format.setBackground(QColor(255, 0, 0, 100));

	const QVector<QPair<QChar, QChar>>* braces = m_highlighter->braces();
	int direction, position = textCursor().position();
	QChar brace, matchingBrace;
	bool inverted = false;

	for (auto& pair : *braces)
	{
		// if cursor is in the middle of open and close brace, 
		// no need to search for anything and highlight both prevChar and currentChar
		if (pair.first == prevChar && pair.second == currentChar)
		{
			selection.cursor = textCursor();
			selection.cursor.clearSelection();
			selection.cursor.movePosition(
				QTextCursor::MoveOperation::Left,
				QTextCursor::MoveMode::MoveAnchor,
				1
			);

			selection.cursor.movePosition(
				QTextCursor::MoveOperation::Right,
				QTextCursor::MoveMode::KeepAnchor,
				2
			);

			extraSelections.append(selection);

			return;
		}
		else if (pair.first == currentChar)
		{
			direction = 1;
			brace = currentChar;
			matchingBrace = pair.second;
		}
		else if (pair.second == prevChar)
		{
			direction = -1;
			brace = prevChar;
			matchingBrace = pair.first;
			position--;
		}
		else if (pair.first == prevChar)
		{
			direction = 1;
			inverted = true;
			brace = prevChar;
			matchingBrace = pair.second;
		}
		else if (pair.second == currentChar)
		{
			direction = -1;
			inverted = true;
			brace = currentChar;
			matchingBrace = pair.first;
			position--;
		}
		else
		{
			continue;
		}

		// run through the text in the given direction and find the matching brace
		int counter = 1;
		QChar c;
		while (counter != 0 &&
			position > 0 &&
			position < document()->characterCount() - 1)
		{
			// move position and get the char
			position += direction;
			c = document()->characterAt(position);

			// check if there are other brace of the same type
			if (c == brace)
				++counter;
			else if (c == matchingBrace)
				--counter;
		}

		if (counter == 0)
		{
			QTextCursor::MoveOperation directionEnum =
				direction < 0 ?
				QTextCursor::MoveOperation::Left :
				QTextCursor::MoveOperation::Right;

			selection.cursor = textCursor();
			selection.cursor.clearSelection();
			int pos = std::abs(textCursor().position() - position);
			selection.cursor.movePosition(
				directionEnum,
				QTextCursor::MoveMode::MoveAnchor,
				std::abs(textCursor().position() - position)
			);

			selection.cursor.movePosition(
				QTextCursor::MoveOperation::Right,
				QTextCursor::MoveMode::KeepAnchor,
				1
			);

			extraSelections.append(selection);

			// highlight selected char
			if (inverted)
			{
				directionEnum = directionEnum ==
					QTextCursor::MoveOperation::Left ?
					QTextCursor::MoveOperation::Right :
					QTextCursor::MoveOperation::Left;
			}

			selection.cursor = textCursor();
			selection.cursor.clearSelection();
			selection.cursor.movePosition(
				directionEnum,
				QTextCursor::MoveMode::KeepAnchor,
				1
			);

			extraSelections.append(selection);
		}

		break;
	}

}

#pragma endregion


#pragma region Helpers

int Editor::firstVisibleBlock()
{
	// Detect the first block for which bounding rect - once translated 
	// in absolute coordinated - is contained by the editor's text area

	// Costly way of doing but since "blockBoundingGeometry(...)" doesn't 
	// exists for "QTextEdit"...

	QTextCursor curs = QTextCursor(document());
	curs.movePosition(QTextCursor::Start);

	QRect r1 = viewport()->geometry();

	for (int i = 0; i < document()->blockCount(); ++i)
	{
		QTextBlock block = curs.block();
		
		// if the block doesn't contain text, relativeBlockBoundingRect.width() = 0
		// and r1.intersects(relativeBlockBoundingRect) return false
		// so force the relativeBlockBoundingRect width to be at least 1
		if (r1.intersects(relativeBlockBoundingRect(block, r1, 1)))
			return i;

		curs.movePosition(QTextCursor::NextBlock);
	}

	return 0;
}


QRect Editor::relativeBlockBoundingRect(QTextBlock& block, QRect& viewportRect, unsigned int minWidth)
{
	QRect r = document()
		->documentLayout()
		->blockBoundingRect(block)
		.translated(
			viewportRect.x(),
			viewportRect.y() - verticalScrollBar()->sliderPosition()
		).toRect();

	if (minWidth > 0 && r.width() < minWidth)
		r.setWidth(minWidth);

	return r;
}


QChar Editor::charUnderCursor(int offset) const
{
	int index = textCursor().positionInBlock() + offset;
	QString blockText = textCursor().block().text();

	if (index < 0 || index >= blockText.size())
		return {};

	return blockText[index];
}

#pragma endregion


void Editor::setHighlighter(BaseHighlighter* highlighter)
{
	if (m_highlighter)
		m_highlighter->setDocument(nullptr);

	m_highlighter = highlighter;
	if (m_highlighter)
	{
		m_highlighter->setDocument(document());
	}
}


float Editor::lineHeightMultiplier()
{
	return m_lineHeightMultiplier;
}


void Editor::setLineHeightMultiplier(float multiplier)
{
	m_lineHeightMultiplier = multiplier;
}


void Editor::setFont(const QFont& font)
{
	QWidget::setFont(font);

	verticalScrollBar()->setSingleStep(fontMetrics().height() * 2);
	QTextEdit::setTabStopWidth(m_tabStop * fontMetrics().width(' '));
	m_lineHeight = static_cast<float>(fontMetrics().height()) * m_lineHeightMultiplier;
	setBottomMargin();
}


void Editor::setBottomMargin()
{
	QTextFrameFormat format = document()->rootFrame()->frameFormat();
	format.setBottomMargin(static_cast<int>(
		contentsRect().height() - (m_minVisibleLines + 1) * fontMetrics().height()));

	document()->rootFrame()->setFrameFormat(format);
}


void Editor::setStylePrivate()
{
	// disable selection color to keep syntax highlighting when a selection occurs
	QPalette p = palette();
	p.setBrush(QPalette::HighlightedText, QBrush(Qt::NoBrush));
	setPalette(p);

	//remove scrollbars style
	verticalScrollBar()->setStyle(new QCommonStyle());
	horizontalScrollBar()->setStyle(new QCommonStyle());

	//remove wrapping
	setLineWrapMode(QTextEdit::NoWrap);
}
