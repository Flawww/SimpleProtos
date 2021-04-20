#include <iostream>
#include <string.h>
#include "SimpleProtos.h"

constexpr uint8_t example_data[] = {        0x08, 0x02, 0x10, 0x05, 0x18, 0x00, 0x22, 0x1c, 0x23, 0x53, 0x46, 0x55, 0x49, 0x5f, 0x76, 0x6f,
                                            0x74, 0x65, 0x5f, 0x6b, 0x69, 0x63, 0x6b, 0x5f, 0x70, 0x6c, 0x61, 0x79, 0x65, 0x72, 0x5f, 0x6f,
                                            0x74, 0x68, 0x65, 0x72, 0x2a, 0x09, 0x2d, 0x3d, 0x56, 0x69, 0x6d, 0x65, 0x52, 0x3d, 0x2d, 0x32,
                                            0x20, 0x23, 0x53, 0x46, 0x55, 0x49, 0x5f, 0x6f, 0x74, 0x68, 0x65, 0x72, 0x74, 0x65, 0x61, 0x6d,
                                            0x5f, 0x76, 0x6f, 0x74, 0x65, 0x5f, 0x6b, 0x69, 0x63, 0x6b, 0x5f, 0x70, 0x6c, 0x61, 0x79, 0x65,
                                            0x72, 0x40, 0x02 };

constexpr uint8_t embedded_test_data[] = {  0x0A, 0x02, 0x08, 0x03,                             /* 0x0A - field 1, length delimited (length 2), payload is <VARINT, FIELD 1> 3 */
                                            0x12, 0x03, 0x08, 0x8E, 0x02,                       /* 0x12 - field 2, length delimeted (length 3), payload is <VARINT, FIELD 1> 270 */
                                            0x12, 0x04, 0x08, 0x9E, 0xA7, 0x05,                 /* 0x12 - field 2, lengt delimited (length 4) payload is <VARINT, FIELD 1> 86942 */
                                            0x18, 0x96, 0x01,                                   /* 0x18 - field 3, varint, value is 150*/
                                            0x20, 0x03,                                         /* 0x20 - field 4, signed varint, value is -2 */
                                            0x2A, 0x06, 0x03, 0x8E, 0x02, 0x9E, 0xA7, 0x05 };   /* 0x2A - field 5, length delimited (length 6) payload is packed varint with values 3, 270, 87942 */

struct EXAMPLE_EMBEDDED_MESSAGE { 
    FIELDS(
        FIELD_VARINT(1, bruh)
    )

    ADD_FIELD_OPTIONAL(int32_t, bruh);
};

struct EXAMPLE_TEST_MESSAGE {
    FIELDS(
        FIELD_MESSAGE(1, msg)
        REPEATED_FIELD_MESSAGE(2, msgvec)
        FIELD_VARINT(3, integer)
        FIELD_SIGNED_VARINT(4, signed_integer)
        PACKED_FIELD_VARINT(5, integervec)
    )

    ADD_FIELD_OPTIONAL(EXAMPLE_EMBEDDED_MESSAGE, msg);
    ADD_FIELD_OPTIONAL(std::vector<EXAMPLE_EMBEDDED_MESSAGE>, msgvec);
    ADD_FIELD_OPTIONAL(int32_t, integer);
    ADD_FIELD_OPTIONAL(int32_t, signed_integer);
    ADD_FIELD_OPTIONAL(std::vector<int32_t>, integervec);
};

/* Example struct from https://github.com/SteamDatabase/Protobufs/blob/master/csgo/cstrike15_usermessages.proto, data to test was taken from a live game */
struct CCSUsrMsg_VoteStart {
    FIELDS(
        FIELD_VARINT(1, team)
        FIELD_VARINT(2, ent_idx)
        FIELD_VARINT(3, vote_type)
        FIELD_BUFFER(4, disp_str)
        FIELD_BUFFER(5, details_str)
        FIELD_BUFFER(6, other_team_str)
        FIELD_VARINT(7, is_yes_no_vote)
        FIELD_VARINT(8, entidx_target)
    );

    ADD_FIELD_OPTIONAL(int32_t, team);
    ADD_FIELD_OPTIONAL(int32_t, ent_idx);
    ADD_FIELD_OPTIONAL(int32_t, vote_type);
    ADD_FIELD_OPTIONAL(std::string, disp_str);
    ADD_FIELD_OPTIONAL(std::string, details_str);
    ADD_FIELD_OPTIONAL(std::string, other_team_str);
    ADD_FIELD_OPTIONAL(bool, is_yes_no_vote);
    ADD_FIELD_OPTIONAL(int32_t, entidx_target);
};

#if 1
int main()
{
    printf("DATA DUMP FOR EMBEDDED MESSAGE TEST:\n");
    proto_reader((uint8_t*)embedded_test_data, sizeof(embedded_test_data)).print();
    EXAMPLE_TEST_MESSAGE test_msg;
    if (!test_msg.deserialize((uint8_t*)embedded_test_data, sizeof(embedded_test_data))) {
        printf("Failed to deserialize embedded message\n");
        return 1;
    }

    printf("\nmsg.msg.bruh = %i\nmsg.integer = %i\nmsg.signed_integer = %i\n\nmsg.msgvec.size() = %i\n", test_msg.msg.get().bruh.get(), test_msg.integer.get(), test_msg.signed_integer.get(), (uint32_t)test_msg.msgvec.get().size());
    for (uint32_t i = 0; i < test_msg.msgvec.get().size(); i++)
        printf("msg.msgvec[%i].bruh = %i\n", i, test_msg.msgvec.get()[i].bruh.get());
    printf("\nmsg.integervec.size() = %i\n", (uint32_t)test_msg.integervec.get().size());
    for (uint32_t i = 0; i < test_msg.integervec.get().size(); i++)
        printf("mss.integervec[%i] = %i\n", i, test_msg.integervec.get()[i]);

    proto_writer* test_writer = test_msg.serialize();
    if (test_writer->m_pos == sizeof(embedded_test_data) && !memcmp(test_writer->m_buf, embedded_test_data, test_writer->m_pos))
        printf("\nFully re-serialized messaged correctly!\n");
    else
        printf("\nERROR: failed to re-serialize message\n");

    delete test_writer;

    printf("\n\nDATA DUMP FOR 'REAL-LIFE' MESSAGE:\n");
    proto_reader((uint8_t*)example_data, sizeof(example_data)).print();
    CCSUsrMsg_VoteStart msg;
    if (!msg.deserialize((uint8_t*)example_data, sizeof(example_data))) {
        printf("Failed deserialize\n");
        return 1;
    }
        
    printf("\nteam = %i\nent_idx = %i\ndisp_str = %s\ndetails_str = %s\n", msg.team.get(), msg.ent_idx.get(), msg.disp_str.get().c_str(), msg.details_str.get().c_str());
    
    proto_writer* writer = msg.serialize();
    if (writer->m_pos == sizeof(example_data) && !memcmp(writer->m_buf, example_data, writer->m_pos))
        printf("\nFully re-serialized messaged correctly!\n");
    else
        printf("\nERROR: failed to re-serialize message\n");

    delete writer;

    return 0;
}
#endif