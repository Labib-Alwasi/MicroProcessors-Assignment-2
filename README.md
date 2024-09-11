# Raspberry Pi Pico Morse Code Project
Programming a Raspberry Pi Pico using C and Arm Assembly to create a Morse Code game using interrupts, buttons and LEDs.
(Microprocessors Assignment)
## Report

Report can be accessed [here](https://docs.google.com/document/d/16RKJ-n3LtSh2d0rVcJt9_j-bWo5DYeZhtlXz2NsJ3xg/edit?usp=sharing)


### Workflow


For proper Workflow management, add the individual contribution in the [Google Doc](https://docs.google.com/document/d/1w2TPiovsNV9KAnugQtNTTOijNtJ0zG-WWCioce6xo6E/edit?usp=sharing)


## Pseudo Code Top Level

```
   call welcome_message()
-> Recieve input for set_level (integer)
b	call load_level(1)
		call display_level_rules(1)
		level_1(wins, lives)
b 	call load_level(2)
		call display_level_rules(2)
		level_2(wins, lives
```

## Pseudo Code Level 1

```
if(wins!=5)

call display_character_level_1() for a random character
	-> receive morse_code_input (string) (probably will have to be a assembly subroutine with gpio interrupts*)

	if (check_pattern(set_level, given_char, morse_code_input) == 1)
		if (lives != 3)
			live--
			call set_rgb(lives)
		wins++
        call display_result
		repeat
	else
		lives++
		call set_rgb(lives)
		if (lives < 1)
			call game_over_failure()
		wins = 0
        call display_result()
		repeat
set_level++
```
