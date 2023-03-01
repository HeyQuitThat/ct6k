#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>


#define MAX_PUNCH 32
#define CHARS_PER_WORD 4
#define MAX_PACKED (MAX_PUNCH * CHARS_PER_WORD)

// Pack characters into a 32-bit word, Most Significant Byte first
// Output is padded with zeros if input length is less than an even
// multiple of four. This is detected by looking for terminating null.
uint32_t PackMSB(const char *Input)
{
    uint32_t retval {0};
    int pos = 24;
    for (int i = 0; i < CHARS_PER_WORD; i++) {
        if (*Input == 0)
            break;
        retval |= *Input << pos;
        pos -= 8;
        Input++;
    }
    return retval;
}

// Pack characters into a 32-bit word, Least Significant Byte first
// Output is padded with zeros if input length is less than an even
// multiple of four. This is detected by looking for terminating null.
uint32_t PackLSB(const char *Input)
{
    uint32_t retval {0};
    int pos = 0;
    for (auto i = 0; i < CHARS_PER_WORD; i++) {
        if (*Input == 0)
            break;
        retval |= *Input << pos;
        pos += 8;
        Input++;
    }
    return retval;
}

// Write a line of text to stdout, in Comp-o-Tron 3CS punch card format.
void Punch(char Mode, std::string Line)
{
    std::cout << '<' << Mode << "> ";
    std::cout << std::dec << Line.length() << '\n';
    std::cout << std::hex;
    if (Mode == 'U') {
        for (unsigned int i = 0; i < Line.length(); i++) {
            std::cout << (uint32_t)*(Line.data() + i);
            if ((i > 0) && ((i % 8) == 0))
                std::cout << '\n';
            else
                std::cout << ' ';
        }
    } else if ((Mode == 'M') || (Mode == 'L')) {
        for (unsigned int i = 0; i < Line.length(); i += 4) {
            uint32_t Word = (Mode == 'M' ?
                             PackMSB(Line.data() + i) :
                             PackLSB(Line.data() + i));
            std::cout << Word;
            if ((i > 0) && ((i % 32) == 0))
                std::cout << '\n';
            else
                std::cout << ' ';
        }
    } else {
        std::cerr << "Internal error: invalid punch mode\n";
        exit(1);
    }
    std::cout << '\n';
}

// Print simple usage to the console
void usage()
{
    std::cerr << "punch - a virtual card punch for the Comp-o-Tron 6000\n";
    std::cerr << "\tThis utility is only for text files!\n";
    std::cerr << "Usage:\n";
    std::cerr << "punch -[L|M|U]\n";
    std::cerr << "\t-L packs text LSB first\n";
    std::cerr << "\t-M packs text MSB first\n";
    std::cerr << "\t-U does not pack text; one character per 32-bit word\n\n";
}

// main(). You know, main(). Check command line params, then loop through
// standard input until we see EOF. For each line, call Punch() and have it
// spit out a record to stdout.
int main(int argc, char **argv)
{
    if ((argc != 2) || (argv[1][0] != '-') || (strlen(argv[1]) != 2)) {
        usage();
        exit(1);
    }
    char Option = argv[1][1];
    if ((Option != 'L') && (Option != 'M') && (Option != 'U')) {
        usage();
        exit(1);
    }
    std::cout << std::showbase;
    int LineCount {0};
    while (!std::cin.eof()) {
        std::string Input;
        std::getline(std::cin, Input); // getline lets us ignore line terminations
        if (Input.empty()) // Don't punch blank lines.
            continue;
        if ((Option == 'U') && (Input.length() > MAX_PUNCH)) {
            std::cerr << "Warning: input line " << LineCount << " truncated.\n";
            std::cerr << "Max length is " << MAX_PUNCH << ". Consider using -L or -M.\n";
            Input.resize(MAX_PUNCH);

        } else if (Input.length() > MAX_PACKED) {
            std::cerr << "Warning: input line " << LineCount << " truncated.\n";
            std::cerr << "Max length is " << MAX_PACKED << "\n";
            Input.resize(MAX_PACKED);
        }
        Punch(Option, Input);
        LineCount++;
    }
    return 0;
}
