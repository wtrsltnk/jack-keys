#include "key.h"
#include "octave.h"
#include "piano.h"

Key::Key(Octave* o, unsigned char n)
    : QGraphicsPolygonItem(o), _parent(o), _note(n), _isEnabled(true)
{
    this->setAcceptHoverEvents(true);
    QPolygon p;
    if (n % 12 == 0 || n % 12 == 2 || n % 12 == 4 || n % 12 == 5 || n % 12 == 7 || n % 12 == 9 || n % 12 == 11)
    {
        int a = 0;
        if (n % 12 == 2) a = 1;
        if (n % 12 == 4) a = 2;
        if (n % 12 == 5) a = 3;
        if (n % 12 == 7) a = 4;
        if (n % 12 == 9) a = 5;
        if (n % 12 == 11) a = 6;
        p << QPoint(a * Piano::KeyWidth + 0, 0)
          << QPoint(a * Piano::KeyWidth + Piano::KeyWidth, 0)
          << QPoint(a * Piano::KeyWidth + Piano::KeyWidth, Piano::KeyHeight)
          << QPoint(a * Piano::KeyWidth + 0, Piano::KeyHeight);
        this->setBrush(QBrush(Qt::white));
        this->setZValue(1);
        this->_isSharp = false;
        QString txt = QString("%1%2").arg(QString('A' + a), QString::number(o->_octave));
        QGraphicsTextItem* t = new QGraphicsTextItem(txt);
        t->setPos(a * Piano::KeyWidth, Piano::KeyHeight);
        t->setDefaultTextColor(Qt::white);
        o->addToGroup(t);
    }
    else
    {
        int a = 0;
        if (n % 12 == 3) a = 1;
        if (n % 12 == 6) a = 3;
        if (n % 12 == 8) a = 4;
        if (n % 12 == 10) a = 5;
        p << QPoint(a * Piano::KeyWidth + ((Piano::KeyWidth/3) * 2), 0)
          << QPoint(a * Piano::KeyWidth + ((Piano::KeyWidth/3) * 2) + ((Piano::KeyWidth/3) * 2), 0)
          << QPoint(a * Piano::KeyWidth + ((Piano::KeyWidth/3) * 2) + ((Piano::KeyWidth/3) * 2), Piano::KeyHeight * 0.6)
          << QPoint(a * Piano::KeyWidth + ((Piano::KeyWidth/3) * 2), Piano::KeyHeight * 0.6);
        this->setBrush(QBrush(Qt::black));
        this->setZValue(2);
        this->_isSharp = true;
    }
    this->setPolygon(p);
}

Key::~Key()
{ }

void Key::SetOn(bool on)
{
    if (this->_isEnabled)
    {
        if (on)
        {
            if (this->_isSharp)
                this->setBrush(QBrush(QColor::fromRgb(0, 48, 64)));
            else
                this->setBrush(QBrush(QColor::fromRgb(0, 172, 229)));
        }
        else
        {
            if (this->_isSharp)
                this->setBrush(QBrush(Qt::black));
            else
                this->setBrush(QBrush(Qt::white));
        }
        this->update();
    }
}

void Key::SetEnabled(bool enabled)
{
    this->_isEnabled = enabled;
    if (this->_isEnabled == false)
    {
        if (this->_isSharp)
            this->setBrush(QBrush(Qt::gray));
        else
            this->setBrush(QBrush(Qt::lightGray));
    }
    else
    {
        if (this->_isSharp)
            this->setBrush(QBrush(Qt::black));
        else
            this->setBrush(QBrush(Qt::white));
    }
    this->update();
}

void Key::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    this->SetOn(true);
}

void Key::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    this->SetOn(false);
}
