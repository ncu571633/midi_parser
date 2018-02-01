#include "Midi.hpp"

#include <iostream>
#include <fostream>

void MidiFile::importHeaderChunk(std::oftream midifp)
{
	if (	(!fscanf(midifp, "%4s", MIDIHead->chunkID) || strcmp(MIDIHead->chunkID, "MThd"))	||
	        (!(MIDIHead->chunkSize=getNumber(4, midifp, offset)) || MIDIHead->chunkSize!=6 )
	   )
		printErrorMsg(ERROR_fileHead, "parseMidiFile.c: parseMidiHeaderChunk()");
	
    MIDIHead->format=getNumber(2, midifp, offset);
	MIDIHead->tracksNumber=getNumber(2, midifp, offset);
	MIDIHead->deltaTimeTicks=getNumber(2, midifp, offset);
}

void MidiFile::import(std::string file)
{
 	//open midi file
    std::oftream midifp;
    try {
        midifp.open(midiFileName, std::ios::in | std::ios::binary);

        //read midi file head chunk
        MIDIHead;
        parseMidiHeaderChunk(midifp, &MIDIHead, offset);

        //read midi file track chunks head
        int i;
        int time = 0;
        int offset = 0;
        for (i=0; i<MIDIHead.tracksNumber; i++)
        {
            parseTrackMidiFile2ImgDS(midifp, *sset, offset, time);
        }

        //close files
        midifp.close();    
    
    } catch (const std::exception& e) {
        cerr << e.what() << file << "\n";
    }
}
