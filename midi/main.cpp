#include "Midi.hpp"
#include "Music.hpp"

#include <iostream>

using namespace std;

int main()
{
    Midi::MidiFile midi;
    midi.importMidiFile("close.mid");
    midi.exportMidiFile("close1.mid");
    midi.exportXMLFile("close.xml");
   
    Midi::MidiFile midi2;
    midi2.importMidiTXT("midi.txt");
    midi2.exportMidiFile("midi.mid");
    midi2.exportXMLFile("midi.xml");
    midi2.exportMidiTXT("midi2.txt");

    midi2.deleteTrackEvent((size_t)0, Midi::EventType::midiEvent, 0x2d, 0x4e);
    cout<< midi2.editTrackEvent((size_t)0, (size_t)1, Midi::EventType::metaEvent, 0, 0) << endl;
    midi2.exportMidiTXT("midi3.txt");
    return 1;
}
