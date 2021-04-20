#pragma once
#include <string>
#include <vector>

#define PROTO_PRINTER 1
#ifdef _WIN32
#define LONG_INTEGER_FORMAT "%lli"
#define LONG_HEX_FORMAT "%llx"
#else
#define LONG_INTEGER_FORMAT "%li"
#define LONG_HEX_FORMAT "%lx"
#endif

/*
	For your own sanity, never read these macros.
	If a future employer ever sees this, ~~no you didnt~~
*/

#define FIELD(n, name, wire_type, read_op, write_op) case n: { if (is_deserialize) { decltype(name.m_value) proto_value; if (varint_key.m_wire_type != wire_type) return 0; read_op; name.set(proto_value); break; }	\
													 else { if (name.m_exists) { proto_write->write_key(proto_key(n, wire_type)); write_op; } } }

#define FIELD_VARINT_ENCODED(n, name, zigzag) FIELD(n, name, VARINT, if (!proto_read.read_varint(proto_value, zigzag)) return 0;, proto_write->write_varint(name.get(), zigzag))
#define FIELD_VARINT(n, name) FIELD_VARINT_ENCODED(n, name, false)
#define FIELD_SIGNED_VARINT(n, name) FIELD_VARINT_ENCODED(n, name, true)

#define FIELD_FIXED32(n, name) FIELD(n, name, FIXED32, if (!proto_read.read_fixed(proto_value)) return 0;, proto_write->write_fixed(name.get()))
#define FIELD_FIXED64(n, name) FIELD(n, name, FIXED64, if (!proto_read.read_fixed(proto_value)) return 0;, proto_write->write_fixed(name.get()))
#define FIELD_BUFFER(n, name) FIELD(n, name, LENGTH_DELIMITED, if (!proto_read.read_buffer(proto_value)) return 0;, proto_write->write_buffer(name.get()))
#define FIELD_MESSAGE(n, name) FIELD(n, name, LENGTH_DELIMITED, std::string proto_message_buf; if (!proto_read.read_buffer(proto_message_buf) || !proto_value.deserialize((uint8_t*)proto_message_buf.data(), proto_message_buf.length())) return 0;, proto_writer* proto_message_writer = name.get().serialize(); proto_write->write_buffer((void*)proto_message_writer->m_buf, proto_message_writer->m_pos); delete proto_message_writer;)

#define REPEATED_FIELD(n, name, wire_type, read_op, write_op) case n: { if (is_deserialize) { std::remove_reference<decltype(name.m_value.front())>::type proto_value; if (varint_key.m_wire_type != wire_type) return 0; read_op; name.m_exists = true; name.m_value.push_back(proto_value); break; }	\
													 else { if (name.m_exists) { for (auto& proto_iter : name.get()) { proto_write->write_key(proto_key(n, wire_type)); write_op; } } } }

#define REPEATED_FIELD_VARINT_ENCODED(n, name, zigzag) REPEATED_FIELD(n, name, VARINT, if (!proto_read.read_varint(proto_value, zigzag)) return 0;, proto_write->write_varint(proto_iter, zigzag))
#define REPEATED_FIELD_VARINT(n, name) REPEATED_FIELD_VARINT_ENCODED(n, name, false)
#define REPEATED_FIELD_SIGNED_VARINT(n, name) REPEATED_FIELD_VARINT_ENCODED(n, name, true)
#define REPEATED_FIELD_FIXED32(n, name) REPEATED_FIELD(n, name, FIXED32, if (!proto_read.read_fixed(proto_value)) return 0;, proto_write->write_fixed(proto_iter))
#define REPEATED_FIELD_FIXED64(n, name) REPEATED_FIELD(n, name, FIXED64, if (!proto_read.read_fixed(proto_value)) return 0;, proto_write->write_fixed(proto_iter))
#define REPEATED_FIELD_BUFFER(n, name) REPEATED_FIELD(n, name, LENGTH_DELIMITED, if (!proto_read.read_buffer(proto_value)) return 0;, proto_write->write_buffer(proto_iter))
#define REPEATED_FIELD_MESSAGE(n, name) REPEATED_FIELD(n, name, LENGTH_DELIMITED, std::string proto_message_buf; if (!proto_read.read_buffer(proto_message_buf) || !proto_value.deserialize((uint8_t*)proto_message_buf.data(), proto_message_buf.length())) return 0;, proto_writer* proto_message_writer = proto_iter.serialize(); proto_write->write_buffer((void*)proto_message_writer->m_buf, proto_message_writer->m_pos); delete proto_message_writer;)

#define PACKED_FIELD(n, name, wire_type, read_op, write_op) case n: { if (is_deserialize) { std::remove_reference<decltype(name.m_value.front())>::type proto_value; size_t proto_packed_size; if (varint_key.m_wire_type != LENGTH_DELIMITED || !proto_read.read_varint(proto_packed_size)) return 0; size_t proto_packed_end = proto_read.m_pos + proto_packed_size; while (proto_read.m_pos < proto_packed_end) { read_op; name.m_exists = true; name.m_value.push_back(proto_value); } break; }	\
													 else { if (name.m_exists) { std::string proto_packed_buf; proto_writer* proto_write_packed = new proto_writer(); for (auto& proto_iter : name.get()) { write_op; } proto_write->write_key(proto_key(n, LENGTH_DELIMITED)); proto_write->write_buffer((void*)proto_write_packed->m_buf, proto_write_packed->m_pos); delete proto_write_packed; } } }

#define PACKED_FIELD_VARINT_ENCODED(n, name, zigzag) PACKED_FIELD(n, name, VARINT, if (!proto_read.read_varint(proto_value, zigzag)) return 0;, proto_write_packed->write_varint(proto_iter, zigzag))
#define PACKED_FIELD_VARINT(n, name) PACKED_FIELD_VARINT_ENCODED(n, name, false)
#define PACKED_FIELD_SIGNED_VARINT(n, name) PACKED_FIELD_VARINT_ENCODED(n, name, true)
#define PACKED_FIELD_FIXED32(n, name) PACKED_FIELD(n, name, FIXED32, if (!proto_read.read_fixed(proto_value)) return 0;, proto_write_packed->write_fixed(proto_iter))
#define PACKED_FIELD_FIXED64(n, name) PACKED_FIELD(n, name, FIXED64, if (!proto_read.read_fixed(proto_value)) return 0;, proto_write_packed->write_fixed(proto_iter))

#define DESERIALIZE(args)   bool deserialize(uint8_t* buf, size_t size) {                                       \
                                proto_reader proto_read = proto_reader(buf, size);                              \
								proto_writer* proto_write = nullptr;											\
								bool is_deserialize = true;														\
                                while (!proto_read.finished()) {                                                \
                                    proto_key varint_key; if(!proto_read.read_key(varint_key)) return false;    \
                                    switch (varint_key.m_field_number) {                                        \
                                        args                                                                    \
                                        default: { return false; }                                              \
                                    }                                                                           \
                                }                                                                               \
                                return true;                                                                    \
                            }

#define SERIALIZE(args)		proto_writer* serialize() {															\
								proto_writer* proto_write = new proto_writer(true);								\
								proto_reader proto_read = proto_reader();										\
								bool is_deserialize = false;													\
								proto_key varint_key; /* not actually used here but we need it declared... */	\
								switch (-1) { /* this is whole-heartedly fucking cursed and I love it */		\
								case -1:																		\
									args																		\
								}																				\
								return proto_write;																\
							}

#define FIELDS(args) DESERIALIZE(args) SERIALIZE(args)

#define ADD_FIELD_OPTIONAL(type, name) optional_field<type> name = optional_field<type>(false)
#define ADD_FIELD_REQUIRED(type, name) optional_field<type> name = optional_field<type>(true)

struct proto_key {
	proto_key() : m_field_number(0), m_wire_type(0) {}
	proto_key(int n, int t) : m_field_number(n), m_wire_type(t) {}
	uint64_t m_field_number;
	uint8_t m_wire_type;
};

template <typename t>
struct optional_field {
	optional_field(bool e = false) : m_exists(e) {}
	t& get() { return m_value; }
	void set(t val) { m_value = val; m_exists = true; }

	bool m_exists = false;
	t m_value;
};

enum wire_types {
	VARINT,
	FIXED64,
	LENGTH_DELIMITED,
	START_GROUP, // deprecated
	END_GROUP, // deprecated
	FIXED32,
};

static void print_ascii(uint8_t* data, uint16_t size, int num_indents);

class proto_reader {
public:
	proto_reader() : m_buf(nullptr), m_size(0), m_pos(0) {}
	proto_reader(uint8_t* buf, size_t size) : m_buf(buf), m_size(size), m_pos(0) { }

	bool read_key(proto_key& key) {
		uint64_t value;
		if (!read_varint(value))
			return false;

		key.m_wire_type = value & 0x7;
		key.m_field_number = value >> 3;
		return true;
	}

	template <typename t>
	bool read_varint(t& val, bool zigzag = false) {
		uint64_t value = 0;
		size_t offset = 0;

		while (true) {
			if (m_pos >= m_size) {
				return false;
			}

			unsigned char c;
			c = m_buf[m_pos++];
			// transfer 7 bits per byte
			value |= uint64_t(c & 0x7f) << offset;
			// check terminator
			if ((c & 0x80) == 0)
				break;
			offset += 7;
		}

		// decode ZigZag encoding
		if (zigzag) {
			value = (value >> 1) ^ -(int64_t)(value & 1);
		}

		val = value;
		return true;
	}

	template <typename t>
	bool read_fixed(t& val) {
		if (m_pos + sizeof(t) > m_size)
			return false;

		val = *(t*)(m_buf + m_pos);
		m_pos += sizeof(t);
		return true;
	}

	bool read_buffer(std::string& val) {
		uint64_t length; // read the length of the buffer first
		if (!read_varint(length) || m_pos + length > m_size)
			return false;

		val = std::string((const char*)(m_buf + m_pos), length);
		m_pos += length;
		return true;
	}

	bool finished() {
		return m_pos == m_size;
	}

	// print-related functions at bottom of file to make em easy to #ifdef away
#if PROTO_PRINTER
	void print() {
		while (m_size > m_pos) {
			proto_key key;
			if (!read_key(key))
				return;

			printf("FIELD " LONG_INTEGER_FORMAT ":\n\t", key.m_field_number);
			switch (key.m_wire_type) {
			case VARINT:
			{
				uint64_t value;
				if (!read_varint(value))
					return;

				printf("Varint: " LONG_HEX_FORMAT "\n", value);
			}
			break;
			case FIXED64:
			{
				uint64_t value;
				if (!read_fixed(value))
					return;

				double double_value = *reinterpret_cast<double*>(&value);
				printf("64-bit: " LONG_HEX_FORMAT " | %f\n", value, double_value);
			}
			break;
			case LENGTH_DELIMITED:
			{
				std::string buf;
				if (!read_buffer(buf))
					return;

				printf("Buffer: len %i\n", (uint32_t)buf.length());
				print_ascii((uint8_t*)buf.data(), buf.length(), 2);
			}
			break;
			case FIXED32:
			{
				uint32_t value;
				if (!read_fixed(value))
					return;

				float flt_value = *reinterpret_cast<float*>(&value);
				printf("32-bit: %x | %f\n", value, flt_value);
			}
			break;
			default:
			{
				printf("Invalid wire type %i\n", key.m_wire_type);
			}
			}
		}
	}
#endif
	size_t m_pos;
private:

	uint8_t* m_buf;
	size_t m_size;
};

class proto_writer {
public:
	proto_writer(bool alloc = true) : m_buf(nullptr), m_buf_size(0), m_pos(0) {
		if (alloc) {
			m_buf = new uint8_t[0x1000];
			m_buf_size = 0x1000;
			m_pos = 0;
		}
	}

	~proto_writer() {
		delete[] m_buf;
	}

	void expand_buffer() {
		auto tmp = new uint8_t[m_buf_size + 0x1000];

		if (m_buf) {
			memcpy(tmp, m_buf, m_pos);
			delete[] m_buf;
		}

		m_buf_size += 0x1000;
		m_buf = tmp;
	}

	void write_key(proto_key key) {
		write_varint((key.m_field_number << 3) | key.m_wire_type);
	}

	void write_varint(uint64_t val, bool zigzag = false) {
		if (zigzag) {
			val = (val << 1) ^ ((int64_t)val >> 63);
		}

		while (true) {
			if (m_pos >= m_buf_size)
				expand_buffer();

			unsigned char c = val & 0x7f;
			val >>= 7;
			if (val) {
				c |= 0x80;
				m_buf[m_pos++] = c;
			}
			else {
				// no more bits left
				m_buf[m_pos++] = c;
				break;
			}
		}
	}

	template <typename t>
	void write_fixed(t val) {
		if (m_pos + sizeof(t) > m_buf_size)
			expand_buffer(); // no need for a while loop here, fixed values can only be 32 or 64 bits anyways.

		*(t*)(m_buf + m_pos) = val;
		m_pos += sizeof(t);
	}

	void write_buffer(std::string val) {
		write_buffer((void*)val.data(), val.length());
	}

	void write_buffer(void* buf, size_t size) {
		write_varint(size);

		// can be arbitrary size, we need to loop until our buffer is big enough.
		while (m_pos + size > m_buf_size)
			expand_buffer();

		memcpy((void*)(m_buf + m_pos), buf, size);
		m_pos += size;
	}

	uint8_t* m_buf;
	size_t m_pos;
private:
	size_t m_buf_size;
};

/* PRINT-RELATED FUNCTIONS */
#if PROTO_PRINTER
static void print_ascii(uint8_t* data, uint16_t size, int num_indents) {
	for (int y = 0; y < num_indents; y++)
		printf("\t");

	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		}
		else {
			ascii[i % 16] = '.';
		}
		if ((i + 1) % 8 == 0 || i + 1 == size) {
			printf(" ");
			if ((i + 1) % 16 == 0) {
				printf("|  %s \n", ascii);
				for (int y = 0; y < num_indents; y++)
					printf("\t");
			}
			else if (i + 1 == size) {
				ascii[(i + 1) % 16] = '\0';
				if ((i + 1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i + 1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}

	printf("\n");
}
#endif