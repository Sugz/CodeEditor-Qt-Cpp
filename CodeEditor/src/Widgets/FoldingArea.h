#pragma once

#include <QFrame>
#include <QTextEdit>
#include <QTextCursor>

class Editor;
class FoldedTextAttr;
enum class FoldType;

struct Fold
{
	bool closed = false, hovered = false, arrowHovered = false;
	int start, end, offset, foldHeight;//, rectsOffset;
	QRect arrowRect, hoverRect, baseArrowRect, baseHoverRect;
	FoldType foldType;

	static bool comparePtr(const Fold* a, const Fold* b) {
		return ((*a).start < (*b).start);
	}
};

class FoldingArea : public QFrame
{
	Q_OBJECT

public:
	explicit FoldingArea(Editor* parent);

	QSize sizeHint() const override;

protected:
	void paintEvent(QPaintEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;

private:
	friend class LineNumberArea;

	Editor* m_editor;
	FoldedTextAttr* foldedHandler;
	QList<Fold*> m_folds;
	QList<int> m_foldedLines;
	bool m_recalculateFolds;
	void getFolds();
	void updateFoldsOffset();
	void setCursor(QTextCursor& cursor, unsigned int firstLine, unsigned int endLine = 0, bool endOfLine = false, QTextCursor::MoveMode mode = QTextCursor::KeepAnchor);
	void highlightEditorLines(QTextEdit::ExtraSelection&, QList<QTextEdit::ExtraSelection>&, Fold*);
};
