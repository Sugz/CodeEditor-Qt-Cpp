#pragma once

#include "../Highlighters/BaseHighlighter.h"

#include <QTextEdit>
#include <QWidget>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QSize>


class Editor : public QTextEdit
{
	Q_OBJECT

public:
	Editor(QWidget* parent = nullptr);
	~Editor();

	void setHighlighter(BaseHighlighter*);

	void lineNumberAreaPaintEvent(QPaintEvent* event);  //TODO: private ?
	int lineNumberAreaWidth();  //TODO: private ?
	int getFirstVisibleBlockId(); //TODO: private ?

	void setFont(const QFont& font);
	float lineHeightMultiplier();
	void setLineHeightMultiplier(float);

protected:
	void resizeEvent(QResizeEvent* event) override;
	
	

private Q_SLOTS:
	void updateLineNumberAreaWidth();
	void updateLineNumberArea();
	void onCursorPositionChanged();
	

private:
	QWidget* m_lineNumberArea;
	BaseHighlighter* m_highlighter;
	int m_tabStop;

	float m_lineHeight;
	float m_lineHeightMultiplier;
	int m_minVisibleLines;
	bool updateLineHeight;

	void highlightCurrentLine(QList<QTextEdit::ExtraSelection>&);
	void highlightBraces(QList<QTextEdit::ExtraSelection>&);
	QChar charUnderCursor(int offset = 0) const;
	void setBottomMargin();
};


class LineNumberArea : public QWidget
{
public:
	LineNumberArea(Editor* editor) : QWidget(editor) 
	{
		m_editor = editor;
	}

	QSize sizeHint() const override 
	{
		return QSize(m_editor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent* e) override 
	{
		m_editor->lineNumberAreaPaintEvent(e);
	}

private:
	Editor* m_editor;
};

