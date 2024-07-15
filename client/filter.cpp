#include "filter.h"

/**
 * @brief Конструктор класса NoTildeFilter.
 *
 * @param parent Указатель на родительский объект.
 */
NoTildeFilter::NoTildeFilter(QObject *parent) : QObject(parent) {}

/**
 * @brief  Этот метод перехватывает события клавиатуры и игнорирует ввод символа "~".
 * @param obj Указатель на объект, для которого фильтруются события.
 * @param event Указатель на событие, которое фильтруется.
 * @return Возвращает true, если событие было обработано (игнорируемо),
 *         и false, если событие должно быть передано дальше.
 */
bool NoTildeFilter::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->text() == "~") {
            return true; // Игнорируем событие
        }
    }
    return QObject::eventFilter(obj, event);
}
