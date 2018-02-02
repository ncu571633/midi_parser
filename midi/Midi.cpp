#include "Midi.hpp"

#include <iostream>
#include <fostream>


//get a fixed length(bits) word.
int getNBitsNumber(std::ifstream &midifp, int bits)
{
    int number = 0;
    while (bits--)
    {
        char c;
        midifp.read(&c, 1);
        number = (number<<8) | c;
    }
    return number;
}

//get a variable length word.
DWORD getDWord(std::ifstream &midifp)
{
    DWORD number = 0;
    midifp.read(&number, 1);
    if ( (number=t) & 0x80 )
    {
        (*offset)++;
        number = number & 0x7f;
        do
        {
            unsigned char c = '\0';
            number = (number<<7) + ((c=getc(midifp))&0x7f);
        } while (c&0x80);
    }
    return number;
}

void MidiFile::importHeaderChunk(std::iftream midifp)
{
    char buffer[128];
    memset(buffer, 0);
    midifp.read(buffer, 4);
    

headerChunk->chunkID
    if (    (!fscanf(midifp, "%4s", MIDIHead->chunkID) || strcmp(MIDIHead->chunkID, "MThd"))    ||
            (!(MIDIHead->chunkSize=getNumber(4, midifp, offset)) || MIDIHead->chunkSize!=6 )
       )
        printErrorMsg(ERROR_fileHead, "parseMidiFile.c: parseMidiHeaderChunk()");
    
    MIDIHead->format=getNumber(2, midifp, offset);
    MIDIHead->tracksNumber=getNumber(2, midifp, offset);
    MIDIHead->deltaTimeTicks=getNumber(2, midifp, offset);
}

void MidiFile::import(std::string file)
{
    try {
        std::iftream midifp(midiFileName, std::ios::in | std::ios::binary);

        //read midi file head chunk
        parseMidiHeaderChunk(midifp);

        //read midi file track chunks head
        int time = 0;
        for (int i=0; i<MIDIHead.tracksNumber; i++)
        {
            parseTrackMidiFile2ImgDS(midifp, time);
        }

        //close files
        midifp.close();    
    
    } catch (const std::exception& e) {
        cerr << e.what() << file << "\n";
    }
}
