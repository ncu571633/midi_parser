#ifndef __MUSIC_HPP_
#define __MUSIC_HPP_

#include <string>
#include <vector>

class MusicUtility
{
    private:
        static const std::string pianoKeyBoard[];
        static int PianoOctaveKeyNumber;
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
};
#endif
