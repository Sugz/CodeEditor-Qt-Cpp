#include "FoldingArea.h"
#include "Editor.h"


FoldingArea::FoldingArea(Editor* parent)
	: QFrame(parent),
	m_editor(parent)
{
}


QSize FoldingArea::sizeHint() const
{
	if (!m_editor)
		return QWidget::sizeHint();

	return { 24, 0 };
}


//TODO: to detect which fold / unfold box is clicked, each time i draw a box, store it's rect. Thenon mouse press, loop over each box to know if it contains the event pos