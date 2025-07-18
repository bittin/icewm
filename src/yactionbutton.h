#ifndef YACTIONBUTTON_H
#define YACTIONBUTTON_H

#include "ybutton.h"

class YActionButton: public YButton {
public:
    YActionButton(YWindow* parent, const mstring& str, int hotkey,
                  YActionListener* listener, EAction action = actionNull);
    operator YAction() const { return getAction(); }

    virtual void handleExpose(const XExposeEvent& expose) {}
    virtual void configure(const YRect2& r);
    virtual void repaint();
    virtual YDimension getTextSize();
    virtual YSurface getSurface();
};

#endif

// vim: set sw=4 ts=4 et:
