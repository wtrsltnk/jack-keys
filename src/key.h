#ifndef KEY_H
#define KEY_H

#include <QGraphicsPolygonItem>

class Octave;

class Key : public QGraphicsPolygonItem
{
public:
    Key(Octave* _parent, unsigned char n);
    virtual ~Key();

    void SetOn(bool on);
    void SetEnabled(bool enabled);

    Octave* _parent;
    unsigned char _note;
    bool _isSharp;
    bool _isEnabled;

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

};

#endif // KEY_H
