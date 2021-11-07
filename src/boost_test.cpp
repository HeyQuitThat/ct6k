#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <stdio.h>
#include <string>
#include <iostream>

int main()
{
    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    std::string s = "\tMOVE 0x9988, R15 999999 444444444444444444444 ferd";
    tokenizer tok{s};
//    BOOST_FOREACH(const std::string& t, tok) {
    for (tokenizer::iterator it = tok.begin(); it != tok.end(); ++it) {
        std::string foo = *it;
        unsigned long ul;
        std::cout << foo << "\t";
        try {
            ul = std::stoul(foo,nullptr,0);
        } catch(std::invalid_argument) {
            std::cout << "Hmmm. Maybe this isn't a number after all\n";
            continue;
        } catch(std::out_of_range) {
            std::cout << "Error: argument out of range!\n";
            break;
        }
        std::cout << "Got a number, y'all: " << ul << "\n";
   }


}
