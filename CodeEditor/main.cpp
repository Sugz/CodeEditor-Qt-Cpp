#include "src/CodeEditor.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QFile File("Resources/StyleSheetDark.qss");
	File.open(QFile::ReadOnly);
	QString StyleSheet = QLatin1String(File.readAll());
	qApp->setStyleSheet(StyleSheet);


	CodeEditor w;
	w.show();
	return a.exec();
}
