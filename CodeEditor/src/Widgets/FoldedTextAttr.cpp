#include "FoldedTextAttr.h"
#include "FoldingArea.h"

#include <QFontMetrics>
#include <QPainter>
#include <QTextDocument>
#include <QTextDocumentFragment>

#include <memory>

FoldedTextAttr::FoldedTextAttr(QObject* parent) : QObject(parent) 
{
}


QSizeF FoldedTextAttr::intrinsicSize(QTextDocument* doc, int posInDocument, const QTextFormat& format) 
{
	Q_UNUSED(doc)
	Q_UNUSED(posInDocument)
	Q_ASSERT(format.type() == format.CharFormat);

	const QTextCharFormat& tf = reinterpret_cast<const QTextCharFormat&>(format);

	QFont fn = tf.font();
	QFontMetrics fm(fn);

	QSizeF sz = fm.boundingRect(m_text).size();
	return sz;
}


void FoldedTextAttr::drawObject(QPainter* painter, const QRectF& rect, QTextDocument* doc, int posInDocument, const QTextFormat& format) 
{
	Q_UNUSED(doc)
	Q_UNUSED(posInDocument)
	Q_ASSERT(format.type() == format.CharFormat);

	QRectF r = rect;
	r.setHeight(r.height() - 1);

	painter->setPen(QColor(128, 128, 128));
	painter->drawRect(r);
	painter->drawText(rect, m_text);
	
}


Q_DECLARE_METATYPE(QTextDocumentFragment)


void FoldedTextAttr::fold(QTextCursor cursor, Fold& fold) 
{
	switch (fold.foldType)
	{
	case FoldType::Function:
		m_text = " { ... }  ";
		break;
	case FoldType::Include:
		m_text = " #include ...  ";
		break;
	case FoldType::MultiLineComment:
		m_text = " /* ... */  ";
		break;
	case FoldType::SingleLineComment:
		m_text = " // ...  ";
		break;
	case FoldType::Region:
		m_text = " region " + fold.text + "  ";
		break;
	default:
		break;
	}

	QTextCharFormat textCharFormat;
	textCharFormat.setObjectType(type());
	QVariant variant; 
	variant.setValue(cursor.selection());
	textCharFormat.setProperty(prop(), variant);
	cursor.insertText(QString(QChar::ObjectReplacementCharacter), textCharFormat);
}




bool FoldedTextAttr::unfold(QTextCursor cursor) 
{
	if (!cursor.hasSelection())
	{
		QTextCharFormat textCharFormat = cursor.charFormat();
		if (textCharFormat.objectType() == type()) 
		{
			cursor.movePosition(cursor.Left, cursor.KeepAnchor);
			QVariant variant = textCharFormat.property(prop());
			QTextDocumentFragment q = variant.value<QTextDocumentFragment>();
			cursor.insertFragment(q);
			return true;
		}
	}
	return false;
}