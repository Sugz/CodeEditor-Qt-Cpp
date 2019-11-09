#include "CodeEditor.h"


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


CodeEditor::CodeEditor(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::CodeEditorClass)
{
	ui->setupUi(this);
	/*ui->mainToolBar->setIconSize(QSize(16, 16));*/
	ui->mainToolBar->setFixedHeight(30);

	QFont font;
	font.setFamily("Consolas");
	font.setFixedPitch(true);
	font.setPointSize(11);

	ui->editor->setFont(font);

	cppHighlighter = new CppHighlighter();
	ui->editor->setHighlighter(cppHighlighter);


	QFile file("src/Widgets/Editor.cpp");
	//QFile file("test.txt");
	if (file.open(QFile::ReadOnly | QFile::Text))
		ui->editor->setPlainText(file.readAll());
}


CodeEditor::~CodeEditor()
{
	delete ui;
}
