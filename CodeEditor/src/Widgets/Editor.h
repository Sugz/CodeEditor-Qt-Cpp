#pragma once

#include <QTextEdit>

class BaseHighlighter;
class LineNumberArea;
class FoldingArea;

struct Brackets
{
	int startPos, endPos;
};


class Editor : public QTextEdit
{
	Q_OBJECT

public:
	Editor(QWidget* parent = nullptr);
	~Editor();

	int firstVisibleBlock();
	void setHighlighter(BaseHighlighter*);
	void setFont(const QFont& font);
	float lineHeightMultiplier();
	void setLineHeightMultiplier(float);

protected:
	void resizeEvent(QResizeEvent*) override;
	void paintEvent(QPaintEvent*) override;

	
private Q_SLOTS:
	void updateViewportMargins(int);
	void updateLeftAreas(int);
	void onCursorPositionChanged();
	

private:
	LineNumberArea* m_lineNumberArea;
	FoldingArea* m_foldingArea;
	BaseHighlighter* m_highlighter;
	int m_tabStop;

	float m_lineHeight = 0;
	float m_lineHeightMultiplier;
	int m_minVisibleLines;

	void setConnections();
	
	void highlightCurrentLine(QList<QTextEdit::ExtraSelection>&);
	void highlightBraces(QList<QTextEdit::ExtraSelection>&);
	QRect relativeBlockBoundingRect(QTextBlock&, QRect&, unsigned int minWidth = 0);
	QChar charUnderCursor(int offset = 0) const;
	void setBottomMargin();
	void setStylePrivate();

};
