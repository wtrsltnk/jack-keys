#ifndef JACKOUTPUT_H
#define JACKOUTPUT_H

#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>

/*
 * From source by the Jack-Keyboard project on http://sourceforge.net/projects/jack-keyboard/
 */
#define OUTPUT_PORT_NAME	"midi_out"
#define INPUT_PORT_NAME		"midi_in"
#define PACKAGE_NAME		"jack-piano"

#define MIDI_NOTE_ON		0x90
#define MIDI_NOTE_OFF		0x80
#define MIDI_PROGRAM_CHANGE	0xC0
#define MIDI_CONTROLLER		0xB0
#define MIDI_RESET		0xFF
#define MIDI_HOLD_PEDAL		64
#define MIDI_ALL_SOUND_OFF	120
#define MIDI_ALL_MIDI_CONTROLLERS_OFF	121
#define MIDI_ALL_NOTES_OFF	123
#define MIDI_BANK_SELECT_MSB	0
#define MIDI_BANK_SELECT_LSB	32

struct MidiMessage {
    jack_nframes_t	time;
    int		len;	/* Length of MIDI message, in bytes. */
    unsigned char	data[3];
};

#define RINGBUFFER_SIZE		1024*sizeof(struct MidiMessage)

class JackOutput
{
    JackOutput();
public:
    static JackOutput& Instance();
    virtual ~JackOutput();

    int initJack();

    void noteOn(char note, char velocity);
    void noteOff(char note);
    void changeChannel(char index);
    void changeBank(char index);
    void changeProgram(char index);

private:
    jack_client_t *jack_client;
    jack_port_t	*output_port;
    jack_port_t	*input_port;
    jack_ringbuffer_t *ringbuffer;
    double rate_limit;
    int time_offsets_are_zero;
    int channel;
    static int process(jack_nframes_t nframes, void *arg);
    static int graph_order_callback(void *notused);
    void process_midi_output(jack_nframes_t nframes);
    double nframes_to_ms(jack_nframes_t nframes);

    void queue_message(struct MidiMessage *ev);
    void queue_new_message(int b0, int b1, int b2);

};

#endif // JACKOUTPUT_H
