#pragma once

#include <QWidget>

class Editor;

class LineNumberArea : public QWidget
{
	Q_OBJECT

public:
	explicit LineNumberArea(Editor *parent);

	// Disable copying
	LineNumberArea(const LineNumberArea&) = delete;
	LineNumberArea& operator=(const LineNumberArea&) = delete;

	QSize sizeHint() const override;

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	Editor* m_editor;

};
