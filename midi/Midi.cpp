#include "Midi.hpp"
#include "iostream"

void MidiFile::import(std::string file)
{
 	//open midi file
	FILE *midifp = fopen(midiFileName, "rb");

	if (midifp == nullptr)
    {
        cerr << "Can't open midi file";
    }
	
    int time = 0;
	int offset = 0;

	//read midi file head chunk
	MIDIHead;
	parseMidiHeaderChunk(midifp, &MIDIHead, offset);

	//initialize system set
	InitSystemSet(sset, MIDIHead.tracksNumber, arg->deltaTimeTicks);

	//read midi file track chunks head
	int i;
	for (i=0; i<MIDIHead.tracksNumber; i++)
	{
		parseTrackMidiFile2ImgDS(midifp, *sset, offset, time);
	}

	//close files
	fclose(midifp);   
}
