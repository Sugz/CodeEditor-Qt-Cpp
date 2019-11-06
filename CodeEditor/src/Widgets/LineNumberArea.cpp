#include "LineNumberArea.h"
#include "Editor.h"

// Qt
#include <QTextEdit>
#include <QPainter>
#include <QPaintEvent>
#include <QTextBlock>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>

LineNumberArea::LineNumberArea(Editor *parent)
	: QWidget(parent),
	m_editor(parent)
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

	int space = 20 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

	return { space, 0 };
}


void LineNumberArea::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);

	// Clearing rect to update
	painter.fillRect(event->rect(), Qt::lightGray);

	int blockNumber = m_editor->firstVisibleBlock();
	QTextBlock block = m_editor->document()->findBlockByNumber(blockNumber);
	int top = static_cast<int>(m_editor->document()->documentLayout()->blockBoundingRect(block).translated(0, -m_editor->verticalScrollBar()->value()).top());
	int bottom = top + static_cast<int>(m_editor->document()->documentLayout()->blockBoundingRect(block).height());

	QColor currentLine(90, 255, 30);      // Current line (custom green)
	QColor otherLines(120, 120, 120);    // Other lines  (custom darkgrey)

	painter.setFont(m_editor->font());

	while (block.isValid() && top <= event->rect().bottom())
	{
		if (block.isVisible() && bottom >= event->rect().top())
		{
			QString number = QString::number(blockNumber + 1);

			auto isCurrentLine = m_editor->textCursor().blockNumber() == blockNumber;
			painter.setPen(isCurrentLine ? currentLine : otherLines);

			painter.drawText(
				-10,
				top,
				sizeHint().width(),
				m_editor->fontMetrics().height(),
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
