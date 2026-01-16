#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <cctype>
#include <algorithm>
#include <random>

// Cifra Csar s em ASCII A-Z / a-z.
// UTF-8 (acentos etc.) passa intacto e NO  cifrado.
static inline char caesar_char(char c, int shift) {
    if (c >= 'A' && c <= 'Z') {
        int off = (c - 'A' + shift) % 26;
        if (off < 0) off += 26;
        return static_cast<char>('A' + off);
    }
    if (c >= 'a' && c <= 'z') {
        int off = (c - 'a' + shift) % 26;
        if (off < 0) off += 26;
        return static_cast<char>('a' + off);
    }
    return c;
}

static inline std::string apply_caesar(const std::string &text, int shift) {
    std::string out;
    out.reserve(text.size());
    for (char c : text) out.push_back(caesar_char(c, shift));
    return out;
}

// Frequncias aproximadas de letras em PT (A-Z) em %
static constexpr std::array<double, 26> PT_LETTER_FREQ = {
    14.6, 1.0, 3.9, 4.9, 12.6, 1.0, 1.3, 1.0, 6.2, 0.4, 0.0, 2.8, 4.7,
    5.0, 10.7, 2.5, 1.3, 6.5, 7.9, 4.3, 4.6, 1.7, 0.0, 0.3, 0.0, 0.4
};

static inline std::array<double, 26> count_letters(const std::string &text, double &total_letters) {
    std::array<double, 26> counts{};
    counts.fill(0.0);
    total_letters = 0.0;

    for (unsigned char uc : text) {
        char c = static_cast<char>(uc);
        if (c >= 'A' && c <= 'Z') {
            counts[static_cast<size_t>(c - 'A')] += 1.0;
            total_letters += 1.0;
        } else if (c >= 'a' && c <= 'z') {
            counts[static_cast<size_t>(c - 'a')] += 1.0;
            total_letters += 1.0;
        }
    }
    return counts;
}

// Chi-quadrado: menor => mais parecido com PT
static inline double chi_square_pt(const std::string &text) {
    double total = 0.0;
    auto counts = count_letters(text, total);
    if (total < 1.0) return 1e9;

    double chi2 = 0.0;
    for (size_t i = 0; i < 26; ++i) {
        double expected = PT_LETTER_FREQ[i] * total / 100.0;
        if (expected <= 0.0) continue;
        double diff = counts[i] - expected;
        chi2 += (diff * diff) / expected;
    }
    return chi2;
}

static const std::vector<std::string> COMMON_WORDS = {
    " que ", " de ", " da ", " do ", " das ", " dos ",
    " nao ", " no ", " uma ", " para ", " por ", " com ",
    " mais ", " como ", " esta ", " est ", " voce ", " voc ",
    " isso ", " tudo ", " ser ", " tem ", " foi ", " mas ",
    " em ", " se ", " eu ", " ele ", " ela ", " eles ", " elas "
};

static inline std::string to_lower_ascii(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char uc : s) {
        out.push_back(static_cast<char>(std::tolower(uc)));
    }
    return out;
}

static inline int count_occurrences(const std::string &text, const std::string &pat) {
    if (pat.empty()) return 0;
    int count = 0;
    size_t pos = 0;
    while (true) {
        pos = text.find(pat, pos);
        if (pos == std::string::npos) break;
        ++count;
        ++pos; // permite sobreposio leve
    }
    return count;
}

static inline double semantic_score(const std::string &text) {
    std::string lower = to_lower_ascii(text);
    std::string padded = " " + lower + " ";

    double score = 0.0;

    for (const auto &w : COMMON_WORDS) {
        int c = count_occurrences(padded, w);
        if (c > 0) score += 5.0 * static_cast<double>(c);
    }

    int spaces = static_cast<int>(std::count(text.begin(), text.end(), ' '));
    score += static_cast<double>(spaces) * 0.1;

    int weird = 0;
    for (unsigned char ch : text) {
        if (ch < 0x09 || (ch >= 0x0E && ch < 0x20)) ++weird;
    }
    score -= static_cast<double>(weird) * 10.0;

    return score;
}

// Score final: maior => melhor (mais "portugus")
static inline double score_candidate(const std::string &text) {
    double chi2 = chi_square_pt(text);
    double sem = semantic_score(text);

    // chi2 baixo -> bom. Transformamos em score alto.
    double chi_score = -chi2 * 0.5;
    return chi_score + sem;
}

struct CrackResult {
    int best_shift = 0;
    double best_score = -1e300;
    std::string best_plaintext;
};

static inline CrackResult crack_caesar_pt(const std::string &ciphertext) {
    CrackResult r;
    for (int shift = 0; shift < 26; ++shift) {
        std::string candidate = apply_caesar(ciphertext, -shift);
        double s = score_candidate(candidate);
        if (s > r.best_score) {
            r.best_score = s;
            r.best_shift = shift;
            r.best_plaintext = std::move(candidate);
        }
    }
    return r;
}

static void print_usage(const char *prog) {
    std::cerr
        << "Uso:\n"
        << "  " << prog << " --encrypt SHIFT       < texto em stdin >\n"
        << "  " << prog << " --decrypt SHIFT       < texto em stdin >\n"
        << "  " << prog << " --encrypt-random      < texto em stdin >\n"
        << "  " << prog << " --crack               < texto em stdin >\n\n"
        << "Notas:\n"
        << "  - So cifra letras ASCII (A-Z/a-z). Acentos/UTF-8 passam intactos.\n"
        << "  - --encrypt-random NAO revela o shift (caixa-preta total).\n\n"
        << "Exemplos:\n"
        << "  echo \"O importante nao e vencer\" | " << prog << " --encrypt 7\n"
        << "  echo \"...\" | " << prog << " --encrypt-random\n"
        << "  echo \"...\" | " << prog << " --crack\n";
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const std::string mode = argv[1];
    int shift = 0;

    if (mode == "--encrypt" || mode == "--decrypt") {
        if (argc < 3) {
            std::cerr << "Erro: faltou o SHIFT.\n";
            print_usage(argv[0]);
            return 1;
        }
        try {
            shift = std::stoi(argv[2]);
        } catch (...) {
            std::cerr << "Erro: SHIFT invalido.\n";
            return 1;
        }
        // Normaliza shifts grandes
        shift %= 26;
        if (shift < 0) shift += 26;
    } else if (mode == "--encrypt-random" || mode == "--crack") {
        // ok
    } else {
        std::cerr << "Modo desconhecido: " << mode << "\n";
        print_usage(argv[0]);
        return 1;
    }

    // L tudo de stdin
    const std::string input(
        (std::istreambuf_iterator<char>(std::cin)),
        std::istreambuf_iterator<char>()
    );

    if (mode == "--encrypt") {
        std::cout << apply_caesar(input, shift);
        return 0;
    }

    if (mode == "--decrypt") {
        std::cout << apply_caesar(input, -shift);
        return 0;
    }

    if (mode == "--encrypt-random") {
        // CAIXA-PRETA: no imprime shift em lugar nenhum.
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 25);
        int rshift = dist(gen);
        std::cout << apply_caesar(input, rshift);
        return 0;
    }

    // --crack
    {
        CrackResult r = crack_caesar_pt(input);
        std::cout << "Melhor shift estimado: " << r.best_shift << "\n";
        std::cout << "Score: " << r.best_score << "\n";
        std::cout << "Texto decifrado:\n";
        std::cout << r.best_plaintext;
    }

    return 0;
}
