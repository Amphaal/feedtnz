#include "LightWidget.h"

LightWidget::LightWidget(const QColor &color, QWidget *parent) : QWidget(parent), m_color(color), m_on(true) {
    this->setLayout(new QHBoxLayout);
};

bool LightWidget::isOn() const { return m_on; };

void LightWidget::setOn(bool on) {
    if (on == m_on)
        return;
    m_on = on;
    update();
};

void LightWidget::turnOff() { setOn(false); };
void LightWidget::turnOn() { setOn(true); };

void LightWidget::paintEvent(QPaintEvent *) {
    if (!m_on) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(m_color);

    auto height = this->height();

    painter.drawEllipse(
        qFloor(height * .25), 
        qFloor(height * .25), 
        qFloor(height * .75), 
        qFloor(height * .75)
    );

};