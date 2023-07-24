/*! \mainpage Project Description
 *
 * \section intro_sec Introduction
 *
 * In this assignment, the four members were tasked to create a game to teach a player Morse code.\n
 * A mixture of C code and ARM assembly language was used to build this game.\n
 * The player interacts with the game by using the MAKER-PI-PICO.\n

 * When the player starts the game, the UART console displays the following instructions on how to play the game as\n
 * well as four levels for the player to choose from.\n

 * On the MAKER-PI-PICO, the player will press the “GP21” button to interact with the game. If pressed for a short\n
 * duration, it will input a Morse “dot”. If pressed for a longer duration, it will input a Morse “dash”. If there\n
 * is no new input for over 1 second, it represents a “space” character. And if there’s no new input,\n
 * considered complete. The sequence then is passed to the pertinent game functions for pattern matching.\n\n
 
 * Level 1: requires the player to match individual characters (with the expected Morse code pattern).\n

 * Level 2: requires the player to match individual characters (without the expected Morse code pattern).\n

 * Level 3: requires the player to match individual words (with the equivalent Morse code pattern).\n

 * Level 4: requires the player to match individual words (without the equivalent Morse code pattern).\n\n

 * The RGB LED on the MAKER-PI-PICO will change colour depending on the status of the player in the game.\n
 * Blue: Game is not in progress.\n
 * Green: Game in Progress; Lives = 3\n
 * Yellow: Lives = 2\n
 * Orange: Lives = 1\n
 * Red: Game Over\n\n

 * The player gets 3 lives throughout the game, every time an incorrect pattern is entered, a life is lost.\n
 * However, if they get 5 correct answers in a row, they can proceed to the next level. 


 
 * \subsection Group Members
 * Joesph Brown\n
 * Shivraj Bhathal\n
 * Labib Alwasi\n
 * Urvee Aggarwal\n
 
 */

/* Import header files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/gpio.h"
#include "assign02.pio.h"



char *set_input_array[256]; ///< Set input
const char *alpha_morse[] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
                       "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-",
                       "...-", ".--", "-..-", "-.--", "--.."}; ///< Alphabet Morse Sequences 
const char *num_morse[] = {"-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."};
const char *words[] = {"cave",
                 "copy",
                 "dock",
                 "lick",
                 "run",
                 "owl",
                 "free",
                 "sink",
                 "scold",
                 "hold",
                 "smoke",
                 "part",
                 "vex",
                 "able",
                 "bang",
                 "nose",
                 "tan",
                 "van",
                 "sob",
                 "blue",
                 "nap"}; ///< Words for Level 3/4
const char *words_morse[] = {
    "-.-. .- ...- .",
    "-.-. --- .--. -.--"
    "-.. --- -.-. -.-",
    ".-.. .. -.-. -.-",
    ".-. ..- -.",
    "--- .-- .-..",
    "..-. .-. . .",
    "... .. -. -.-",
    "... -.-. --- .-.. -..",
    ".... --- .-.. -..",
    "... -- --- -.- .",
    ".--. .- .-. -",
    "...- . -..- ",
    ".- -... .-.. .",
    "-... .- -. --.",
    "-. --- ... ."
    "- .- -.",
    "...- .- -.",
    "... --- -...",
    "-... .-.. ..- .",
    "-. .- .--."}; ///< Morse Code for Level 3/4


/** @defgroup group1 Hashing
 *  Hash Functions, Structs, Variables and Initialization
 *  @{
 */
/**
 * Hash table to store words and their associated morse code
 * @param word The word is hashed into a string
 *               
 */
struct words_hash_table
{
    char *word;
    char *morse;
};

/**
 * Hash table to store words and their associated morse code for Levels 3 and 4
 * Hashes all the words to their morse code (making a dictionary)        
 * This makes it easier to compare when checking the morse code input
 */
struct words_hash_table *hashArray[20]; ///< Hash table for Levels 3 and 4

struct words_hash_table *item; ///< Item for comparison for hash Table

/**
 * Hashing Function - djb2 hashing algorthm
 * @param word The word is hashed into a string
 *               
 */
unsigned long hashstring(unsigned char *word) // djb2 hashing algorithm
{
    unsigned long hash = 5381;
    int c;

    while (c = word++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}


///Find the hashed index from the hash table            
struct words_hash_table *search(char *word)
{
    // get the hash
    int hashIndex = hashstring(word);

    // move in array until an empty
    while (hashArray[hashIndex] != NULL)
    {

        if (hashArray[hashIndex]->word == word)
            return hashArray[hashIndex];

        // go to next cell
        ++hashIndex;

        // wrap around the table
        hashIndex %= 20;
    }

    return NULL;
}

///Allocate memory for a new table          
void insert(char *word, char *morse)
{
    struct words_hash_table *item = (struct words_hash_table *)malloc(sizeof(struct words_hash_table));
    item->word = word;
    item->morse = morse;

    int hashIndex = hashstring(word);

    // move in array until an empty cell
    while (hashArray[hashIndex] != NULL)
    {
        // go to next cell
        ++hashIndex;

        // wrap around the table
        hashIndex %= 20;
    }
}


///Initilise Hash Table         
void initialise_hash_table()
{
    for (int i = 0; i < 20; i++)
    {
        insert(words[i], words_morse[i]);
    }
}

/** @} * */ // end of group4


/// Declare the main assembly code entry point.
void main_asm();

/** @defgroup group2 Game
 *  Welcome to this Morse Code Game!
 *  @{
 */
absolute_time_t start_time; ///< start the timer
int level_number; ///< Chosen Level by user to enter
int lives; ///< Total Number of Lives
char level_selection[5]; ///< Levels available in the game
bool game_status = false; ///< Begin or not
/**
 * Responsible for displaying the welcome message
 * Contains the rules and options to choose level
 */
void welcome_message();

/**
 * Sets the LED color to indicate the status of the game
 * Blue - Game not in progress
 * Green - Game in Progress; Lives = 3
 * Yellow - Lives = 2
 * Orange - Lives = 1
 * Red - Game Over
 */
void set_rgb();

/**
 * Loads the level corresponding to the level chosen by the user
 */
void load_level(int level_number = 1);

/**
 * @brief Checks Morse Pattern
 * 
 *
 * Checks the input morse code against the actual morse code of teh given character
 * Returns 1 if correctly matched, 0 otherwise
 */
int check_pattern(int level_number, char *given_char, char *morse_code_input);


///Function to generate random character for use in levels 1 and 2
char generate_random_character();

///Function to generate random word for use in levels 3 and 4
char generate_random_word();


///Displays the message banner when player wins the game
void game_over_success();


///Displays the message banner for when player loses all its lives
void game_over_failure();

/** @defgroup group3 LevelFunctions
 *  @ingroup group2
 *  Logic for each Game level
 *  @{
 */
///Subroutine to play level 1 of the game
void level_1(int num_wins);
///Subroutine to play level 2 of the game
void level_2(int num_wins);
///Subroutine to play level 3 of the game
void level_3(int num_wins);
///Subroutine to play level 4 of the game
void level_4(int num_wins);
/** @} */ // end of group3

///Subroutine to print stats of the level
void print_level_stats(int num_wins, int num_losses);

/** @} */ // end of group2



/// Main entry point for the code
int main()
{
    stdio_init_all();
    initialise_hash_table();

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, 0, offset, WS2812_PIN, 800000, IS_RGBW);

    main_asm();
    return (0);
}

/** @defgroup group4 ARM
 *  ARM assembly code set up
 *  @{
 */

/// Initialise a GPIO pin – see SDK for detail on gpio_init()
void asm_gpio_init(uint pin)
{
    gpio_init(pin);
}

/// Set direction of a GPIO pin – see SDK for detail on gpio_set_dir()
void asm_gpio_set_dir(uint pin, bool out)
{
    gpio_set_dir(pin, out);
}

/// Get the value of a GPIO pin – see SDK for detail on gpio_get()
bool asm_gpio_get(uint pin)
{
    return gpio_get(pin);
}

/// Set the value of a GPIO pin – see SDK for detail on gpio_put()
void asm_gpio_put(uint pin, bool value)
{
    gpio_put(pin, value);
}

/// Enable falling-edge interrupt – see SDK for detail on gpio_set_irq_enabled()
void asm_gpio_set_irq(uint pin)
{
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_FALL, true);
}
///Grab the current time 
void start_timer()
{
    start_time = get_absolute_time();
}
///End the timer
int end_timer()
{
    int end_time = (int)absolute_time_diff_us(start_time, get_absolute_time());
    return end_time / 100000;
}

void set_input(int case_received, int input_character_index)
{
    printf("Just to pass Assembly\n");
}

/** @} */ // end of group4


///Prints out Welcome message banner with Group number and member names. 
void welcome_message()
{
    printf("----------------------------------------------------------------------------------------------\n----------------------------------------------------------------------------------------------\n");
    printf("#       #       #  # # # #  #        # # # #   # # # #  #       #   # # # #\n");
    printf(" #     # #     #   #        #        #         #     #  ##     ##   #      \n");
    printf("  #   #   #   #    # # # #  #        #         #     #  # #   # #   # # # #\n");
    printf("   # #     # #     #        #        #         #     #  #  # #  #   #      \n");
    printf("    #       #      # # # #  # # # #  # # # #   # # # #  #   #   #   # # # #\n");
    printf("Group Number:\t0\nGroup Members:\tJoesph Brown, Shivraj Bhathal, Labib Alwasi, Urvee Aggarwal\n----------------------------------------------------------------------------------------------\n----------------------------------------------------------------------------------------------\n");
}

///Prints out Rules of each level
void load_level_rules(int level_number)
{
    printf("----------------------------------------------------------------------------------------------\n");

    if (level_number == 1)
    {
        printf("Level 1 Begins!\n");

        printf("\t\t\t\tMorse Code to English Text\n");

        printf("Convert individual characters from their morse code:\n");
        printf("A [.-]\tB[-...]\tC [-.-.]\tD [-..]\tE [.]\tF [..-.]\n");
        printf("G [--.]\tH [....]\tI [..]\tJ [.---]\tK [-.-]\tL [.-..]\n");
        printf("M [--]\tN [-.]\tO [---]\tP [.--.]\tQ [--.-]\tR [.-.]");
        printf("S [...]\tT [-]\tU [..-]\tV [...-]\tW [.--]\tX [-..-]");
        printf("Y [-.--]\tZ [--..]\tDigits[0-9]\n");
    }
    else if (level_number == 2)
    {
        printf("Level 2 reached!\n");

        printf("Convert individual characters from their morse code:\n");
        printf("A\tB\tC\tD\tE\tF\n");
        printf("G\tH\tI\tJ\tK\tL\n");
        printf("M\tN\tO\tP\tQ\tR\n");
        printf("S\tT\tU\tV\tW\tX\n");
        printf("Y\tZ\tDigits[0-9]\n");
    }
    else if (level_number == 3)
    {
        printf("Level 3 reached!\n");

        printf("\t\t\t\tMorse Code to English Text\n");

        printf("Convert individual characters from their morse code:\n");
        printf("A [.-]\tB[-...]\tC [-.-.]\tD [-..]\tE [.]\tF [..-.]\n");
        printf("G [--.]\tH [....]\tI [..]\tJ [.---]\tK [-.-]\tL [.-..]\n");
        printf("M [--]\tN [-.]\tO [---]\tP [.--.]\tQ [--.-]\tR [.-.]");
        printf("S [...]\tT [-]\tU [..-]\tV [...-]\tW [.--]\tX [-..-]");
        printf("Y [-.--]\tZ [--..]\tDigits[0-9]\n");
    }
    else if (level_number == 4)
    {
        printf("Level 4 reached!\n");

        printf("\t\t\t\tMorse Code to English Text\n");

        printf("Convert individual characters from their morse code:\n");
        printf("A\tB\tC\tD\tE\tF\n");
        printf("G\tH\tI\tJ\tK\tL\n");
        printf("M\tN\tO\tP\tQ\tR\n");
        printf("S\tT\tU\tV\tW\tX\n");
        printf("Y\tZ\tDigits[0-9]\n");
    }
    fflush(stdout);
}

/**
 * Changes LED light colours           
 */
void set_rgb()
{

    if (game_start == 0)
    {
        // Set LED to BLUE once the game opens but hasnt started
        put_pixel(urgb_u32(0x00, 0x00, 0xFF));
    }
    else
    {
        switch (lives)
        {
        case 3:
            put_pixel(urgb_u32(0x80, 0xFF, 0x00));
            break;

        case 2:
            put_pixel(urgb_u32(0xFF, 0xFF, 0x00));
            break;

        case 1:
            put_pixel(urgb_u32(0xFF, 0x80, 0x00));
            break;

        case 0:
            put_pixel(urgb_u32(0xFF, 0x00, 0x00));
            break;
        }

        printf("You have %d lives left\n", lives);
    }
}



///Initilise level entered by the player for the first time
void load_level(int level_number = 1)
{

    set_rgb();

    printf("Please choose a level using the corresponding morse code:\n");
    printf("Level 1 ( .---- ) :\tIndividual characters with their equivalent Morse code provided.\n")
    printf("Level 2 ( ..--- ) :\tIndividual characters without their equivalent Morse code provided.\n");
    printf("Level 3 ( ...-- ) :\tIndividual words with their equivalent Morse code provided.\n");
    printf("Level 4 ( ....- ) :\tIndividual words without their equivalent Morse code provided.\n")
        // scanf("%d", level_number);

        char input_buffer[5];
    while (1)
    {
        main_asm();
        // set_input_array used in the following code is the global array which stores the user input
        // - name may need to be changed to correspond to the actual variable (as of right now it has not been defined)
        if (strcmp(set_input_array, ".----") == 0)
        {
            printf("Level 1 selected!\n");
            level_number = 1;
            break;
        }
        else if (strcmp(set_input_array, "..---") == 0)
        {
            printf("Level 2 selected!\n");
            level_number = 2;
            break;
        }
        else if (strcmp(set_input_array, "...--") == 0)
        {
            printf("Level 3 selected!\n");
            level_number = 3;
            break;
        }
        else if (strcmp(set_input_array, "....-") == 0)
        {
            printf("Level 4 selected!\n");
            level_number = 4;
            break;
        }
        else
        {
            printf("Invalid input, try again!\n");
            memset(set_input_array, 0, sizeof set_input_array);
        }
    }

    memset(set_input_array, 0, sizeof set_input_array);

    switch (level_number)
    {
    case 1:
        level_1(0);
        break;
    case 2:
        level_2(0);
        break;
    case 3:
        level_3(0);
        break;
    case 4:
        level_4(0);
        break;
    default:
        level_1(0);
        break;
    }
}

char generate_random_character()
{
    char randomletter = 'A' + (random() % 26);
    return randomletter;
}

/**
 * Checks patterns
 * 
 * @param level_number Current level the player is at
 * @param given_char Character we are dealing with
 * @param morse_code_input user input            
 */
int check_pattern(int level_number, char *given_char, char *morse_code_input)
{
    switch (level_number)
    {
    case 1:
        for (int i = 0; i < sizeof(alpha_morse); i++)
        {
            if (strcmp(alpha_morse[i], given_char) == 0)
            {
                if (strcmp(alpha_morse[i], morse_code_input) == 0)
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
        }
    }

    for (int i = 0; i < sizeof(num_morse); i++)
    {
        if (strcmp(num_morse[i], given_char) == 0)
        {
            if (strcmp(num_morse[i], morse_code_input) == 0)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }
    case 2:
        for (int i = 0; i < sizeof(alpha_morse); i++)
        {
            if (strcmp(alpha_morse[i], given_char) == 0)
            {
                if (strcmp(alpha_morse[i], morse_code_input) == 0)
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
        }   
        for (int i = 0; i < sizeof(num_morse); i++)
        {
            if (strcmp(num_morse[i], given_char) == 0)
            {
                if (strcmp(num_morse[i], morse_code_input) == 0)
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
        }
    case 3:
        for (int i = 0; i < sizeof(words); i++)
        {
            if (strcmp(words[i], given_char) == 0)
            {
                int hashIndex = hashstring(&given_char);
                if (strcmp(hashArray[hashIndex]->morse, morse_code_input) == 0)
                {
                    return 1;
                }
            else
            {
                return 0;
            }
        }
        }
    case 4:
        for (int i = 0; i < sizeof(words); i++)
        {
            if (strcmp(words[i], given_char) == 0)
            {
                int hashIndex = hashstring(&given_char);
                if (strcmp(hashArray[hashIndex]->morse, morse_code_input) == 0)
                {
                    return 1;
                }
            else
            {
                return 0;
            }
        }
    }
}


void level_1(int num_wins) {
    lives = 3;
    int correct_try_count = 0;
    int fail_count = 0; 

    game_status = true;
    set_rgb();

    while(1) {
        given_char = generate_random_character(); // given char is a global variable

        int index_for_morse;
        for (i = 0; i < sizeof alphabet; i++) {
            if (strcmp(given_char, alphabet[i]) == 0) {
                index_for_morse = i;
                break;
            }
        }

        printf("Input the corresponding morse code for the following letter to progress to the next level:\n");
        printf("Letter: %c\n", given_char);
        printf("Morse code: %s\n", alphamorse[index_for_morse]);

        while(1) {
            main_asm();
            if (check_pattern(1, given_char, morse_code_input) == 1) {
                correct_try_count++;
                
                if (lives < 3) {
                    lives++;
                }

                printf("Congratulations, that is correct! You are %i/5 of the way to the next level!\n %i lives remaining\n", correct_try_count, lives);
                break;
            } else {
                lives--;
                fail_count++;
                printf("That is incorrect - %i lives remaining\n", lives);
            }

            set_rgb();

            if (lives == 0) {
                printf("You have run out of lives - Game Over!\n")
                break;
            }
        }

        if (lives == 0) {
            print_level_stats(correct_try_count, fail_count);
            game_over_failure();
        }

        if (correct_try_count == 5) {
            printf("You have now completed this level. Moving to level 2.\n");
            print_level_stats(correct_try_count, fail_count);
            level_2();
        }
    }
}

void level_2(int num_wins) {
    lives = 3;
    int correct_try_count = 0;
    int fail_count = 0;

    game_status = true;
    set_rgb();

    while(1) {
        given_char = generate_random_character(); // given char is a global variable

        int index_for_morse;
        for (i = 0; i < sizeof alphabet; i++) {
            if (strcmp(given_char, alphabet[i]) == 0) {
                index_for_morse = i;
                break;
            }
        }

        printf("Input the corresponding morse code for the following letter to progress to the next level:\n");
        printf("Letter: %c\n", given_char);

        while(1) {
            main_asm();
            if (check_pattern(1, given_char, morse_code_input) == 1) {
                correct_try_count++;

                if (lives < 3) {
                    lives++;
                }

                printf("Congratulations, that is correct! You are %i/5 of the way to the next level!\n %i lives remaining\n", correct_try_count, lives);
                break;
            } else {
                lives--;
                fail_count++;
                printf("That is incorrect - %i lives remaining\n", lives);
            }

            set_rgb();

            if (num_lives == 0) {
                printf("You have run out of lives - Game Over!\n")
                break;
            }
        }

        if (lives == 0) {
            print_level_stats(correct_try_count, fail_count);
            game_over_failure();
        }

        if (correct_try_count == 5) {
            printf("You have now completed this level. Moving to level 3.\n");
            print_level_stats(correct_try_count, fail_count);
            level_3();
        }
    } 
}

void level_3(int num_wins) {
    lives = 3;
    int correct_try_count = 0;
    int fail_count = 0; 

    game_status = true;
    set_rgb();

    while(1) {
        int random_index = random() % 20;

        printf("Input the corresponding morse code for the following word to progress to the next level:\n");
        printf("Word: %c\n", words[random_index]);
        printf("Morse code: %s\n", words_morse[random_index]);

        while(1) {
            main_asm();
            if (check_pattern(1, given_char, morse_code_input) == 1) {
                correct_try_count++;
                
                if (lives < 3) {
                    lives++;
                }

                printf("Congratulations, that is correct! You are %i/5 of the way to the next level!\n %i lives remaining\n", correct_try_count, lives);
                break;
            } else {
                lives--;
                fail_count++;
                printf("That is incorrect - %i lives remaining\n", lives);
            }

            set_rgb();

            if (lives == 0) {
                printf("You have run out of lives - Game Over!\n")
                break;
            }
        }

        if (lives == 0) {
            print_level_stats(correct_try_count, fail_count);
            game_over_failure();
        }

        if (correct_try_count == 5) {
            printf("You have now completed this level. Moving to level 2.\n");
            print_level_stats(correct_try_count, fail_count);
            level_2();
        }
    }
}

void level_4(int num_wins) {
    lives = 3;
    int correct_try_count = 0;
    int fail_count = 0; 

    game_status = true;
    set_rgb();

    while(1) {
        int random_index = random() % 20;

        printf("Input the corresponding morse code for the following word to progress to the next level:\n");
        printf("Word: %c\n", words[random_index]);

        while(1) {
            main_asm();
            if (check_pattern(1, given_char, morse_code_input) == 1) {
                correct_try_count++;
                
                if (lives < 3) {
                    lives++;
                }

                printf("Congratulations, that is correct! You are %i/5 of the way to the next level!\n %i lives remaining\n", correct_try_count, lives);
                break;
            } else {
                lives--;
                fail_count++;
                printf("That is incorrect - %i lives remaining\n", lives);
            }

            set_rgb();

            if (lives == 0) {
                printf("You have run out of lives - Game Over!\n")
                break;
            }
        }

        if (lives == 0) {
            print_level_stats(correct_try_count, fail_count);
            game_over_failure();
        }

        if (correct_try_count == 5) {
            printf("You have now completed this level. Moving to level 2.\n");
            print_level_stats(correct_try_count, fail_count);
            level_2();
        }
    }
}




/**
 * Prints out scoreboard when called.
 * 
 * @param num_wins Number of Wins
 * @param num_losses Number of losses
 * @return scoreboard with win percentage and number of attempts
 * @note if necessary  
 */
void print_level_stats(int num_wins, int num_losses) {
    int win_percentage = num_wins / (num_wins + num_losses);
    printf("Total number of attempts: %i", num_wins + num_losses);
    printf("Number of successful attempts: %i\n", num_wins);
    printf("Number of failed attempts: %i\n", num_losses);
    printf("Success Rate: %i%% ", win_percentage)
}
