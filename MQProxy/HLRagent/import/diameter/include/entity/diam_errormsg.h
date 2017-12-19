#ifndef __DIAM_ERRORMSG_H__
#define __DIAM_ERRORMSG_H__

#include <api/diam_message.h>
#include <util/diam_sharedptr.h>

class DiamTransChannel;
class DiamErrorMsg
{
    /*

    7.2.  Error Bit

    The 'E' (Error Bit) in the Diameter header is set when the request
    caused a protocol-related error (see Section 7.1.3).  A message with
    the 'E' bit MUST NOT be sent as a response to an answer message.
    Note that a message with the 'E' bit set is still subjected to the
    processing rules defined in Section 6.2.  When set, the answer
    message will not conform to the ABNF specification for the command,
    and will instead conform to the following ABNF:

    Message Format

    <answer-message> ::= < Diameter Header: code, ERR [PXY] >
    0*1< Session-Id >
    { Origin-Host }
    { Origin-Realm }
    { Result-Code }
    [ Origin-State-Id ]
    [ Error-Reporting-Host ]
    [ Proxy-Info ]
    * [ AVP ]

    Note that the code used in the header is the same than the one found
    in the request message, but with the 'R' bit cleared and the 'E' bit
    set.  The 'P' bit in the header is set to the same value as the one
    found in the request message.
    */

public:
    static SharedPtr<DiamMsg> Generate(DiamMsg &request, DiamUINT32 rcode);

    static SharedPtr<DiamMsg> Generate(DiamHeader& hdr, DiamUINT32 rcode);
};

#endif
