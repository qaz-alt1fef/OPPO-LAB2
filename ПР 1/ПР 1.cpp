#include <iostream>
#include <clocale>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <map>
#include <functional>
#include <regex>
#include <stdexcept>

class CurrencyRate {
public:
    CurrencyRate() = delete;

    CurrencyRate(const std::string& line) : CurrencyRate(std::move(parse(line))) {}  // дделегирующий конструктор. парсит строку и передает данные основному конструктору

    CurrencyRate(std::string n1, std::string n2, double r, std::chrono::year_month_day d)    // основной конструктор инициализации
        : name1(n1), name2(n2), rate(r), date(d) {
    }

    double getRate() const { return rate; }       // геттеры для инкапсулированных данных
    std::chrono::year_month_day getDate() const { return date; }

    friend std::ostream& operator<<(std::ostream& os, const CurrencyRate& p) {    // дружественная функция -- потоковый вывод
        return os << p.name1 << " / " << p.name2 << " | курс: " << p.rate
            << " | дата: " << (int)p.date.year() << "." << (unsigned)p.date.month() << "." << (unsigned)p.date.day();
    }

private:
    std::string name1, name2;
    double rate;
    std::chrono::year_month_day date;

    const static CurrencyRate parse(const std::string& s) {
        static const std::regex re("^\\s*\"([^\"]*)\"\\s+\"([^\"]*)\"\\s+(\\S+)\\s+(\\S+)\\s*$");

        std::smatch m;

        if (!std::regex_match(s, m, re)) throw std::runtime_error("не хватает данных или неверный формат");

        std::string n1 = m[1], n2 = m[2], t1 = m[3], t2 = m[4];
        static const std::regex dRe(R"(^(\d{4})\.(\d{1,2})\.(\d{1,2})$)");
        std::smatch dm;

        std::string dStr = std::regex_match(t1, dm, dRe) ? t1 : (std::regex_match(t2, dm, dRe) ? t2 : "");    // тернарные операторы. Определяют порядок токенов (дата/курс или курс/дата)
        if (dStr.empty()) throw std::runtime_error("не хватает данных (отсутствует дата формата гггг.мм.дд)");
        std::string rStr = (dStr == t1) ? t2 : t1;

        std::chrono::year_month_day ymd{
            std::chrono::year{std::stoi(dm[1])},
            std::chrono::month{static_cast<unsigned>(std::stoi(dm[2]))},
            std::chrono::day{static_cast<unsigned>(std::stoi(dm[3]))}
        };
        if (!ymd.ok()) throw std::runtime_error("несуществующая дата: " + dStr);

        try {
            size_t pos;
            double r = std::stod(rStr, &pos);
            if (pos != rStr.size()) throw std::runtime_error("");
            return { n1, n2, r, ymd };
        }
        catch (...) { throw std::runtime_error("не хватает данных (курс не является числом): " + rStr); }
    }
};

bool askSort(const std::string& criterion) {     // функция ввода порядка сортировки
    std::string s;
    while (std::cout << "Сортировка по " << criterion << " (1 - по убыванию, 2 - по возрастанию): \n Выбор: " && std::getline(std::cin, s)) {
        if (s == "1") return true;
        if (s == "2") return false;
        std::cout << "Неверный выбор. Пожалуйстаа, повторите.\n";
    }
    return true;
}

int main() {
    std::setlocale(LC_CTYPE, "Rus");
    std::ifstream file("in.txt");
    if (!file) return std::cout << "Ошибка: Файл не найден\n", 1;

    std::vector<CurrencyRate> vect;
    std::string line;
    for (int lineNum = 1; std::getline(file, line); ++lineNum) {
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue; //пропуск пустых строк
        try { vect.emplace_back(line); }
        catch (const std::exception& e) { std::cout << "Строка " << lineNum << ": " << e.what() << "\n"; } //вывод ошибок
    }
    if (vect.empty()) return std::cout << "Нет корректных записей для обработки.\n", 0;

    struct MenuItem {        //меню
        std::string title;
        std::function<void(std::vector<CurrencyRate>&)> action;
    };

    std::map<int, MenuItem> menu = {
        {1, {"курсу", [](std::vector<CurrencyRate>& v) {
            bool desc = askSort("курсу");
            std::sort(v.begin(), v.end(), [desc](const auto& a, const auto& b) {
                return desc ? a.getRate() > b.getRate() : a.getRate() < b.getRate();
            });
        }}},
        {2, {"дате", [](std::vector<CurrencyRate>& v) {
            bool desc = askSort("дате");
            std::sort(v.begin(), v.end(), [desc](const auto& a, const auto& b) {
                return desc ? b.getDate() < a.getDate() : a.getDate() < b.getDate();
            });
        }}}
    };

    std::cout << "\nВыберете режим сортировки:\n";
    for (const auto& [id, item] : menu) std::cout << " " << id << " - Сортировка по " << item.title << "\n";

    int choice = 0;
    while (std::cout << " Выбор: " && std::getline(std::cin, line)) {
        try { choice = std::stoi(line); if (menu.count(choice)) break; }
        catch (...) {}
        std::cout << "Неверный выбор. Пожалуйста, повторите.\n";
    }

    menu[choice].action(vect);     // вызов выбранного действия меню

    std::cout << "\n----------------- Результаты -----------------\n";
    for (const auto& item : vect) std::cout << item << "\n";
}
