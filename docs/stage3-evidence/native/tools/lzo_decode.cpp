//20261110-8 add: Decode captured native LZO only; no renderer or compiler is linked.
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include "lzokay.hpp"

using u8 = unsigned char; //20261110-8 add: Keep captured pixel bytes in the project byte type.

/******************************************************************************
 * Function Name   : main
 * Description     : Decode one bounded captured LZO stream into exact raw bytes.
 * Input Parameters: input filename, output filename, decompressed byte length.
 * Return Value    : Zero only for a valid exact-length decompression.
 * Date            : 2026.11.10
 ******************************************************************************/
int main(int argc, char **argv) //20261110-8 add
{
    std::ifstream input;
    std::ofstream output;
    std::vector<u8> compressed;
    std::vector<u8> decoded;
    std::size_t actual = 0;
    std::size_t size = 0;
    lzokay::EResult result;

    if (argc != 4) { return 2; }
    size = std::stoull(argv[3]);
    if (size == 0 || size > 768000) { return 3; }
    input.open(argv[1], std::ios::binary);
    if (!input) { return 4; }
    compressed.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    decoded.resize(size);
    result = lzokay::decompress(compressed.data(), compressed.size(), decoded.data(), decoded.size(), actual);
    if (result != lzokay::EResult::Success || actual != size) { return 5; }
    output.open(argv[2], std::ios::binary);
    output.write(reinterpret_cast<const char *>(decoded.data()), decoded.size());
    return output.good() ? 0 : 6;
}
