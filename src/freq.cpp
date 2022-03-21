#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <fcntl.h>
#include <getopt.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <timer.h>

namespace {

using Dictionary = std::unordered_map<std::string, int>;

struct AppArgs
{
    std::string in;
    std::string out;
};

AppArgs ParseCommanLineArguments(int argc, char* argv[])
{
    AppArgs appArgs{};
    int res{};
    const option options[] = {
        {"in",  required_argument, NULL, 'i'},
        {"out", required_argument, NULL, 'o'},
        {NULL, 0, NULL, 0}
    };

    while ((res = getopt_long(argc, argv, "i:o:", options, NULL)) != -1)
    {
        switch (res)
        {
            case 'i':
            {
                appArgs.in = optarg;
                break;
            }
            case 'o':
            {
                appArgs.out = optarg;
                break;
            }
            default:
            {
                throw std::invalid_argument{"Invalid command line argument"};
            }
        }
    }

    return appArgs;
}

char* MmapFile(const std::string& in, std::size_t& size)
{
    int fdin{0};
    if (fdin = open(in.c_str(), O_RDONLY); fdin < 0)
    {
        throw std::invalid_argument{"Unable to open " + in + " for reading"};
    }
    struct stat statbuf{};
    if (fstat(fdin, &statbuf) < 0)
    {
        throw std::invalid_argument{"Fstat error"};
    }
    void* src{nullptr};
    if ((src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0)) == MAP_FAILED)
    {
        throw std::invalid_argument{"Error in mmap function"};
    }
    size = statbuf.st_size;

    return static_cast<char*>(src);
}

Dictionary GetDictionary(const std::string& in, const utils::timer::Timer& timer)
{
    Dictionary dictionary;
    std::size_t size{};
    auto ch = MmapFile(in, size);
    std::string str;
    str.reserve(10);
    for (std::size_t i = 0; i < size; i++)
    {
        if (ch[i] >= 0 && ch[i] <= 255 && isalpha(ch[i]) != 0)
        {
            str += std::tolower(ch[i]);
        }
        else
        {
            if (!str.empty())
            {
                dictionary[str] += 1;
            }
            str.clear();
        }
    }

    std::cout << "Fill: " << timer.Elapsed() << std::endl;
    return dictionary;
}

std::vector<std::pair<std::string, int>> SortMap(Dictionary& dictionaryMap, const utils::timer::Timer& timer)
{
    std::vector<std::pair<std::string, int>> dictionaryVec;
    dictionaryVec.reserve(dictionaryMap.size());

    for (auto& it : dictionaryMap)
    {
        dictionaryVec.push_back(std::move(it));
    }

    sort(
        dictionaryVec.begin(), dictionaryVec.end(), [](std::pair<std::string, int>& a, std::pair<std::string, int>& b) {
            return a.second == b.second ? a.first < b.first : a.second > b.second;
        });
    std::cout << "Sort: " << timer.Elapsed() << std::endl;

    return dictionaryVec;
}

} // namespace

int main(int argc, char* argv[]) try
{
    utils::timer::Timer timer{};
    auto appArgs = ParseCommanLineArguments(argc, argv);

    std::ofstream fileOut{appArgs.out, std::ios::out | std::ios::binary | std::ios::trunc};
    auto dictionaryMap = GetDictionary(appArgs.in, timer);
    for (auto& [key, value] : SortMap(dictionaryMap, timer))
    {
        fileOut << value << " " << key << std::endl;
    }
    std::cout << "Time elapsed: " << timer.Elapsed() << std::endl;

    return EXIT_SUCCESS;
}
catch (std::exception& ex)
{
    std::cerr << "*** Caught unidentified exception\n"
        << ex.what()
        << '\n';
    std::exit(EXIT_FAILURE);
}
catch (...)
{
    std::cerr << "*** Caught unknown exception\n";
    std::exit(EXIT_FAILURE);
}