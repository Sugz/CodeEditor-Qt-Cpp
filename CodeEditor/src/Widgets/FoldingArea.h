#pragma once

#include <QFrame>

class Editor;

class FoldingArea : public QFrame
{
	Q_OBJECT

public:
	explicit FoldingArea(Editor* parent);
	

	QSize sizeHint() const override;

private:
	Editor* m_editor;
};
