/* The MIT License (MIT)
 *
 * Copyright (c) 2013, Yichao Zhou
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* Sorry, windows only....  I really have no time. */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include "vfrex.h"
#include <Shlwapi.h>

int top;
char files[1000][256];

int num_filter;
const char *file_filter[1000];
const char *pattern;

bool recursive;
bool ignore_case;

void blue(void)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_BLUE |
                            FOREGROUND_INTENSITY);
}

void red(void)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
}

void green(void)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
}

void yellow(void)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_RED |
                            FOREGROUND_INTENSITY);
}

void white(void)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN |
                            FOREGROUND_BLUE);
}

void usage(void)
{
    puts("Usage: mgrep [OPTIONs] PATTERN [FILEs]\n"
         "  Search PATTERN in [FILEs]\n"
         "  Example: mgrep \"Hello World\" main.c mgrep.c\n\n"
         "[OPTIONs]\n"
         "      -h, --help:             Show this message.\n"
         "      -r, -R, --recursive:    Grep recursively.  All the files whose file\n"
         "                              name contains one of [FILEs] will be greped\n"
         "      -i, -I, --ignore-case:  Ignore case difference");
}

char buffer[512];
/* Check whether there is a \0 in it.  This do NOT work for all the UCS (RAW
 * unicode) encoding! */
bool is_binary(const char *file)
{
    FILE *fin = fopen(file, "rb");
    if (!fin)
        return true;
    size_t size = fread(buffer, 1, 512, fin);
    for (size_t i = 0; i < size; ++i)
        if (buffer[i] == 0) {
            fclose(fin);
            return true;
        }
    fclose(fin);
    return false;
}


bool is_exists(const char *file_name)
{
    DWORD fileAtt = GetFileAttributesA(file_name);

    if(fileAtt == INVALID_FILE_ATTRIBUTES)
        return false;
    return ((fileAtt & FILE_ATTRIBUTE_DIRECTORY) == 0); 
}

bool valid_file(const char *file_name)
{
    if (num_filter == 0)
        return true;

    bool succ = false;
    for (int i = 0; i < num_filter; ++i)
        if (strstr(file_name, file_filter[i])) {
            succ = true;
            break;
        }
    return succ;
}

/* Copy and modify from Internet. */
void findAllFile(char * pFilePath){
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char DirSpec[MAX_PATH + 1];
    DWORD dwError;
    strncpy (DirSpec, pFilePath, strlen(pFilePath) + 1);
    SetCurrentDirectory(pFilePath);
    strncat (DirSpec, "\\*", 3);
    hFind = FindFirstFile(DirSpec, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE){
        puts(DirSpec);
        printf ("Invalid file handle. Error is %u\n",
                (unsigned)GetLastError());
        return ;
    }
    else{
        if (FindFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY ) {
            if (valid_file(FindFileData.cFileName)) {
                strncpy(files[top], pFilePath, 255); 
                strncat(files[top], "\\", 255); 
                strncat(files[top++], FindFileData.cFileName, 255); 
            }
        }
        else if(FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY&& strcmp(FindFileData.cFileName, ".") != 0&& strcmp(FindFileData.cFileName, "..") != 0){
            char Dir[MAX_PATH + 1];
            strcpy(Dir, pFilePath);
            strncat(Dir, "\\", 2);
            strcat(Dir, FindFileData.cFileName);
            findAllFile(Dir);
        }
        while (FindNextFile(hFind, &FindFileData) != 0){
            if (FindFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY){
                if (valid_file(FindFileData.cFileName)) {
                    strncpy(files[top], pFilePath, 255); 
                    strncat(files[top], "\\", 255); 
                    strncat(files[top++], FindFileData.cFileName, 255); 
                }
            }
            else if(FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY&& strcmp(FindFileData.cFileName, ".") != 0&& strcmp(FindFileData.cFileName, "..") != 0){
                char Dir[MAX_PATH + 1];
                strcpy(Dir, pFilePath);
                strncat(Dir, "\\", 2);
                strcat(Dir, FindFileData.cFileName);
                findAllFile(Dir);
            }
        }
        dwError = GetLastError();
        FindClose(hFind);
        if (dwError != ERROR_NO_MORE_FILES)     {
            printf("FindNextFile error. Error is %u\n",
                   (unsigned)dwError);
            return;
        }
    }
}

void grep(const char *file_name)
{
    int line = 0;
    FILE *fin = fopen(file_name, "r");
    for (;;) {
        ++line;
        fgets(buffer, 512, fin);
        if (feof(fin))
            break;

        vfrex_option_t option = {
            REGEX_STYLE_PERL,
            REGEX_MATCH_PARTIAL_BOUNDARY,
            ignore_case,
        };
        vfrex_t result = vfrex_match(buffer, pattern, option);
        if (result) {
            green();
            printf("  line %d:", line);
            white();
            const char *left, *right;
            vfrex_group(0, &left, &right, result);
            for (const char *c = buffer; c < left; ++c)
                putchar(*c);
            red();
            for (const char *c = left; c < right; ++c)
                putchar(*c);
            white();
            for (const char *c = right; *c; ++c)
                putchar(*c);
        }
    }
    fclose(fin);
}

int main(int argc, char const *argv[])
{
    if (argc == 1) {
        usage();
        return 0;
    }

    bool has_pattern = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage();
            return 0;
        }
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "-R") == 0 ||
            strcmp(argv[i], "--recursive") == 0) {
            recursive = true;
        }
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "-I") == 0 ||
            strcmp(argv[i], "--ignore-case") == 0) {
            ignore_case = true;
        }
        if (argv[i][0] != '-') {
            if (!has_pattern) {
                has_pattern = true;
                pattern = argv[i];
            } else
                file_filter[num_filter++] = argv[i];
        }
    }

    if (recursive) {
        char dir[MAX_PATH + 1];
        GetCurrentDirectory(MAX_PATH, dir);
        findAllFile(dir);
        SetCurrentDirectory(dir);
    } else {
        for (int i = 0; i < num_filter; ++i)
            if (is_exists(file_filter[i])) {
                strcpy(files[top++], file_filter[i]);
            }
    }

    for (int i = 0; i < top; ++i) {
        char path[MAX_PATH + 1];
        char dir[MAX_PATH + 1];
        GetCurrentDirectory(MAX_PATH, dir);
        PathRelativePathTo(path, dir, FILE_ATTRIBUTE_DIRECTORY, files[i], 0);
        if (!is_binary(files[i])) {
            blue();
            printf("File %s:\n", path);
            white();
            grep(files[i]);
        } else {
            blue();
            printf("File %s:\t", path);
            yellow();
            puts("Binary file");
            white();
        }
    }

    return 0;
}
