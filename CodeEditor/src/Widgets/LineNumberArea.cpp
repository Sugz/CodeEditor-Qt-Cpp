#include "LineNumberArea.h"
#include "Editor.h"

// Qt
#include <QTextEdit>
#include <QPainter>
#include <QPaintEvent>
#include <QTextBlock>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>
#include <QDebug>


LineNumberArea::LineNumberArea(Editor* parent)
	: QFrame(parent),
	m_editor(parent),
	m_dragStartPos(QPoint())
{
}


QSize LineNumberArea::sizeHint() const
{
	if (!m_editor)
		return QWidget::sizeHint();

	// Calculating width
	int digits = 1;
	int max = qMax(1, m_editor->document()->blockCount());
	while (max >= 10)
	{
		max /= 10;
		++digits;
	}

	int space = 30 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

	return { space, 0 };
}


void LineNumberArea::mousePressEvent(QMouseEvent* e)
{
	m_dragStartPos = e->pos();
	QTextCursor cursor = m_editor->cursorForPosition(m_dragStartPos);
	
	cursor.movePosition(QTextCursor::StartOfLine);
	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
	m_editor->setTextCursor(cursor);
}


void LineNumberArea::mouseMoveEvent(QMouseEvent* e)
{
	// get the new position for the cursor
	QTextCursor cursor = m_editor->cursorForPosition(e->pos());
	int pos = cursor.position();

	// put the cursor back before mouse start moving
	cursor = m_editor->cursorForPosition(m_dragStartPos);

	if (e->pos().y() < m_dragStartPos.y())
	{
		cursor.movePosition(QTextCursor::EndOfLine);
		cursor.setPosition(pos, QTextCursor::KeepAnchor);
		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
		/*cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);*/
	}
	else
	{
		cursor.movePosition(QTextCursor::StartOfLine);
		cursor.setPosition(pos, QTextCursor::KeepAnchor);
		cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
		cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
	}

	m_editor->setTextCursor(cursor);
}


void LineNumberArea::mouseReleaseEvent(QMouseEvent* e)
{
	m_dragStartPos = QPoint();
}


void LineNumberArea::paintEvent(QPaintEvent* e)
{
	QPainter painter(this);

	// Clearing rect to update
	/*QColor background = palette().color(QPalette::Background);
	painter.fillRect(e->rect(), background);*/

	int blockNumber = m_editor->firstVisibleBlock();
	QTextBlock block = m_editor->document()->findBlockByNumber(blockNumber);
	int top = static_cast<int>(m_editor->document()->documentLayout()->blockBoundingRect(block).translated(0, -m_editor->verticalScrollBar()->value()).top());
	int bottom = top + static_cast<int>(m_editor->document()->documentLayout()->blockBoundingRect(block).height());

	QColor currentLine(31, 165, 255);
	QColor selectedLines(200, 200, 200);
	QColor otherLines(120, 120, 120);

	QFont font = m_editor->font();
	font.setPointSize(font.pointSize() - 1);
	painter.setFont(font);
	
	int selectionStart = m_editor->textCursor().selectionStart();
	int selectionEnd = m_editor->textCursor().selectionEnd();

	while (block.isValid() && top <= e->rect().bottom())
	{
		if (block.isVisible() && bottom >= e->rect().top())
		{
			QString number = QString::number(blockNumber + 1);

			bool isCurrentLine = m_editor->textCursor().blockNumber() == blockNumber;
			bool isSelected = block.position() >= selectionStart && block.position() <= selectionEnd;
			painter.setPen(isCurrentLine ? currentLine : (isSelected ? selectedLines : otherLines));

			painter.drawText(
				-15,
				top + 1,
				sizeHint().width(),
				m_editor->fontMetrics().height() + 1,
				Qt::AlignRight,
				number
			);
		}

		block = block.next();
		top = bottom;
		bottom = top + static_cast<int>(m_editor->document()->documentLayout()->blockBoundingRect(block).height());
		++blockNumber;
	}
}
