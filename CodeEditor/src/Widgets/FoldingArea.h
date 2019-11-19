#pragma once

#include <QFrame>

class Editor;
class FoldedTextAttr;
enum class FoldType;

struct Fold
{
	bool closed = false, hovered = false, arrowHovered = false;
	int start, end;
	QRect arrowRect, hoverRect;
	FoldType foldType;
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
	
};
