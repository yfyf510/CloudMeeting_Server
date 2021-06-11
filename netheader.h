#ifndef NETHEADER_H
#define NETHEADER_H

enum MSG_TYPE
{
    IMG_SEND = 0,
    IMG_RECV,
    AUDIO_SEND,
    AUDIO_RECV,
    TEXT_SEND,
    TEXT_RECV,
    CREATE_MEETING,
    EXIT_MEETING,
    JOIN_MEETING,

    CREATE_MEETING_RESPONSE = 20,
    PARTNER_EXIT = 21

};


#endif // NETHEADER_H
