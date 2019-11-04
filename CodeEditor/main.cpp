#include "src/CodeEditor.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	CodeEditor w;
	w.show();
	return a.exec();
}
