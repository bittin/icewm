/*
 * IceWM
 *
 * Copyright (C) 1997-2001 Marko Macek
 */
#include "config.h"
#include "yscrollview.h"
#include "yscrollbar.h"
#include "prefs.h"

YScrollView::YScrollView(YWindow *aParent, YScrollable* scroll):
    YWindow(aParent),
    scrollable(scroll),
    scrollVert(new YScrollBar(YScrollBar::Vertical, this)),
    scrollHoriz(new YScrollBar(YScrollBar::Horizontal, this))
{
    setTitle("ScrollView");
    setBackground(YScrollBar::background());
}

void YScrollView::setView(YScrollable *s) {
    scrollable = s;
}

void YScrollView::setListener(YScrollBarListener* l) {
    scrollVert->setScrollBarListener(l);
    scrollHoriz->setScrollBarListener(l);
}

void YScrollView::getGap(int& dx, int& dy) {
    unsigned const cw(scrollable ? scrollable->contentWidth() : 0);
    unsigned const ch(scrollable ? scrollable->contentHeight() : 0);
    dx = (height() < ch || (width() < cw && height() < ch + scrollBarHeight))
        ? scrollBarWidth : 0;
    dy = (width() < cw || (height() < ch && width() < cw + scrollBarWidth))
        ? scrollBarHeight : 0;
}

void YScrollView::layout() {
    int const w(width()), h(height());
    int dx, dy;
    getGap(dx, dy);

    if (dx > 0 && w > dx) {
        scrollVert->setGeometry(YRect(w - dx, 0,
                                int(scrollBarWidth),
                                h > dy ? h - dy : 1));
        scrollVert->enable();
    } else {
        scrollVert->hide();
        dx = 0;
    }
    if (dy > 0 && h > dy) {
        scrollHoriz->setGeometry(YRect(0, h - dy,
                                 w > dx ? w - dx : 1,
                                 int(scrollBarHeight)));
        scrollHoriz->enable();
    } else {
        scrollHoriz->hide();
        dy = 0;
    }
    if (scrollable) {
        YWindow* ywin = dynamic_cast<YWindow *>(scrollable);
        if (ywin) {
            if (w > dx && h > dy) {
                ywin->setGeometry(YRect(0, 0, w - dx, h - dy));
                ywin->show();
            }
        }
    }
}

void YScrollView::configure(const YRect2& r) {
    if (r.resized()) {
        layout();
    }
}

bool YScrollView::handleScrollKeys(const XKeyEvent& key) {
    const int v = scrollVert->handleScrollKeys(key);
    const int h = scrollHoriz->handleScrollKeys(key);
    return bool(v | h);
}

void YScrollView::handleExpose(const XExposeEvent& expose) {
    if (expose.count == 0 && notbit(getStyle(), wsNoExpose)) {
        addStyle(wsNoExpose);
        ref<YPixmap> pixmap = YPixmap::create(1, 1, depth());
        Graphics g(pixmap);
        g.setColor(YScrollBar::background());
        g.drawPoint(0, 0);
        setBackgroundPixmap(pixmap);
        clearWindow();
    }
}

// vim: set sw=4 ts=4 et:
