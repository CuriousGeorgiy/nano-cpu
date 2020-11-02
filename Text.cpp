#include "Text.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <Windows.h>

Text::Text(const char *inputFileName)
: buf(nullptr), lines(nullptr), inputFileName(inputFileName), nLines(0)
{
    assert(inputFileName != nullptr);

    readTextFileToBuf();
    countLinesInBuf();
    splitBufIntoLines();
}

Text::~Text()
{
    std::free(lines);
    std::free(buf);
}

void Text::readTextFileToBuf()
{
    HANDLE inputFileHandle = CreateFileA(inputFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL);
    DWORD inputFileSize = GetFileSize(inputFileHandle, NULL);
    HANDLE inputFileMappingHandle = CreateFileMappingA(inputFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
    LPVOID inputFileMapView = MapViewOfFile(inputFileMappingHandle, FILE_MAP_READ, 0, 0, 0);

    buf = (char *) std::calloc(inputFileSize + 2, sizeof(char));
    std::memcpy(buf + 1, inputFileMapView, inputFileSize);

    UnmapViewOfFile(inputFileMapView);
    CloseHandle(inputFileMappingHandle);
    CloseHandle(inputFileHandle);
}

void Text::countLinesInBuf()
{
    int prePreChar = EOF;
    int preChar = EOF;

    const char *reader = buf + 1;

    while (*reader) {
        if ((*reader == '\n') && (preChar == '\r') && (prePreChar != '\n') && (prePreChar != EOF)) ++nLines;

        prePreChar = preChar;
        preChar = (unsigned char) *(reader++);
    }

    if ((preChar != '\n') && (preChar != EOF) && (prePreChar != '\r')) ++nLines;
}

void Text::splitBufIntoLines()
{
    lines = (Line *) std::calloc(nLines + 1, sizeof(Text::Line));

    lines[0].str = std::strtok(buf + 1, "\r\n");

    lines[0].str[-1] = '\0';
    lines[0].len = strlen(lines[0].str);

    for (size_t i = 1; i < nLines; ++i) {
        lines[i].str = std::strtok(nullptr, "\r\n");

        lines[i].str[-1] = '\0';
        lines[i].len = strlen(lines[i].str);
    }
}
