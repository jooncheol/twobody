#ifndef PREVIEW_H
#define PREVIEW_H

#include <QtGui/QWidget>

namespace Ui {
    class Preview;
}

class Preview : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(Preview)
public:
    explicit Preview(QWidget *parent = 0);
    virtual ~Preview();

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::Preview *m_ui;
};

#endif // PREVIEW_H
