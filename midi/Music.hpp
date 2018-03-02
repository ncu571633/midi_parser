#ifndef __MUSIC_HPP_
#define __MUSIC_HPP_

#include <string>
#include <vector>

// MusicUtility class does the conversion among the notations below:
// music note(English notation): pitch-scale: like 4c, 5d
// piano key number(88 key): 40 (c4) 
// midi number

class MusicUtility
{
    private:
        // 12-tone chromatic scale
        static const std::string scale[];
        static int scaleSize;
       
        // NoteArray:   index: 
        //              value: note
        // pre-calculate
        static const int NoteArraySize = 1024;
        static std::vector<int> NoteArray;
        static void initNoteArray();

        static int code(const std::string& note);
        static std::string decode(int number);
    public:
        // midi number <-> piano key number
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
