/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   Makefile_am;
    const int            Makefile_amSize = 1096;

    extern const char*   soundtouch_config_h_in;
    const int            soundtouch_config_h_inSize = 188;

    extern const char*   COPYING;
    const int            COPYINGSize = 246;

    extern const char*   COPYING2;
    const int            COPYING2Size = 1774;

    extern const char*   __init___py;
    const int            __init___pySize = 0;

    extern const char*   flatfileparser_py;
    const int            flatfileparser_pySize = 187;

    extern const char*   stockmarketparser_py;
    const int            stockmarketparser_pySize = 2168;

    extern const char*   syntheticdata_py;
    const int            syntheticdata_pySize = 5156;

    extern const char*   README_md;
    const int            README_mdSize = 706;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 9;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
