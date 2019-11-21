#include "FoldingArea.h"
#include "Editor.h"
#include "FoldedTextAttr.h"

#include <QTextBlock>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QDebug>

#include <algorithm>


FoldingArea::FoldingArea(Editor* parent)
	: QFrame(parent),
	m_editor(parent),
	m_recalculateFolds(true),
	foldedHandler(new FoldedTextAttr(this))
{
	setMouseTracking(true);

	connect(m_editor, &Editor::textChanged, [=] { getFolds(); });

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
			m_recalculateFolds = false;
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
				selection.cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, fold->start - fold->offset);
				selection.cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);

				foldedHandler->unfold(selection.cursor);

				selection.cursor.clearSelection();
				selection.format.setBackground(QColor(45, 45, 50, 128));
				selection.format.setProperty(QTextFormat::FullWidthSelection, true);
				selection.cursor.movePosition(QTextCursor::Start);
				selection.cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, fold->start);
				selection.cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, fold->end - fold->start + 1 - fold->offset);

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
				selection.cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, fold->start - fold->offset);
				selection.cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, fold->end - fold->start);
				selection.cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

				foldedHandler->fold(selection.cursor, *fold);

			}

			updateFoldsOffset();
			m_recalculateFolds = true;
			break;
		}
	}

	if (needRepaint)
	{
		repaint();

		/*m_recalculateFolds = false;
		repaint();
		m_recalculateFolds = true;*/
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
			selection.cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, fold->start - fold->offset);
			selection.cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, fold->end - fold->start + 1 - fold->offset);

			extraSelections.append(selection);
		}
		else
		{
			fold->hovered = false;
		}
	}

	m_editor->setExtraSelections(extraSelections);
	/*m_recalculateFolds = false;*/
	repaint();
	/*m_recalculateFolds = true;*/
}


void FoldingArea::getFolds()
{
	if (!m_recalculateFolds)
		return;

	qDebug() << "getFolds" << "\n";

	QList<Fold*> closedFolds;
	for (Fold* fold : m_folds)
	{
		if (fold->closed)
		{
			closedFolds.append(fold);
		}
		else
		{
			delete fold;
		}
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

	int regionDepth = 0;

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
				/*fold->arrowRect = box;
				fold->hoverRect = hover;*/
				fold->baseArrowRect = box;
				fold->baseHoverRect = hover;
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
					fold->baseHoverRect.setHeight(fold->baseHoverRect.height() + box.height());
					fold->baseHoverRect.setHeight(fold->baseHoverRect.height() + emptyLinesHeight);
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

	updateFoldsOffset();
	repaint();
}


void FoldingArea::updateFoldsOffset()
{
	//qDebug() << "updateFoldsOffset" << "\n";

	// sort the fold list based on their start attribute
	std::sort(m_folds.begin(), m_folds.end(), Fold::comparePtr);

	// store the real start of the fold
	// for each closed fold, increment offset from the number of lines removed from the closed folds
	// store the offset on the fold
	int offset = 0, rectOffset = 0;
	for (Fold* fold : m_folds)
	{
		//qDebug() << "Fold start: " << fold->start << "\n";

		fold->offset = offset;


		//fold->rectsOffset = rectOffset;

		QRect hoverRect = fold->baseHoverRect;
		hoverRect.setTop(hoverRect.top() - rectOffset);
		hoverRect.setHeight(fold->baseHoverRect.height());
		fold->hoverRect = hoverRect;

		QRect arrowRect = fold->baseArrowRect;
		arrowRect.setTop(arrowRect.top() - rectOffset);
		arrowRect.setHeight(fold->baseArrowRect.height());
		fold->arrowRect = arrowRect;

		if (fold->closed)
		{
			offset += fold->end - fold->start;
			rectOffset += fold->foldHeight;
			//qDebug() << "offset: " << offset << "\n";
		}
		else
		{
			// get the height of the qtextblock that will be removed when folding
			QTextBlock block;
			int blockHeight = 0;
			for (int i = fold->start + 1; i <= fold->end; ++i)
			{
				block = m_editor->document()->findBlockByNumber(i);
				blockHeight += static_cast<int>(m_editor->document()->documentLayout()->blockBoundingRect(block).height());
			}
			fold->foldHeight = blockHeight;
			//qDebug() << "Fold height: " << fold->foldHeight << "\n";
		}
	}
}


void FoldingArea::paintEvent(QPaintEvent* e)
{
	/*if (m_recalculateFolds)
		getFolds();*/
	
	QPainter painter(this);
	for (Fold* fold : m_folds)
	{
		QColor arrowColor = fold->arrowHovered ? QColor(225, 225, 225) : QColor(150, 150, 150);

		//qDebug() << "fold hovered: " << fold->hovered << "\n";
		if (fold->hovered)
		{
			/*QRect hoverRect = fold->hoverRect;
			hoverRect.setTop(hoverRect.top() - fold->rectsOffset);
			hoverRect.setHeight(fold->hoverRect.height());
			painter.fillRect(hoverRect, QColor(255, 255, 255, 50)); */
			painter.fillRect(fold->hoverRect, QColor(255, 255, 255, 50));
		}

		/*QRect arrowRect = fold->arrowRect;
		arrowRect.setTop(arrowRect.top() - fold->rectsOffset);
		arrowRect.setHeight(fold->arrowRect.height());*/
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

		/*if (fold->closed)
		{
			points[0] = QPointF(arrowRect.left() + 4, arrowRect.top());
			points[1] = QPointF(arrowRect.right() - 2, arrowRect.top() + 6);
			points[2] = QPointF(arrowRect.left() + 4, arrowRect.bottom());
		}
		else
		{
			points[0] = QPointF(arrowRect.left(), arrowRect.top() + 4);
			points[1] = QPointF(arrowRect.left() + 6, arrowRect.bottom() - 2);
			points[2] = QPointF(arrowRect.right(), arrowRect.top() + 4);
		}*/

		QPainterPath path;
		path.moveTo(points[0]);
		path.lineTo(points[1]);
		path.lineTo(points[2]);
		path.lineTo(points[0]);
		painter.fillPath(path, arrowColor);

	}

}


void FoldingArea::setCursor(QTextCursor& cursor, int firstLine, unsigned int endLine, bool endOfLine)
{
	cursor.clearSelection();
	cursor.movePosition(QTextCursor::Start);
	cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, firstLine);

	if (endLine != 0)
		cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, endLine);

	if (endOfLine)
		cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
}


//TODO: to detect which fold / unfold box is clicked, each time i draw a box, store it's rect. Then on mouse press, loop over each box to know if it contains the event pos