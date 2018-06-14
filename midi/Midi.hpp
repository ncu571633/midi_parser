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
//  MidiFile
//      HeadChunk
//      TrackChunk
//          MetaEvent
//          MidiEvent
enum class EventType { midiEvent, metaEvent, sysexEvent };

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
        EventType eventType;
    public:
        virtual ~Event() {}
        inline size_t getSize() { return size; }
        inline void setDeltaTime(size_t d) { deltaTime = d; }
        
        virtual void importEvent(const std::string& midistr, size_t& offset) = 0; 
        virtual void exportEvent(std::string& midistr) {}
        virtual void exportEvent2XML(std::ofstream& midifp) {}
        virtual void exportEvent2TXT(std::ofstream& midifp, size_t& time) {}

        virtual bool compare(EventType t, int v1, int v2, int v3=0, int v4=0, int v5=0);
};

class MetaEvent: public Event
{
    private:
        int v3 = -1; 
        int v4 = -1; 
        int v5 = -1; 
    public:
        MetaEvent() {
            baseType = 0xff;
            eventType = EventType::metaEvent;
        }
        void importEvent(const std::string& midistr, size_t& offset);
        void exportEvent(std::string& midistr);
        void exportEvent2XML(std::ofstream& midifp);

        void setContent(unsigned char type, std::string c);
        void setTempo(int tempo); // 0x51
        void setSMPTEOffset(int hour, int minute, int second, int frame, int subframe); // 0x54
        void setTimeSignature(int numer, int denom, int interval); // 0x58
        void setKeySignature(int sf, int mi); // 0x59
        
        bool compare(EventType t, int v1, int v2, int v3, int v4, int v5);
};

class MidiEvent: public Event
{
    private:
        void setEvent(unsigned char Type, size_t DeltaTime, int noteChannel, int V1, int V2);
    public:
        MidiEvent() {
            eventType = EventType::midiEvent;
        }
        void importEvent(const std::string& midistr, size_t& offset);
        void exportEvent(std::string& midistr);
        void exportEvent2XML(std::ofstream& midifp);
        
        void importEventFromTXT(const std::string& midistr, int noteChannel, int& lastTime);
        void exportEvent2TXT(std::ofstream& midifp, size_t& time);

        void setNoteOff(size_t deltaTime, int noteChannel, int noteNumber, int velocity); // 8
        void setNoteOn(size_t deltaTime, int noteChannel, int noteNumber, int velocity); // 9
        void setKeyAfterTouch(int noteChannel, int noteNumber, int velocity); // A
        void setControlChange(int noteChannel, int controllerNumber, int newValue); // B
        void setProgramChange(int noteChannel, int newProgramNumber); // C
        void setChannelAfterTouch(int noteChannel, int channelNumber); // D
        void setPitchWheel(int NoteChannel, int pitch); //E
        void setSoundVolume(int noteChannel, int volume);
};

class SysexEvent: public Event
{
    public:
        SysexEvent() {
            eventType = EventType::sysexEvent;
        }
        void importEvent(const std::string& midistr, size_t& offset);
};

class Chunk
{
    protected:
        std::string chunkID;
        size_t chunkSize = 0;
    public:
        virtual ~Chunk() {}
        virtual void importChunk(const std::string& midistr, size_t& offset) = 0;
        virtual void exportChunk(std::string& midistr) = 0;
        virtual void exportChunk2XML(std::ofstream& midifp, size_t trackNumber=0) = 0;
};

class HeaderChunk : public Chunk
{
    private:
        size_t format = 0;      //format: 0, 1, 2. Default 0: single track
        size_t tracksNumber = 1;
        size_t deltaTimeTicks = 960;
    public:
        HeaderChunk() {
            chunkID = "MThd";
            chunkSize = 6;  // length 6
        }
        void importChunk(const std::string& midistr, size_t& offset);
        void exportChunk(std::string& midistr);
        void exportChunk2XML(std::ofstream& midifp, size_t trackNumber=0);
        
        size_t getTracksNumber() { return tracksNumber; }
};

class TrackChunk : public Chunk
{
    private:
        std::vector<Event*> Events;
    public:
        TrackChunk() {
            chunkID = "MTrk";
        }
        ~TrackChunk();
        void importChunk(const std::string& midistr, size_t& offset);
        void exportChunk(std::string& midistr);
        void exportChunk2XML(std::ofstream& midifp, size_t trackNumber);
       
        Event* importEvent(const std::string& midistr, size_t& offset);
        void importChunkFromTXT(const std::string& midistr);
        void exportChunk2TXT(const std::string& midifp);

        // return true if deletion successful
        bool deleteEvent(EventType t, int v1, int v2, int v3=0, int v4=0, int v5=0);
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
        void importMidiTXT(const std::string& txtName);
        void exportMidiTXT(const std::string& txtName);

        bool deleteTrackEvent(int trackNumber, EventType t, int v1, int v2, int v3=0, int v4=0, int v5=0);
};

#endif
