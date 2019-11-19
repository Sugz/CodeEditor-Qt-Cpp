#include "FoldingArea.h"
#include "Editor.h"
#include "FoldedTextAttr.h"

#include <QTextBlock>
#include <QPainter>
#include <QAbstractTextDocumentLayout>

#include <QDebug>


FoldingArea::FoldingArea(Editor* parent)
	: QFrame(parent),
	m_editor(parent),
	m_recalculateFolds(true),
	foldedHandler(new FoldedTextAttr(this))
{
	setMouseTracking(true);

	connect(m_editor, &Editor::textChanged, [=] { repaint(); });

	m_editor->document()->documentLayout()->registerHandler(foldedHandler->type(), foldedHandler);
}


QSize FoldingArea::sizeHint() const
{
	if (!m_editor)
		return QWidget::sizeHint();

	return { 23, 0 };
}


void FoldingArea::mousePressEvent(QMouseEvent* e)
{
	bool needRepaint = false;
	for (Fold* fold : m_folds)
	{
		if (fold->arrowRect.contains(e->pos()))
		{
			needRepaint = true;

			if (fold->closed)
			{
				fold->closed = false;
				fold->hovered = true;

				for (int i = fold->start + 1; i <= fold->end - fold->start; i++)
					m_foldedLines.removeAll(i);

				QTextEdit::ExtraSelection selection;
				selection.cursor = m_editor->textCursor();
				selection.cursor.clearSelection();
				selection.cursor.movePosition(QTextCursor::Start);
				selection.cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);

				foldedHandler->unfold(selection.cursor);

				selection.cursor.clearSelection();
				selection.format.setBackground(QColor(45, 45, 50, 128));
				selection.format.setProperty(QTextFormat::FullWidthSelection, true);
				selection.cursor.movePosition(QTextCursor::Start);
				selection.cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, fold->start);
				selection.cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, fold->end - fold->start + 1);

				QList<QTextEdit::ExtraSelection> extraSelections;
				extraSelections.append(selection);
				m_editor->setExtraSelections(extraSelections);
			}
			else
			{
				fold->closed = true;
				fold->hovered = false;

				for (int i = fold->start + 1; i <= fold->end - fold->start; i++)
					m_foldedLines.append(i);

				QTextEdit::ExtraSelection selection;
				selection.cursor = m_editor->textCursor();
				selection.cursor.clearSelection();
				selection.cursor.movePosition(QTextCursor::Start);
				selection.cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, fold->start);
				selection.cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, fold->end - fold->start + 1);
				selection.cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

				foldedHandler->fold(selection.cursor, *fold);

			}
		}
	}

	if (needRepaint)
	{
		m_recalculateFolds = false;
		repaint();
		m_recalculateFolds = true;
	}
}


void FoldingArea::mouseMoveEvent(QMouseEvent* e)
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	for (Fold* fold : m_folds)
	{
		fold->arrowHovered = fold->arrowRect.contains(e->pos());

		if (!fold->closed && fold->hoverRect.contains(e->pos()))
		{
			fold->hovered = true;

			// highlight editor lines
			QTextEdit::ExtraSelection selection;
			selection.format.setBackground(QColor(45, 45, 50, 128));
			selection.format.setProperty(QTextFormat::FullWidthSelection, true);
			selection.cursor = m_editor->textCursor();
			selection.cursor.clearSelection();

			selection.cursor.movePosition(QTextCursor::Start);
			selection.cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, fold->start);
			selection.cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, fold->end - fold->start + 1);

			extraSelections.append(selection);
		}
		else
		{
			fold->hovered = false;
		}
	}

	m_editor->setExtraSelections(extraSelections);
	m_recalculateFolds = false;
	repaint();
	m_recalculateFolds = true;
}


void FoldingArea::getFolds()
{
	QList<Fold*> closedFolds;
	for (Fold* fold : m_folds)
	{
		if (fold->closed)
			closedFolds.append(fold);
	}

	m_folds.clear();
	m_folds = closedFolds;

	QTextBlock block = m_editor->document()->firstBlock();
	QRect editorViewport = m_editor->viewport()->geometry();
	int blockHeight = static_cast<int>(m_editor->document()->documentLayout()->blockBoundingRect(block).height());

	int blockNumber = 0;
	int depth = 0;
	int emptyLinesHeight = 0;
	bool include = false;

	QList<Fold*> currentFolds;

	for (int i = 0; i < m_editor->document()->blockCount(); ++i)
	{
		//qDebug(block.text().toLatin1());

		QRect box = m_editor->relativeBlockBoundingRect(block, editorViewport);

		if (block.text().isEmpty())
		{
			emptyLinesHeight += box.height();
		}

		else if (block.text().startsWith("#include"))
		{
			if (!include)
			{
				include = true;
				QRect hover = box;

				box.setLeft(6);
				box.setTop(box.top() + 3);
				box.setWidth(12);
				box.setHeight(12);

				hover.setLeft(4);
				hover.setWidth(sizeHint().width() - 8);

				Fold* fold = new Fold;
				fold->arrowRect = box;
				fold->hoverRect = hover;
				fold->start = blockNumber;
				fold->end = blockNumber;
				fold->foldType = FoldType::Include;

				currentFolds.append(fold);
				m_folds.append(fold);

			}

			else
			{
				for (Fold* fold : currentFolds)
				{
					fold->hoverRect.setHeight(fold->hoverRect.height() + box.height());
					fold->hoverRect.setHeight(fold->hoverRect.height() + emptyLinesHeight);
					fold->end = blockNumber;
				}

				emptyLinesHeight = 0;
			}
		}

		else
		{
			include = false;
			emptyLinesHeight = 0;
			currentFolds.clear();
		}

		block = block.next();
		++blockNumber;
	}
}


void FoldingArea::paintEvent(QPaintEvent* e)
{
	if (m_recalculateFolds)
		getFolds();
	
	QPainter painter(this);
	for (Fold* fold : m_folds)
	{
		QColor arrowColor = fold->arrowHovered ? QColor(225, 225, 225) : QColor(150, 150, 150);

		qDebug() << "fold hovered: " << fold->hovered << "\n";
		if (fold->hovered)
			painter.fillRect(fold->hoverRect, QColor(255, 255, 255, 50));

		QPointF points[3];
		if (fold->closed)
		{
			points[0] = QPointF(fold->arrowRect.left() + 4, fold->arrowRect.top());
			points[1] = QPointF(fold->arrowRect.right() - 2, fold->arrowRect.top() + 6);
			points[2] = QPointF(fold->arrowRect.left() + 4, fold->arrowRect.bottom());
		}
		else
		{
			points[0] = QPointF(fold->arrowRect.left(), fold->arrowRect.top() + 4);
			points[1] = QPointF(fold->arrowRect.left() + 6, fold->arrowRect.bottom() - 2);
			points[2] = QPointF(fold->arrowRect.right(), fold->arrowRect.top() + 4);
		}

		QPainterPath path;
		path.moveTo(points[0]);
		path.lineTo(points[1]);
		path.lineTo(points[2]);
		path.lineTo(points[0]);
		painter.fillPath(path, arrowColor);

	}

}


//TODO: to detect which fold / unfold box is clicked, each time i draw a box, store it's rect. Then on mouse press, loop over each box to know if it contains the event pos