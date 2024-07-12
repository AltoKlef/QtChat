
#include <QObject>
#include <QEvent>
#include <QKeyEvent>

class NoTildeFilter : public QObject {
    Q_OBJECT
public:
    explicit NoTildeFilter(QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

