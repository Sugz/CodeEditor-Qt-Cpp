#include "Editor.h"
#include "LineNumberArea.h"
#include "../Highlighters/CppHighlighter.h"

#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QScrollBar>
#include <QApplication>
#include <QDebug>


Editor::Editor(QWidget* parent) :
	QTextEdit(parent),
	m_lineNumberArea(new LineNumberArea(this)),
	m_highlighter(nullptr),
	m_tabStop(4),
	m_lineHeightMultiplier(1),
	m_minVisibleLines(5)
{
	connect(document(), &QTextDocument::blockCountChanged, this, &Editor::updateLineNumberAreaWidth);
	connect(verticalScrollBar(), &QScrollBar::valueChanged, [this](int) { m_lineNumberArea->update(); });
	connect(this, &Editor::cursorPositionChanged, this, &Editor::onCursorPositionChanged);

	//connect(this, &Editor::textChanged, this, &Editor::updateLineNumberArea);
	//TODO if i don't need to connect textChanged & updateLineNumberArea, I can try to connect to setLineSpacing ?

}


Editor::~Editor()
{
	delete m_highlighter;
	delete m_lineNumberArea;
	
	m_highlighter = nullptr;
	m_lineNumberArea = nullptr;
	
}


void Editor::resizeEvent(QResizeEvent* e)
{
	QTextEdit::resizeEvent(e);

	setBottomMargin();
	updateLineNumberArea();
}


#pragma region LineNumberArea

void Editor::updateLineNumberAreaWidth(int)
{
	setViewportMargins(m_lineNumberArea->sizeHint().width(), 0, 0, 0);
}


void Editor::updateLineNumberArea()
{
	QRect cr = contentsRect();
	m_lineNumberArea->setGeometry(
		QRect(cr.left(),
			cr.top(),
			m_lineNumberArea->sizeHint().width(),
			cr.height()
		)
	);
}

#pragma endregion


#pragma region Highlight current line and braces

void Editor::onCursorPositionChanged()
{
	QList<QTextEdit::ExtraSelection> extraSelections;
	highlightCurrentLine(extraSelections);
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
	bool inverted = false;

	for (auto& pair : *braces)
	{
		// if cursor is in the middle of open and close brace, 
		// no need to search for anything and highlight both prevChar and currentChar
		if (pair.first == prevChar && pair.second == currentChar)
		{
			QTextEdit::ExtraSelection selection;
			selection.format.setBackground(Qt::green);

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
			selection.format.setBackground(Qt::green);


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
		QRect r2 = document()
			->documentLayout()
			->blockBoundingRect(block)
			.translated(
				r1.x(),
				r1.y() - verticalScrollBar()->sliderPosition()
			).toRect();

		if (r1.intersects(r2))
			return i;

		curs.movePosition(QTextCursor::NextBlock);
	}

	return 0;
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

	verticalScrollBar()->setSingleStep(fontMetrics().height());
	QTextEdit::setTabStopWidth(m_tabStop * fontMetrics().width(' '));
	m_lineHeight = static_cast<float>(fontMetrics().height()) * m_lineHeightMultiplier;
	setBottomMargin();
}


void Editor::setBottomMargin()
{
	int maxVisibleLines = contentsRect().height() / m_lineHeight;
	QTextFrameFormat format = document()->rootFrame()->frameFormat();
	format.setBottomMargin((maxVisibleLines - m_minVisibleLines) * m_lineHeight);
	document()->rootFrame()->setFrameFormat(format);
}
