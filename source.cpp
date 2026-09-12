#include <iostream>
#include <windows.h>
#include <mmeapi.h>
#include <vector>
#include <string>
#include <sstream>
#include<fstream>
#include<map>
#include <iterator>
#include <stdexcept>
#pragma comment(lib, "winmm.lib")
int loop_max = 3;
int sleep = 167;
void sendMIDI(HMIDIOUT hMidi, BYTE status, BYTE data1, BYTE data2) {
    DWORD msg = status | (data1 << 8) | (data2 << 16);
    midiOutShortMsg(hMidi, msg);
}

std::vector<int> getMelodyNotes() {
    std::vector<int> notes;
    std::string input;
    
    std::cout << "=== Welcome to the Housemade Melody Maker! [by lilias -winmm]" << std::endl;
    std::cout << "Skip manually enter to play default sound track, or pass in a file. use -help for infos!" << std::endl;
    std::cout << "Melody >> ";
    std::getline(std::cin, input);
    
    std::stringstream ss(input);
    int note;
    while (ss >> note) {
        notes.push_back(note);
    }
    if (notes.empty()) {
        notes = { 
            // Teil 1: Düsteres Intro
            60, 63, 65, 67, 60, 63, 65, 67, 
            62, 65, 67, 68, 62, 65, 67, 68,
            72, 70, 68, 67, 68, 67, 65, 63, 
            65, 67, 68, 70, 72, 74, 75, 79
        };
    }
    return notes;
}

std::vector<int> getBassNotes() {
    std::vector<int> notes;
    std::string input;
    
    std::cout << "Bass   >> ";
    std::getline(std::cin, input);
    
    std::stringstream ss(input);
    int note;
    while (ss >> note) {
        notes.push_back(note);
    }
    if (notes.empty()) {
        notes = { 
            // Passt zu Teil 1
            36, 36, 36, 36, 39, 39, 39, 39, 
            38, 38, 38, 38, 41, 41, 41, 41,
            // Passt zu Teil 2
            43, 43, 43, 43, 41, 41, 41, 41, 
            39, 39, 39, 39, 46, 46, 47, 48
        };
    }
    return notes;
}
enum EnumIdent {
    MELODY,
    BASS
};
enum FLAGS {
    SLEEP,
    LOOP
};
bool has{};
std::vector<int> cache;
bool hasNumber(const std::string& s) {
    return s.find_first_of("0123456789") != std::string::npos;
}
bool isNumeric(const std::string& s) {
    size_t i = (s.size() > 1 && (s[0] == '-' || s[0] == '+')) ? 1 : 0;
    if (i >= s.size()) {
        return false;
    }
    return s.find_first_not_of("0123456789", i) == std::string::npos;
}
std::vector<int> getMelody(std::string file, std::string targetheader, EnumIdent action) {
    if (has) {
        return cache;
    }
    bool found_header{};
    bool found_melody{};
    bool found_bass{};
    bool found_flags{};
    bool readFlags{};
    int init_counter{};
    int sec_counter{};
    std::string strmel{};
    std::vector<int> melody_;
    std::vector<int> bass_;
    std::string tmplit;
    char svlit{};
    for (const char lit : file) {
        if (found_bass) {
            // finish
            std::cout << "loaded melody!" << std::endl;
            break;
        }
        /*
            Bass handling
        */
        if (found_melody) {
            // parse bass
            if (lit == '\n') {
                if (hasNumber(strmel)) {
                    bass_.push_back(std::stoi(strmel));
                }
                strmel.clear();
                if (bass_.empty()) {
                    init_counter++;
                    continue;
                }
                if (bass_.empty()) {
                    throw "melody cant be empty!";
                } else {
                    found_bass = true;
                    init_counter++;
                    continue;
                }
            }
            if (lit == ' ') {
                if (hasNumber(strmel)) {
                    bass_.push_back(std::stoi(strmel));
                }
                strmel.clear();
            }
            strmel += lit;

            init_counter++;
            continue;
        }
        /*
            flag handling
        */
        if (readFlags) {
            if (lit == ' ' || lit == '\n') {
                if (svlit == 's' || svlit == 'l') {
                    if (tmplit.empty()) {
                        std::cout << "why is your " << svlit << " flag empty? usage: "
                                << svlit << "170 or " << svlit << "1" << std::endl;
                    } else if (!isNumeric(tmplit)) {
                        std::cout << "you serious? u need to do <f><num>, only "
                                << svlit << "2 or " << svlit << "23 etc. got: "
                                << svlit << tmplit << std::endl;
                    } else if (svlit == 's') {
                        sleep = std::stoi(tmplit);
                    } else {
                        loop_max = std::stoi(tmplit);
                    }
                }
                tmplit.clear();
                svlit = {};
                if (lit == '\n') {
                    readFlags = false;
                }
                init_counter++;
                continue;
            }
            if (svlit == 0) {
                if (lit == 's' || lit == 'l') {
                    svlit = lit;
                }
                init_counter++;
                continue;
            }
            tmplit += lit;
            init_counter++;
            continue;
        }
        /*
            after Header Handling
        */
        if (found_header) {
            if (lit == 's' || lit == 'l') { // flag: sleep
                svlit = lit;
                readFlags = true;
                init_counter++;
                continue;
            }
            /*
                Melody Handling
            */
            if (lit == '\n') {
                if (hasNumber(strmel)) {
                    melody_.push_back(std::stoi(strmel));
                }
                strmel.clear();
                if (melody_.empty()) {
                    init_counter++;
                    continue;
                }
                if (melody_.empty()) {
                    throw "melody cant be empty!";
                } else {
                    found_melody = true;
                    init_counter++;
                    continue;
                }
            }
            if (lit == ' ') {
                if (hasNumber(strmel)) {
                    melody_.push_back(std::stoi(strmel));
                }
                strmel.clear();
            }
            strmel += lit;

            init_counter++;
            continue;
        }
        /* Search Header Quotes *[*]* */
        if (lit == '[') {
            sec_counter = init_counter + 1;
        }
        if (lit == ']') {
            if (sec_counter == 0) {
                throw "error in for(const char lit : file) {\n  ... undefined error\n}";
            } else {
                int offset = init_counter - sec_counter;
                std::string header = file.substr(sec_counter, offset);
                if (header == targetheader) {
                    found_header = true;
                }
            }
        }
        init_counter++;
    }
    if (!found_header) {
        throw "the provided header was not found in the sound library. did you spelled it right?\nFormat is:\n[HEADER]\n...\n[HEADER2]\n...";
    }
    if (found_melody && !found_bass && hasNumber(strmel)) { // /flag or opt
        bass_.push_back(std::stoi(strmel));
    }
    if (melody_.empty() || bass_.empty()) {
        std::cout << "collected: f-s" << sleep << ".l" << loop_max << std::endl;
       throw "error collecting tones in function getMelody!"; 
    }
    switch (action) {
        case MELODY:
            cache = bass_;
            has = true;
            return melody_;
        case BASS:
            cache = melody_;
            has = true;
            return bass_;
            break;
        default:
            std::cout << "error in getMelody(file, targetheader, action)\n                                           /\\ HERE" << std::endl;
            break;
    }
    return {};
}
int main(int argc, char** argv) {
    // main globals
    bool file_used{};
    std::string filename{};
    std::string titlename{};
    // General Flag checks for flag
    for (int i = 0; i < argc; i++) {
        std::string c = argv[i];
        if (c == "-f") {
            filename = argv[i + 1];
            titlename = argv[i + 2];
            file_used = true;
        }
        if (c == "-help") {
            std::cout << "This is My Personal Music Maker based on 2 integar arrays of MELODY and BASS!" << std::endl;
            std::cout << "Usage:" << std::endl;
            std::cout << "  run the program normally to type in your own custom Melody/Bass logic" << std::endl;
            std::cout << "  pass argumentes -f <file> <title> to use file-written melodys: " << std::endl;
            std::cout << "     mymelody.txt " << std::endl;
            std::cout << "     [MY_TITLE]" << std::endl;
            std::cout << "     l10 s170 ; optional flags that behave like the -s, -l flags" << std::endl;
            std::cout << "     74 29 46 284 83 89 46 -34 -23 3" << std::endl;
            std::cout << "     29 47 29 47 29 43 20 26" << std::endl;
            std::cout << "     [OTHER]" << std::endl;
            std::cout << "  Then u do \"./melody mymelodie.txt\" \"MY_TITLE\"" << std::endl;
            std::cout << "  For setting the loop the sound will be played. use the -l flag followed by an 4 byte integer" << std::endl;
            std::cout << "  For setting the sleep duration between sound-samples use the -s flag followed by an 4 byte integer" << std::endl;
            std::cout << "  For using a own file with predefined melodies, use -f PATH TITLE" << std::endl;
            std::exit(0);
        }
        if (c == "-l") {
            loop_max = std::stol(argv[i + 1]);
            std::cout << "set -l (loop) flag to " << loop_max << std::endl;
        }
        if (c == "-s") {
            sleep = std::stol(argv[i + 1]);
            std::cout << "set -s (sleep) flag to " << sleep << std::endl;
        }
    }
    std::vector<int> melody{};
    std::vector<int> bass{};
    std::string file;
    std::string openquote = "[";
    std::string closequote = "]";
    if (file_used) {
        std::ifstream samples(filename);
        std::string target = titlename;
        if (!samples.is_open()) {
            std::cout << "file not found!" << std::endl;
            return 1;
        }
        file.assign((std::istreambuf_iterator<char>(samples)), std::istreambuf_iterator<char>());
        try {
            melody = getMelody(file, target, MELODY);
            bass = getMelody(file, target, BASS);
        } catch (const char* err) {
            std::cerr << err << std::endl;
            return 1;
        } catch (const std::exception& err) {
            std::cerr << "parse error: " << err.what() << std::endl;
            return 1;
        }
    } else {
        melody = getMelodyNotes();
        bass = getBassNotes();    
    }

    HMIDIOUT hMidi;
    if (midiOutOpen(&hMidi, MIDI_MAPPER, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        std::cerr << "Could not open Windows MIDI device!" << std::endl;
        return 1;
    }
    std::cout << "Playing..." << std::endl; 
    sendMIDI(hMidi, 0xC0, 80, 0); 
    sendMIDI(hMidi, 0xC1, 38, 0);

    size_t length = melody.size();
    for (int loop = 0; loop < loop_max; loop++) { 
        for (size_t i = 0; i < length; i++) {
            int currentMelody = melody[i];
            int currentBass = bass[i % bass.size()];
            sendMIDI(hMidi, 0x90, currentMelody, 127);
            sendMIDI(hMidi, 0x91, currentBass, 110);
            Sleep(sleep);
            sendMIDI(hMidi, 0x80, currentMelody, 0);
            sendMIDI(hMidi, 0x81, currentBass, 0);
        }
    }
    midiOutClose(hMidi);
    std::cout << "finished...!" << std::endl;
    return 0;
}