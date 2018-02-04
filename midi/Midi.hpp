#ifndef __MIDI_HPP_
#define __MIDI_HPP_

#include <vector>
#include <string>

//#define NOTEVELOCITY 0x50	//notes velocity

/********************************************/
/*******Midi Structure	*********************/
/********************************************/
//
//	MidiFile
//		HeadChunk
//		TrackChunk
//			MetaEvent
//			MidiEvent

typedef struct Event
{
    unsigned char baseType = 0;   // meta or midi
    unsigned char type = 0;
    int deltaTime = 0;
    size_t size = 0;
    int v1 = 0; 
    int v2 = 0; 
    std::string content;
}Event;

typedef struct MetaEvent: public Event
{
    int v3 = 0; 
    int v4 = 0; 
    int v5 = 0; 
}MetaEvent;

struct MidiEvent: public Event
{
    int	nn = 0;
    int	cc = 0;
    // int	deltaTime;
};

typedef struct HeaderChunk
{
    std::string chunkID;
    size_t chunkSize = 0;	//length: 6
    size_t format = 0;		//format: 0, 1, 2
    size_t tracksNumber = 0;
    size_t deltaTimeTicks = 0;
}HeaderChunk;

typedef struct TrackChunk
{
    ~TrackChunk();
    std::string chunkID;
    size_t chunkSize = 0;
    std::vector<Event*> Events;
}TrackChunk;

class MidiFile
{
    private:
        ~MidiFile();

        HeaderChunk headerChunk;      	        //header chunk
        std::vector<TrackChunk*> trackChunks;	//track chunk list

        //get a fixed length(bits) word.
        int getNBitsNumber(const std::string &midistr, size_t& offset, int bits);
        //get a variable length word.
        size_t getDWord(const std::string &midistr, size_t& offset);
        //get content
        std::string getDWordContent(const std::string &midistr, size_t& offset, int type);
        
        MetaEvent* importMetaEvent(const std::string& midistr, size_t& offset);
        MidiEvent* importMidiEvent(const std::string& midistr, size_t& offset, unsigned char head);
        Event* importEvent(const std::string& midistr, size_t& offset);

        void importHeaderChunk(const std::string& midistr, size_t& offset);
        void importTrackChunks(const std::string& midistr, size_t& offset);
    public:
        void importMidiFile(const std::string& fileName);
        void exportMidiFile(const std::string& fileName);
        void exportXMLFile(const std::string& fileName);
};

#endif
