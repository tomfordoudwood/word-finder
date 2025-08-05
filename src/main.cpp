#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <string>
#include <filesystem>

struct Out {
    size_t row_num;
    size_t pos_num;
    size_t length;
    std::string content;
};

std::vector<Out> find(const std::filesystem::path& file, const std::regex& r) {

    //std::cout << "file: " << file << std::endl;

    std::vector<Out> res;

    std::ifstream is { file.c_str() };

    //std::cout << "is: " << is << std::endl;

    std::string s;
    std::smatch sm;
    for (size_t line {1}; getline(is, s); ++line) {

        if (regex_search(s, sm, r)) {

            size_t pos = sm.position();
            size_t len = sm.length();

            //std::cout << "pos: " << pos << " len: " << len << std::endl;

            Out out{line, pos, len, move(s)};
            res.emplace_back(out);
        }
    }

    return res;
}


int main(int argc, char *argv[]) {

    std::filesystem::path p;

    std::string pattern;
    std::regex re;

    std::cout << "Input:";
    for(int i=0; i<argc; ++i) {
        std::cout << " " << argv[i];
    }
    std::cout << std::endl;

    if(argc < 2) {
        std::cerr << "Bad amount of arguments!" << std::endl;
        return 1;
    } else if (argc == 2) {
        p.assign(std::filesystem::current_path());
        pattern.assign(argv[1]);
        re.assign(pattern);

        try { re = std::regex{pattern}; }
        catch (const std::regex_error &e) {
            std::cout << "Invalid regular expression provided.\n";
            return 1;
        }

    } else if (argc == 3) {
        p.assign(std::filesystem::canonical(argv[1]));
        pattern.assign(argv[2]);
        re.assign(pattern);

        try { re = std::regex{pattern}; }
        catch (const std::regex_error &e) {
            std::cout << "Invalid regular expression provided.\n";
            return 1;
        }
    }

    std::cout << "Search \"" << pattern << "\" in " << p << std::endl;

    for (const auto &entry : std::filesystem::recursive_directory_iterator{p}) {

        //cout << "entry.path: " << entry.path() << " \n";

        auto res { find(entry.path(), re) };

        std::string red = "\033[41m";
        std::string rst = "\033[0m";

        for (const auto &[number, pos, len, text] : res) {

            //std::cout << "pos* " << pos << " len* " << len << std::endl;

            std::string colored_res = text.substr(0, pos) +
                    red +
                    text.substr(pos, len) +
                    rst +
                    text.substr(pos+len, text.size()-pos-len);

            std::cout << entry << ":" << number
                 << " - " << colored_res << '\n';
        }
    }

}
