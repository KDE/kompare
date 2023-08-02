/*
    SPDX-FileCopyrightText: 2001-2004, 2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2004 Jeff Snyder <jeff@caffeinated.me.uk>
    SPDX-FileCopyrightText: 2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPARELISTVIEW_H
#define KOMPARELISTVIEW_H

#include <QHash>
#include <QLabel>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QFrame>
#include <QTreeWidget>
#include <QStyledItemDelegate>

namespace KompareDiff2 {
class DiffModel;
class DiffHunk;
class Difference;
class DifferenceString;
}
class ViewSettings;
class KompareSplitter;
class KompareListView;
class KompareListViewItem;
class KompareListViewItemDelegate;
class KompareListViewDiffItem;
class KompareListViewLineContainerItem;

class KompareListView : public QTreeWidget
{
    Q_OBJECT

    friend class KompareListViewItemDelegate;

public:
    KompareListView(bool isSource, ViewSettings* settings, QWidget* parent, const char* name = nullptr);
    ~KompareListView() override;

    KompareListViewItem* itemAtIndex(int i);
    int                  firstVisibleDifference();
    int                  lastVisibleDifference();
    QRect                itemRect(int i);
    int                  minScrollId();
    int                  maxScrollId();
    int                  contentsHeight();
    int                  contentsWidth();
    int                  visibleHeight();
    int                  visibleWidth();
    int                  contentsX();
    int                  contentsY();
    int                  nextPaintOffset() const;
    void                 setNextPaintOffset(int offset);

    bool                 isSource() const { return m_isSource; };
    ViewSettings*        settings() const { return m_settings; };

    void setSelectedDifference(const KompareDiff2::Difference* diff, bool scroll);

public Q_SLOTS:
    void slotSetSelection(const KompareDiff2::DiffModel* model, const KompareDiff2::Difference* diff);
    void slotSetSelection(const KompareDiff2::Difference* diff);
    void setXOffset(int x);
    void scrollToId(int id);
    int  scrollId();
    void slotApplyDifference(bool apply);
    void slotApplyAllDifferences(bool apply);
    void slotApplyDifference(const KompareDiff2::Difference* diff, bool apply);
    void copyTextSelectionToClipboard();
    void unselectAllText();

Q_SIGNALS:
    void differenceClicked(const KompareDiff2::Difference* diff);
    void applyDifference(bool apply);
    void resized();
    void textSelectionChanged();

protected:
    void wheelEvent(QWheelEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override {};
    void mouseMoveEvent(QMouseEvent* me) override;

private:
    QRect totalVisualItemRect(QTreeWidgetItem* item);
    KompareListViewDiffItem* diffItemAt(const QPoint& pos);
    void renumberLines();

    QList<KompareListViewDiffItem*> m_items;
    QHash<const KompareDiff2::Difference*, KompareListViewDiffItem*> m_itemDict;
    bool                              m_isSource;
    ViewSettings*                     m_settings;
    int                               m_scrollId;
    const KompareDiff2::DiffModel*           m_selectedModel;
    const KompareDiff2::Difference*          m_selectedDifference;
    int                               m_nextPaintOffset;
    int m_textSelectionStartLine = 0;
    int m_textSelectionStartPos = 0;
};

class KompareListViewFrame : public QFrame
{
    Q_OBJECT

public:
    KompareListViewFrame(bool isSource, ViewSettings* settings, KompareSplitter* parent, const char* name = nullptr);
    ~KompareListViewFrame() override {};
    KompareListView* view() { return &m_view; };

public Q_SLOTS:
    void slotSetModel(const KompareDiff2::DiffModel* model);

private:
    KompareListView      m_view;
    QLabel               m_label;
    QVBoxLayout          m_layout;
};

class KompareListViewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit KompareListViewItemDelegate(QObject* parent);
    ~KompareListViewItemDelegate() override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

class KompareListViewItem : public QTreeWidgetItem
{
public:
    KompareListViewItem(KompareListView* parent, int type);
    KompareListViewItem(KompareListView* parent, KompareListViewItem* after, int type);
    KompareListViewItem(KompareListViewItem* parent, int type);
    KompareListViewItem(KompareListViewItem* parent, KompareListViewItem* after, int type);

    virtual void paintCell(QPainter* p, const QStyleOptionViewItem& option, int column);

    void repaint();
    int height() const;
    void setHeight(int h);
    int paintHeight() const;
    int paintOffset() const;
    bool isCurrent() const;
    int scrollId() const { return m_scrollId; };

    virtual int maxHeight() = 0;

    KompareListView* kompareListView() const;

    virtual void onTextSelectionChanged(int /*startIndex*/, int /*startPos*/, int /*lastIndex*/, int /*lastPos*/){};
    virtual QString getSelectedText() const {return QString();}

    enum ListViewItemType { Diff = QTreeWidgetItem::UserType + 1, Container = QTreeWidgetItem::UserType + 2, Line = QTreeWidgetItem::UserType + 3, Blank = QTreeWidgetItem::UserType + 4, Hunk = QTreeWidgetItem::UserType + 5 };

private:
    int     m_scrollId;
    int     m_height;
    int     m_paintHeight;
    int     m_paintOffset;
};

class KompareListViewDiffItem : public KompareListViewItem
{
public:
    KompareListViewDiffItem(KompareListView* parent, KompareDiff2::Difference* difference);
    KompareListViewDiffItem(KompareListView* parent, KompareListViewItem* after, KompareDiff2::Difference* difference);
    ~KompareListViewDiffItem() override;

    void applyDifference(bool apply);

    KompareDiff2::Difference* difference() { return m_difference; };

    int maxHeight() override;

private:
    void init();
    void setVisibility();

private:
    KompareDiff2::Difference* m_difference;
    KompareListViewLineContainerItem* m_sourceItem;
    KompareListViewLineContainerItem* m_destItem;
};

class KompareListViewLineItem;
class KompareListViewBlankLineItem;

class KompareListViewLineContainerItem : public KompareListViewItem
{
public:
    KompareListViewLineContainerItem(KompareListViewDiffItem* parent, bool isSource);
    ~KompareListViewLineContainerItem() override;

    int maxHeight() override { return 0; }

    KompareListViewDiffItem* diffItemParent() const;

private:
    int lineCount() const;
    int lineNumber() const;
    KompareDiff2::DifferenceString* lineAt(int i) const;

private:
    KompareListViewBlankLineItem* m_blankLineItem;
    bool m_isSource;
};

class KompareListViewLineItem : public KompareListViewItem
{
public:
    KompareListViewLineItem(KompareListViewLineContainerItem* parent, int line, KompareDiff2::DifferenceString* text);
    KompareListViewLineItem(KompareListViewLineContainerItem* parent, int line, KompareDiff2::DifferenceString* text, int type);
    ~KompareListViewLineItem() override;

    int maxHeight() override { return 0; }

    void paintCell(QPainter* p, const QStyleOptionViewItem& option, int column) override;

    KompareListViewDiffItem* diffItemParent() const;

    void onTextSelectionChanged(int startIndex, int startPos, int lastIndex, int lastPos) override;
    QString getSelectedText() const override;

protected:
    virtual void paintText(QPainter* p, const QColor& bg, int column, int width, int align);

private:
    void init(int line, KompareDiff2::DifferenceString* text);
    void expandTabs(QString& text, int tabstop, int startPos = 0) const;

private:
    KompareDiff2::DifferenceString* m_text;

    int m_selectionFirstCharacter = 0;
    int m_selectionLastCharacter = 0;

    QFont m_font;
};

class KompareListViewBlankLineItem : public KompareListViewLineItem
{
public:
    explicit KompareListViewBlankLineItem(KompareListViewLineContainerItem* parent);

protected:
    void paintText(QPainter* p, const QColor& bg, int column, int width, int align) override;
};

class KompareListViewHunkItem : public KompareListViewItem
{
public:
    KompareListViewHunkItem(KompareListView* parent, KompareDiff2::DiffHunk* hunk, bool zeroHeight = false);
    KompareListViewHunkItem(KompareListView* parent, KompareListViewItem* after, KompareDiff2::DiffHunk* hunk, bool zeroHeight = false);
    ~KompareListViewHunkItem() override;

    void paintCell(QPainter* p, const QStyleOptionViewItem& option, int column) override;

    int maxHeight() override;

    void onTextSelectionChanged(int startIndex, int startPos, int lastIndex, int lastPos) override;
    QString getSelectedText() const override;

private:
    bool             m_zeroHeight;
    KompareDiff2::DiffHunk* m_hunk;

    int m_selectionFirstCharacter = 0;
    int m_selectionLastCharacter = 0;

    QFont m_font;
};

#endif
