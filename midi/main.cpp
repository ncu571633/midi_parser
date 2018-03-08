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
    midi2.exportXMLFile("midi.xml");
    midi2.exportMidiTXT("midi2.txt");

    return 1;
}
