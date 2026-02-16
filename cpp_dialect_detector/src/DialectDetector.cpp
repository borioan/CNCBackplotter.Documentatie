#include "DialectDetector.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <sstream>

namespace cnc {
namespace {

std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return s;
}

std::string firstLines(const std::string &text, std::size_t maxLines) {
    std::istringstream in(text);
    std::ostringstream out;
    std::string line;
    std::size_t count = 0;

    while (count < maxLines && std::getline(in, line)) {
        out << line << '\n';
        ++count;
    }
    return out.str();
}

int regexCount(const std::string &text, const std::regex &re) {
    int count = 0;
    auto begin = std::sregex_iterator(text.begin(), text.end(), re);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        ++count;
    }
    return count;
}

bool regexAny(const std::string &text, const std::regex &re) {
    return std::regex_search(text, re);
}

std::unordered_map<std::string, std::regex> featureRegex() {
    return {
        {"O_program", std::regex(R"((^|\n)\s*O\d+\b)")},
        {"block_numbers_N", std::regex(R"((^|\n)\s*N\d+\b)")},
        {"tool_change_M06", std::regex(R"(\bM0?6\b)")},
        {"length_comp_H", std::regex(R"(\bG43\b[^\n]*\bH\d+\b)")},
        {"M30", std::regex(R"(\bM30\b)")},
        {"G91_1", std::regex(R"(\bG91\.1\b)")},
        {"ijk_full_arcs", std::regex(R"(\bG0?[23]\b[^\n]*\bI[-+]?\d*\.?\d+[^\n]*\bJ[-+]?\d*\.?\d+)")},
        {"gcode_concatenation", std::regex(R"(\bG\d+G\d+)")},
        {"G150", std::regex(R"(\bG150\b)")},
        {"G63", std::regex(R"(\bG63\b)")},
        {"generic_gm_presence", std::regex(R"(\bG\d+\b.*\bM\d+\b|\bM\d+\b.*\bG\d+\b)")}
    };
}

} // namespace

std::string TextNormalizer::stripParenthesisComments(const std::string &line) {
    std::string out;
    int depth = 0;
    for (char c : line) {
        if (c == '(') {
            ++depth;
            continue;
        }
        if (c == ')' && depth > 0) {
            --depth;
            continue;
        }
        if (depth == 0) {
            out.push_back(c);
        }
    }
    return out;
}

std::string TextNormalizer::stripSemicolonComment(const std::string &line) {
    std::size_t pos = line.find(';');
    if (pos == std::string::npos) {
        return line;
    }
    return line.substr(0, pos);
}

std::string TextNormalizer::separateConcatenatedTokens(const std::string &line) {
    std::string out;
    out.reserve(line.size() + 16);

    auto isLetterToken = [](char c) {
        return c == 'G' || c == 'M' || c == 'X' || c == 'Y' || c == 'Z' || c == 'I' || c == 'J' || c == 'K' || c == 'F' ||
               c == 'S' || c == 'T' || c == 'P' || c == 'R' || c == 'H';
    };

    char prev = '\0';
    for (char c : line) {
        if (isLetterToken(c) && (std::isdigit(static_cast<unsigned char>(prev)) || prev == '.')) {
            out.push_back(' ');
        }
        out.push_back(c);
        prev = c;
    }
    return out;
}

std::string TextNormalizer::normalize(const std::string &raw) {
    std::istringstream in(raw);
    std::ostringstream out;
    std::string line;

    while (std::getline(in, line)) {
        line = toUpper(line);
        line = stripParenthesisComments(line);
        line = stripSemicolonComment(line);
        line = separateConcatenatedTokens(line);

        if (!line.empty()) {
            out << line << '\n';
        }
    }

    return out.str();
}

std::vector<std::string> TextNormalizer::splitLines(const std::string &text) {
    std::istringstream in(text);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::unordered_map<std::string, double> FeatureExtractor::extract(const std::string &normalizedText) {
    const auto fmap = featureRegex();
    std::unordered_map<std::string, double> features;

    for (const auto &[name, regex] : fmap) {
        features[name] = static_cast<double>(regexCount(normalizedText, regex));
    }

    features["line_count"] = static_cast<double>(TextNormalizer::splitLines(normalizedText).size());
    return features;
}

std::vector<DialectRuleSet> RuleRepository::defaultRules() {
    return {
        {Dialect::Heidenhain,
         {std::regex(R"((^|\n)\s*\d+\s+BEGIN\s+PGM\b)"), std::regex(R"(\bTOOL\s+CALL\b)"),
          std::regex(R"(\bCYCL\s+DEF\b)")},
         {}},
        {Dialect::Isel,
         {std::regex(R"(\bFASTABS\b)"), std::regex(R"(\bMOVEABS\b)"), std::regex(R"(\bPROGEND\b)")},
         {}},
        {Dialect::Woodwop,
         {std::regex(R"((^|\n)\[H\s*(\n|$))"), std::regex(R"((^|\n)\[\d{3}\s*(\n|$))"),
          std::regex(R"((^|\n)\$E\d+\s*(\n|$))")},
         {}},
        {Dialect::BiesseCix,
         {std::regex(R"((^|\n)BEGIN\s+MAINDATA\b)"), std::regex(R"((^|\n)BEGIN\s+MACRO\b)"),
          std::regex(R"((^|\n)END\s+MACRO\b)")},
         {}},
        {Dialect::BusselattoEvolution,
         {std::regex(R"((^|\n)SIDE#\d+\{)"), std::regex(R"((^|\n)W#\d+\{)")},
         {}},
        {Dialect::Fanuc,
         {},
         {{"O_program", 2.0}, {"block_numbers_N", 1.0}, {"tool_change_M06", 1.5}, {"length_comp_H", 1.5}, {"M30", 1.0}}},
        {Dialect::Mach3,
         {},
         {{"G91_1", 2.5}, {"ijk_full_arcs", 1.0}, {"M30", 0.5}, {"generic_gm_presence", 0.25}}},
        {Dialect::BusselattoGenesis,
         {},
         {{"gcode_concatenation", 2.0}, {"G150", 2.0}, {"G63", 1.0}, {"generic_gm_presence", 0.5}}},
    };
}

DialectDetector::DialectDetector(std::vector<DialectRuleSet> rules, std::size_t maxLinesToScan)
    : rules_(std::move(rules)), maxLinesToScan_(maxLinesToScan) {}

void DialectDetector::setThresholds(double minScore, double ambiguityDelta) {
    minScore_ = minScore;
    ambiguityDelta_ = ambiguityDelta;
}

std::optional<DetectionResult> DialectDetector::tryHardSignatures(const std::string &normalizedText) const {
    std::vector<DetectionResult> matches;

    for (const auto &rule : rules_) {
        if (rule.hardSignatures.empty()) {
            continue;
        }

        int hitCount = 0;
        std::vector<Evidence> ev;
        for (const auto &sig : rule.hardSignatures) {
            if (regexAny(normalizedText, sig)) {
                ++hitCount;
                ev.push_back({"hard_signature", 1.0, "matched hard signature"});
            }
        }

        if (hitCount >= 2 || (hitCount == 1 && rule.hardSignatures.size() == 1)) {
            DetectionResult result;
            result.dialect = rule.dialect;
            result.family = mapFamily(rule.dialect);
            result.confidence = std::min(1.0, 0.7 + 0.15 * hitCount);
            result.evidence = std::move(ev);
            result.topCandidates.push_back({rule.dialect, static_cast<double>(hitCount)});
            matches.push_back(std::move(result));
        }
    }

    if (matches.empty()) {
        return std::nullopt;
    }

    auto best = std::max_element(matches.begin(), matches.end(), [](const auto &a, const auto &b) { return a.confidence < b.confidence; });
    return *best;
}

DetectionResult DialectDetector::scoreBasedDetection(const std::string &normalizedText) const {
    const auto features = FeatureExtractor::extract(normalizedText);

    std::vector<std::pair<Dialect, double>> scores;
    std::unordered_map<Dialect, std::vector<Evidence>> evidenceMap;

    for (const auto &rule : rules_) {
        if (rule.weightedFeatures.empty()) {
            continue;
        }

        double score = 0.0;
        for (const auto &[featureName, weight] : rule.weightedFeatures) {
            auto fit = features.find(featureName);
            if (fit == features.end()) {
                continue;
            }

            if (fit->second > 0.0) {
                const double contribution = weight * fit->second;
                score += contribution;
                evidenceMap[rule.dialect].push_back({featureName, contribution, "feature hit count=" + std::to_string(fit->second)});
            }
        }

        scores.push_back({rule.dialect, score});
    }

    std::sort(scores.begin(), scores.end(), [](const auto &a, const auto &b) { return a.second > b.second; });

    DetectionResult out;
    out.topCandidates = scores;

    if (scores.empty() || scores.front().second < minScore_) {
        out.dialect = Dialect::Unknown;
        out.family = Family::Unknown;
        out.confidence = 0.0;
        out.evidence.push_back({"min_score", 0.0, "below threshold"});
        return out;
    }

    const auto best = scores.front();
    const double second = scores.size() > 1 ? scores[1].second : 0.0;

    if ((best.second - second) < ambiguityDelta_) {
        out.dialect = Dialect::Ambiguous;
        out.family = Family::Unknown;
        out.confidence = 0.4;
        out.evidence.push_back({"ambiguous", best.second - second, "top scores too close"});
        return out;
    }

    out.dialect = best.first;
    out.family = mapFamily(best.first);
    out.confidence = std::min(0.99, 0.5 + (best.second / (best.second + second + 1.0)));
    out.evidence = evidenceMap[best.first];
    return out;
}

DetectionResult DialectDetector::detectText(const std::string &text) const {
    const auto limited = firstLines(text, maxLinesToScan_);
    const auto normalized = TextNormalizer::normalize(limited);

    if (auto hard = tryHardSignatures(normalized); hard.has_value()) {
        return *hard;
    }

    return scoreBasedDetection(normalized);
}

DetectionResult DialectDetector::detectFile(const std::string &path) const {
    std::ifstream in(path);
    if (!in.is_open()) {
        DetectionResult err;
        err.dialect = Dialect::Unknown;
        err.family = Family::Unknown;
        err.confidence = 0.0;
        err.evidence.push_back({"file_open_error", 0.0, path});
        return err;
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    return detectText(buffer.str());
}

Family DialectDetector::mapFamily(Dialect dialect) {
    switch (dialect) {
    case Dialect::Fanuc:
    case Dialect::Mach3:
    case Dialect::BusselattoGenesis:
        return Family::IsoGcodeCore;
    case Dialect::Heidenhain:
        return Family::Conversational;
    case Dialect::Isel:
    case Dialect::BusselattoEvolution:
        return Family::VendorMacro;
    case Dialect::Woodwop:
    case Dialect::BiesseCix:
        return Family::StructuredParametric;
    case Dialect::Ambiguous:
    case Dialect::Unknown:
    default:
        return Family::Unknown;
    }
}

std::string toString(Dialect dialect) {
    switch (dialect) {
    case Dialect::Unknown:
        return "unknown_iso_variant";
    case Dialect::Ambiguous:
        return "ambiguous";
    case Dialect::Fanuc:
        return "fanuc";
    case Dialect::Mach3:
        return "mach3";
    case Dialect::Heidenhain:
        return "heidenhain";
    case Dialect::Isel:
        return "isel";
    case Dialect::Woodwop:
        return "woodwop";
    case Dialect::BiesseCix:
        return "biesse_cix";
    case Dialect::BusselattoGenesis:
        return "busselatto_genesis";
    case Dialect::BusselattoEvolution:
        return "busselatto_evolution";
    }
    return "unknown_iso_variant";
}

std::string toString(Family family) {
    switch (family) {
    case Family::Unknown:
        return "unknown";
    case Family::IsoGcodeCore:
        return "iso_gcode_core";
    case Family::VendorMacro:
        return "vendor_macro";
    case Family::Conversational:
        return "conversational";
    case Family::StructuredParametric:
        return "structured_parametric";
    }
    return "unknown";
}

} // namespace cnc

