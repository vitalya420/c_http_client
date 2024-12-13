#include <stdio.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>

int is_ipv4(const char* buff) {
    const char* pattern = "^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\\.([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])){3}$";
    
    regex_t regex;
    int reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti != 0) {
        fprintf(stderr, "Could not compile regex\n");
        return 0;
    }

    reti = regexec(&regex, buff, 0, NULL, 0);
    
    regfree(&regex);

    return reti == 0;
}