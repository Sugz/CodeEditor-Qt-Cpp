#include "Editor.h"
#include "../Highlighters/CppHighlighter.h"

#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QScrollBar>


Editor::Editor(QWidget* parent) : QTextEdit(parent)
{
	m_lineNumberArea = new LineNumberArea(this);
	m_highlighter = nullptr;

	this->verticalScrollBar()->setSingleStep(this->fontMetrics().height()); //TODO useless here (since font can change, place it in setFont())

	connect(this->document(), &QTextDocument::blockCountChanged, this, &Editor::updateLineNumberAreaWidth);
	connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &Editor::updateLineNumberArea);
	connect(this, &Editor::textChanged, this, &Editor::updateLineNumberArea);
	connect(this, &Editor::cursorPositionChanged, this, &Editor::onCursorPositionChanged);
	
	updateLineNumberAreaWidth();
}

Editor::~Editor()
{
	delete m_highlighter;
	delete m_lineNumberArea;
	
	m_highlighter = nullptr;
	m_lineNumberArea = nullptr;
	
}


int Editor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax(1, document()->blockCount());
	while (max >= 10) 
	{
		max /= 10;
		++digits;
	}

	int space = 20 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
	return space;
}


void Editor::updateLineNumberAreaWidth()
{
	this->setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}


void Editor::updateLineNumberArea()
{
	/*
	 * When the signal is emitted, the sliderPosition has been adjusted according to the action,
	 * but the value has not yet been propagated (meaning the valueChanged() signal was not yet emitted),
	 * and the visual display has not been updated. In slots connected to this signal you can thus safely
	 * adjust any action by calling setSliderPosition() yourself, based on both the action and the
	 * slider's value.
	 */
	 // Make sure the sliderPosition triggers one last time the valueChanged() signal with the actual value !!!!
	this->verticalScrollBar()->setSliderPosition(this->verticalScrollBar()->sliderPosition());

	// Since "QTextEdit" does not have an "updateRequest(...)" signal, we chose
	// to grab the imformations from "sliderPosition()" and "contentsRect()".
	// See the necessary connections used (Class constructor implementation part).

	QRect rect = contentsRect();
	m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
	updateLineNumberAreaWidth();

	int dy = this->verticalScrollBar()->sliderPosition();
	if (dy > -1) 
	{
		m_lineNumberArea->scroll(0, dy);
	}

	// Addjust slider to alway see the number of the currently being edited line...
	// Addjust slider to alway see the number of the currently being edited line...
	int first_block_id = getFirstVisibleBlockId();
	if (first_block_id == 0 || this->textCursor().block().blockNumber() == first_block_id - 1)
		this->verticalScrollBar()->setSliderPosition(dy - this->document()->documentMargin());

}


int Editor::getFirstVisibleBlockId()
{
	// Detect the first block for which bounding rect - once translated 
	// in absolute coordinated - is contained by the editor's text area

	// Costly way of doing but since "blockBoundingGeometry(...)" doesn't 
	// exists for "QTextEdit"...

	QRect r1 = this->viewport()->geometry();
	QTextCursor curs = QTextCursor(this->document());
	curs.movePosition(QTextCursor::Start);

	for (int i = 0; i < this->document()->blockCount(); ++i)
	{
		QTextBlock block = curs.block();
		QRect r2 = this->document()->documentLayout()->blockBoundingRect(block).translated(
			r1.x(), r1.y() - this->verticalScrollBar()->sliderPosition()).toRect();
		
		if (r1.contains(r2, true))
			return i;

		curs.movePosition(QTextCursor::NextBlock);
	}

	return 0;
}


void Editor::lineNumberAreaPaintEvent(QPaintEvent* event)
{
	this->verticalScrollBar()->setSliderPosition(this->verticalScrollBar()->sliderPosition());

	QPainter painter(m_lineNumberArea);
	painter.fillRect(event->rect(), Qt::lightGray);
	int blockNumber = this->getFirstVisibleBlockId();

	QTextBlock block = this->document()->findBlockByNumber(blockNumber);
	QTextBlock prev_block = (blockNumber > 0) ? this->document()->findBlockByNumber(blockNumber - 1) : block;
	int translate_y = (blockNumber > 0) ? -this->verticalScrollBar()->sliderPosition() : 0;

	int top = this->viewport()->geometry().top();

	// Adjust text position according to the previous "non entirely visible" block 
	// if applicable. Also takes in consideration the document's margin offset.
	int additional_margin;
	if (blockNumber == 0)
		// Simply adjust to document's margin
		additional_margin = (int)this->document()->documentMargin() - 1 - this->verticalScrollBar()->sliderPosition();
	else
		// Getting the height of the visible part of the previous "non entirely visible" block
		additional_margin = (int)this->document()->documentLayout()->blockBoundingRect(prev_block)
		.translated(0, translate_y).intersected(this->viewport()->geometry()).height();

	// Shift the starting point
	top += additional_margin;

	int bottom = top + (int)this->document()->documentLayout()->blockBoundingRect(block).height();

	QColor col_1(90, 255, 30);      // Current line (custom green)
	QColor col_0(120, 120, 120);    // Other lines  (custom darkgrey)

	// Draw the numbers (displaying the current line number in green)
	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(QColor(120, 120, 120));
			painter.setPen((this->textCursor().blockNumber() == blockNumber) ? col_1 : col_0);
			painter.drawText(
				-10, 
				top,
				m_lineNumberArea->width(), 
				fontMetrics().height(),
				Qt::AlignRight, 
				number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int)this->document()->documentLayout()->blockBoundingRect(block).height();
		++blockNumber;
	}

}


void Editor::resizeEvent(QResizeEvent* e)
{
	QTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}


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


void Editor::onCursorPositionChanged()
{
	QList<QTextEdit::ExtraSelection> extraSelections;
	highlightCurrentLine(extraSelections);
	//matchParentheses();
	if (m_highlighter && m_highlighter->braces() != 0)
		highlightBraces(extraSelections);

	setExtraSelections(extraSelections);
}


void Editor::highlightCurrentLine(QList<QTextEdit::ExtraSelection>& extraSelections)
{
	if (!isReadOnly()) 
	{
		QTextEdit::ExtraSelection selection;

		QColor lineColor = QColor(Qt::yellow).lighter(160);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	m_lineNumberArea->repaint();
}


void Editor::highlightBraces(QList<QTextEdit::ExtraSelection>& extraSelections)
{
	QChar currentChar = charUnderCursor();
	QChar prevChar = charUnderCursor(-1);
	if (currentChar.isNull() && prevChar.isNull())
		return;

	const QVector<QPair<QChar, QChar>>* braces = m_highlighter->braces();
	int direction, position = textCursor().position();
	QChar brace, matchingBrace;

	for (auto& pair : *braces)
	{
		if (pair.first == currentChar)
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
			QTextEdit::ExtraSelection selection;
			QTextCharFormat format = selection.format;
			format.setBackground(Qt::green);
			selection.format = format;

			QTextCursor::MoveOperation directionEnum =
				direction < 0 ?
				QTextCursor::MoveOperation::Left :
				QTextCursor::MoveOperation::Right;

			selection.cursor = textCursor();
			selection.cursor.clearSelection();
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


QChar Editor::charUnderCursor(int offset) const
{
	int index = textCursor().positionInBlock() + offset;
	QString blockText = textCursor().block().text();

	if (index < 0 || index >= blockText.size())
		return {};
	
	return blockText[index];
}
