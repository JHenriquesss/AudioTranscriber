#include "whisper_engine/WhisperOptionsMapper.hpp"

#include <algorithm>
#include <cctype>

namespace whisper_engine {

namespace {

std::string toLowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return value;
}

} // namespace

TranscriptionLanguageOption parseLanguageOption(const std::string &language) {
    const auto normalized = toLowerCopy(language);
    if (normalized == "pt" || normalized == "portuguese" || normalized == "por") {
        return TranscriptionLanguageOption::Portuguese;
    }
    if (normalized == "en" || normalized == "english" || normalized == "eng") {
        return TranscriptionLanguageOption::English;
    }
    return TranscriptionLanguageOption::Auto;
}

std::string mapLanguageToWhisperCode(TranscriptionLanguageOption language) {
    switch (language) {
    case TranscriptionLanguageOption::Portuguese:
        return "pt";
    case TranscriptionLanguageOption::English:
        return "en";
    case TranscriptionLanguageOption::Auto:
        return "auto";
    }
    return "auto";
}

std::string mapLanguageToWhisperCode(const std::string &language) {
    return mapLanguageToWhisperCode(parseLanguageOption(language));
}

} // namespace whisper_engine
