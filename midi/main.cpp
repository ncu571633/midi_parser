#include "Midi.hpp"

int main()
{
    MidiFile midi;
    midi.importMidiFile("close.mid");
    midi.exportMidiFile("close1.mid");
    midi.exportXMLFile("close.xml");
    
    return 1;
}
