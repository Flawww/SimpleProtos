# SimpleProtos
## Single Header protobuf parser and serializer
If you are like me and dislike having big external library dependencies in your projects, but you still want to be able to use Google's Protocol Buffers, this is for you. 

## Running
To run the tests you need C++11 or newer. With the compiler of your choosing simply compile test.cpp and run it and make sure none of the tests fail.
With gcc:
```
g++ test.cpp -o simple_proto_test
./simple_proto_test
```

## How it's implemented
Fairly intense preprocessor (ab)use to generate the structs and the proper parsing and serialization functions for the struct. 
The caveat with this approach over using the the normal library is that each protobuf struct you create will generate a considerable amount of code, adding to your binary.

## Usage
Refer to test.cpp to see an example of a "full" implementation.

### Reverse Engineering
You can dump (print) the contents of an unkown message for which you don't have the struct for. This is very useful when reverse-engineering network protocols which uses the "Lite" version of the messages, where you don't have the "reflection" and the structs aren't public.

To dump a message from its binary buffer:
```c++
proto_reader((uint8_t*)embedded_test_data, sizeof(embedded_test_data)).print();
```
This produces something like
![protodump](https://i.imgur.com/ktBuC26.png)

Where you see the fields, the datatype of the field, a hex-dump and an ascii-dump

### Implementation of proto structs
To create a struct you need to use the supplied macros like so:
```c++
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
```
Now to deserialize a binary buffer into this struct and accessing a member:
```c++
EXAMPLE_TEST_MESSAGE test_msg;
test_msg.deserialize((uint8_t*)embedded_test_data, sizeof(embedded_test_data))

printf("%i", test_msg.signed_integer.get()); // .get() returns as reference, so it is Read/Write with a = operator
```

To serialize a struct into a binary buffer you call the .serialize() function:
```c++
proto_writer* test_writer = test_msg.serialize();

// test_writer->m_pos is now the size
// test_writer->m_buf is the buffer for binary data.
```

