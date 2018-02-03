#include "Midi.hpp"

#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>

//get a fixed length(bits) word.
int getNBitsNumber(const std::string &midistr, int bits, size_t& offset)
{
    int number = 0;
    while (bits--)
    {
        number = (number<<8) | midistr[offset];
        offset ++;
    }
    return number;
}

//get a variable length word.
DWORD getDWord(const std::string &midistr, size_t& offset)
{
    DWORD number = midistr[offset++];
    if ( number & 0x80 )
    {
        number = number & 0x7f;
        do
        {
            number = (number<<7) + (midistr[offset]&0x7f);
        } while (midistr[offset++]&0x80);
    }
    return number;
}

void getDWordContent(const std::string &midistr, int type, size_t& offset)
{
	for (size_t i=0; i<getDWord(midistr, offset); i++)
	{
		if (type)// print the ASCII content
			std::cout << midistr[offset++];
		else//system sysex event "%x "
			std::cout << std::hex << midistr[offset++];
	}
}

void MidiFile::importHeaderChunk(const std::string& midistr, size_t& offset)
{
    if (midistr.size()<4 || midistr.substr(offset, 4)!="MThd"
            || !(headerChunk.chunkSize=getNBitsNumber(midistr, 4, offset=offset+4)) || headerChunk.chunkSize!=6 )
    {
        throw std::runtime_error("HeaderChunk Wrong file head.\n");
    }

    headerChunk.format = getNBitsNumber(midistr, 2, offset);
    headerChunk.tracksNumber = getNBitsNumber(midistr, 2, offset);
    headerChunk.deltaTimeTicks = getNBitsNumber(midistr, 2, offset);
}

void MidiFile::importTrackChunks(const std::string& midistr, int& time, size_t& offset)
{
    TrackChunk trackChunk;

    //track chunk head: MTrk
    if (midistr.size()<offset+4 || midistr.substr(offset, 4)!="MTrk"
        //track chunk size:
        || !(trackChunk.chunkSize = getNBitsNumber(midistr, 4, offset=offset+4)))
    {
        throw std::runtime_error("TrackChunk Wrong file head.\n");
    }
            
            /*
    int itemp=0, finished=false, deltaTime=0;
    pm->time = 0;


    if(parseverbose)
        fprintf(fp, "<Track Size=\"%d\"/>\n", itemp);

    //read event
    while (!finished)
    {
        deltaTime = getDWord(midifp, &pm->offset);
        pm->time = pm->time + deltaTime;
        pm->deltaTime = deltaTime;  //used for check whether it is a new chord

        fprintf(fp, "  <Event>\n");
        fprintf(fp, "    <Delta>%d</Delta>\n", pm->deltaTime);
        if( (pm->deltaTime!=0 && pm->lastNote==true)
          )   //a new chord
            pm->ChordCount++;
        if(parseverbose)
        {
            fprintf(fp, "    <Absolute>%d</Absolute>\n", pm->time);
            fprintf(fp, "    <OffSet>%d</OffSet>\n", pm->offset);
        }

        ctemp1 = getc(midifp);
        if ( ctemp1 && 0x80 )
        {
            if (ctemp1==0xff)
            {
                if(parseverbose) 
                    fprintf(fp, "  <MetaEvent Value=\"0x%x\">", ctemp1);
                finished = parseMetaEvent(midifp, fp, true, &pm->offset);
                if(parseverbose) 
                    fprintf(fp, "</MetaEvent>");
            }
            else
            {
                if ((ctemp1>>4)==0xf)
                {
                    if(parseverbose)
                        fprintf(fp, "  <SysexEvent Value=\"0x%x\">", ctemp1);
                    parseDWordContent(midifp, 0, fp, true, &pm->offset);
                    if(parseverbose) 
                        fprintf(fp, "  </SysexEvent>");
                }
                else
                {
                    if (ctemp1>>4)
                    {
                        if(parseverbose)
                            fprintf(fp, "  <MidiEvent Value=\"0x%x\">", ctemp1);
                        parseMidiEvent(ctemp1, midifp, fp, pm);
                        if(parseverbose) 
                            fprintf(fp, "  </MidiEvent>");
                    }
                    else
                    {
                        fclose(fp);
                        ctemp[0]=ctemp1;
                        ctemp[1]='\0';
                        printErrorMsg(ERROR_midiEvent, ctemp);
                    }
                }
            }
        }
        else
        {
            fprintf(fp, "Useless data: %x\n", ctemp1);
            //printErrorMsg(ERROR_midiEvent, (void *)NULL);
        }
        fprintf(fp, "  </Event>\n");
    }
*/
    trackChunks.push_back(trackChunk);
}

void MidiFile::importMidiFile(const std::string& fileName)
{
    try {
        std::ifstream midifp(fileName.c_str(), std::ios::in | std::ios::binary);
        if(!midifp)
        {
            throw std::runtime_error(std::strerror(errno));
        }
        std::string midistr = std::string((std::istreambuf_iterator<char>(midifp)), std::istreambuf_iterator<char>());

        size_t offset = 0;
        //read midi file head chunk
        importHeaderChunk(midistr, offset);

        //read midi file track chunks
        int time = 0;
        for (size_t i=0; i<headerChunk.tracksNumber; i++)
        {
            importTrackChunks(midistr, time, offset);
        }

    } catch (const std::exception& e) {
        std::cerr << fileName << " " << e.what();
    }
}
