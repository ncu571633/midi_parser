#include "Midi.hpp"
#include "Music.hpp"

#include "iostream"

int main()
{
    MidiFile midi;
    midi.importMidiFile("close.mid");
    midi.exportMidiFile("close1.mid");
    midi.exportXMLFile("close.xml");
    
    return 1;
}
