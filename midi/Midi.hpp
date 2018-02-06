#ifndef __MIDI_HPP_
#define __MIDI_HPP_

#include <vector>
#include <string>

/********************************************/
/*******Midi Utility    *********************/
/********************************************/
class MidiUtility
{
    public:
        //get a fixed length(bits) word.
        static int getNBitsNumber(const std::string &midistr, size_t& offset, int bits);
        //return 0xFFFF (2*bits)
        static size_t writeNBitsNumberHelper(int bits);
        static void writeNBitsNumber(std::string &midistr, size_t value, int bits);
        //get a variable length word.
        static size_t getDWord(const std::string &midistr, size_t& offset);
        static void writeDWord(std::string &midistr, size_t value);
        //get string 
        static std::string getString(const std::string &midistr, size_t& offset, int type);
        //write string
        static void writeString(std::string &midistr, const std::string& str);
};

/********************************************/
/*******Midi Structure  *********************/
/********************************************/
//
//  MidiFile
//      HeadChunk
//      TrackChunk
//          MetaEvent
//          MidiEvent

class Event
{
    protected:
        unsigned char baseType = 0;   // meta or midi
        unsigned char type = 0;
        size_t deltaTime = 0;
        size_t size = 0;
        int v1 = -1; 
        int v2 = -1; 
        std::string content;
    public:
        Event() {}
        virtual ~Event() {}
        inline size_t getSize() { return size; }
        inline void setDeltaTime(size_t d) { deltaTime = d; }
        
        virtual void importEvent(const std::string& midistr, size_t& offset) {}
        virtual void exportEvent(std::string& midistr) {}
};

class MetaEvent: public Event
{
    private:
        int v3 = -1; 
        int v4 = -1; 
        int v5 = -1; 
    public:
        MetaEvent() {}
        ~MetaEvent() {}
        void importEvent(const std::string& midistr, size_t& offset);
        void exportEvent(std::string& midistr);
};

class MidiEvent: public Event
{
    public:
        MidiEvent() {}
        ~MidiEvent() {}
        void importEvent(const std::string& midistr, size_t& offset);
        void exportEvent(std::string& midistr);
};

class SysexEvent: public Event
{
    public:
        SysexEvent() {}
        ~SysexEvent() {}
        void importEvent(const std::string& midistr, size_t& offset);
        void exportEvent(std::string& midistr) {}
};

class HeaderChunk
{
    private:
        std::string chunkID;
        size_t chunkSize = 0;   //length: 6
        size_t format = 0;      //format: 0, 1, 2
        size_t tracksNumber = 0;
        size_t deltaTimeTicks = 0;
    public:
        size_t getTracksNumber() { return tracksNumber; }
        void importChunk(const std::string& midistr, size_t& offset);
        void exportChunk(std::string& midistr);
};

class TrackChunk
{
    private:
        std::string chunkID;
        size_t chunkSize = 0;
        std::vector<Event*> Events;
    public:
        ~TrackChunk();
        Event* importEvent(const std::string& midistr, size_t& offset);
        void importChunk(const std::string& midistr, size_t& offset);
        void exportChunk(std::string& midistr);
};

class MidiFile
{
    private:
        HeaderChunk headerChunk;                //header chunk
        std::vector<TrackChunk*> trackChunks;   //track chunk list
        
    public:
        ~MidiFile();
        
        void importMidiFile(const std::string& fileName);
        void exportMidiFile(const std::string& fileName);
        void exportXMLFile(const std::string& fileName);
};

#endif
