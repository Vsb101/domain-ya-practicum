#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <string_view>
#include <vector>
#include <set>
#include <cassert>

using namespace std;

// Класс Domain представляет доменное имя.
// Внутри хранит обратный порядок частей (например, "a.b.com" → "com.b.a"),
// чтобы легко проверять, является ли один домен суффиксом другого (через префикс в обратной форме).
class Domain {
public:
    explicit Domain(const string& domain)
        : reversed_domain_(ReverseDomain(domain)) {}

    bool operator==(const Domain& other) const {
        return reversed_domain_ == other.reversed_domain_;
    }

    bool operator<(const Domain& other) const {
        return reversed_domain_ < other.reversed_domain_;
    }

    const string& GetReversed() const {
        return reversed_domain_;
    }

private:
    // Разбивает домен на части по точкам, затем собирает их в обратном порядке.
    // Например: "math.gdz.ru" → части: ["math", "gdz", "ru"] → результат: "ru.gdz.math".
    // Используется для сравнения суффиксов через лексикографический порядок.
    static string ReverseDomain(const string& domain) {
        vector<string> parts;
        stringstream ss(domain);
        string part;

        // getline(ss, part, '.')
        // Читает символы из ss пока не встретит '.' или конец строки.
        // Считанная часть (без точки!) записывается в part.
        while (getline(ss, part, '.')) {
            parts.push_back(part);
        }

        string result;
        for (auto it = parts.rbegin(); it != parts.rend(); ++it) {
            if (it != parts.rbegin()) result += '.';
            result += *it;
        }
        return result;
    }

    string reversed_domain_;
};

// Проверяет, запрещён ли домен или его супердомен.
// Хранит множество обращённых запрещённых доменов.
// При проверке собирает все возможные суффиксы домена (в обратной форме)
// и ищет, есть ли среди них запрещённый.
class DomainChecker {
public:
    template <typename Iterator>
    // Конструктор: заполняет множество обращённых доменов из диапазона [begin, end).
    // Использует GetReversed() для получения ключа.
    // Быстро проверяет поддомены за счёт поиска в set.
    /*DomainChecker(Iterator begin, Iterator end) {
        for (const auto& domain : ranges::subrange(begin, end)) {
            forbidden_reversed_.insert(domain.GetReversed());
        }
    }*/
    /* без range и с++20*/
    DomainChecker(Iterator begin, Iterator end) {
        while (begin != end) {
            forbidden_reversed_.insert((begin++)->GetReversed());
        }
    }


    // Проверяет, является ли домен или любой его супердомен запрещённым.
    // Собирает суффиксы домена по частям (в обратной записи) и ищет их в множестве.
    // Например: для "ru.gdz.math" проверяет "ru", "ru.gdz", "ru.gdz.math".
    bool IsForbidden(const Domain& domain) const {
        const string& rev = domain.GetReversed();
        stringstream ss(rev);
        string part;
        string suffix;

        // getline(ss, part, '.')
        // Читает символы из ss пока не встретит '.' или конец строки.
        // Считанная часть (без точки!) записывается в part.
        while (getline(ss, part, '.')) {
            if (!suffix.empty()) { suffix += '.'; } 
            suffix += part;
            if (forbidden_reversed_.count(suffix)) { return true; }
        }
        return false;
    }

private:
    set<string> forbidden_reversed_;
};

namespace {

// Читает из потока указанное количество доменов (по одному на строке).
// Создаёт объекты Domain и возвращает вектор.
// Используется для чтения как запрещённых, так и проверяемых доменов.
vector<Domain> ReadDomains(istream& input, size_t count) {
    vector<Domain> domains;
    domains.reserve(count);
    string line;
    for (size_t i = 0; i < count; ++i) {
        getline(input, line);
        // Удаляем \r, если есть (для Windows)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        domains.emplace_back(line);
    }
    return domains;
}

// Читает число с отдельной строки.
// Использует stringstream для преобразования строки в число любого типа (size_t, int и т.д.).
// Применяется для чтения количества доменов.
template <typename Number>
Number ReadNumberOnLine(istream& input) {
    string line;
    getline(input, line);

    Number num;
    std::istringstream(line) >> num;

    return num;
}

// Тесты
void RunTests() {
    // Тест 1: Конструктор и GetReversed
    {
        Domain d("math.gdz.ru");
        assert(d.GetReversed() == "ru.gdz.math");

        Domain d2("com");
        assert(d2.GetReversed() == "com");

        Domain d3("a.b.c.com");
        assert(d3.GetReversed() == "com.c.b.a");
    }

    // Тест 2: operator== работает правильно
    {
        Domain d1("math.gdz.ru");
        Domain d2("math.gdz.ru");
        Domain d3("history.gdz.ru");

        assert(d1 == d2);           // одинаковые строки → равны
        assert(!(d1 == d3));        // разные → не равны
        assert(!(d2 == d3));
    }

    // Тест 4: ReadDomains правильно читает домены
    {
        stringstream input("site.com\nsub.example.net\nfinal.test\n");
        auto domains = ReadDomains(input, 3);

        assert(domains.size() == 3);
        assert(domains[0].GetReversed() == "com.site");
        assert(domains[1].GetReversed() == "net.example.sub");
        assert(domains[2].GetReversed() == "test.final");
    }

    // Тест 5: IsForbidden — простые случаи
    {
        vector<Domain> forbidden = {
            Domain("gdz.ru"),
            Domain("maps.me"),
            Domain("com")
        };
        DomainChecker checker(forbidden.begin(), forbidden.end());

        assert(checker.IsForbidden(Domain("gdz.ru")) == true);
        assert(checker.IsForbidden(Domain("math.gdz.ru")) == true);
        assert(checker.IsForbidden(Domain("history.gdz.ru")) == true);
        assert(checker.IsForbidden(Domain("freegdz.ru")) == false);
        assert(checker.IsForbidden(Domain("gdz.com")) == true);
        assert(checker.IsForbidden(Domain("m.maps.me")) == true);
        assert(checker.IsForbidden(Domain("maps.org")) == false);
        assert(checker.IsForbidden(Domain("xyz.maps.me")) == true);
    }

    // Тест 6: короткие суффиксы
    {
        vector<Domain> forbidden = { Domain("com") };
        DomainChecker checker(forbidden.begin(), forbidden.end());

        assert(checker.IsForbidden(Domain("a.com")) == true);
        assert(checker.IsForbidden(Domain("b.a.com")) == true);
        assert(checker.IsForbidden(Domain("ru")) == false);
        assert(checker.IsForbidden(Domain("com")) == true);
    }

    // Тест 7: запрещён поддомен, но не супердомен
    {
        vector<Domain> forbidden = { Domain("m.gdz.ru") };
        DomainChecker checker(forbidden.begin(), forbidden.end());

        assert(checker.IsForbidden(Domain("m.gdz.ru")) == true);
        assert(checker.IsForbidden(Domain("math.m.gdz.ru")) == true);
        assert(checker.IsForbidden(Domain("gdz.ru")) == false);
        assert(checker.IsForbidden(Domain("a.gdz.ru")) == false);
    }

    // Тест 8: пустой список запрещённых
    {
        vector<Domain> forbidden;
        DomainChecker checker(forbidden.begin(), forbidden.end());

        assert(checker.IsForbidden(Domain("any.com")) == false);
        assert(checker.IsForbidden(Domain("com")) == false);
    }

    // Тест 9: один символ
    {
        vector<Domain> forbidden = { Domain("a") };
        DomainChecker checker(forbidden.begin(), forbidden.end());

        assert(checker.IsForbidden(Domain("a")) == true);
        assert(checker.IsForbidden(Domain("b.a")) == true);
        assert(checker.IsForbidden(Domain("ab")) == false);
        assert(checker.IsForbidden(Domain("a.b")) == false);
    }

    cerr << "All tests passed!" << endl;
}

} // namespace

int main() {
    RunTests();

    // 1. Читает число N и N запрещённых доменов.
    // 2. Создаёт DomainChecker с этими доменами.
    // 3. Читает число M и M проверяемых доменов.
    // 4. Для каждого выводит "Bad", если запрещён (или его супердомен), иначе "Good".
    
    const std::vector<Domain> forbidden_domains = ReadDomains(cin, ReadNumberOnLine<size_t>(cin));
    DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());

    const std::vector<Domain> test_domains = ReadDomains(cin, ReadNumberOnLine<size_t>(cin));
    for (const Domain& domain : test_domains) {
        cout << (checker.IsForbidden(domain) ? "Bad"sv : "Good"sv) << endl;
    }
}
