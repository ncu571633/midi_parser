# C++ Midi Parser/API

Functions:
1. Import midi file into internal data structure;
2. Export Internal data structure to XML file;
3. Export Internal data structure to midi file;
4. APIs to do conversions between: midi event number, piano key number and music note notation;

Implementation:
Hierarchical Midi Data Structures (The hierarchical Midi data structures are designed in Midi.hpp)
1.  MidiFile class
2.      HeadChunk class
3.      TrackChunk class
4.          MetaEvent class
5.          MidiEvent class

Only need to use three APIs defined in MidiFile class for Midi file importing and exporting. The other classes and functions are encapsulated.
        void importMidiFile(const std::string& fileName);
        void exportMidiFile(const std::string& fileName);
        void exportXMLFile(const std::string& fileName);

Example see main.cpp
    int main()
    {
        MidiFile midi;
        midi.importMidiFile("close.mid");
        midi.exportMidiFile("close1.mid");
        midi.exportXMLFile("close.xml");
        return 1;
    }
