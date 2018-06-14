#include "Midi.hpp"
#include "Music.hpp"

#include "iostream"

int main()
{
    MidiFile midi;
    midi.importMidiFile("close.mid");
    midi.exportMidiFile("close1.mid");
    midi.exportXMLFile("close.xml");
   
    MidiFile midi2;
    midi2.importMidiTXT("midi.txt");
    midi2.exportMidiFile("midi.mid");
    midi2.exportXMLFile("midi.xml");
    midi2.exportMidiTXT("midi2.txt");

    midi2.deleteTrackEvent(0, EventType::midiEvent, 0x2d, 0x4e);
    midi2.exportMidiTXT("midi3.txt");
    return 1;
}
