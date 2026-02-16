#pragma once

#include <optional>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

namespace cnc {

enum class Dialect {
    Unknown,
    Ambiguous,
    Fanuc,
    Mach3,
    Heidenhain,
    Isel,
    Woodwop,
    BiesseCix,
    BusselattoGenesis,
    BusselattoEvolution
};

enum class Family {
    Unknown,
    IsoGcodeCore,
    VendorMacro,
    Conversational,
    StructuredParametric
};

struct Evidence {
    std::string key;
    double contribution = 0.0;
    std::string detail;
};

struct DetectionResult {
    Dialect dialect = Dialect::Unknown;
    Family family = Family::Unknown;
    double confidence = 0.0;
    std::vector<Evidence> evidence;
    std::vector<std::pair<Dialect, double>> topCandidates;
};

struct DialectRuleSet {
    Dialect dialect = Dialect::Unknown;
    std::vector<std::regex> hardSignatures;
    std::unordered_map<std::string, double> weightedFeatures;
};

class TextNormalizer {
  public:
    static std::string normalize(const std::string &raw);
    static std::vector<std::string> splitLines(const std::string &text);

  private:
    static std::string stripParenthesisComments(const std::string &line);
    static std::string stripSemicolonComment(const std::string &line);
    static std::string separateConcatenatedTokens(const std::string &line);
};

class FeatureExtractor {
  public:
    static std::unordered_map<std::string, double> extract(const std::string &normalizedText);
};

class RuleRepository {
  public:
    static std::vector<DialectRuleSet> defaultRules();
};

class DialectDetector {
  public:
    explicit DialectDetector(std::vector<DialectRuleSet> rules = RuleRepository::defaultRules(),
                             std::size_t maxLinesToScan = 500);

    DetectionResult detectText(const std::string &text) const;
    DetectionResult detectFile(const std::string &path) const;

    void setThresholds(double minScore, double ambiguityDelta);

  private:
    std::optional<DetectionResult> tryHardSignatures(const std::string &normalizedText) const;
    DetectionResult scoreBasedDetection(const std::string &normalizedText) const;
    static Family mapFamily(Dialect dialect);

    std::vector<DialectRuleSet> rules_;
    std::size_t maxLinesToScan_;
    double minScore_ = 1.0;
    double ambiguityDelta_ = 0.75;
};

std::string toString(Dialect dialect);
std::string toString(Family family);

} // namespace cnc

