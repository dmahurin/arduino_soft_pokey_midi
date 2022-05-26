#define SOUND_OUTPUT_PIN 9

#ifdef PC
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#include <usbmidi.h>

#define MIDI_NOTE_OFF   0b10000000
#define MIDI_NOTE_ON    0b10010000
#define MIDI_NOTE_AFTERTOUCH 0b10100000
#define MIDI_PITCH_BEND 0b11100000
#define MIDI_PROGRAM_CHANGE 0b11000000

#ifdef PC
#define SENDOUT(out) putc(out<<1,stdout);
#define NOASM 1
#else
#define SENDOUT(out) OCR1AL = out ? out: 1
#endif

#define CHANNELS 4

#ifdef __cplusplus
extern "C" {
#endif
uint8_t calc_poly4(uint8_t);
uint8_t calc_poly5(uint8_t);
uint16_t calc_poly9(uint16_t);
uint32_t calc_poly17(uint32_t);
#ifdef __cplusplus
}
#endif

#ifdef NOASM
uint8_t calc_poly4(uint8_t v) { return (v<<1u) | ( ((v>>2u) ^ (v>>3u)) & 1u); }
#else
__asm__(
"calc_poly4:\n"
"mov r30,r24\n" // copy input to R30
"lsr r30\n"  // shift right 2
"lsr r30\n"
"mov r31,r30\n" // copy shifted poly to R31
"lsr r31\n"  // shift right 1 (input shifted 3)
"eor r30,r31\n" // xor R30 and R31
"andi r30,0x01\n" // bit 0 of R30
"lsl r24\n" // left shift input/output
"or r24,r30\n" // or new bit from R30 to as bit 0 of output
"ret\n"
);
#endif

#ifdef NOASM
uint8_t calc_poly5(uint8_t v) { return (v<<1) | ( ((v>>2) ^ (v>>4)) & 0x1); }
#else
__asm__(
"calc_poly5:\n"
"mov r30,r24\n" // copy input to R30
"lsr r30\n"  // shift left 2
"lsr r30\n"
"mov r31,r30\n" // copy shifted poly to R31
"lsr r31\n"  // shift right 2 (input shifted 4)
"lsr r31\n"  //
"eor r30,r31\n" // xor R30 and R31
"andi r30,0x01\n" // bit 0 of R30
"lsl r24\n" // left shift input/output
"or r24,r30\n" // or new bit from R30 to as bit 0 of output
"ret\n"
);
#endif
uint16_t calc_poly9(uint16_t v) { return (v<<1) | ( ((v>>3) ^ (v>>8)) & 0x1); }

#ifdef NOASM
uint32_t calc_poly17(uint32_t v) { return (v<<1) | ( ((v>>2) ^ (v>>16)) & 0x1); }
#else
__asm__(
"calc_poly17:\n" // input/output is big to little: R25 R24 R23 R22
// we will get R22 >> 2 and R24 bit 0, then shift all left and combine
"mov r30,r22\n" // copy input to R30
"lsr r30\n" // shift left 2
"lsr r30\n"
"mov r31,r24\n" // copy shifted poly to R31
"eor r30,r31\n" // xor R30 and R31
"andi r30,0x01\n" // bit 0 of R30
"lsl r22\n" // left shift 32 bit input/output
"rol r23\n"
"rol r24\n"
"rol r25\n"
"or r22,r30\n" // OR new bit from R30 to as bit 0 of output
"ret\n"
);
#endif

struct {
	uint8_t audf;
	uint8_t remain;
	uint8_t dist;
	uint8_t volume;
	uint8_t position;
} channel [CHANNELS];


const uint8_t midi_note_bend_to_audf[][8] =
{
{255,255,255,255,255,255,254,252},
{250,249,247,245,243,242,240,238},
{236,235,233,231,230,228,226,225},
{223,221,220,218,217,215,214,212},
{210,209,207,206,204,203,202,200},
{199,197,196,194,193,192,190,189},
{187,186,185,183,182,181,179,178},
{177,176,174,173,172,171,169,168},
{167,166,164,163,162,161,160,159},
{157,156,155,154,153,152,151,150},
{149,147,146,145,144,143,142,141},
{140,139,138,137,136,135,134,133},
{132,131,130,129,128,127,127,126},
{125,124,123,122,121,120,119,119},
{118,117,116,115,114,113,113,112},
{111,110,109,109,108,107,106,106},
{105,104,103,102,102,101,100,100},
{99,98,97,97,96,95,95,94},
{93,93,92,91,91,90,89,89},
{88,87,87,86,85,85,84,84},
{83,82,82,81,81,80,79,79},
{78,78,77,77,76,75,75,74},
{74,73,73,72,72,71,71,70},
{70,69,69,68,68,67,67,66},
{66,65,65,64,64,63,63,62},
{62,61,61,61,60,60,59,59},
{58,58,57,57,57,56,56,55},
{55,55,54,54,53,53,53,52},
{52,51,51,51,50,50,50,49},
{49,49,48,48,47,47,47,46},
{46,46,45,45,45,44,44,44},
{43,43,43,43,42,42,42,41},
{41,41,40,40,40,39,39,39},
{39,38,38,38,37,37,37,37},
{36,36,36,36,35,35,35,35},
{34,34,34,34,33,33,33,33},
{32,32,32,32,31,31,31,31},
{30,30,30,30,30,29,29,29},
{29,28,28,28,28,28,27,27},
{27,27,27,26,26,26,26,26},
{25,25,25,25,25,24,24,24},
{24,24,24,23,23,23,23,23},
{23,22,22,22,22,22,22,21},
{21,21,21,21,21,20,20,20},
{20,20,20,20,19,19,19,19},
{19,19,19,18,18,18,18,18},
{18,18,17,17,17,17,17,17},
{17,17,16,16,16,16,16,16},
{16,16,15,15,15,15,15,15},
{15,15,14,14,14,14,14,14},
{14,14,14,14,13,13,13,13},
{13,13,13,13,13,13,12,12},
{12,12,12,12,12,12,12,12},
{11,11,11,11,11,11,11,11},
{11,11,11,11,10,10,10,10},
{10,10,10,10,10,10,10,10},
{9,9,9,9,9,9,9,9},
{9,9,9,9,9,9,8,8},
{8,8,8,8,8,8,8,8},
{8,8,8,8,8,8,7,7},
{7,7,7,7,7,7,7,7},
{7,7,7,7,7,7,7,6},
{6,6,6,6,6,6,6,6},
{6,6,6,6,6,6,6,6},
{6,6,6,5,5,5,5,5},
{5,5,5,5,5,5,5,5},
{5,5,5,5,5,5,5,5},
{5,5,4,4,4,4,4,4},
{4,4,4,4,4,4,4,4},
{4,4,4,4,4,4,4,4},
{4,4,4,4,4,4,3,3},
{3,3,3,3,3,3,3,3},
{3,3,3,3,3,3,3,3},
{3,3,3,3,3,3,3,3},
{3,3,3,3,3,3,3,3},
{3,2,2,2,2,2,2,2},
{2,2,2,2,2,2,2,2},
{2,2,2,2,2,2,2,2},
{2,2,2,2,2,2,2,2},
{2,2,2,2,2,2,2,2},
{2,2,2,2,2,2,2,1},
{1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,1}
};

uint8_t note_to_audf(uint8_t n, uint8_t bend) {
	if(n < 47) return 255;
	return midi_note_bend_to_audf[n-47][bend>>9];
	// f = 440.0*pow(2.0,(n-69+((bend-8192)/4096))/12)
	// audf = 3579545 / 28 / (f << 2) - 1
}

#ifndef PC
void setup() {
	pinMode(SOUND_OUTPUT_PIN, OUTPUT);
	TCCR1A = 0x80;
	TCCR1B = 0x11;
	ICR1H = 0;
	ICR1L = 0x7f;
	TIMSK1 = 1;
	OCR1AH = 0;

	for (uint8_t i=0; i<CHANNELS; i++)
		reset_channel(i);
}
#endif

void set_pokey_audf_audc(uint8_t c, uint8_t audf, uint8_t audc) {
	channel[c].volume = audc & 0xf ? 15: 0;
	channel[c].dist = audc >> 4;
	channel[c].audf = audf;

	// note: atari ~64k clock = 1789772.5/28
	//       arduino ~64k clock = 16000000/0xff
}

void reset_channel (uint8_t c) {
	channel[c].audf = 0;
	channel[c].volume = 0;
}

uint8_t poly4 = 1;
uint8_t poly5 = 1;
uint32_t poly17 = 1;

uint8_t check_poly(uint8_t dist, uint8_t value) {
	if(dist & 0x8) {
		return dist & 0x2 ? value + 1 : dist & 0x4 ? poly4 : poly17;
	}
	return dist & 0x2 ? poly5 : dist & 0x4 ? poly5 & poly4 : poly5 & poly17;
}

static uint8_t s = 0;

#define INCREMENT 2

#ifdef PC
static void handler() {
#else
ISR(TIMER1_OVF_vect) {
#endif
	static uint8_t out = 0;

	// The 4 and 5 bit poly's need to be sampled at ~64Khz (every iteration) to sound correct. Current implementation does not allow cycles for this.
	// For now, alternate between poly calculation and signal output.
	if (++s & 1) {
		poly4 = calc_poly4(poly4);
		poly5 = calc_poly5(poly5);
		poly17 = calc_poly17(poly17);

		return;
	}


	static uint8_t i;
	for (i=0; i<CHANNELS; i++) {
		if (!channel[i].audf) continue;
		if (channel[i].remain < INCREMENT) {
			// frequency divider (audf + 1) - INCREMENT = audf
			channel[i].remain = channel[i].audf;
			channel[i].position = check_poly(channel[i].dist, channel[i].position);
		} else {
			channel[i].remain -= INCREMENT;
		}
		out += (channel[i].position & 1? channel[i].volume : 0);
	}

	SENDOUT(out);
	out = 0;
}

#ifdef PC
void delay(int n) {
	// 1ms = 16000000/0x7f/2/1000' 16Mhz 0x7f divider, cycle takes 2 calls
	for(int i=0; i< n*63; i++) {
		handler();
	}
}
#endif

uint8_t dist[4] = { 0xa, 0xa, 0xa, 0xa };

void loop() {
	static u8 command=0, mchannel=0, note=0, pitchbend=0, velocity=0;
	USBMIDI.poll();

	while (USBMIDI.available()) {

		// skip to MIDI data
		while(!(USBMIDI.peek() & 0b10000000)) USBMIDI.read();

		command = USBMIDI.read();
		mchannel = (command & 0b00001111) & 0x3;
		command = command & 0b11110000;

		switch(command) {
			case MIDI_NOTE_ON:
			case MIDI_NOTE_AFTERTOUCH:
			case MIDI_NOTE_OFF:
				if(USBMIDI.peek() & 0b10000000) continue; note = USBMIDI.read();
				if(USBMIDI.peek() & 0b10000000) continue; velocity = USBMIDI.read();
			break;
			case MIDI_PROGRAM_CHANGE:
				if(USBMIDI.peek() & 0b10000000) continue;
				switch(USBMIDI.read()) {
				case 117: dist[mchannel] = 0x8; break;
				case 116: dist[mchannel] = 0xc; break;
				case 115: dist[mchannel] = 0x4; break;
				case 114: dist[mchannel] = 0x2; break;
				default: dist[mchannel] = 0xa; break;
				}
				break;
			case MIDI_PITCH_BEND:
				if(USBMIDI.peek() & 0b10000000) continue; pitchbend = USBMIDI.read();
				if(USBMIDI.peek() & 0b10000000) continue; pitchbend |= (USBMIDI.read() << 7);
				break;
			break;
		}

		if((command == MIDI_NOTE_ON || MIDI_NOTE_AFTERTOUCH) && velocity > 0) {
			set_pokey_audf_audc(mchannel, note_to_audf(note, pitchbend), (dist[mchannel] << 4) | (velocity >> 3));
		}
		if(command == MIDI_NOTE_OFF) {
			set_pokey_audf_audc(mchannel, 0, 0);
		}
	}
}

#ifdef PC
int main(int argc, char* argv[]) {
	loop();
}
#endif

