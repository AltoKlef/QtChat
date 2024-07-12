#include "filter.h"

NoTildeFilter::NoTildeFilter(QObject *parent) : QObject(parent) {}

bool NoTildeFilter::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->text() == "~") {
            return true; // Игнорируем событие
        }
    }
    return QObject::eventFilter(obj, event);
}
