
#ifndef SUB_CHUNK_HH
#define SUB_CHUNK_HH

#include <string>
#include <cstring>
#include <map>
#include <sstream>
#include <cassert>
#include <iostream>

#include <BlockType.h>


class SubChunk {
    struct Iter {
    private:
        struct Location {
            Location(int x, int y, int z, uint8_t type) : x(x), y(y), z(z), type(type) { /* empty */ };
            int x;
            int y;
            int z;
            uint8_t type;
        };

        int curx;
        int cury;
        int curz;
        int type_filter;
        SubChunk *subchunk;

    public:
        Iter(int tf, SubChunk *sc) : curx(0), cury(0), curz(0), type_filter(tf), subchunk(sc) { /* empty */ }
        static Iter get_end() {
            Iter retval(-1, 0x0);
            retval.curx = retval.cury = retval.curz = -1;
            return retval;
        };

        // Previously provided by std::iterator - see update below
        typedef Location            value_type;
        typedef std::ptrdiff_t          difference_type;
        typedef Location*               pointer;
        typedef Location&               reference;
        typedef std::input_iterator_tag iterator_category;

        Location operator*() const {
            assert(curx>=0 && cury>=0 && curz>=0 && subchunk);
            return Location(subchunk->chunkx*16+curx, subchunk->chunky*16+cury, subchunk->chunkz*16+curz,
                            subchunk->get_type_at(curx,cury,curz));
        }

        bool operator==(const Iter& other) const {
            return (other.curx == curx) && (other.cury == cury) && (other.curz == curz);
        }
        bool operator!=(const Iter& other) const { return !(*this == other); }
#if 0
        // when do I have to have post incr?
        intholder operator++(int) {

            intholder ret(value_);
            ++*this;
            return ret;
        }
#endif
        Iter& operator++() {
            if ((curx==-1) || (cury==-1) || (curz==-1)) { curx = cury = curz = -1; }

            while (1) {
                curz++;
                if (curz>=16) {
                    curz = 0;
                    curx++;
                    if (curx >= 16) {
                        curx = 0;
                        cury++;
                        if (cury>=16) {
                            curx = cury = curz = -1;
                            break;
                        }
                    }
                }
                //std::cout << "looing in " << curx << "," << cury << "," << curz << "\n";
                if (type_filter != -1) {
                    if (type_filter != subchunk->get_type_at(curx,cury,curz)) {
                        continue;
                    }
                }
                break;
            }
            return *this;
        };

    };
public:
    SubChunk();

    int chunkx;
    int chunky;
    int chunkz;


    uint8_t block_type_ids[16][16][16];

    void    set_type_at(int x, int y, int z, uint8_t type);
    uint8_t get_type_at(int x, int y, int z);

    Iter begin(int type_filter=-1) {
        Iter retval(type_filter, this);
        if (type_filter != -1 &&  (*retval).type != type_filter) {
            ++retval;
        }
        return retval;
    }
    Iter end()   { return Iter::get_end(); }
};





#endif
