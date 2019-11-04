#include "CodeEditor.h"


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


CodeEditor::CodeEditor(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::CodeEditorClass)
{
	ui->setupUi(this);


	QFont font;
	font.setFamily("Consolas");
	font.setFixedPitch(true);
	font.setPointSize(10);

	ui->editor->setFont(font);

	cppHighlighter = new CppHighlighter();
	ui->editor->setHighlighter(cppHighlighter);


	QFile file("src/Widgets/Editor.cpp");
	if (file.open(QFile::ReadOnly | QFile::Text))
		ui->editor->setPlainText(file.readAll());
}


CodeEditor::~CodeEditor()
{
	delete ui;
}
