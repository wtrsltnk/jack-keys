/*
  ZynAddSubFX - a software synthesizer

  keyboard.cpp - Keyboard widget for running notes to the synth
  Copyright (C) 2014 Wouter Saaltink
  Author: Wouter Saaltink

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2 or later) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/
#include "piano.h"
#include "ui_piano.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTextDocument>
#include <QTextOption>
#include <iostream>
#include "jackoutput.h"

using namespace std;

int Piano::KeyWidth = 24;
int Piano::KeyHeight = 72;

unsigned char Piano::_characterNoteMapping[128] = { 0 };

Piano::Piano(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Keyboard), _sendnotes(false)
{
    JackOutput::Instance().initJack();

    ui->setupUi(this);

    this->ui->graphicsView->setMouseTracking(true);
    this->ui->graphicsView->viewport()->setMouseTracking(true);

    this->_scene = new QGraphicsScene();
    for (int i = 0; i < MAX_OCTAVES; i++)
    {
        this->_octaves[i] = new Octave(i);
        this->_scene->addItem(this->_octaves[i]);
        this->_octaves[i]->setPos(Piano::KeyWidth * 7 * i, 0);
    }
    this->_scene->setBackgroundBrush(QBrush(QColor::fromRgb(53, 53, 53)));

    this->ui->graphicsView->setScene(this->_scene);

    for (int i = 0; i < 128; i++)
        this->_selectedNotes[i] = false;

    int startoctave = 60;
    Piano::_characterNoteMapping['Z'] = startoctave+0;
     Piano::_characterNoteMapping['S'] = startoctave+1;
    Piano::_characterNoteMapping['X'] = startoctave+2;
     Piano::_characterNoteMapping['D'] = startoctave+3;
    Piano::_characterNoteMapping['C'] = startoctave+4;
    Piano::_characterNoteMapping['V'] = startoctave+5;
     Piano::_characterNoteMapping['G'] = startoctave+6;
    Piano::_characterNoteMapping['B'] = startoctave+7;
     Piano::_characterNoteMapping['H'] = startoctave+8;
    Piano::_characterNoteMapping['N'] = startoctave+9;
     Piano::_characterNoteMapping['J'] = startoctave+10;
    Piano::_characterNoteMapping['M'] = startoctave+11;
    Piano::_characterNoteMapping[','] = startoctave+12;
     Piano::_characterNoteMapping['L'] = startoctave+13;
    Piano::_characterNoteMapping['.'] = startoctave+14;
     Piano::_characterNoteMapping[';'] = startoctave+15;
    Piano::_characterNoteMapping['/'] = startoctave+16;
}

Piano::~Piano()
{
    delete ui;
}

void Piano::SelectCharacter(char c, bool on)
{
    unsigned char note = Piano::_characterNoteMapping[c];
    if (note >= 0 && note < 128)
    {
        int key = note % 12;
        int octave = (note - key) / 12;
        if (octave < MAX_OCTAVES && key < 12)
            this->_octaves[(note - (note % 12)) / 12]->_keys[note % 12]->SetOn(on);
        if (this->_selectedNotes[note] != on)
        {
            if (on)
                JackOutput::Instance().noteOn(note, 100);
            else
                JackOutput::Instance().noteOff(note);

            this->_selectedNotes[note] = on;
        }
    }
}

void Piano::HighlightNote(unsigned char note, bool on)
{
    if (note >= 0 && note < 128)
    {
        int key = note % 12;
        int octave = (note - key) / 12;
        if (octave < MAX_OCTAVES && key < 12)
            this->_octaves[(note - (note % 12)) / 12]->_keys[note % 12]->SetOn(on);
    }
}

void Piano::EnableNote(unsigned char note, bool enabled)
{
    if (note >= 0 && note < 128)
    {
        int key = note % 12;
        int octave = (note - key) / 12;
        if (octave < MAX_OCTAVES && key < 12)
            this->_octaves[octave]->_keys[key]->SetEnabled(enabled);
    }
}

void Piano::SetNoteColor(unsigned char note, const QColor& color)
{
    if (note >= 0 && note < 128)
    {
        int key = note % 12;
        int octave = (note - key) / 12;
        if (octave < MAX_OCTAVES && key < 12)
            this->_octaves[octave]->_keys[key]->setPen(QPen(color));
    }
}

void Piano::keyPressEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat() == false)
        this->SelectCharacter(char(event->key()), true);
}

void Piano::keyReleaseEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat() == false)
        this->SelectCharacter(char(event->key()), false);
}
