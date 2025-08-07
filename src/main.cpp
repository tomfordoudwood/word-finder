#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <string>
#include <filesystem>
#include <optional>

/**
 * @brief The Inclusion struct - информация о включении паттерна в текст
 *                               (используется при раскраске текста)
 *
 * pos - позиция начального символа подстроки в строке
 * len - длинна подстроки
 * id - идентификатор паттерна
 */
struct Inclusion {
    size_t pos;
    size_t len;
    size_t id;
};

/**
 * @brief The LineData struct - информация о строке текста
 *
 * num - номер строки в файле
 * inclusions - информация о включениях паттернов
 * content - содержимое строки
 */
struct LineData {
    size_t num;
    std::vector<Inclusion> inclusions;
    std::string content;
};

std::vector<LineData> find(const std::filesystem::path& file, const std::vector<std::regex>& patterns) {

    std::vector<LineData> res;

    std::ifstream is { file.c_str() };

    std::string s;
    std::smatch sm;
    for (size_t line {1}; getline(is, s); ++line) {

        LineData lineData{ line, std::vector<Inclusion>(), s};

        unsigned int patternId{ 0 };
        for(const auto& pattern : patterns) {
            patternId++;
            if (regex_search(s, sm, pattern)) {

                size_t pos = sm.position();
                size_t len = sm.length();

                Inclusion inc{ pos, len, patternId };

                lineData.inclusions.push_back(inc);
            }
        }

        if(!lineData.inclusions.empty()) {
            res.emplace_back(lineData);
        }
    }

    return res;
}

std::string colorize(std::vector<Inclusion> incs, std::string text) {

    std::sort(std::begin(incs), std::end(incs),
              [](const auto& a, const auto& b){ return a.pos < b.pos; });

    size_t position { 0 };

    std::string colorized_text = "";

    for(const auto& [pos, len, id] : incs) {

        colorized_text +=
                text.substr(position, pos-position) +
                "\033[1;3"+std::to_string(id%5)+"m" +
                text.substr(pos, len) +
                "\033[0m";

        position = pos + len;
    }

    colorized_text += text.substr(position, text.size()-position);

    return colorized_text;
}

int main(int argc, char *argv[]) {

    if(argc < 2) {

        std::cerr << "No pattern data to search" << std::endl;
        return 1;

    }

    std::optional<std::filesystem::path> path {
        (std::string(argv[1]) == "-p" && argc > 2) ?
                                 std::filesystem::canonical(argv[2]) :
                                 std::filesystem::current_path() };

    std::vector<std::regex> patterns;

    for(int i = path.has_value() ? 3 : 1; i<argc; ++i) {

        std::regex re;

        try { re = std::regex{argv[i]}; }
        catch (const std::regex_error &e) {
            std::cout << "Invalid regular expression \""+std::string(argv[i])+"\" provided.\n";
            return 1;
        }

        patterns.push_back({re});

    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator{ path.value() }) {

        auto res { find(entry.path(), patterns) };

        for (auto&[number, incs, text] : res) {

            auto colorized_text { colorize(incs, text) };

            std::cout << std::filesystem::path(entry).string() << ":" << number
                      << " - " << colorized_text << std::endl;
        }

    }

}
