/**
 * Name: Anthony Wong
 * GTID: 903579250
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "arraylist.h"
#include "arraylist_tests.h"

/*
 * Dictionary and Dictionary length used in generating the pseudorandom message.
 */
int dictionary_length = 15;
char *dictionary[] = {
    "go",
    "congratulations",
    "yellow",
    "potato",
    "jackets",
    "viola",
    "piano",
    "unruly",
    "science",
    "friend",
    "this",
    "carrot",
    "is",
    "red",
    "correct",
};

int main(int argc, char *argv[]);
char *generateMessage();
