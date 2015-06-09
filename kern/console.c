/** @file console.c 
 *
 *  @brief Implementation of Console device-driver library.
 *  
 *  @author Shalini Priya Ashok Kumar
 * 
 *  @bug None
 */

#include <p1kern.h>
#include <stdio.h>
#include <asm.h>

#define CONSOLE_OUT_OF_RANGE  2500
int cursor_row = 0, cursor_col = 0;
char current_color;
int is_cursor_visible = 0;
int putbyte(char ch) {
	/*int row,col;
	row = current_cursor_position/CONSOLE_WIDTH;
	col = (current_cursor_position%CONSOLE_WIDTH)%CONSOLE_HEIGHT;*/

	switch(ch) {
		case '\n':	cursor_col = 0;
					cursor_row++;
					break;
		case '\r':	cursor_col = 0;
					break;
		case '\b':	cursor_col--;
					if(cursor_col<0) {
						cursor_col = CONSOLE_WIDTH - 1;
						cursor_row --;
					}
					*(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * cursor_row + cursor_col)) = ' ';
					*(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * cursor_row + cursor_col) + 1) = current_color;
					break;
		default:	*(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * cursor_row + cursor_col)) = ch;
					*(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * cursor_row + cursor_col) + 1) = current_color;					
					cursor_col++;
					if(cursor_col>CONSOLE_WIDTH) {
						cursor_col = 0;
						cursor_row ++;
					}
	}
	return ch;
}

/** @brief Prints the string s, starting at the current
 *         location of the cursor.
 *
 *  If the string is longer than the current line, the
 *  string fills up the current line and then
 *  continues on the next line. If the string exceeds
 *  available space on the entire console, the screen
 *  scrolls up one line, and then the string
 *  continues on the new line.  If '\n', '\r', and '\b' are
 *  encountered within the string, they are handled
 *  as per putbyte. If len is not a positive integer or s
 *  is null, the function has no effect.
 *
 *  @param s The string to be printed.
 *  @param len The length of the string s.
 *  @return Void.
 */
void putbytes(const char* s, int len) {
	if (s==NULL)
		return;
	while(len>=0) {
		putbyte(*s);
		s++;		
		len--;
	}
}

/** @brief Changes the foreground and background color
 *         of future characters printed on the console.
 *
 *  If the color code is invalid, the function has no effect.
 *
 *  @param color The new color code.
 *  @return 0 on success or integer error code less than 0 if
 *          color code is invalid.
 */
int set_term_color(int color) {
	int foreground = color & 0x0F;
	int background = color & 0xF0;
	if (foreground < 0 || foreground > 15 || background < 0 || background > 7)
		return -1;
	current_color = color;
	return 0;
}

/** @brief Writes the current foreground and background
 *         color of characters printed on the console
 *         into the argument color.
 *  @param color The address to which the current color
 *         information will be written.
 *  @return Void.
 */
void get_term_color(int* color) {
	*color = current_color;
}

/** @brief Sets the position of the cursor to the
 *         position (row, col).
 *
 *  Subsequent calls to putbytes should cause the console
 *  output to begin at the new position. If the cursor is
 *  currently hidden, a call to set_cursor() does not show
 *  the cursor.
 *
 *  @param row The new row for the cursor.
 *  @param col The new column for the cursor.
 *  @return 0 on success or integer error code less than 0 if
 *          cursor location is invalid.
 */
int set_cursor(int row, int col) {
	if (row < 0 || row > 24 || col < 0 || col > 79)
		return -1;
	cursor_row = row;
	cursor_col = col;
	return 0;
}

/** @brief Writes the current position of the cursor
 *         into the arguments row and col.
 *  @param row The address to which the current cursor
 *         row will be written.
 *  @param col The address to which the current cursor
 *         column will be written.
 *  @return Void.
 */
void get_cursor(int* row, int* col){
	*row = cursor_row;
	*col = cursor_col;
}

/** @brief Hides the cursor.
 *
 *  Subsequent calls to putbytes do not cause the
 *  cursor to show again.
 *
 *  @return Void.
 */
void hide_cursor() {
	int cursor_pos = CONSOLE_OUT_OF_RANGE;
	outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
	outb(CRTC_DATA_REG, cursor_pos>>8);
	outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
	outb(CRTC_DATA_REG, cursor_pos);
	is_cursor_visible = 0;
	return;
}

/** @brief Shows the cursor.
 *  
 *  If the cursor is already shown, the function has no effect.
 *
 *  @return Void.
 */
void show_cursor() {
	int cursor_pos = cursor_row * CONSOLE_WIDTH + cursor_col;
	outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
	outb(CRTC_DATA_REG, cursor_pos>>8);
	outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
	outb(CRTC_DATA_REG, cursor_pos);
	is_cursor_visible = 1;
	return;
}

/** @brief Clears the entire console.
 *
 * The cursor is reset to the first row and column
 *
 *  @return Void.
 */
void clear_console() {
	int i,j;
	for (i=0;i<CONSOLE_HEIGHT;i++) {
		for(j=0;j<CONSOLE_WIDTH;j++) {
			draw_char(i,j,' ',FGND_WHITE | BGND_BLACK);
		}
	}
	cursor_row = 0;
	cursor_col = 0;
}

/** @brief Prints character ch with the specified color
 *         at position (row, col).
 *
 *  If any argument is invalid, the function has no effect.
 *
 *  @param row The row in which to display the character.
 *  @param col The column in which to display the character.
 *  @param ch The character to display.
 *  @param color The color to use to display the character.
 *  @return Void.
 */
void draw_char(int row, int col, int ch, int color){
	*(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * row + col)) = ch;
	*(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * row + col) + 1) = color;
	return;
}

/** @brief Returns the character displayed at position (row, col).
 *  @param row Row of the character.
 *  @param col Column of the character.
 *  @return The character at (row, col).
 */
char get_char(int row, int col) {
	return *(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * row + col));
}

