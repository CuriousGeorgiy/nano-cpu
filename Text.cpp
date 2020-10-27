#include "Text.hpp"

#include "error.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <Windows.h>

Text::Text(const char *inputFileName)
: buffer(nullptr), lines(nullptr), inputFileName(inputFileName), nLines(0)
{
    assert(inputFileName != nullptr);

    readTextFileToBuffer();
    countLinesInBuffer();
    parseBuffer();
}

Text::~Text()
{
    std::free(lines);
    std::free(buffer);
}

void Text::readTextFileToBuffer()
{
    HANDLE inputFileHandle = CreateFileA(inputFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL);
    DWORD inputFileSize = GetFileSize(inputFileHandle, NULL);
    HANDLE inputFileMappingHandle = CreateFileMappingA(inputFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
    LPVOID inputFileMapView = MapViewOfFile(inputFileMappingHandle, FILE_MAP_READ, 0, 0, 0);

    buffer = (char *) std::calloc(inputFileSize + 2, sizeof(char));
    std::memcpy(buffer + 1, inputFileMapView, inputFileSize);

    UnmapViewOfFile(inputFileMapView);
    CloseHandle(inputFileMappingHandle);
    CloseHandle(inputFileHandle);
}

void Text::countLinesInBuffer()
{
    int prePreChar = EOF;
    int preChar = EOF;

    const char *reader = buffer + 1;

    while (*reader) {
        if ((*reader == '\n') && (preChar == '\r') && (prePreChar != '\n') && (prePreChar != EOF)) ++nLines;

        prePreChar = preChar;
        preChar = (unsigned char) *(reader++);
    }

    if ((preChar != '\n') && (preChar != EOF) && (prePreChar != '\r')) ++nLines;
}

void Text::parseBuffer()
{
    lines = (Line *) std::calloc(nLines + 1, sizeof(Text::Line));

    lines[0].str = std::strtok(buffer + 1, "\r\n");

    lines[0].str[-1] = '\0';
    lines[0].len = strlen(lines[0].str);

    for (size_t i = 1; i < nLines; ++i) {
        lines[i].str = std::strtok(nullptr, "\r\n");

        lines[i].str[-1] = '\0';
        lines[i].len = strlen(lines[i].str);
    }
}
