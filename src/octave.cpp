#include "octave.h"
#include "key.h"

Octave::Octave(unsigned char o)
    : QGraphicsItemGroup(), _octave(o)
{
    this->setHandlesChildEvents(false);
    for (int i = 0; i < 12; i++)
    {
        if (o * 12 + i < 128)
        {
            this->_keys[i] = new Key(this, o * 12 + i);
            this->addToGroup(this->_keys[i]);
        }
        else
            this->_keys[i] = 0;
    }
}

Octave::~Octave()
{ }
