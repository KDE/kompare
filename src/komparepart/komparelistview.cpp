/*
    SPDX-FileCopyrightText: 2001-2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2004 Jeff Snyder <jeff@caffeinated.me.uk>
    SPDX-FileCopyrightText: 2007-2012 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "komparelistview.h"

#include <QStyle>
#include <QPainter>
#include <QRegExp>
#include <QTimer>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>

#include <KSharedConfig>

#include <libkomparediff2/diffmodel.h>
#include <libkomparediff2/diffhunk.h>
#include <libkomparediff2/difference.h>
#include <libkomparediff2/komparemodellist.h>

#include <komparepartdebug.h>
#include "viewsettings.h"
#include "komparesplitter.h"

#define COL_LINE_NO      0
#define COL_MAIN         1

#define BLANK_LINE_HEIGHT 3
#define HUNK_LINE_HEIGHT  5

#define ITEM_MARGIN 3

using namespace Diff2;

KompareListViewFrame::KompareListViewFrame(bool isSource,
                                           ViewSettings* settings,
                                           KompareSplitter* parent,
                                           const char* name):
    QFrame(parent),
    m_view(isSource, settings, this, name),
    m_label(isSource ? QStringLiteral("Source") : QStringLiteral("Dest"), this),
    m_layout(this)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    m_label.setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
    QFrame* bottomLine = new QFrame(this);
    bottomLine->setFrameShape(QFrame::HLine);
    bottomLine->setFrameShadow(QFrame::Plain);
    bottomLine->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
    bottomLine->setFixedHeight(1);
    m_label.setMargin(3);
    m_layout.setSpacing(0);
    m_layout.setContentsMargins(0, 0, 0, 0);
    m_layout.addWidget(&m_label);
    m_layout.addWidget(bottomLine);
    m_layout.addWidget(&m_view);

    connect(&m_view, &KompareListView::differenceClicked,
            parent, &KompareSplitter::slotDifferenceClicked);

    connect(parent, &KompareSplitter::scrollViewsToId, &m_view, &KompareListView::scrollToId);
    connect(parent, &KompareSplitter::setXOffset, &m_view, &KompareListView::setXOffset);
    connect(&m_view, &KompareListView::resized, parent, &KompareSplitter::slotUpdateScrollBars);
}

void KompareListViewFrame::slotSetModel(const DiffModel* model)
{
    if (model)
    {
        if (view()->isSource()) {
            if (!model->sourceRevision().isEmpty())
                m_label.setText(model->sourceFile() + QLatin1String(" (") + model->sourceRevision() + QLatin1Char(')'));
            else
                m_label.setText(model->sourceFile());
        } else {
            if (!model->destinationRevision().isEmpty())
                m_label.setText(model->destinationFile() + QLatin1String(" (") + model->destinationRevision() + QLatin1Char(')'));
            else
                m_label.setText(model->destinationFile());
        }
    } else {
        m_label.setText(QString());
    }
}

KompareListView::KompareListView(bool isSource,
                                 ViewSettings* settings,
                                 QWidget* parent, const char* name) :
    QTreeWidget(parent),
    m_isSource(isSource),
    m_settings(settings),
    m_scrollId(-1),
    m_selectedModel(nullptr),
    m_selectedDifference(nullptr)
{
    setObjectName(QLatin1String(name));
    setItemDelegate(new KompareListViewItemDelegate(this));
    setHeaderHidden(true);
    setColumnCount(3);   // Line Number, Main, Blank
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setIndentation(0);
    setFrameStyle(QFrame::NoFrame);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::NoFocus);
    setFont(m_settings->m_font);
    setFocusProxy(parent->parentWidget());
}

KompareListView::~KompareListView()
{
    m_settings = nullptr;
    m_selectedModel = nullptr;
    m_selectedDifference = nullptr;
}

KompareListViewItem* KompareListView::itemAtIndex(int i)
{
    return m_items[ i ];
}

int KompareListView::firstVisibleDifference()
{
    QTreeWidgetItem* item = itemAt(QPoint(0, 0));

    if (item == nullptr)
    {
        qCDebug(KOMPAREPART) << "no item at viewport coordinates (0,0)" ;
    }

    while (item) {
        KompareListViewLineItem* lineItem = dynamic_cast<KompareListViewLineItem*>(item);
        if (lineItem && lineItem->diffItemParent()->difference()->type() != Difference::Unchanged)
            break;
        item = itemBelow(item);
    }

    if (item)
        return m_items.indexOf(((KompareListViewLineItem*)item)->diffItemParent());

    return -1;
}

int KompareListView::lastVisibleDifference()
{
    QTreeWidgetItem* item = itemAt(QPoint(0, visibleHeight() - 1));

    if (item == nullptr)
    {
        qCDebug(KOMPAREPART) << "no item at viewport coordinates (0," << visibleHeight() - 1 << ")" ;
        // find last item
        item = itemAt(QPoint(0, 0));
        if (item) {
            QTreeWidgetItem* nextItem = item;
            do {
                item = nextItem;
                nextItem = itemBelow(item);
            } while (nextItem);
        }
    }

    while (item) {
        KompareListViewLineItem* lineItem = dynamic_cast<KompareListViewLineItem*>(item);
        if (lineItem && lineItem->diffItemParent()->difference()->type() != Difference::Unchanged)
            break;
        item = itemAbove(item);
    }

    if (item)
        return m_items.indexOf(((KompareListViewLineItem*)item)->diffItemParent());

    return -1;
}

QRect KompareListView::totalVisualItemRect(QTreeWidgetItem* item)
{
    QRect total = visualItemRect(item);
    int n = item->childCount();
    for (int i = 0; i < n; ++i) {
        QTreeWidgetItem* child = item->child(i);
        if (!child->isHidden())
            total = total.united(totalVisualItemRect(child));
    }
    return total;
}

QRect KompareListView::itemRect(int i)
{
    QTreeWidgetItem* item = itemAtIndex(i);
    return totalVisualItemRect(item);
}

int KompareListView::minScrollId()
{
    return visibleHeight() / 2;
}

int KompareListView::maxScrollId()
{
    int n = topLevelItemCount();
    if (!n) return 0;
    KompareListViewItem* item = (KompareListViewItem*)topLevelItem(n - 1);
    int maxId = item->scrollId() + item->maxHeight() - minScrollId();
    qCDebug(KOMPAREPART) << "Max ID = " << maxId ;
    return maxId;
}

int KompareListView::contentsHeight()
{
    return verticalScrollBar()->maximum() + viewport()->height() - style()->pixelMetric(QStyle::PM_ScrollBarExtent);
}

int KompareListView::contentsWidth()
{
    return (columnWidth(COL_LINE_NO) + columnWidth(COL_MAIN));
}

int KompareListView::visibleHeight()
{
    return viewport()->height();
}

int KompareListView::visibleWidth()
{
    return viewport()->width();
}

int KompareListView::contentsX()
{
    return horizontalOffset();
}

int KompareListView::contentsY()
{
    return verticalOffset();
}

int KompareListView::nextPaintOffset() const
{
    return m_nextPaintOffset;
}

void KompareListView::setNextPaintOffset(int offset)
{
    m_nextPaintOffset = offset;
}

void KompareListView::setXOffset(int x)
{
    qCDebug(KOMPAREPART) << "SetXOffset : Scroll to x position: " << x ;
    horizontalScrollBar()->setValue(x);
}

void KompareListView::scrollToId(int id)
{
//     qCDebug(KOMPAREPART) << "ScrollToID : Scroll to id : " << id ;
    int n = topLevelItemCount();
    KompareListViewItem* item = nullptr;
    if (n) {
        int i = 1;
        for (; i < n; ++i) {
            if (((KompareListViewItem*)topLevelItem(i))->scrollId() > id)
                break;
        }
        item = (KompareListViewItem*)topLevelItem(i - 1);
    }

    if (item) {
        QRect rect = totalVisualItemRect(item);
        int pos = rect.top() + verticalOffset();
        int itemId = item->scrollId();
        int height = rect.height();
        double r = (double)(id - itemId) / (double)item->maxHeight();
        int y = pos + (int)(r * (double)height) - minScrollId();
//         qCDebug(KOMPAREPART) << "scrollToID: " ;
//         qCDebug(KOMPAREPART) << "            id = " << id ;
//         qCDebug(KOMPAREPART) << "           pos = " << pos ;
//         qCDebug(KOMPAREPART) << "        itemId = " << itemId ;
//         qCDebug(KOMPAREPART) << "             r = " << r ;
//         qCDebug(KOMPAREPART) << "        height = " << height ;
//         qCDebug(KOMPAREPART) << "         minID = " << minScrollId() ;
//         qCDebug(KOMPAREPART) << "             y = " << y ;
//         qCDebug(KOMPAREPART) << "contentsHeight = " << contentsHeight() ;
//         qCDebug(KOMPAREPART) << "         c - y = " << contentsHeight() - y ;
        verticalScrollBar()->setValue(y);
    }

    m_scrollId = id;
}

int KompareListView::scrollId()
{
    if (m_scrollId < 0)
        m_scrollId = minScrollId();
    return m_scrollId;
}

void KompareListView::setSelectedDifference(const Difference* diff, bool scroll)
{
    qCDebug(KOMPAREPART) << "KompareListView::setSelectedDifference(" << diff << ", " << scroll << ")" ;

    // When something other than a click causes this function to be called,
    // it'll only get called once, and all is simple.
    //
    // When the user clicks on a diff, this function will get called once when
    // komparesplitter::slotDifferenceClicked runs, and again when the
    // setSelection signal from the modelcontroller arrives.
    //
    // the first call (which will always be from the splitter) will have
    // scroll==false, and the second call will bail out here.
    // Which is why clicking on a difference does not cause the listviews to
    // scroll.
    if (m_selectedDifference == diff)
        return;

    m_selectedDifference = diff;

    KompareListViewItem* item = m_itemDict[ diff ];
    if (!item) {
        qCDebug(KOMPAREPART) << "KompareListView::slotSetSelection(): couldn't find our selection!" ;
        return;
    }

    // why does this not happen when the user clicks on a diff? see the comment above.
    if (scroll)
        scrollToId(item->scrollId());
    setUpdatesEnabled(false);
    int x = horizontalScrollBar()->value();
    int y = verticalScrollBar()->value();
    setCurrentItem(item);
    horizontalScrollBar()->setValue(x);
    verticalScrollBar()->setValue(y);
    setUpdatesEnabled(true);
}

void KompareListView::slotSetSelection(const Difference* diff)
{
    qCDebug(KOMPAREPART) << "KompareListView::slotSetSelection( const Difference* diff )" ;

    setSelectedDifference(diff, true);
}

void KompareListView::slotSetSelection(const DiffModel* model, const Difference* diff)
{
    qCDebug(KOMPAREPART) << "KompareListView::slotSetSelection( const DiffModel* model, const Difference* diff )" ;

    if (m_selectedModel && m_selectedModel == model) {
        slotSetSelection(diff);
        return;
    }

    clear();
    m_items.clear();
    m_itemDict.clear();
    m_selectedModel = model;

    DiffHunkListConstIterator hunkIt = model->hunks()->begin();
    DiffHunkListConstIterator hEnd   = model->hunks()->end();

    KompareListViewItem* item = nullptr;
    m_nextPaintOffset = 0;

    for (; hunkIt != hEnd; ++hunkIt)
    {
        if (item)
            item = new KompareListViewHunkItem(this, item, *hunkIt, model->isBlended());
        else
            item = new KompareListViewHunkItem(this, *hunkIt, model->isBlended());

        DifferenceListConstIterator diffIt = (*hunkIt)->differences().begin();
        DifferenceListConstIterator dEnd   = (*hunkIt)->differences().end();

        for (; diffIt != dEnd; ++diffIt)
        {
            item = new KompareListViewDiffItem(this, item, *diffIt);

            int type = (*diffIt)->type();

            if (type != Difference::Unchanged)
            {
                m_items.append((KompareListViewDiffItem*)item);
                m_itemDict.insert(*diffIt, (KompareListViewDiffItem*)item);
            }
        }
    }

    resizeColumnToContents(COL_LINE_NO);
    resizeColumnToContents(COL_MAIN);

    slotSetSelection(diff);
}

KompareListViewDiffItem* KompareListView::diffItemAt(const QPoint& pos)
{
    KompareListViewItem* item = static_cast<KompareListViewItem*>(itemAt(pos));
    if (!item)
        return nullptr;
    switch (item->type()) {
        case KompareListViewItem::Hunk:
            if (item->paintHeight()) return nullptr;  // no diff item here
            // zero height (fake 1 pixel height), so a diff item shines through
            return static_cast<KompareListViewDiffItem*>(itemBelow(item));
        case KompareListViewItem::Line:
        case KompareListViewItem::Blank:
            return static_cast<KompareListViewLineItem*>(item)->diffItemParent();
        case KompareListViewItem::Container:
            return static_cast<KompareListViewLineContainerItem*>(item)->diffItemParent();
        case KompareListViewItem::Diff:
            return static_cast<KompareListViewDiffItem*>(item);
        default:
            return nullptr;
    }
}

void KompareListView::mousePressEvent(QMouseEvent* e)
{
    QPoint vp = e->pos();
    KompareListViewDiffItem* diffItem = diffItemAt(vp);
    if (diffItem && diffItem->difference()->type() != Difference::Unchanged) {
        Q_EMIT differenceClicked(diffItem->difference());
    }
}

void KompareListView::mouseDoubleClickEvent(QMouseEvent* e)
{
    QPoint vp = e->pos();
    KompareListViewDiffItem* diffItem = diffItemAt(vp);
    if (diffItem && diffItem->difference()->type() != Difference::Unchanged) {
        // FIXME: make a new signal that does both
        Q_EMIT differenceClicked(diffItem->difference());
        Q_EMIT applyDifference(!diffItem->difference()->applied());
    }
}

void KompareListView::renumberLines()
{
//     qCDebug(KOMPAREPART) << "Begin" ;
    unsigned int newLineNo = 1;
    if (!topLevelItemCount()) return;
    KompareListViewItem* item = (KompareListViewItem*)topLevelItem(0);
    while (item) {
//         qCDebug(KOMPAREPART) << "type: " << item->type() ;
        if (item->type() != KompareListViewItem::Container
                && item->type() != KompareListViewItem::Blank
                && item->type() != KompareListViewItem::Hunk)
        {
//             qCDebug(KOMPAREPART) << QString::number( newLineNo ) ;
            item->setText(COL_LINE_NO, QString::number(newLineNo++));
        }
        item = (KompareListViewItem*)itemBelow(item);
    }
}

void KompareListView::slotApplyDifference(bool apply)
{
    m_itemDict[ m_selectedDifference ]->applyDifference(apply);
    // now renumber the line column if this is the destination
    if (!m_isSource)
        renumberLines();
}

void KompareListView::slotApplyAllDifferences(bool apply)
{
    QHash<const Diff2::Difference*, KompareListViewDiffItem*>::ConstIterator it = m_itemDict.constBegin();
    QHash<const Diff2::Difference*, KompareListViewDiffItem*>::ConstIterator end = m_itemDict.constEnd();
    for (; it != end; ++it)
        it.value()->applyDifference(apply);

    // now renumber the line column if this is the destination
    if (!m_isSource)
        renumberLines();
    update();
}

void KompareListView::slotApplyDifference(const Difference* diff, bool apply)
{
    m_itemDict[ diff ]->applyDifference(apply);
    // now renumber the line column if this is the destination
    if (!m_isSource)
        renumberLines();
}

void KompareListView::wheelEvent(QWheelEvent* e)
{
    e->ignore(); // we want the parent to catch wheel events
}

void KompareListView::resizeEvent(QResizeEvent* e)
{
    QTreeWidget::resizeEvent(e);
    Q_EMIT resized();
}

KompareListViewItemDelegate::KompareListViewItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

KompareListViewItemDelegate::~KompareListViewItemDelegate()
{
}

void KompareListViewItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    int column = index.column();
    QStyleOptionViewItem changedOption = option;
    if (column == COL_LINE_NO)
        changedOption.displayAlignment = Qt::AlignRight;
    KompareListViewItem* item = static_cast<KompareListViewItem*>(static_cast<KompareListView*>(parent())->itemFromIndex(index));
    item->paintCell(painter, changedOption, column);
}

QSize KompareListViewItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    KompareListViewItem* item = static_cast<KompareListViewItem*>(static_cast<KompareListView*>(parent())->itemFromIndex(index));
    QSize hint = QStyledItemDelegate::sizeHint(option, index);
    return QSize(hint.width() + ITEM_MARGIN, item->height());
}

KompareListViewItem::KompareListViewItem(KompareListView* parent, int type)
    : QTreeWidgetItem(parent, type),
      m_scrollId(0),
      m_height(0),
      m_paintHeight(0),
      m_paintOffset(parent->nextPaintOffset())
{
//     qCDebug(KOMPAREPART) << "Created KompareListViewItem with scroll id " << m_scrollId ;
}

KompareListViewItem::KompareListViewItem(KompareListView* parent, KompareListViewItem* after, int type)
    : QTreeWidgetItem(parent, after, type),
      m_scrollId(after->scrollId() + after->maxHeight()),
      m_height(0),
      m_paintHeight(0),
      m_paintOffset(parent->nextPaintOffset())
{
//     qCDebug(KOMPAREPART) << "Created KompareListViewItem with scroll id " << m_scrollId ;
}

KompareListViewItem::KompareListViewItem(KompareListViewItem* parent, int type)
    : QTreeWidgetItem(parent, type),
      m_scrollId(0),
      m_height(0),
      m_paintHeight(0),
      m_paintOffset(parent->kompareListView()->nextPaintOffset())
{
}

KompareListViewItem::KompareListViewItem(KompareListViewItem* parent, KompareListViewItem* /*after*/, int type)
    : QTreeWidgetItem(parent, type),
      m_scrollId(0),
      m_height(0),
      m_paintHeight(0),
      m_paintOffset(parent->kompareListView()->nextPaintOffset())
{
}

int KompareListViewItem::height() const
{
    return m_height;
}

void KompareListViewItem::setHeight(int h)
{
    m_height = m_paintHeight = h;
    // QTreeWidget doesn't like zero height, fudge around it.
    m_height -= m_paintOffset;
    if (m_height <= 0) {
        kompareListView()->setNextPaintOffset(1 - m_height);
        m_height = 1;
    } else kompareListView()->setNextPaintOffset(0);
}

int KompareListViewItem::paintHeight() const
{
    return m_paintHeight;
}

int KompareListViewItem::paintOffset() const
{
    return m_paintOffset;
}

bool KompareListViewItem::isCurrent() const
{
    return treeWidget()->currentItem() == this;
}

KompareListView* KompareListViewItem::kompareListView() const
{
    return (KompareListView*)treeWidget();
}

void KompareListViewItem::paintCell(QPainter* p, const QStyleOptionViewItem& option, int column)
{
    // Default implementation for zero-height items.
    // We have to paint the item which shines through or we'll end up with glitches.
    KompareListViewItem* nextItem = (KompareListViewItem*)kompareListView()->itemBelow(this);
    if (nextItem) {
        QStyleOptionViewItem changedOption = option;
        changedOption.rect.translate(0, height());
        nextItem->paintCell(p, changedOption, column);
    }
}

KompareListViewDiffItem::KompareListViewDiffItem(KompareListView* parent, Difference* difference)
    : KompareListViewItem(parent, Diff),
      m_difference(difference),
      m_sourceItem(nullptr),
      m_destItem(nullptr)
{
    init();
}

KompareListViewDiffItem::KompareListViewDiffItem(KompareListView* parent, KompareListViewItem* after, Difference* difference)
    : KompareListViewItem(parent, after, Diff),
      m_difference(difference),
      m_sourceItem(nullptr),
      m_destItem(nullptr)
{
    init();
}

KompareListViewDiffItem::~KompareListViewDiffItem()
{
    m_difference = nullptr;
}

void KompareListViewDiffItem::init()
{
    setHeight(0);
    setExpanded(true);
    int nextPaintOffset = kompareListView()->nextPaintOffset();
    m_destItem = new KompareListViewLineContainerItem(this, false);
    kompareListView()->setNextPaintOffset(nextPaintOffset);
    m_sourceItem = new KompareListViewLineContainerItem(this, true);
    setVisibility();
}

void KompareListViewDiffItem::setVisibility()
{
    m_sourceItem->setHidden(!(kompareListView()->isSource() || m_difference->applied()));
    m_destItem->setHidden(!m_sourceItem->isHidden());
}

void KompareListViewDiffItem::applyDifference(bool apply)
{
    qCDebug(KOMPAREPART) << "KompareListViewDiffItem::applyDifference( " << apply << " )" ;
    setVisibility();
}

int KompareListViewDiffItem::maxHeight()
{
    int lines = qMax(m_difference->sourceLineCount(), m_difference->destinationLineCount());
    if (lines == 0)
        return BLANK_LINE_HEIGHT;
    else
        return lines * treeWidget()->fontMetrics().height();
}

KompareListViewLineContainerItem::KompareListViewLineContainerItem(KompareListViewDiffItem* parent, bool isSource)
    : KompareListViewItem(parent, Container),
      m_blankLineItem(nullptr),
      m_isSource(isSource)
{
//     qCDebug(KOMPAREPART) << "isSource ? " << (isSource ? " Yes!" : " No!") ;
    setHeight(0);
    setExpanded(true);

    int lines = lineCount();
    int line = lineNumber();
//     qCDebug(KOMPAREPART) << "LineNumber : " << lineNumber() ;
    if (lines == 0) {
        m_blankLineItem = new KompareListViewBlankLineItem(this);
        return;
    }

    for (int i = 0; i < lines; ++i, ++line) {
        new KompareListViewLineItem(this, line, lineAt(i));
    }
}

KompareListViewLineContainerItem::~KompareListViewLineContainerItem()
{
}

KompareListViewDiffItem* KompareListViewLineContainerItem::diffItemParent() const
{
    return (KompareListViewDiffItem*)parent();
}

int KompareListViewLineContainerItem::lineCount() const
{
    return m_isSource ? diffItemParent()->difference()->sourceLineCount() :
           diffItemParent()->difference()->destinationLineCount();
}

int KompareListViewLineContainerItem::lineNumber() const
{
    return m_isSource ? diffItemParent()->difference()->sourceLineNumber() :
           diffItemParent()->difference()->destinationLineNumber();
}

DifferenceString* KompareListViewLineContainerItem::lineAt(int i) const
{
    return m_isSource ? diffItemParent()->difference()->sourceLineAt(i) :
           diffItemParent()->difference()->destinationLineAt(i);
}

KompareListViewLineItem::KompareListViewLineItem(KompareListViewLineContainerItem* parent, int line, DifferenceString* text)
    : KompareListViewItem(parent, Line)
{
    init(line, text);
}

KompareListViewLineItem::KompareListViewLineItem(KompareListViewLineContainerItem* parent, int line, DifferenceString* text, int type)
    : KompareListViewItem(parent, type)
{
    init(line, text);
}

KompareListViewLineItem::~KompareListViewLineItem()
{
    m_text = nullptr;
}

void KompareListViewLineItem::init(int line, DifferenceString* text)
{
    setHeight(treeWidget()->fontMetrics().height());
    setText(COL_LINE_NO, QString::number(line));
    setText(COL_MAIN, text->string());
    m_text = text;
}

void KompareListViewLineItem::paintCell(QPainter* p, const QStyleOptionViewItem& option, int column)
{
    int width = option.rect.width();
    Qt::Alignment align = option.displayAlignment;

    p->setRenderHint(QPainter::Antialiasing);
    p->translate(option.rect.topLeft());
    p->translate(0, -paintOffset());

    QColor bg(Qt::white);   // Always make the background white when it is not a real difference
    if (diffItemParent()->difference()->type() == Difference::Unchanged)
    {
        if (column == COL_LINE_NO)
        {
            bg = QColor(Qt::lightGray);
        }
    }
    else
    {
        bg = kompareListView()->settings()->colorForDifferenceType(
                 diffItemParent()->difference()->type(),
                 diffItemParent()->isCurrent(),
                 diffItemParent()->difference()->applied());
    }

    // Paint background
    p->fillRect(0, 0, width, paintHeight(), bg);

    // Paint foreground
    if (diffItemParent()->difference()->type() == Difference::Unchanged)
        p->setPen(QColor(Qt::darkGray));     // always make normal text gray
    else
        p->setPen(QColor(Qt::black));     // make text with changes black

    paintText(p, bg, column, width, align);

    // Paint darker lines around selected item
    if (diffItemParent()->isCurrent())
    {
        p->translate(0.5, 0.5);
        p->setPen(bg.darker(135));
        QTreeWidgetItem* parentItem = parent();
        if (this == parentItem->child(0))
            p->drawLine(0, 0, width, 0);
        if (this == parentItem->child(parentItem->childCount() - 1))
            p->drawLine(0, paintHeight() - 1, width, paintHeight() - 1);
    }

    p->resetTransform();
}

void KompareListViewLineItem::paintText(QPainter* p, const QColor& bg, int column, int width, int align)
{
    if (column == COL_MAIN)
    {
        QString textChunk;
        int offset = ITEM_MARGIN;
        int prevValue = 0;
        int charsDrawn = 0;
        int chunkWidth;
        QBrush changeBrush(bg, Qt::Dense3Pattern);
        QBrush normalBrush(bg, Qt::SolidPattern);
        QBrush brush;

        if (m_text->string().isEmpty())
        {
            p->fillRect(0, 0, width, paintHeight(), normalBrush);
            return;
        }

        p->fillRect(0, 0, offset, paintHeight(), normalBrush);

        if (!m_text->markerList().isEmpty())
        {
            MarkerListConstIterator markerIt = m_text->markerList().begin();
            MarkerListConstIterator mEnd     = m_text->markerList().end();
            Marker* m = *markerIt;

            for (; markerIt != mEnd; ++markerIt)
            {
                m  = *markerIt;
                textChunk = m_text->string().mid(prevValue, m->offset() - prevValue);
//                 qCDebug(KOMPAREPART) << "TextChunk   = \"" << textChunk << "\"" ;
//                 qCDebug(KOMPAREPART) << "c->offset() = " << c->offset() ;
//                 qCDebug(KOMPAREPART) << "prevValue   = " << prevValue ;
                expandTabs(textChunk, kompareListView()->settings()->m_tabToNumberOfSpaces, charsDrawn);
                charsDrawn += textChunk.length();
                prevValue = m->offset();
                if (m->type() == Marker::End)
                {
                    QFont font(p->font());
                    font.setBold(true);
                    p->setFont(font);
//                     p->setPen( Qt::blue );
                    brush = changeBrush;
                }
                else
                {
                    QFont font(p->font());
                    font.setBold(false);
                    p->setFont(font);
//                     p->setPen( Qt::black );
                    brush = normalBrush;
                }
                chunkWidth = p->fontMetrics().horizontalAdvance(textChunk);
                p->fillRect(offset, 0, chunkWidth, paintHeight(), brush);
                p->drawText(offset, 0,
                            chunkWidth, paintHeight(),
                            align, textChunk);
                offset += chunkWidth;
            }
        }
        if (prevValue < m_text->string().length())
        {
            // Still have to draw some string without changes
            textChunk = m_text->string().mid(prevValue, qMax(1, m_text->string().length() - prevValue));
            expandTabs(textChunk, kompareListView()->settings()->m_tabToNumberOfSpaces, charsDrawn);
//             qCDebug(KOMPAREPART) << "TextChunk   = \"" << textChunk << "\"" ;
            QFont font(p->font());
            font.setBold(false);
            p->setFont(font);
            chunkWidth = p->fontMetrics().horizontalAdvance(textChunk);
            p->fillRect(offset, 0, chunkWidth, paintHeight(), normalBrush);
            p->drawText(offset, 0,
                        chunkWidth, paintHeight(),
                        align, textChunk);
            offset += chunkWidth;
        }
        p->fillRect(offset, 0, width - offset, paintHeight(), normalBrush);
    }
    else
    {
        p->fillRect(0, 0, width, paintHeight(), bg);
        p->drawText(ITEM_MARGIN, 0,
                    width - ITEM_MARGIN, paintHeight(),
                    align, text(column));
    }
}

void KompareListViewLineItem::expandTabs(QString& text, int tabstop, int startPos) const
{
    int index;
    while ((index = text.indexOf(QChar(9))) != -1)
        text.replace(index, 1, QString(tabstop - ((startPos + index) % tabstop), QLatin1Char(' ')));
}

KompareListViewDiffItem* KompareListViewLineItem::diffItemParent() const
{
    KompareListViewLineContainerItem* p = (KompareListViewLineContainerItem*)parent();
    return p->diffItemParent();
}

KompareListViewBlankLineItem::KompareListViewBlankLineItem(KompareListViewLineContainerItem* parent)
    : KompareListViewLineItem(parent, 0, new DifferenceString(), Blank)
{
    setHeight(BLANK_LINE_HEIGHT);
}

void KompareListViewBlankLineItem::paintText(QPainter* p, const QColor& bg, int column, int width, int /* align */)
{
    if (column == COL_MAIN)
    {
        QBrush normalBrush(bg, Qt::SolidPattern);
        p->fillRect(0, 0, width, paintHeight(), normalBrush);
    }
}

KompareListViewHunkItem::KompareListViewHunkItem(KompareListView* parent, DiffHunk* hunk, bool zeroHeight)
    : KompareListViewItem(parent, Hunk),
      m_zeroHeight(zeroHeight),
      m_hunk(hunk)
{
    setHeight(maxHeight());
    setFlags(flags() & ~Qt::ItemIsSelectable);
}

KompareListViewHunkItem::KompareListViewHunkItem(KompareListView* parent, KompareListViewItem* after, DiffHunk* hunk,  bool zeroHeight)
    : KompareListViewItem(parent, after, Hunk),
      m_zeroHeight(zeroHeight),
      m_hunk(hunk)
{
    setHeight(maxHeight());
    setFlags(flags() & ~Qt::ItemIsSelectable);
}

KompareListViewHunkItem::~KompareListViewHunkItem()
{
    m_hunk = nullptr;
}

int KompareListViewHunkItem::maxHeight()
{
    if (m_zeroHeight) {
        return 0;
    } else if (m_hunk->function().isEmpty()) {
        return HUNK_LINE_HEIGHT;
    } else {
        return treeWidget()->fontMetrics().height();
    }
}

void KompareListViewHunkItem::paintCell(QPainter* p, const QStyleOptionViewItem& option, int column)
{
    if (m_zeroHeight) {
        KompareListViewItem::paintCell(p, option, column);
    } else {
        int x = option.rect.left();
        int y = option.rect.top() - paintOffset();
        int width = option.rect.width();
        Qt::Alignment align = option.displayAlignment;

        p->fillRect(x, y, width, paintHeight(), QColor(Qt::lightGray));     // Hunk headers should be lightgray
        p->setPen(QColor(Qt::black));     // Text color in hunk should be black
        if (column == COL_MAIN) {
            p->drawText(x + ITEM_MARGIN, y, width - ITEM_MARGIN, paintHeight(),
                        align, m_hunk->function());
        }
    }
}
