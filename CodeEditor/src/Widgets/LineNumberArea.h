#pragma once

#include <QFrame>

class Editor;

class LineNumberArea : public QFrame
{
	Q_OBJECT

public:
	explicit LineNumberArea(Editor *parent);

	// Disable copying
	LineNumberArea(const LineNumberArea&) = delete;
	LineNumberArea& operator=(const LineNumberArea&) = delete;

	QSize sizeHint() const override;

protected:
	void paintEvent(QPaintEvent*) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

private:
	Editor* m_editor;
	QPoint m_dragStartPos;

};
