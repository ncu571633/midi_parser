#ifndef __MIDI_HPP_
#define __MIDI_HPP_

#include <vector>
#include <string>
#include <fstream>
#include <type_traits>

/********************************************/
/*******Midi Utility    *********************/
/********************************************/
class MidiUtility
{
    public:
        //a fixed length(bits) word.
        static int getNBitsNumber(const std::string &midistr, size_t& offset, int bits);
        //return 0xFFFF (2*bits)
        static size_t writeNBitsNumberHelper(int bits);
        static void writeNBitsNumber(std::string &midistr, size_t value, int bits);
        //a variable length word.
        static size_t getDWord(const std::string &midistr, size_t& offset);
        static void writeDWord(std::string &midistr, size_t value);
        //get string 
        static std::string getString(const std::string &midistr, size_t& offset, int type);
        //write string
        static void writeString(std::string &midistr, const std::string& str);

        template<class T>
        typename std::enable_if<std::is_fundamental<T>::value, std::string>::type
        static addXMLAttribute(const std::string& attribute, T& value);
        
        template<class T>
        typename std::enable_if<!std::is_fundamental<T>::value, std::string>::type
        static addXMLAttribute(const std::string& attribute, T& value);
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
        virtual void exportEvent2XML(std::ofstream& midifp) {}
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
        void exportEvent2XML(std::ofstream& midifp);

        void setMetaTempo(int tempo); // 0x51
        void setTimeSignature(int numer, int denom, int interval); // 0x58
        void setKeySignature(int sf, int mi); // 0x59
};

class MidiEvent: public Event
{
    public:
        MidiEvent() {}
        MidiEvent(unsigned char Type, size_t DeltaTime, int V1, int V2)
        {
            baseType = 0, size = 2;
            type = Type, deltaTime = DeltaTime, v1 = V1, v2 = V2;
        }
        ~MidiEvent() {}
        void importEvent(const std::string& midistr, size_t& offset);
        void exportEvent(std::string& midistr);
        void exportEvent2XML(std::ofstream& midifp);
};

class SysexEvent: public Event
{
    public:
        SysexEvent() {}
        ~SysexEvent() {}
        void importEvent(const std::string& midistr, size_t& offset);
        void exportEvent(std::string& midistr) {}
        void exportEvent2XML(std::ofstream& midifp) {}
};

class HeaderChunk
{
    private:
        std::string chunkID = "MThd";
        size_t chunkSize = 6;   //length: 6
        size_t format = 0;      //format: 0, 1, 2. Default 0: single track
        size_t tracksNumber = 0;
        size_t deltaTimeTicks = 960;
    public:
        size_t getTracksNumber() { return tracksNumber; }
        void importChunk(const std::string& midistr, size_t& offset);
        void exportChunk(std::string& midistr);
        void exportChunk2XML(std::ofstream& midifp);
};

class TrackChunk
{
    private:
        std::string chunkID = "MTrk";
        size_t chunkSize = 0;
        std::vector<Event*> Events;
    public:
        ~TrackChunk();
        Event* importEvent(const std::string& midistr, size_t& offset);
        void importChunk(const std::string& midistr, size_t& offset);
        void exportChunk(std::string& midistr);
        void exportChunk2XML(std::ofstream& midifp, size_t trackNumber);
        void importMidiTXTFile(const std::string& txtName);
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
        
        // import/export the notes information from/to a text file
        void importMidiTXTFile(const std::string& txtName);
        void exportMidiTXTFile(const std::string& txtName);
};

#endif
