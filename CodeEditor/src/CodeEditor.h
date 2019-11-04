#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CodeEditor.h"
#include "Highlighters/CppHighlighter.h"

class CodeEditor : public QMainWindow
{
	Q_OBJECT

public:
	explicit CodeEditor(QWidget *parent = Q_NULLPTR);
	~CodeEditor();

private:
	Ui::CodeEditorClass* ui;
	CppHighlighter* cppHighlighter;
};
