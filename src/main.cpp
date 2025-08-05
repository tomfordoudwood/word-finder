#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <string>
#include <filesystem>

#include <optional>

struct Outs {
    size_t row_num;
    std::vector<std::pair<size_t, size_t>> pos_len;
    std::string content;
};

std::vector<Outs> find(const std::filesystem::path& file, const std::vector<std::regex>& r) {

    //std::cout << "file: " << file << std::endl;

    std::vector<Outs> res;

    std::ifstream is { file.c_str() };

    //std::cout << "is: " << is << std::endl;

    std::string s;
    std::smatch sm;
    for (size_t line {1}; getline(is, s); ++line) {

        Outs outs;
        outs.row_num = line;
        outs.content = s;

        for(const auto& re : r) {

            if (regex_search(s, sm, re)) {

                size_t pos = sm.position();
                size_t len = sm.length();

                std::pair<size_t, size_t> pl{pos, len};

                outs.pos_len.push_back(pl);

                //std::cout << "pos: " << pos << " len: " << len << std::endl;

            }

        }

        if(!outs.pos_len.empty()) {
            res.emplace_back(outs);
        }

    }

    return res;
}

int main(int argc, char *argv[]) {

    std::optional<std::filesystem::path> p;

    std::cout << "Input:";
    for(int i=0; i<argc; ++i) {
        std::cout << " " << argv[i];
    }
    std::cout << std::endl;

    if(argc < 2) {

        std::cerr << "No pattern data to search[1]" << std::endl;
        return 1;

    }

    std::map<std::string, std::regex> patterns;

    p = (std::string(argv[1]) == "-p" && argc > 2) ?
                std::filesystem::canonical(argv[2]) :
                std::filesystem::current_path();

    for(int i = p.has_value() ? 3 : 1; i<argc; ++i) {

        std::string pattern { argv[i] };
        std::regex re;

        try { re = std::regex{pattern}; }
        catch (const std::regex_error &e) {
            std::cout << "Invalid regular expression \""+pattern+"\" provided.\n";
            return 1;
        }

        patterns.insert({pattern, re});

    }

    std::cout << "Search";
    for(const auto& [pattern, re] : patterns) {
        std::cout << " \"" << pattern << "\"";
    }
    std::cout << " in " << p.value() << std::endl;

    for (const auto& entry : std::filesystem::recursive_directory_iterator{p.value()}) {

        std::cout << ">>> " << entry.path() << std::endl;;

        std::vector<std::regex> rexs;
        std::transform(std::begin(patterns), std::end(patterns),
                       std::back_inserter(rexs),
                       [](const auto& pair){ return pair.second; });

//        //for(const auto& [pattern, re] : patterns) {

            auto res { find(entry.path(), rexs) };

            std::cout << "SIZE OF res: " << res.size() << std::endl;

            std::string red = "\033[41m";
            std::string rst = "\033[0m";

            for (auto&[number, pos_len_vec, text] : res) {

                std::cout << "--> " << text << std::endl;

                std::sort(std::begin(pos_len_vec), std::end(pos_len_vec),
                          [](const auto& a, const auto& b){ return a.first < b.first; });

                for(const auto& [pos, len] : pos_len_vec) {
                    std::cout << "pos " << pos << " len " << len << std::endl;
                }

                size_t position { 0 };
                //size_t new_position { pos_len_vec.at(0).first };

                std::string colored_res = "";//text.substr(position, new_position);

                for(const auto& [pos, len] : pos_len_vec) {
                    colored_res += text.substr(position, pos-position) +
                            red + text.substr(pos, len) + rst;
                    position = pos + len;
                }
                colored_res += text.substr(position, text.size()-position);

                std::cout << entry << ":" << number
                     << " - " << colored_res << '\n';
            }

        //}


    }

}
