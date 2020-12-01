#ifndef PARSER_HPP
#define PARSER_HPP

#include "messages.hpp"

class Parser
{
public:
    Parser(uint16_t protocol_version);
    Message decode(const char * data);

private:
    uint16_t protocol_version_;
};


#endif //PARSER_HPP
