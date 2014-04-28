#include "jackoutput.h"
#include <memory.h>
#include <iostream>

using namespace std;

JackOutput::JackOutput()
    : jack_client(0), output_port(0)
{ }

JackOutput& JackOutput::Instance()
{
    static JackOutput instance;

    return instance;
}

JackOutput::~JackOutput()
{
    if (this->jack_client != 0)
    {
        if (this->output_port != 0) jack_port_unregister(this->jack_client, this->output_port);
        jack_client_close(this->jack_client);
    }
}

int JackOutput::initJack()
{
    if (this->jack_client != 0)
        return 1;

    int err;

    this->rate_limit = 0.0;
    this->time_offsets_are_zero = 0;

    jack_client = jack_client_open(PACKAGE_NAME, JackNoStartServer, NULL);

    if (jack_client == NULL) {
        cout << ("Could not connect to the JACK server; run jackd first?");
        return(0);
    }

    ringbuffer = jack_ringbuffer_create(RINGBUFFER_SIZE);

    if (ringbuffer == NULL) {
        cout << ("Cannot create JACK ringbuffer.");
        return(0);
    }

    jack_ringbuffer_mlock(ringbuffer);

    err = jack_set_process_callback(jack_client, JackOutput::process, this);
    if (err) {
        cout << ("Could not register JACK process callback.");
        return(0);
    }

    err = jack_set_graph_order_callback(jack_client, JackOutput::graph_order_callback, 0);
    if (err) {
        cout << ("Could not register JACK graph order callback.");
        return(0);
    }

    output_port = jack_port_register(jack_client, OUTPUT_PORT_NAME, JACK_DEFAULT_MIDI_TYPE,
        JackPortIsOutput, 0);

    if (output_port == NULL) {
        cout << ("Could not register JACK output port.");
        return(0);
    }

//    input_port = jack_port_register(jack_client, INPUT_PORT_NAME, JACK_DEFAULT_MIDI_TYPE,
//        JackPortIsInput, 0);

//    if (input_port == NULL) {
//        cout << ("Could not register JACK input port.");
//        return(0);
//    }

    if (jack_activate(jack_client)) {
        cout << ("Cannot activate JACK client.");
        return(0);
    }

    const char** ports = jack_get_ports(jack_client, NULL, NULL,0);
    if (ports != 0)
    {
        for (int i = 0; i < jack_port_name_size(); i++)
        {
            if (ports[i] != 0)
            {
                cout << ports[i] << endl;
//                jack_connect(jack_client, jack_port_name(this->output_port), ports[i]);
                break;
            }
        }
    }
    return 1;
}

int JackOutput::process(jack_nframes_t nframes, void *arg)
{
    JackOutput* kb = (JackOutput*)arg;
    kb->process_midi_output(nframes);
    return 0;
}

void warn_from_jack_thread_context(const char *str)
{
//	g_idle_add(warning_async, (gpointer)str);
}

double JackOutput::nframes_to_ms(jack_nframes_t nframes)
{
    jack_nframes_t sr;

    sr = jack_get_sample_rate(jack_client);

//    assert(sr > 0);

    return ((nframes * 1000.0) / (double)sr);
}

void JackOutput::process_midi_output(jack_nframes_t nframes)
{
    int read, t, bytes_remaining;
    unsigned char *buffer;
    void *port_buffer;
    jack_nframes_t last_frame_time;
    struct MidiMessage ev;

    last_frame_time = jack_last_frame_time(jack_client);

    port_buffer = jack_port_get_buffer(output_port, nframes);
    if (port_buffer == NULL) {
        warn_from_jack_thread_context("jack_port_get_buffer failed, cannot send anything.");
        return;
    }

#ifdef JACK_MIDI_NEEDS_NFRAMES
    jack_midi_clear_buffer(port_buffer, nframes);
#else
    jack_midi_clear_buffer(port_buffer);
#endif

    /* We may push at most one byte per 0.32ms to stay below 31.25 Kbaud limit. */
    bytes_remaining = nframes_to_ms(nframes) * rate_limit;

    while (jack_ringbuffer_read_space(ringbuffer)) {
        read = jack_ringbuffer_peek(ringbuffer, (char *)&ev, sizeof(ev));

        if (read != sizeof(ev)) {
            warn_from_jack_thread_context("Short read from the ringbuffer, possible note loss.");
            jack_ringbuffer_read_advance(ringbuffer, read);
            continue;
        }

        bytes_remaining -= ev.len;

        if (rate_limit > 0.0 && bytes_remaining <= 0) {
            warn_from_jack_thread_context("Rate limiting in effect.");
            break;
        }

        t = ev.time + nframes - last_frame_time;

        /* If computed time is too much into the future, we'll need
           to send it later. */
        if (t >= (int)nframes)
            break;

        /* If computed time is < 0, we missed a cycle because of xrun. */
        if (t < 0)
            t = 0;

        if (time_offsets_are_zero)
            t = 0;

        jack_ringbuffer_read_advance(ringbuffer, sizeof(ev));

#ifdef JACK_MIDI_NEEDS_NFRAMES
        buffer = jack_midi_event_reserve(port_buffer, t, ev.len, nframes);
#else
        buffer = jack_midi_event_reserve(port_buffer, t, ev.len);
#endif

        if (buffer == NULL) {
            warn_from_jack_thread_context("jack_midi_event_reserve failed, NOTE LOST.");
            break;
        }

        memcpy(buffer, ev.data, ev.len);
    }
}

int JackOutput::graph_order_callback(void *notused)
{
//    g_idle_add(update_window_title_async, NULL);
//    g_idle_add(update_connected_to_combo_async, NULL);

    return (0);
}

void JackOutput::queue_message(struct MidiMessage *ev)
{
    int written;

    if (jack_ringbuffer_write_space(ringbuffer) < sizeof(*ev)) {
        cout << ("Not enough space in the ringbuffer, NOTE LOST.") << endl;
        return;
    }

    written = jack_ringbuffer_write(ringbuffer, (char *)ev, sizeof(*ev));

    if (written != sizeof(*ev))
        cout << ("jack_ringbuffer_write failed, NOTE LOST.") << endl;
}

void JackOutput::queue_new_message(int b0, int b1, int b2)
{
    struct MidiMessage ev;

    /* For MIDI messages that specify a channel number, filter the original
       channel number out and add our own. */
    if (b0 >= 0x80 && b0 <= 0xEF) {
        b0 &= 0xF0;
        b0 += this->channel;
    }

    if (b1 == -1) {
        ev.len = 1;
        ev.data[0] = b0;

    } else if (b2 == -1) {
        ev.len = 2;
        ev.data[0] = b0;
        ev.data[1] = b1;

    } else {
        ev.len = 3;
        ev.data[0] = b0;
        ev.data[1] = b1;
        ev.data[2] = b2;
    }

    ev.time = jack_frame_time(jack_client);

    queue_message(&ev);
}

void JackOutput::noteOn(char note, char velocity)
{
    queue_new_message(MIDI_NOTE_ON, note, velocity);
}

void JackOutput::noteOff(char note)
{
    queue_new_message(MIDI_NOTE_OFF, note, 0);
}

void JackOutput::changeChannel(char index)
{
    this->channel = index;
}

void JackOutput::changeBank(char index)
{
    queue_new_message(MIDI_CONTROLLER, MIDI_BANK_SELECT_LSB, index % 128);
    queue_new_message(MIDI_CONTROLLER, MIDI_BANK_SELECT_MSB, index / 128);
}

void JackOutput::changeProgram(char index)
{
    queue_new_message(MIDI_PROGRAM_CHANGE, index, -1);
}
