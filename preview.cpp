#include "preview.h"
#include "ui_preview.h"

Preview::Preview(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::Preview)
{
    m_ui->setupUi(this);
}

Preview::~Preview()
{
    delete m_ui;
}

void Preview::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
