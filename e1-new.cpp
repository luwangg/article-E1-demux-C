/**  Revision 1: Created. Added version Write4
     Revision 2: Added Write8
  */

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <typeinfo>
#include <stdio.h>

#include "timer.h"
#include "mymacros.h"

typedef unsigned char byte;

static const size_t NUM_TIMESLOTS = 32;
static const size_t DST_SIZE = 64;
static const size_t SRC_SIZE = NUM_TIMESLOTS * DST_SIZE;
static const unsigned ITERATIONS = 1000000;

using namespace std;

class Demux
{
public:
    virtual void demux (const byte * src, size_t src_length, byte ** dst) const = 0;
};

class Reference : public Demux
{
public:
    void demux (const byte * src, size_t src_length, byte ** dst) const
    {
        assert (src_length % NUM_TIMESLOTS == 0);

        size_t dst_pos = 0;
        size_t dst_num = 0;
        for (size_t src_pos = 0; src_pos < src_length; src_pos++) {
            dst [dst_num][dst_pos] = src [src_pos];
            if (++ dst_num == NUM_TIMESLOTS) {
                dst_num = 0;
                ++ dst_pos;
            }
        }
    }
};

inline uint32_t make_32 (byte b0, byte b1, byte b2, byte b3)
{
    return ((uint32_t) b0 << 0)
         | ((uint32_t) b1 << 8)
         | ((uint32_t) b2 << 16)
         | ((uint32_t) b3 << 24);
}

class Write4 : public Demux
{
public:
    void demux (const byte * src, size_t src_length, byte ** dst) const
    {
        assert (src_length == NUM_TIMESLOTS * DST_SIZE);
        assert (DST_SIZE % 4 == 0);

        for (size_t dst_num = 0; dst_num < NUM_TIMESLOTS; ++ dst_num) {
            byte * d = dst [dst_num];
            for (size_t dst_pos = 0; dst_pos < DST_SIZE; dst_pos += 4) {
                byte b0 = src [(dst_pos + 0) * NUM_TIMESLOTS + dst_num];
                byte b1 = src [(dst_pos + 1) * NUM_TIMESLOTS + dst_num];
                byte b2 = src [(dst_pos + 2) * NUM_TIMESLOTS + dst_num];
                byte b3 = src [(dst_pos + 3) * NUM_TIMESLOTS + dst_num];
                * (uint32_t*) & d [dst_pos] = make_32 (b0, b1, b2, b3);
            }
        }
    }
};

inline uint64_t make_64 (byte b0, byte b1, byte b2, byte b3, byte b4, byte b5, byte b6, byte b7)
{
    return (uint64_t) make_32 (b0, b1, b2, b3)
         | ((uint64_t) b4 << 32)
         | ((uint64_t) b5 << 40)
         | ((uint64_t) b6 << 48)
         | ((uint64_t) b7 << 56);
}

class Write8 : public Demux
{
public:
    void demux (const byte * src, size_t src_length, byte ** dst) const
    {
        assert (src_length == NUM_TIMESLOTS * DST_SIZE);
        assert (DST_SIZE % 8 == 0);

        for (size_t dst_num = 0; dst_num < NUM_TIMESLOTS; ++ dst_num) {
            byte * d = dst [dst_num];
            for (size_t dst_pos = 0; dst_pos < DST_SIZE; dst_pos += 8) {
                byte b0 = src [(dst_pos + 0) * NUM_TIMESLOTS + dst_num];
                byte b1 = src [(dst_pos + 1) * NUM_TIMESLOTS + dst_num];
                byte b2 = src [(dst_pos + 2) * NUM_TIMESLOTS + dst_num];
                byte b3 = src [(dst_pos + 3) * NUM_TIMESLOTS + dst_num];
                byte b4 = src [(dst_pos + 4) * NUM_TIMESLOTS + dst_num];
                byte b5 = src [(dst_pos + 5) * NUM_TIMESLOTS + dst_num];
                byte b6 = src [(dst_pos + 6) * NUM_TIMESLOTS + dst_num];
                byte b7 = src [(dst_pos + 7) * NUM_TIMESLOTS + dst_num];
                * (uint64_t*) & d [dst_pos] = make_64 (b0, b1, b2, b3, b4, b5, b6, b7);
            }
        }
    }
};


byte * generate ()
{
    byte * buf = new byte [SRC_SIZE];
    srand (0);
    for (size_t i = 0; i < SRC_SIZE; i++) buf[i] = (byte) (rand () % 256);
    return buf;
}
    
byte ** allocate_dst ()
{
    byte ** result = new byte * [NUM_TIMESLOTS];
    for (size_t i = 0; i < NUM_TIMESLOTS; i++) {
        result [i] = new byte [DST_SIZE];
        memset (result [i], 0, DST_SIZE);
    }
    return result;
}

void delete_dst (byte ** dst)
{
    for (size_t i = 0; i < NUM_TIMESLOTS; i++) {
        delete dst [i];
    }
    delete dst;
}
    
void check (const Demux & demux)
{
    byte * src = generate ();
    byte ** dst0 = allocate_dst ();
    byte ** dst = allocate_dst ();
    Reference().demux (src, SRC_SIZE, dst0);
    demux.demux (src, SRC_SIZE, dst);

    for (int i = 0; i < NUM_TIMESLOTS; i++) {
        if (memcmp (dst0[i], dst[i], DST_SIZE)) {
            cout << "Results not equal: line " << i << "\n";
            exit (1);
        }
    }
    delete src;
    delete_dst (dst0);
    delete_dst (dst);
}

byte * src;
byte ** dst;

void measure (const Demux & demux)
{
    check (demux);

    uint64_t t0 = currentTimeMillis ();
    for (int i = 0; i < ITERATIONS; i++) {
        demux.demux (src, SRC_SIZE, dst);
    }
    uint64_t t = currentTimeMillis () - t0;
    cout << typeid (demux).name() << ": " << t << endl;
}

int main (void)
{
    src = generate ();
    dst = allocate_dst ();

    measure (Reference ());
    measure (Write4 ());
    measure (Write8 ());

    return 0;
}
