#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 8
#define PRINT_ERROR() printf("[error]")

const char*TAG  = "div";
const size_t SPACES_COUNT = 4;
enum lexems {STR_TOKEN, START_TAG_TOKEN, END_TAG_TOKEN};

struct lexemStream{
    char* string;
    enum lexems lexem;
    struct lexemStream* next;
};

int substr(char *haystack, char* pre_needle, const char *needle, const int from){
    if(needle && haystack && strlen(needle) > strlen(haystack))
        return -1;
    size_t haystack_iter = (size_t)from;
    size_t needle_iter = 0;
    size_t pre_needle_iter = 0;
    while(haystack_iter < strlen(haystack)){
        if(*(haystack + haystack_iter) == *(pre_needle)) {
            pre_needle_iter = 1;
            while (
                    pre_needle_iter < strlen(pre_needle) &&
                    *(haystack + haystack_iter + pre_needle_iter) == *(pre_needle + pre_needle_iter)
                    ) {
                pre_needle_iter++;
            }
            if (pre_needle_iter == strlen(pre_needle)) {
                if (*(haystack + pre_needle_iter + haystack_iter) == *(needle)) {
                    needle_iter = 1;
                    while (needle_iter < strlen(needle) &&
                           *(haystack + haystack_iter + pre_needle_iter + needle_iter) == *(needle + needle_iter)) {
                        needle_iter++;
                    }
                    if (needle_iter == strlen(needle)) {
                        return (int)haystack_iter;
                    }
                }
            }

        }
        haystack_iter++;
    }
    return -1;
}


/**
 * if no symbols (or there no of the type %c) return end of string \0
 * @return char of input stream
 */
char getNextChar(){
    char symbol = '\0';
    int result = 0;
    result = scanf("%c", &symbol);
    return (result == 1) ? symbol : (char)EOF;
}

/**
 * free each string of pointer
 * free pointer
 * @param inputStream
 * @param length
 */
void deleteStrings(char** inputStream){
    for(size_t i = 0; inputStream[i]; i ++){
        free(inputStream[i]);
    }
    free(inputStream);
}

/**
 * simple container for pointer of strs
 */
struct stringPointer{
    char** inputStream;
    size_t cntOfStr;
    size_t toFreed;
};
/**
 * scans input stream and parse it to pointer of pointers
 * implements on dynamic buffer, grows by exp
 * @return pointer of strings
 */
struct stringPointer * parseStrings(struct stringPointer * strPtr) {
    size_t size_of_stream = INITIAL_CAPACITY;
    size_t size_of_string = INITIAL_CAPACITY;
    char **inputStream = (char**) calloc(size_of_stream, sizeof(char*));
    if(inputStream == NULL){
        PRINT_ERROR();
        free(inputStream);
        return NULL;
    }
    char symbol;
    char **tmpStream = (char**) calloc(size_of_stream, sizeof(char*));
    char *tmpString = (char*) calloc(size_of_string, sizeof(char));
    size_t cur_strings_count = 0;
    symbol = getNextChar();
    while (symbol != EOF)
    {
        size_t curSize = 0;
        char* string = (char*) calloc(size_of_string,sizeof(char));
        while(symbol != '\n' && symbol != EOF){
            if(curSize + 1 > size_of_string){
                memcpy(tmpString, string, curSize * sizeof(char));
                size_of_string *= 2;
                free(string);
                string = (char*)calloc(size_of_string+1, sizeof(char));
                memcpy(string, tmpString, curSize * sizeof(char));
                free(tmpString);
                tmpString = (char*)calloc(size_of_string+1, sizeof(char));
            }
            string[curSize++] = symbol;
            symbol = getNextChar();
        }
        string[curSize] = '\0';
        if(cur_strings_count >= size_of_stream){
            memcpy(tmpStream, inputStream, cur_strings_count * sizeof(char*));
            size_of_stream *= 2;
            free(inputStream);
            inputStream = (char**)calloc(size_of_stream, sizeof(char*));
            memcpy(inputStream, tmpStream, cur_strings_count * sizeof(char*));
            free(tmpStream);
            tmpStream = (char**)calloc(size_of_stream, sizeof(char*));
        }
        inputStream[cur_strings_count++] = string;
        size_of_string = INITIAL_CAPACITY;
        symbol = getNextChar();
    }
    free(tmpString);
    free(tmpStream);
    //not freed string
    strPtr->cntOfStr = cur_strings_count;
    strPtr->toFreed = size_of_stream;
    strPtr->inputStream = inputStream;
    return strPtr;
}

void printStrings(struct stringPointer* strings){
    size_t i =0;
    while(strings->inputStream[i]){
        char* out = strings->inputStream[i++];
        printf("%s", out);
        if(strings->inputStream[i])
            printf("\n");
    }
}
void deleteLexemStream(struct lexemStream* stream){
    struct lexemStream * tmpLex;
    tmpLex = stream->next;
    free(stream);
    stream = tmpLex;
    while(stream){
        tmpLex = stream->next;
        free(stream->string);
        free(stream);
        stream = tmpLex;
    }

}
struct lexemStream * insertAfter(struct lexemStream* lexem, enum lexems token, char* str){
    struct lexemStream * nextLex = (struct lexemStream*) calloc(1,sizeof(struct lexemStream));
    lexem->next = nextLex;
    nextLex->string = (char *) calloc( (size_t) (strlen(str)+1) , sizeof(char));
    memcpy(nextLex->string, str, strlen(str) * sizeof(char));
    nextLex->lexem = token;
    nextLex->next = NULL;
    return nextLex;
}

char* trimRightSpaces(char* untrim, size_t size){
    char* trim = (char*) calloc(size+1, sizeof(char));
    trim = memcpy(trim, untrim, size);
    size_t i = size-1;
    while(isspace(untrim[i]) && i-- > 0);
    trim[i+1] = '\0';

    return trim;
}

size_t getLeftTrimBorder(char* stringToTrime){
    size_t i =0;
    size_t size = strlen(stringToTrime);
    while(isspace(stringToTrime[i]) > 0 && i++ < size);
    return i;
}
char** getStringsFromLexems(struct lexemStream *  lexem, size_t length) {
    char** strings = (char**)calloc(length+1, sizeof(char*));
    size_t idention = 0;
    size_t curElem = 0;
    for (size_t i = 0; i < length && lexem->next; i++) {
        lexem = lexem->next;

        if(strlen(lexem->string) > 0) {
            if (lexem->lexem == END_TAG_TOKEN) {
                idention--;
            }
            char *string = (char *) calloc(strlen(lexem->string) + (idention * SPACES_COUNT) + 1, sizeof(char));
            if (idention > 0) {
                char *spaces = (char *) calloc(SPACES_COUNT * idention, sizeof(char));
                for (size_t i = 0; i < SPACES_COUNT * idention; i++) {
                    spaces[i] = ' ';
                }
                memcpy(string, spaces, SPACES_COUNT * idention);
                free(spaces);
            }
            memcpy(string + (idention * SPACES_COUNT), lexem->string, strlen(lexem->string));
            strings[curElem++] = string;
            if (lexem->lexem == START_TAG_TOKEN) {
                idention++;
            }
        }
    }
    return strings;
}
void deleteVector(char **s) {
    size_t str_cnt = 0;
    char* str = *(s+str_cnt++);
    while (str){
        free(str);
        str = *(s+str_cnt++);
    }
    free(s);
}
char** div_format(char **s) {
    size_t str_cnt = 0;
    char* div_str = *(s+str_cnt++);
    if(strlen(div_str) == 0){
        return s;
    }
    char* open_tag = "<";
    char* close_tag = "</";
    char* closed_tag = (char*)calloc((strlen(TAG) + 2),sizeof(char));
    size_t i = 0;
    for(i=0; i < strlen(TAG); i++){
        closed_tag[i] = TAG[i];
    }
    closed_tag[i] = '>';
    char* full_open_tag = (char*)calloc((strlen(TAG)+3),sizeof(char));
    char* full_close_tag = (char*)calloc((strlen(TAG)+4),sizeof(char));
    i = 0;
    full_open_tag[i] = '<';
    for(i=1; i < strlen(TAG)+1; i++){
        full_open_tag[i] = TAG[i-1];
    }
    full_open_tag[i] = '>';
    full_close_tag[0] = '<';
    full_close_tag[1] = '/';
    for(i=0; i < strlen(TAG); i++){
        full_close_tag[i+2] = TAG[i];
    }
    full_close_tag[i+2] = '>';
    char* parsed_str = (char*)calloc(1,sizeof(char));
    char* full_trimmed;
    struct lexemStream * rootLex = NULL;
    rootLex = (struct lexemStream*) calloc(1,sizeof(struct lexemStream));
    rootLex->string = "root";
    rootLex->lexem = STR_TOKEN;
    struct lexemStream * curLeaf = rootLex;
    int tag_counter = 0;
    size_t length = 0;
    char* trimmed_str;


    while(div_str){
        size_t offset = 0;
        int sub_open = 0;
        int sub_close = 0;
        while(strlen(div_str) > offset)
        {
            sub_open = (int)substr(div_str, open_tag, closed_tag, offset);
            sub_close = (int)substr(div_str, close_tag, closed_tag, offset);
            if(sub_open != -1 || sub_close != -1){
                // get open_tag at sub_open position
                if((sub_open < sub_close && sub_open != -1) || (sub_open != -1 && sub_close == -1)){
                    if(sub_open - (int)offset > 0) {
                        trimmed_str = trimRightSpaces(div_str+offset, (size_t) (sub_open - (int)offset) * sizeof(char));
                        full_trimmed = strdup(trimmed_str+getLeftTrimBorder(trimmed_str));
                        free(trimmed_str);
                        curLeaf = insertAfter(curLeaf, STR_TOKEN, full_trimmed);
                        free(full_trimmed);
                    }
                    curLeaf = insertAfter(curLeaf, START_TAG_TOKEN, full_open_tag);
                    length+=2;
                    tag_counter++;
                    offset += (sub_open - (int)offset) + strlen(open_tag) + strlen(closed_tag);
                }
                //get close_tag at sub_close position
                else{

                    if(sub_close - (int)offset > 0) {
                        trimmed_str = trimRightSpaces(div_str+offset, (size_t) (sub_close - (int)offset) * sizeof(char));
                        full_trimmed = strdup(trimmed_str+getLeftTrimBorder(trimmed_str));
                        free(trimmed_str);
                        curLeaf = insertAfter(curLeaf, STR_TOKEN, full_trimmed);
                        free(full_trimmed);
                    }
                    curLeaf = insertAfter(curLeaf, END_TAG_TOKEN, full_close_tag);
                    tag_counter --;
                    length+=2;
                    if(tag_counter < 0) {
                        PRINT_ERROR();
                        free(parsed_str);
                        free(closed_tag);
                        free(full_close_tag);
                        free(full_open_tag);
                        deleteLexemStream(rootLex);
                        deleteVector(s);
                        return NULL;
                    }
                    offset += (sub_close - (int)offset) + strlen(close_tag) + strlen(closed_tag);
                }

            }
            else{
                trimmed_str = trimRightSpaces(div_str+offset, (size_t) (strlen(div_str) - (int)offset) * sizeof(char));
                full_trimmed = strdup(trimmed_str+getLeftTrimBorder(trimmed_str));
                free(trimmed_str);
                if(strlen(full_trimmed) > 0) {
                    curLeaf = insertAfter(curLeaf, STR_TOKEN, full_trimmed);
                    length++;
                }
                free(full_trimmed);
                offset = strlen(div_str);
            }
        }
        //insert last chunk of string to list
        div_str = *(s+str_cnt++);
    }

    if(tag_counter != 0){
        PRINT_ERROR();
        deleteVector(s);
        free(parsed_str);
        free(closed_tag);
        free(full_close_tag);
        free(full_open_tag);
        deleteLexemStream(rootLex);
        return NULL;
    }
    deleteVector(s);
    s = getStringsFromLexems(rootLex, length);
    deleteLexemStream(rootLex);
    free(parsed_str);
    free(closed_tag);
    free(full_close_tag);
    free(full_open_tag);
    return s;
}

/**
 * reads input strings to a pointer of strings, than parse them to list
 * than print them from list, with correct indention
 * than free list and free strings
 * @return 0
 */
int main() {
    struct stringPointer * strPtr = (struct stringPointer*)calloc(1,sizeof(struct stringPointer));
    strPtr = parseStrings(strPtr);
    if (strPtr->cntOfStr <= 0) PRINT_ERROR();
    strPtr->inputStream = div_format(strPtr->inputStream);
    if(strPtr->inputStream == NULL){
        size_t i=0;
        if(strPtr->inputStream) {
            while (strPtr->inputStream[i]) {
                free(strPtr->inputStream[i++]);
            }
            free(strPtr->inputStream);
        }
        free(strPtr);
        return 0;
    }
    printStrings(strPtr);
    deleteStrings(strPtr->inputStream);
    free(strPtr);
    return 0;
}