#pragma once

#include <QTextEdit>

class BaseHighlighter;
class LineNumberArea;



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
	void resizeEvent(QResizeEvent* event) override;
	
private Q_SLOTS:
	void updateLineNumberAreaWidth(int);
	void onCursorPositionChanged();
	

private:
	LineNumberArea* m_lineNumberArea;
	BaseHighlighter* m_highlighter;
	int m_tabStop;

	float m_lineHeight;
	float m_lineHeightMultiplier;
	int m_minVisibleLines;

	void highlightCurrentLine(QList<QTextEdit::ExtraSelection>&);
	void highlightBraces(QList<QTextEdit::ExtraSelection>&);
	QChar charUnderCursor(int offset = 0) const;
	void setBottomMargin();
	void updateLineNumberArea();
};
