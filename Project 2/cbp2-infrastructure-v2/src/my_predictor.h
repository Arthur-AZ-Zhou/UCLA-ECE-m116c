// my_predictor.h
// This file contains a sample my_predictor class.
// It is a simple 32,768-entry gshare with a history length of 15.
// Note that this predictor doesn't use the whole 32 kilobytes available
// for the CBP-2 contest; it is just an example.

#include <algorithm>
using namespace std;

class my_update : public branch_update {
public:
	unsigned int index;
};

class my_predictor : public branch_predictor {
public:
#define HISTORY_LENGTH	21 //changed this so we can maintain a longer branch history
#define TABLE_BITS	21 //look at above comment
#define BYTE_COUNTERS 4
	my_update u;
	branch_info bi;
	unsigned int history;
	unsigned char tab[1<<TABLE_BITS];

	my_predictor (void) : history(0) { 
		fill(tab, tab + (1 << TABLE_BITS), 0); //CHANGED TO FILL BECAUSE IT IS FASTER THAN MEMSET
	}

	unsigned char readCounter(unsigned int index) { //adjust frequency based on this
		return (tab[index / BYTE_COUNTERS] >> ((index % BYTE_COUNTERS) * 2)) & 0x3; 
	}

	void writeCounter(unsigned int index, unsigned char val) { //adjust frequency based on this
		tab[index / BYTE_COUNTERS] &= ~(0x3 << ((index % BYTE_COUNTERS) * 2));
		tab[index / BYTE_COUNTERS] |= (val & 0x3) << ((index % BYTE_COUNTERS) * 2);
	}

	branch_update *predict (branch_info & b) {
		bi = b;
		if (b.br_flags & BR_CONDITIONAL) {
			u.index = 
				  (history << (TABLE_BITS - HISTORY_LENGTH)) 
				^ (b.address & ((1<<TABLE_BITS)-1));
			u.direction_prediction (readCounter(u.index) >> 1);
		} else {
			u.direction_prediction (true);
		}
		u.target_prediction (0);
		return &u;
	}

	void update (branch_update *u, bool taken, unsigned int target) {
		if (bi.br_flags & BR_CONDITIONAL) {
			unsigned char c = readCounter(((my_update*)u)->index);
			if (taken) {
				if (c < 3) (c)++;
			} else {
				if (c > 0) (c)--;
			}
			writeCounter(((my_update*)u)->index, c);
			history = ((history << 1) | taken) & ((1 << HISTORY_LENGTH) - 1);
		}
	}
};
