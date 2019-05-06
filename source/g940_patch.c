#include <stdio.h>
#include <stdint.h>
#include <conio.h>

#define CRC_O 0x866994b9	// orig. cCRC
#define CRC 0x6d15697		// patched cCRC
#define PATCH_BASE 0x1C00L
#define FW_SIZE 0x8000L - PATCH_BASE
#define FW_END_OFFS 2
#define THR_UNIT_CONST_REL_ADR 0x5200L
#define AXIS_FILT_REL_ADR 0x1FFCL
#define AXIS_MAIN_FILT_REL_ADR 0x2072L
#define FF_COMB_PARAMS_BASE 0x42EEL
#define FF_COMB_PERM_PARAMS_OFFS 0x1CL
#define FF_PARAMS_XY_OFFS 0xC0L
#define REL(OFFS) (OFFS - PATCH_BASE)

int main(int argc, char *argv[]) {
	FILE *p_fw;
	uint8_t buf[FW_SIZE];
	uint32_t ccrc32;

	p_fw = fopen("G940_Update_FW0142.exe","r+b");
	if (!p_fw) {
		printf("Unable to open: G940_Update_FW0142.exe!");
		return 1;
	}

/*	read firmware original blob	*/
	fseek(p_fw, -(FW_SIZE + FW_END_OFFS), SEEK_END);
	fread(&buf, FW_SIZE, 1, p_fw);

/*	check the ccrc value	*/

	ccrc32 = *(uint32_t*)&buf[REL(0x7FFCUL)];
	if (ccrc32 != CRC_O) {
		printf("original firmware checksum is wrong, leaving ... (%x vs. %x)\n", CRC_O, ccrc32);
		return 1;
	}

/*	make ADC1 & DMA_CH1 circular	*/
	*(uint8_t*)&buf[REL(0x1E0E)] = 0xA3;	// DMA_CCR1 CIRC
	*(uint16_t*)&buf[REL(0x1E36)] = 0x4770;	// shorten DMA_CH1_ISR
	*(uint8_t*)&buf[REL(0x2224)] = 0x03;	// ADC1_CR2 CONT

/*	set tolerance for new axis noice filter algorithm, 
	0x4 results in +/-1 LSB noice tolerance	*/
	buf[REL(0x1f0c)] = 0x5; // tolerance for noice filter

/*	increase the main axis active area, for better precision/resolution	*/
	*(uint8_t*)&buf[REL(0x215C)] = 0x48;	// main axis scale, was 90/64, now 72/64

/*	patch the main axis noice filtering function for real and minimal hysteresis	*/
	*(uint32_t*)&buf[REL(AXIS_MAIN_FILT_REL_ADR)] = 0x1B82BFB4; 
	*(uint32_t*)&buf[REL(AXIS_MAIN_FILT_REL_ADR) + 4] = 0xB2121A32; 
	*(uint32_t*)&buf[REL(AXIS_MAIN_FILT_REL_ADR) + 8] = 0xDB182A40; 
	*(uint32_t*)&buf[REL(AXIS_MAIN_FILT_REL_ADR) + 0xC] = 0xBFB44286; 
	*(uint32_t*)&buf[REL(AXIS_MAIN_FILT_REL_ADR) + 0x10] = 0x30403840; 
	*(uint32_t*)&buf[REL(AXIS_MAIN_FILT_REL_ADR) + 0x14] = 0xE7EB8098; 

/*	patch the analog axis noice filtering function for real and minimal hysteresis	*/
	*(uint32_t*)&buf[REL(AXIS_FILT_REL_ADR)] = 0xBFB44287; 
	*(uint32_t*)&buf[REL(AXIS_FILT_REL_ADR) + 4] = 0x1A3E1BC6; 
	*(uint32_t*)&buf[REL(AXIS_FILT_REL_ADR) + 8] = 0x7E1218D2; 
	*(uint32_t*)&buf[REL(AXIS_FILT_REL_ADR) + 0xC] = 0xB2B60192;
	*(uint32_t*)&buf[REL(AXIS_FILT_REL_ADR) + 0x10] = 0xDA1842B2; 
	*(uint32_t*)&buf[REL(AXIS_FILT_REL_ADR) + 0x14] = 0xBFB44287; 
	*(uint32_t*)&buf[REL(AXIS_FILT_REL_ADR) + 0x18] = 0x18821A82; 
	*(uint32_t*)&buf[REL(AXIS_FILT_REL_ADR) + 0x1C] = 0xE7E88522;

/*	remove RUDDER deadzone	*/
	*(uint16_t*)&buf[REL(0x2304)] = 0xBF00;	// RUDDER+

/*	adapt axes processing, register r2 holds the filtered value	*/
	*(uint8_t*)&buf[REL(0x2308)] = 0x10;	// RUDDER
	*(uint8_t*)&buf[REL(0x231A)] = 0x10;	// RUDDER+
	*(uint8_t*)&buf[REL(0x2358)] = 0x10;	// BRAKE_L
	*(uint8_t*)&buf[REL(0x2372)] = 0x10;	// BRAKE_R
	*(uint8_t*)&buf[REL(0x238A)] = 0x10;	// TRIM3
	*(uint8_t*)&buf[REL(0x239A)] = 0x10;	// TRIM1
	*(uint8_t*)&buf[REL(0x23AA)] = 0x10;	// TRIM2
	*(uint32_t*)&buf[REL(0x2406)] = 0x73E20A12;	// MINI_X
	*(uint32_t*)&buf[REL(0x241A)] = 0x43D20A12;	// MINI_Y
	*(uint16_t*)&buf[REL(0x241E)] = 0x7422;		// MINI_Y+
	*(uint8_t*)&buf[REL(0x26AC)] = 0x22;	// THR_L
	*(uint8_t*)&buf[REL(0x26CE)] = 0x22;	// THR_R
	*(uint8_t*)&buf[REL(0x26F0)] = 0x22;	// R1
	*(uint8_t*)&buf[REL(0x2714)] = 0x22;	// R2

/*	set the default force feedback params for combined effect	*/
	*(uint32_t*)&buf[REL(FF_COMB_PARAMS_BASE)] = 0x3F2F1F00;
	*(uint32_t*)&buf[REL(FF_COMB_PARAMS_BASE + FF_PARAMS_XY_OFFS)] = 0x3F2F1F00;

/*	set the permanent force feedback params for combined effect	*/
	*(uint32_t*)&buf[REL(FF_COMB_PARAMS_BASE + FF_COMB_PERM_PARAMS_OFFS)] = 0x7F2F3F00;
	*(uint32_t*)&buf[REL(FF_COMB_PARAMS_BASE + FF_COMB_PERM_PARAMS_OFFS + FF_PARAMS_XY_OFFS)] = 0x7F2F3F00;

/*	scale impact of spring coefficients down, (div 8) to move saturation to max/min x/y	*/
	*(uint16_t*)&buf[REL(0x39B4)] = 0x11D3;

/*	adapt PWM scale, to minimize unwanted deadzone	*/
	*(uint32_t*)&buf[REL(0x3AD2)] = 0x5367F244;//(32767-15000) 
	*(uint32_t*)&buf[REL(0x3ADC)] = 0x2298F643;//+15000
	*(uint32_t*)&buf[REL(0x3B98)] = 0xFFFFC568;//-15000


/*	process checksum	*/
	ccrc32 = CRC;
	printf("ccrc32: %x\n\n", ccrc32);

/*	write checksum of patched blob	*/
	*(uint32_t*)&buf[REL(0x7FFCUL)] = ccrc32;

/*	write the patched data back to binary blob	*/
	fseek(p_fw, -(FW_SIZE + FW_END_OFFS), SEEK_END);
	fwrite(buf, FW_SIZE, 1, p_fw);

	fclose(p_fw);

	printf("The file G940_Update_FW0142.exe was patched successfully.\n\n");
 	printf("Now run it to upload the patched firmware to your device.\n\n");
	printf("Press any key to continue ...\n");

	getch();

	return 0;
}
