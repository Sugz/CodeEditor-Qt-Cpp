#pragma once

#include <QObject>
#include <QTextObjectInterface>

struct Fold;

enum class FoldType 
{
	Function, 
	Include, 
	SingleLineComment, 
	MultiLineComment, 
	Region
};


class FoldedTextAttr : 
	public QObject, public QTextObjectInterface
{
	Q_OBJECT
	Q_INTERFACES(QTextObjectInterface)

public:
	explicit FoldedTextAttr(QObject* parent = nullptr);
	
	static int type() { return QTextFormat::UserObject + 2; }
	static int prop() { return 2; }

	QSizeF intrinsicSize(QTextDocument* doc, int posInDocument, const QTextFormat& format);
	void drawObject(QPainter* painter, const QRectF& rect, QTextDocument* doc, int posInDocument, const QTextFormat& format);

	void fold(QTextCursor, Fold&);

	bool unfold(QTextCursor);

private:
	QString m_text;
};
