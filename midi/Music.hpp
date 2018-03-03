#ifndef __MUSIC_HPP_
#define __MUSIC_HPP_

#include <string>
#include <vector>

// MusicUtility class does the conversion among the notations below:
// music note(English notation): pitch-scale: like 4c, 5d
// piano key number(88 key): 40 -> c4, 28 -> 3c, 16 -> 2c, 4 -> 1c 
// midi number

class MusicUtility
{
    private:
        // 12-tone chromatic scale
        static const std::string scale[];
        static int scaleSize;
       
        // store note string in a 8 bit integer
        // note: 4c, 4c+
        // pitch: 0~7 use 3 bits to store  
        // scale: a~g use 3 bits to store
        // +/-/'\0': use 2 bit to store: '+':0b10, '-':0b00, '\0':0b01
        static int code(const std::string& note);
        static std::string decode(int number);
        
        // NoteArray: used as a pre-calculated map:
        //  index: note integer 
        //  value: piano key number 
        static const int NoteArraySize = 256;
        static std::vector<int> NoteArray;
        static void initNoteArray();

        static void validate();
    public:
        // midi number <-> piano key number (88 keys, 40 -> 4c)
        static int Midi2KeyNumber(int MidiNumber);
        static int KeyNumber2Midi(int KeyNumber);

        // note <-> piano key number
        static int Note2KeyNumber(std::string& note);
        static std::string KeyNumber2Note(int PianoKeyNumber);

        // note <-> midi number
        static int Note2MidiNumber(std::string& note);
        static std::string MidiNumber2Note(int PianoKeyNumber);
};
#endif
