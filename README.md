# Data Packet

This project provides data packets that are represented as structs, but have
support for "automatic" serialization/deserialization, which handles both
dynamic fields and endianess changes.

The only requirement for different systems to communcicate is to have consensus
in what each packets composition is.

All this:

* Without adding a pre-compilation stage;
* As little extra programming as possible;
* Pure C + Basic Preprocessor;
* Near-native performance (to validate).

This can be validated using [packet definitions](#packet-definitions)

## Usage

### Defining packets

Packets are defined via `.def` files. These files must start with:

```C++
// Inside some <new_packet>.def
#define PACKET_NAME <new_packet_name>
PACKET_START()
```

The lines that follow are the packets' fields.

Static fields are defined with:

```C++
PACKET_FIELD(<field_type>, <field_name>)
```

Dynamic fields are defined with:

```C++
PACKET_FIELD(<field_name>)
```

Dynamic fields have a default type of `opaque_memory` which is defined as:

```C++
struct opaque_memory{
    size_t size;
    void*  data;
};
```

An example of a full packet (also found in `executables/tests`) is:

```C++
#define PACKET_NAME packet_dynamic
PACKET_START()
PACKET_FIELD(uint16_t,  two_byte_field)
PACKET_FIELD(uint32_t,  four_byte_field)
PACKET_FIELD_DYNAMIC(   dyn_1)
PACKET_FIELD(uint16_t, two_byte_field_2)
PACKET_FINALIZE()
```

Besides the metadata, this packet is equivalent to the following struct, and can
be used as such:

```C++
struct packet_dynamic {
    base_packet base;
    uint16_t        two_byte_field;
    uint32_t        four_byte_field;
    opaque_memory   dyn_1;
    uint16_t        two_byte_field_2;
}
```

Note the `base` field. It is inserted into the beginning of each packet, is
composed of 1 byte fields (meaning it doesn't change the type of any packets)
and is currently 2 bytes long.

It is necessary to both identify the packet, ensure endianess is correct, and
may provide extra functionality in the future. As such, it should be largely ignored by the user.

### Setting up packets

In order to setup the packets defined in `.def` files, two snippets of code must
be present per packet.

One, in the header where the struct definition will be included from:

```C
// In some <my_header.h> file
#define PACKET_DEF_FILE "<new_packet>.def"
#include "packet_header.h"
#undef PACKET_DEF_FILE
```

And the other one, inside some function that must run before packets are
used/finalized

```C
void your_projects_setup_phase() {
    ...
    #define PACKET_DEF_FILE "<new_packet>.def"
    #include "packet_body.c"
    #undef PACKET_DEF_FILE
    ...
}
```

After all "your_projects_setup_phase()" functions run and all packets have been
set up, run `finalize_packets()`.

This call is necessary to setup the internal structures such that packet
operations run smoothly.

When shutting down, you can run `release_packets()` to free up all packet
operations and metadata.

**WARNING**: After running `release_packets()` packet operations might segfault, so
be sure none will happen after the call.

### Using packets

The main operations for packets are as follows:

```C
/* Allocates packet memory for the provided id
 * Returned packet must be released by the user via packet_deallocate
 */
packet_result packet_allocate(packet_id_t packet_id);

/* Deallocates packet memory. Non-NULL dynamic fields are freed
 */
void packet_deallocate(void* _packet);

/* Allocates memory and fills it with packet representation
 * Releasing this memory (with free) is the users responsibility
 */
memory_result packet_serialize(void* _packet);

/* Allocates and fills packet from provided memory
 * Returned packet must be released by the user via packet_deallocate
 */
packet_result packet_deserialize(opaque_memory mem);

/* Return the packet id that corresponds to the packet name received
 */
packet_id_t get_packet_id(const char* name);
```

Note the `packet_result` and `memory_result` structs. They are defined as:

```C
struct packet_result{
  bool          success;
  base_packet*  pac;
};
struct memory_result{
  bool          success;
  opaque_memory memory;
};
```

Since accessing packet fields is the same as accessing struct fields, all that
is relevant is when we want to \[de]/allocate or \[de]serialize them.

A brief example taken from `executables/tests/dynamic_test.c`:

```C
// From the example above
packet_result       new_packet_res;
packet_dynamic*     new_packet;
packet_id_t         new_packet_id;
// only need to call get_packet_id once and cache the value
//  the packet id wont change while the code is running
new_packet_id = get_packet_id("packet_dynamic");

// allocate and check for success
new_packet_res = packet_allocate(dyn_id);
assert(new_packet_res.success == true);

// use the new packet however you want
new_packet                  = new_packet_res.pac;
dyn_to_send->two_byte_field = 0x0123;
...

// now we want to send it somewhere
memory_result           serialized;
serialized = packet_serialize(dyn_to_send);
assert(serialized.success == true);

// void send_data(void* data, size_t size)
send_data(serialized.memory.data, serialized.memory.size);

// clean everything up that is our responsibility
packet_deallocate(new_packet);
free(serialized.memory.data);
```

### Packet definitions

### How to setup the project

This project is built on top of [ProjectBase](https://gitlab.com/brunoasmauricio/ProjectBase), which is needed to setup the correct dependencies.

To use it, checkout [an example here](https://gitlab.com/brunoasmauricio/ProjectBase/-/wikis/Setup-and-Run#testrepo1-example).

The base invocation of ProjectBase should be:

```shell
./run.sh --url https://gitlab.com/brunoasmauricio/data_packet
```

## Concept

This project makes use of x-macros. This is done so packets only need to be
defined once, and we can use that definition to generate whatever metadata is
required, to setup the appropriate environment.

Each packets' type is inferred from its' fields during `finalize_packets` and
its' operations are set based on that.

The four main operations (\[de]/allocate and \[de]serialize) are nothing more
than accesses to an operations array, indexed by the packet id.

### Field types

Each field in a packet can be one of three types.

**Basic** (**BSC**) fields:
Basic fields are those that are only 1 byte in size.

**Multi byte** (**MBY**) fields:
Fields with more than 1 byte.

**Dynamic** (**DYN**) fields:
Fields with a dynamic size.

### Packet types

Depending on the types of fields, a packet can have one of three types.

The packets' type defines the operations it must perform.

**Basic** packets: Only has **BSC** fields.

**Multi Byte** packets: Has at least 1 **MBY** field, can have **BSC** as well.

**Dynamic** packets: Has at least **DYN** fields, can have **MBY**/**BSC** as well.



## RoadMap / Project Status

This projects' main core is complete!

Valgrind shows no memory leaks if everything is properly shutdown!

There are some TODOs marked below, but they are secondary, and will be implemented
in due time.

### TODOs

- [ ] Opt for network byte order instead of endian field
- [ ] Add option to generate wireshark packet decomposer automatically
- [ ] Multi thread support on packet generation and operations
- [ ] Add basic support for encapsulation and fragmentation (try to reduce
amount oc "allocate, copy, free" necessary)
- [ ] Allow user to change operations of a specific packet id
- [ ] Allow different `base_packet` struct to be defined (as long as `flags` and
`packet_id` fields are present)
- [ ] Improve MBY field parsing. Create map with MBY fields so we jump directly
where we need to act
- [ ] Improve DYN field parsing. Create map with DYN fields so we jump directly
where we need to act
- [ ] Separate dynamic packets into advanced (MBY + DYN fields) and dynamic
packets (just DYN fields)
- [ ] Allow overload of dynamic field struct type
- [ ] Properly check system library return errors and set "sucess" field as
appropriate

Pros:

1. Easily manipulate packets, not worrying about \[de]serialization of dynamic
fields and correct endianess across systems;
2. Only basic macros are used, no extra "pre-compilation" stage required;
3. Placing most complexity (i.e. assembly of operation arrays) in "boot up" time
means Con #1 isn't as relevant;
4. Basic packets are simple structs that are parsed as quickly as structs would
by hand (no field iteration required, just memcpy and malloc/free);
5. Multi byte packets are very efficient, since memcpy is used and only the MBY
fields are worked on.

Cons:

1. The usage of x-macros obfuscates code, preventing debuggers from showing
source correctly
2. To identify the packets (packet_id) and fix endianess, two extra 1 byte fields
are always present as overhead on all packets.
3. Only 256 possible packet types. To increase this number, TODO # 3
(base_packet customization) must be done
