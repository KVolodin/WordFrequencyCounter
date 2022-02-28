#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/iostreams/device/mapped_file.hpp>

#include <getopt.h>

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
                break;
            }
        }
    }

    return appArgs;
}

Dictionary GetDictionary(const std::string& in, const utils::timer::Timer& timer)
{
    Dictionary dictionary;

#ifdef MAPPED_FILE_BOOST
    boost::iostreams::mapped_file mmap(in, boost::iostreams::mapped_file::readonly);
    if (!mmap.is_open())
    {
        throw std::invalid_argument{"Input file " + in + "not found"};
    }
    auto ch = mmap.const_data();
    std::string str;
    str.reserve(10);
    for (std::size_t i = 0; i < mmap.size(); i++)
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
#else
    std::ifstream fileIn{in, std::ios::binary};
    if (!fileIn.good())
    {
        throw std::invalid_argument{"Input file " + in + "not found"};
    }

    std::string strLine;
    std::string str;
    str.reserve(10);

    while (std::getline(fileIn, strLine))
    {
        for (const auto ch : strLine)
        {
            if (ch >= 0 && ch <= 255 && isalpha(ch) != 0)
            {
                str += std::tolower(ch);
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
    }
#endif // MAPPED_FILE_BOOST
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