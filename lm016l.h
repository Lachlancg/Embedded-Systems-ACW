/*
 * Header file for the driver for the LM016L LCD.
 * 
 * Author: Matthew
 */

#ifndef LM016L_H
#define LM016L_H

/* Initialise the LCD. */
void lm016l_init(void);

/* Clear the LCD. */
void lm016l_clear(void);

/* Go to a certain line of the display. Must be an integer between 0 and 3. */
void lm016l_goto_line(char);

/* Write a string to the LCD, at the current cursor position. */
void lm016l_puts(const char *);

#endif /* LM016L_H */
