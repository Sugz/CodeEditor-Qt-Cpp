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
	//m_braces = new std::string("(){}[]<>\"\"\\'\\'");

	this->verticalScrollBar()->setSingleStep(this->fontMetrics().height()); //TODO useless here (since font can change, place it in setFont())

	connect(this->document(), &QTextDocument::blockCountChanged, this, &Editor::updateLineNumberAreaWidth);
	connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &Editor::updateLineNumberArea);
	connect(this, &Editor::textChanged, this, &Editor::updateLineNumberArea);
	connect(this, &Editor::cursorPositionChanged, this, &Editor::onCursorPositionChanged);
	
	updateLineNumberAreaWidth();
}

Editor::~Editor()
{
	delete m_lineNumberArea;
	//delete m_braces;
	m_lineNumberArea = nullptr;
	//m_braces = NULL;
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
	if (m_highlighter && m_highlighter->braces())
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

	const QVector<QPair<QString, QString>>* braces = m_highlighter->braces();

	for (auto& pair : &braces)
	{

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


//void Editor::matchParentheses()
//{
//	TextBlockData* data = static_cast<TextBlockData*>(textCursor().block().userData());
//
//	if (data) 
//	{
//		QVector<ParenthesisInfo*> infos = data->parentheses();
//
//		int pos = textCursor().block().position();
//		for (int i = 0; i < infos.size(); ++i) 
//		{
//			ParenthesisInfo* info = infos.at(i);
//
//			// get the cursor position within the current textblock
//			int curPos = textCursor().positionInBlock();
//			
//			// only check curPos if curPos - 1 isn't a brace
//			if (info->position == curPos || info->position == curPos - 1)
//			{
//				int braceIndex = m_braces->find(info->character);
//				const char* brace = &m_braces->at(braceIndex);
//
//				if (braceIndex != std::string::npos && info->character == *brace)
//				{
//					// check if index is pair or impair
//					bool isPair = braceIndex % 2 == 0;
//
//					//get matching braces
//					const char* matching = &m_braces->at(isPair ? braceIndex + 1 : braceIndex - 1);
//
//					if (isPair && matchLeftParenthesis(textCursor().block(), i + 1, 0, *brace, *matching))
//						createParenthesisSelection(pos + info->position);
//					else if (!isPair && matchRightParenthesis(textCursor().block(), i - 1, 0, *matching, *brace))
//						createParenthesisSelection(pos + info->position);
//				}
//			}
//
//			
//
//
//			/*if ((info->position == curPos || info->position == curPos - 1) 
//				&& info->character == '(') 
//			{
//				if (matchLeftParenthesis(textCursor().block(), i + 1, 0))
//					createParenthesisSelection(pos + info->position);
//			}
//			else if ((info->position == curPos || info->position == curPos - 1) && 
//				info->character == ')') 
//			{
//				if (matchRightParenthesis(textCursor().block(), i - 1, 0))
//					createParenthesisSelection(pos + info->position);
//			}*/
//		}
//	}
//}
//
//
//bool Editor::matchLeftParenthesis(QTextBlock currentBlock, int i, int numLeftParentheses, const char& open, const char& close)
//{
//	TextBlockData* data = static_cast<TextBlockData*>(currentBlock.userData());
//	QVector<ParenthesisInfo*> infos = data->parentheses();
//
//	int docPos = currentBlock.position();
//	for (; i < infos.size(); ++i)
//	{
//		ParenthesisInfo* info = infos.at(i);
//
//		if (info->character == open) 
//		{
//			++numLeftParentheses;
//			continue;
//		}
//
//		if (info->character == close && numLeftParentheses == 0)
//		{
//			createParenthesisSelection(docPos + info->position);
//			return true;
//		}
//		else
//			--numLeftParentheses;
//	}
//
//	currentBlock = currentBlock.next();
//	if (currentBlock.isValid())
//		return matchLeftParenthesis(currentBlock, 0, numLeftParentheses, open, close);
//
//	return false;
//}
//
//
//bool Editor::matchRightParenthesis(QTextBlock currentBlock, int i, int numRightParentheses, const char& open, const char& close)
//{
//	TextBlockData* data = static_cast<TextBlockData*>(currentBlock.userData());
//	QVector<ParenthesisInfo*> parentheses = data->parentheses();
//
//	int docPos = currentBlock.position();
//	for (; i > -1 && parentheses.size() > 0; --i)
//	{
//		ParenthesisInfo* info = parentheses.at(i);
//		if (info->character == close) 
//		{
//			++numRightParentheses;
//			continue;
//		}
//		if (info->character == open && numRightParentheses == 0)
//		{
//			createParenthesisSelection(docPos + info->position);
//			return true;
//		}
//		else
//			--numRightParentheses;
//	}
//
//	currentBlock = currentBlock.previous();
//	if (currentBlock.isValid())
//		return matchRightParenthesis(currentBlock, 0, numRightParentheses, open, close);
//
//	return false;
//}


//bool Editor::matchLeftParenthesis(QTextBlock currentBlock, int i, int numLeftParentheses)
//{
//	TextBlockData* data = static_cast<TextBlockData*>(currentBlock.userData());
//	QVector<ParenthesisInfo*> infos = data->parentheses();
//
//	int docPos = currentBlock.position();
//	for (; i < infos.size(); ++i) 
//	{
//		ParenthesisInfo* info = infos.at(i);
//
//		if (info->character == '(') {
//			++numLeftParentheses;
//			continue;
//		}
//
//		if (info->character == ')' && numLeftParentheses == 0) 
//		{
//			createParenthesisSelection(docPos + info->position);
//			return true;
//		}
//		else
//			--numLeftParentheses;
//	}
//
//	currentBlock = currentBlock.next();
//	if (currentBlock.isValid())
//		return matchLeftParenthesis(currentBlock, 0, numLeftParentheses);
//
//	return false;
//}
//
//
//bool Editor::matchRightParenthesis(QTextBlock currentBlock, int i, int numRightParentheses)
//{
//	TextBlockData* data = static_cast<TextBlockData*>(currentBlock.userData());
//	QVector<ParenthesisInfo*> parentheses = data->parentheses();
//
//	int docPos = currentBlock.position();
//	for (; i > -1 && parentheses.size() > 0; --i) 
//	{
//		ParenthesisInfo* info = parentheses.at(i);
//		if (info->character == ')') {
//			++numRightParentheses;
//			continue;
//		}
//		if (info->character == '(' && numRightParentheses == 0) 
//		{
//			createParenthesisSelection(docPos + info->position);
//			return true;
//		}
//		else
//			--numRightParentheses;
//	}
//
//	currentBlock = currentBlock.previous();
//	if (currentBlock.isValid())
//		return matchRightParenthesis(currentBlock, 0, numRightParentheses);
//
//	return false;
//}


//void Editor::createParenthesisSelection(int pos)
//{
//	QList<QTextEdit::ExtraSelection> selections = extraSelections();
//
//	QTextEdit::ExtraSelection selection;
//	QTextCharFormat format = selection.format;
//	format.setBackground(Qt::green);
//	selection.format = format;
//
//	QTextCursor cursor = textCursor();
//	cursor.setPosition(pos);
//	cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
//	selection.cursor = cursor;
//
//	selections.append(selection);
//
//	setExtraSelections(selections);
//}