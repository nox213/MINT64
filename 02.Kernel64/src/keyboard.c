#include "Types.h"
#include "assembly_utility.h"
#include "keyboard.h"

BOOL is_output_buf_full(void)
{
	if (inport_byte(0x64), & 0x01)
		return TRUE;
	return FALSE;
}

BOOL is_input_buf_full(void)
{
	if (in_port_byte(0x64) & 0x02)
		return TRUE;
	return FALSE;
}

BOOL activate_keyboard(void)
{
	int i, j;
	
	out_port_byte(0x64, 0xAE);

	/* 입력 버퍼가 빌 때까지 기다렸다가 키보드에 활성화 명령 전송 */
	/* 0xFFFF번 만큼 반복후에도 비지 않으면 무시하고 전송 */
	for (i = 0; i < 0xFFFF; i++)
		if (is_input_buf_full() == FLASE)
			break;

	out_port_byte(0x60, 0xF4);
	
	/* ACK가 올 때까지 대기함 */
	/* ACK가 오기 전에 출력 버퍼에 데이터가 저장되어 있을 수 있으므로 */
	/* 키보드에서 전달 된 데이터를 최대 100개까지 수신하여 확인 */
	for (j = 0; j < 100; j++) {
		for (i = 0; i < 0xFFFF; i++)
			if (is_ouput_buf_full() == TRUE)
				break;

		if (in_port_byte(0x60) == 0xFA)
			return TRUE;
		
		return FALSE;
	}
}

BYTE get_keyboard_scancode(void)
{
	while (is_output_buf_full() == FALSE)
		;

	return in_port_byte(0x60);
}

BOOL change_keyboard_led(BOOL caps_lock_on, BOOL num_lock_on, BOOL scroll_lock_on)
{
	int i, j;

	for (i = 0; i < 0xFFFF; i++)
		if (is_input_buf_full() == FALSE)
			break;

	/* wait until keybord fetch input buffer */
	out_port_byte(0x60, 0xED);
	for (i = 0; i < 0xFFFF; i++)
		if (is_input_buf_full() == FALSE)
			break;

	/* wait until recive ACK */
	for (i = 0; i < 100; i++) {
		for (j = 0; j < 0xFFFF; j++)
			if (is_output_buf_full() == TRUE)
				break;

		if (in_port_byte(0x60) == 0xFA)
			break;
	}
	
	if (i >= 100)
		return FALSE;

	/* send led option */
	out_port_byte(0x60, (caps_lock_on << 2) | (num_lock_on << 1) | (scroll_lock_on));
	for (i = 0; i < 0xFFFF; i++)
		if (is_input_buf_full() == FALSE)
			break;

	for (i = 0; i < 100; i++) {
		for (j = 0; j < 0xFFFF; j++)
			if (is_output_buf_full() == TRUE)
				break;

		if (in_port_byte(0x60) == 0xFA)
			break;
	}
	
	if (i >= 100)
		return FALSE;

	return TRUE;
}

void enable_A20gate(void)
{
	BYTE output_port_data;
	int i;

	out_port_byte(0x64, 0xD0);
	for (i = 0; i < 0xFFFF; i++)
		if (is_output_buf_full() == TRUE)
			break;

	/* setting a20 gate bit */ 
	output_port_data = in_port_byte(0x60);
	output_port_data |= 0x01;

	for (i = 0; i < 0xFFFF; i++) 
		if (is_input_buf_full() == FALSE)
			break;

	out_port_byte(0x64, 0xD1);
	out_port_byte(0x60, output_port_data);
}

void reboot(void)
{
	int i;

	for (i = 0; i < 0xFFFF; i++) 
		if (is_input_buf_full() == FALSE)
			break;

	out_port_byte(0x64, 0xD1);
	out_port_byte(0x60, 0x00);

	while (1)
		;
}

static struct keyboard_manager_struct keyboard_manager = {0};

static struct key_mapping_entry_struct key_mapping_table[KEY_MAPPING_TABLE_SIZE] =
{
	/*  0  */ {KEY_NONE			, KEY_NONE			},
	/*  1  */ {KEY_ESC			, KEY_ESC			},
	/*  2  */ {'1'				, '!'				},
	/*  3  */ {'2'				, '@'				},
	/*  4  */ {'3'				, '#'				},
	/*  5  */ {'4'				, '$'				},
	/*  6  */ {'5'				, '%'				},
	/*  7  */ {'6'				, '^'				},
	/*  8  */ {'7'				, '&'				},
	/*  9  */ {'8'				, '*'				},
	/*  10 */ {'9'				, '('				},
	/*  11 */ {'0'				, ')'				},
	/*  12 */ {'-'				, '_'				},
	/*  13 */ {'='				, '+'				},
	/*  14 */ {'BACKSPACE'		, BACKSPACE			},
	/*  15 */ {'TAB'			, TAB				},
	/*  16 */ {'q'				, 'Q'				},
	/*  17 */ {'w'				, 'W'				},
	/*  18 */ {'e'				, 'E'				},
	/*  19 */ {'r'				, 'R'				},
	/*  20 */ {'t'				, 'T'				},
	/*  21 */ {'y'				, 'Y'				},
	/*  22 */ {'u'				, 'U'				},
	/*  23 */ {'i'				, 'I'				},
	/*  25 */ {'o'				, 'O'				},
	/*  26 */ {'p'				, 'P'				},
	/*  27 */ {'['				, '{'				},
	/*  28 */ {']'				, '}'				},
	/*  29 */ {'\n'				, '\n'				},
	/*  30 */ {'a'				, 'A'				},
	/*  31 */ {'s'				, 'S'				},
	/*  32 */ {'d'				, 'D'				},
	/*  33 */ {'f'				, 'F'				},
	/*  34 */ {'g'				, 'G'				},
	/*  35 */ {'h'				, 'H'				},
	/*  36 */ {'j'				, 'J'				},
	/*  37 */ {'k'				, 'K'				},
	/*  38 */ {'l'				, 'L'				},
	/*  39 */ {';'				, ':'				},
	/*  40 */ {'\''				, '\"'				},
	/*  41 */ {'`'				, '~'				},
	/*  42 */ {LSHIFT			, LSHIFT			},
	/*  43 */ {'\\'				, '|'				},
	/*  44 */ {'z'				, 'Z'				},
	/*  45 */ {'x'				, 'X'				},
	/*  46 */ {'c'				, 'C'				},
	/*  47 */ {'v'				, 'V'				},
	/*  48 */ {'b'				, 'B'				},
	/*  49 */ {'n'				, 'N'				},
	/*  50 */ {'m'				, 'M'				},
	/*  51 */ {','				, '<'				},
	/*  52 */ {'.'				, '>'				},
	/*  53 */ {'/'				, '?'				},
	/*  54 */ {RSHIFT			, RSHITF			},
	/*  55 */ {'*'				, '*'				},
	/*  56 */ {LALT				, LALT				},
	/*  57 */ {' '				, ' '				},
	/*  58 */ {CAPSLOCK			, CAPSLOCK			},
	/*  59 */ {F1				, F1	 			},
	/*  60 */ {F2				, F2	 			},
	/*  61 */ {F3				, F3	 			},
	/*  62 */ {F4				, F4	 			},
	/*  63 */ {F5				, F5	 			},
	/*  64 */ {F6				, F6	 			},
	/*  65 */ {F7				, F7	 			},
	/*  66 */ {F8				, F8	 			},
	/*  67 */ {F9				, F9	 			},
	/*  68 */ {F10				, F10	 			},
	/*  69 */ {NUM_LOCK			, NUM_LOCK			},
	/*  70 */ {SCROLL_LOCK		, SCROLL_LOCK		},
	/*  70 */ {SCROLL_LOCK		, SCROLL_LOCK		},

	/*  71 */ {HOME				, '7'				},
	/*  72 */ {UP				, '8'				},
	/*  73 */ {PAGEUP			, '9'				},
	/*  74 */ {'-'				, '-'				},
	/*  75 */ {LEFT				, '4'				},
	/*  76 */ {CENTER			, '5'				},
	/*  77 */ {RIGHT			, '6'				},
	/*  78 */ {'+'				, '+'				},
	/*  79 */ {END				, '1'				},
	/*  80 */ {DOWN				, '2'				},
	/*  81 */ {PAGE_DOWN		, '3'				},
	/*  82 */ {INS				, '0'				},
	/*  83 */ {DEL				, '.'				},
	/*  84 */ {NONE				, NONE				},
	/*  85 */ {NONE				, NONE				},
	/*  86 */ {NONE				, NONE				},
	/*  87 */ {F11				, F11				},
	/*  88 */ {F12				, F12				},
};

BOOL is_alphabet_scancode(BYTE scancode)
{
	if (('a' <= key_mapping_table[scancode].normal) &&
		(key_mapping_table[scancode].normal <= 'z'))
		return TRUE;

	return FALSE;
}

BOOL is_number_or_symbol_scancode(BYTE scancode)
{
	if ((2 <= scancode) && (scancode <= 53) &&
		(is_alpahbet_scancode(scancode) == FALSE))
		return TRUE;

	return FALSE;
}

BOOL is_numberpad_scancode(BYTE scancode)
{
	if ((71 <= scancode) && (scancode <= 83))
		return TRUE;

	return FALSE;
}

BOOL is_combined_code(BYTE scancode)
{
	BYTE down_scancode;
	BOOL is_combined = FALSE;

	down_scancode = scancode & 0x7F;
	if (is_alphabet_scancode(down_scancode) == TRUE) {
		if (keyboard_manager.shift_down ^ keyboard_manager.caps_lock_on)
			is_combined = TRUE;
		else
			is_combined = FALSE;
	}
	else if (is_number_or_symbol_scancode(down_scancode) == TRUE) {
		if (keyboard_manager.shift_down == TRUE)
			is_combined = TRUE;
		else
			is_combined = FALSE;
	}
	else if ((is_numberpad_scancode(down_scancode) == TRUE) &&
			 (keyboard_manager.extened_code_in == FALSE)) {
		if (keyboard_manager.num_lock_on == TRUE)
			is_combined = TRUE;
		else
			is_combined = FALSE;
	}

	return is_combined;
}

void update_combination_key_status_and_led(BYTE scancode)
{
	BOOL down;
	BYTE down_scancode;
	BOOL led_status_changed = FALSE;

	if (scancode & 0x80) {
		down = FALSE;
		down_scancode = scancode & 0x7F;
	}
	else {
		down = TRUE;
		down_scancode = scancode;
	}

	/* 42 is LSHIFT, 58 is RSHIFT */
	if ((down_scancode == 42) || (down_scancode == 58))
		keyboard_manager.shift_down = down;
	/* 58 is caps lock */
	else if ((down_scancode == 58) && (down == TRUE)) {
		keyboard_manger.caps_lock_on ^= TRUE;
		led_status_changed = TRUE;
	}
	/* 70 is scroll lock */
	else if ((down_scancode == 70) && (down == TRUE)) {
		keyboard_manager.scroll_lock_on ^= TRUE;
		led_status_changed = TRUE;
	}

	if (led_status_changed == TRUE)
		change_keyboard_led(keyboard_manage.caps_lock_on,
							keyboard_manage.num_lock_on,
							keyboard_manage.scroll_lock_on);
}


	

