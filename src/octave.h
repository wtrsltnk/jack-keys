#ifndef OCTAVE_H
#define OCTAVE_H

#include <QGraphicsItemGroup>

class Key;

class Octave : public QGraphicsItemGroup
{
public:
    Octave(unsigned char o);
    virtual ~Octave();

    Key* _keys[12];
    unsigned char _octave;
};

#endif // OCTAVE_H
