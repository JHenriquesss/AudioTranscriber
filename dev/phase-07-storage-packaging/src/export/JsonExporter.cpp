#include "export/JsonExporter.hpp"

#include "export/ExportFileWrite.hpp"
#include "transcription/TranscriptValidation.hpp"

#include <cctype>
#include <sstream>
#include <string_view>

namespace export_format {

namespace {

std::string escapeJsonString(const std::string &value) {
    std::ostringstream stream;
    stream << '"';
    for (const char character : value) {
        switch (character) {
        case '"':
            stream << "\\\"";
            break;
        case '\\':
            stream << "\\\\";
            break;
        case '\b':
            stream << "\\b";
            break;
        case '\f':
            stream << "\\f";
            break;
        case '\n':
            stream << "\\n";
            break;
        case '\r':
            stream << "\\r";
            break;
        case '\t':
            stream << "\\t";
            break;
        default:
            stream << character;
            break;
        }
    }
    stream << '"';
    return stream.str();
}

void appendIndent(std::ostringstream &stream, int depth) {
    stream << '\n';
    for (int index = 0; index < depth; ++index) {
        stream << "  ";
    }
}

void appendWordJson(std::ostringstream &stream, const transcription::TranscriptWord &word,
                    int depth) {
    appendIndent(stream, depth);
    stream << '{';
    appendIndent(stream, depth + 1);
    stream << "\"text\": " << escapeJsonString(word.text) << ',';
    appendIndent(stream, depth + 1);
    stream << "\"start_ms\": " << word.startMs << ',';
    appendIndent(stream, depth + 1);
    stream << "\"end_ms\": " << word.endMs << ',';
    appendIndent(stream, depth + 1);
    stream << "\"probability\": " << word.probability;
    appendIndent(stream, depth);
    stream << '}';
}

void appendSegmentJson(std::ostringstream &stream, const transcription::TranscriptSegment &segment,
                       int depth) {
    appendIndent(stream, depth);
    stream << '{';
    appendIndent(stream, depth + 1);
    stream << "\"index\": " << segment.index << ',';
    appendIndent(stream, depth + 1);
    stream << "\"start_ms\": " << segment.startMs << ',';
    appendIndent(stream, depth + 1);
    stream << "\"end_ms\": " << segment.endMs << ',';
    appendIndent(stream, depth + 1);
    stream << "\"text\": " << escapeJsonString(segment.text) << ',';
    appendIndent(stream, depth + 1);
    stream << "\"words\": [";

    for (std::size_t wordIndex = 0; wordIndex < segment.words.size(); ++wordIndex) {
        if (wordIndex > 0) {
            stream << ',';
        }
        appendWordJson(stream, segment.words[wordIndex], depth + 2);
    }

    if (!segment.words.empty()) {
        appendIndent(stream, depth + 1);
    }
    stream << ']';
    appendIndent(stream, depth);
    stream << '}';
}

class JsonParser {
  public:
    explicit JsonParser(std::string_view input) : input_(input) {}

    shared::Result<transcription::TranscriptDocument> parseDocument() {
        skipWhitespace();
        const auto openResult = expectChar('{');
        if (!openResult.ok) {
            return shared::Result<transcription::TranscriptDocument>::failure(openResult.error);
        }

        transcription::TranscriptDocument document;
        skipWhitespace();
        if (matchChar('}')) {
            return shared::Result<transcription::TranscriptDocument>::success(std::move(document));
        }

        while (true) {
            const auto keyResult = parseString();
            if (!keyResult.ok) {
                return shared::Result<transcription::TranscriptDocument>::failure(keyResult.error);
            }

            skipWhitespace();
            const auto colonResult = expectChar(':');
            if (!colonResult.ok) {
                return shared::Result<transcription::TranscriptDocument>::failure(
                    colonResult.error);
            }

            const auto &key = keyResult.value;
            if (key == "id") {
                const auto valueResult = parseString();
                if (!valueResult.ok) {
                    return shared::Result<transcription::TranscriptDocument>::failure(
                        valueResult.error);
                }
                document.id = valueResult.value;
            } else if (key == "source_file") {
                const auto valueResult = parseString();
                if (!valueResult.ok) {
                    return shared::Result<transcription::TranscriptDocument>::failure(
                        valueResult.error);
                }
                document.sourceFile = valueResult.value;
            } else if (key == "requested_language") {
                const auto valueResult = parseString();
                if (!valueResult.ok) {
                    return shared::Result<transcription::TranscriptDocument>::failure(
                        valueResult.error);
                }
                document.requestedLanguage = valueResult.value;
            } else if (key == "detected_language") {
                const auto valueResult = parseString();
                if (!valueResult.ok) {
                    return shared::Result<transcription::TranscriptDocument>::failure(
                        valueResult.error);
                }
                document.detectedLanguage = valueResult.value;
            } else if (key == "model") {
                const auto valueResult = parseString();
                if (!valueResult.ok) {
                    return shared::Result<transcription::TranscriptDocument>::failure(
                        valueResult.error);
                }
                document.modelId = valueResult.value;
            } else if (key == "duration_ms") {
                const auto valueResult = parseInt64();
                if (!valueResult.ok) {
                    return shared::Result<transcription::TranscriptDocument>::failure(
                        valueResult.error);
                }
                document.durationMs = valueResult.value;
            } else if (key == "created_at") {
                const auto valueResult = parseString();
                if (!valueResult.ok) {
                    return shared::Result<transcription::TranscriptDocument>::failure(
                        valueResult.error);
                }
                document.createdAtIso = valueResult.value;
            } else if (key == "segments") {
                const auto segmentsResult = parseSegments();
                if (!segmentsResult.ok) {
                    return shared::Result<transcription::TranscriptDocument>::failure(
                        segmentsResult.error);
                }
                document.segments = std::move(segmentsResult.value);
            } else {
                const auto skipResult = skipValue();
                if (!skipResult.ok) {
                    return shared::Result<transcription::TranscriptDocument>::failure(
                        skipResult.error);
                }
            }

            skipWhitespace();
            if (matchChar('}')) {
                break;
            }

            const auto commaResult = expectChar(',');
            if (!commaResult.ok) {
                return shared::Result<transcription::TranscriptDocument>::failure(
                    commaResult.error);
            }
            skipWhitespace();
        }

        skipWhitespace();
        return shared::Result<transcription::TranscriptDocument>::success(std::move(document));
    }

  private:
    std::string_view input_;
    std::size_t position_ = 0;

    shared::AppError parseError(const std::string &detail) const {
        return shared::AppError{
            shared::ErrorCode::ExportFailed,
            "Transcript JSON is invalid.",
            detail,
        };
    }

    bool isAtEnd() const {
        return position_ >= input_.size();
    }

    char peek() const {
        return isAtEnd() ? '\0' : input_[position_];
    }

    char consume() {
        return isAtEnd() ? '\0' : input_[position_++];
    }

    void skipWhitespace() {
        while (!isAtEnd() && std::isspace(static_cast<unsigned char>(peek())) != 0) {
            ++position_;
        }
    }

    bool matchChar(char expected) {
        if (peek() != expected) {
            return false;
        }
        ++position_;
        return true;
    }

    shared::Result<void> expectChar(char expected) {
        if (!matchChar(expected)) {
            return shared::Result<void>::failure(
                parseError(std::string("Expected '") + expected + "' in transcript JSON."));
        }
        return shared::Result<void>::success();
    }

    shared::Result<std::string> parseString() {
        skipWhitespace();
        if (!matchChar('"')) {
            return shared::Result<std::string>::failure(
                parseError("Expected string value in transcript JSON."));
        }

        std::string value;
        while (!isAtEnd()) {
            const char character = consume();
            if (character == '"') {
                return shared::Result<std::string>::success(std::move(value));
            }

            if (character == '\\') {
                if (isAtEnd()) {
                    return shared::Result<std::string>::failure(
                        parseError("Unterminated escape sequence in transcript JSON."));
                }

                const char escaped = consume();
                switch (escaped) {
                case '"':
                    value.push_back('"');
                    break;
                case '\\':
                    value.push_back('\\');
                    break;
                case '/':
                    value.push_back('/');
                    break;
                case 'b':
                    value.push_back('\b');
                    break;
                case 'f':
                    value.push_back('\f');
                    break;
                case 'n':
                    value.push_back('\n');
                    break;
                case 'r':
                    value.push_back('\r');
                    break;
                case 't':
                    value.push_back('\t');
                    break;
                default:
                    return shared::Result<std::string>::failure(
                        parseError("Unsupported escape sequence in transcript JSON."));
                }
                continue;
            }

            value.push_back(character);
        }

        return shared::Result<std::string>::failure(
            parseError("Unterminated string in transcript JSON."));
    }

    shared::Result<int64_t> parseInt64() {
        skipWhitespace();
        std::size_t start = position_;
        if (peek() == '-') {
            ++position_;
        }

        if (isAtEnd() || !std::isdigit(static_cast<unsigned char>(peek()))) {
            return shared::Result<int64_t>::failure(
                parseError("Expected integer value in transcript JSON."));
        }

        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek())) != 0) {
            ++position_;
        }

        const auto numberText = input_.substr(start, position_ - start);
        try {
            return shared::Result<int64_t>::success(std::stoll(std::string(numberText)));
        } catch (const std::exception &) {
            return shared::Result<int64_t>::failure(
                parseError("Integer value is out of range in transcript JSON."));
        }
    }

    shared::Result<double> parseNumber() {
        skipWhitespace();
        std::size_t start = position_;
        if (peek() == '-') {
            ++position_;
        }

        while (!isAtEnd() &&
               (std::isdigit(static_cast<unsigned char>(peek())) != 0 || peek() == '.')) {
            ++position_;
        }

        if (position_ == start) {
            return shared::Result<double>::failure(
                parseError("Expected numeric value in transcript JSON."));
        }

        const auto numberText = input_.substr(start, position_ - start);
        try {
            return shared::Result<double>::success(std::stod(std::string(numberText)));
        } catch (const std::exception &) {
            return shared::Result<double>::failure(
                parseError("Numeric value is invalid in transcript JSON."));
        }
    }

    shared::Result<std::vector<transcription::TranscriptWord>> parseWords() {
        skipWhitespace();
        const auto openResult = expectChar('[');
        if (!openResult.ok) {
            return shared::Result<std::vector<transcription::TranscriptWord>>::failure(
                openResult.error);
        }

        std::vector<transcription::TranscriptWord> words;
        skipWhitespace();
        if (matchChar(']')) {
            return shared::Result<std::vector<transcription::TranscriptWord>>::success(
                std::move(words));
        }

        while (true) {
            const auto wordResult = parseWordObject();
            if (!wordResult.ok) {
                return shared::Result<std::vector<transcription::TranscriptWord>>::failure(
                    wordResult.error);
            }
            words.push_back(wordResult.value);

            skipWhitespace();
            if (matchChar(']')) {
                break;
            }

            const auto commaResult = expectChar(',');
            if (!commaResult.ok) {
                return shared::Result<std::vector<transcription::TranscriptWord>>::failure(
                    commaResult.error);
            }
            skipWhitespace();
        }

        return shared::Result<std::vector<transcription::TranscriptWord>>::success(
            std::move(words));
    }

    shared::Result<transcription::TranscriptWord> parseWordObject() {
        skipWhitespace();
        const auto openResult = expectChar('{');
        if (!openResult.ok) {
            return shared::Result<transcription::TranscriptWord>::failure(openResult.error);
        }

        transcription::TranscriptWord word;
        skipWhitespace();
        const auto closeImmediately = matchChar('}');
        if (!closeImmediately) {
            while (true) {
                const auto keyResult = parseString();
                if (!keyResult.ok) {
                    return shared::Result<transcription::TranscriptWord>::failure(keyResult.error);
                }

                skipWhitespace();
                const auto colonResult = expectChar(':');
                if (!colonResult.ok) {
                    return shared::Result<transcription::TranscriptWord>::failure(
                        colonResult.error);
                }

                const auto &key = keyResult.value;
                if (key == "text") {
                    const auto valueResult = parseString();
                    if (!valueResult.ok) {
                        return shared::Result<transcription::TranscriptWord>::failure(
                            valueResult.error);
                    }
                    word.text = valueResult.value;
                } else if (key == "start_ms") {
                    const auto valueResult = parseInt64();
                    if (!valueResult.ok) {
                        return shared::Result<transcription::TranscriptWord>::failure(
                            valueResult.error);
                    }
                    word.startMs = valueResult.value;
                } else if (key == "end_ms") {
                    const auto valueResult = parseInt64();
                    if (!valueResult.ok) {
                        return shared::Result<transcription::TranscriptWord>::failure(
                            valueResult.error);
                    }
                    word.endMs = valueResult.value;
                } else if (key == "probability") {
                    const auto valueResult = parseNumber();
                    if (!valueResult.ok) {
                        return shared::Result<transcription::TranscriptWord>::failure(
                            valueResult.error);
                    }
                    word.probability = static_cast<float>(valueResult.value);
                } else {
                    const auto skipResult = skipValue();
                    if (!skipResult.ok) {
                        return shared::Result<transcription::TranscriptWord>::failure(
                            skipResult.error);
                    }
                }

                skipWhitespace();
                if (matchChar('}')) {
                    break;
                }

                const auto commaResult = expectChar(',');
                if (!commaResult.ok) {
                    return shared::Result<transcription::TranscriptWord>::failure(
                        commaResult.error);
                }
                skipWhitespace();
            }
        }

        return shared::Result<transcription::TranscriptWord>::success(std::move(word));
    }

    shared::Result<transcription::TranscriptSegment> parseSegmentObject() {
        skipWhitespace();
        const auto openResult = expectChar('{');
        if (!openResult.ok) {
            return shared::Result<transcription::TranscriptSegment>::failure(openResult.error);
        }

        transcription::TranscriptSegment segment;
        skipWhitespace();
        if (!matchChar('}')) {
            while (true) {
                const auto keyResult = parseString();
                if (!keyResult.ok) {
                    return shared::Result<transcription::TranscriptSegment>::failure(
                        keyResult.error);
                }

                skipWhitespace();
                const auto colonResult = expectChar(':');
                if (!colonResult.ok) {
                    return shared::Result<transcription::TranscriptSegment>::failure(
                        colonResult.error);
                }

                const auto &key = keyResult.value;
                if (key == "index") {
                    const auto valueResult = parseInt64();
                    if (!valueResult.ok) {
                        return shared::Result<transcription::TranscriptSegment>::failure(
                            valueResult.error);
                    }
                    segment.index = static_cast<int>(valueResult.value);
                } else if (key == "start_ms") {
                    const auto valueResult = parseInt64();
                    if (!valueResult.ok) {
                        return shared::Result<transcription::TranscriptSegment>::failure(
                            valueResult.error);
                    }
                    segment.startMs = valueResult.value;
                } else if (key == "end_ms") {
                    const auto valueResult = parseInt64();
                    if (!valueResult.ok) {
                        return shared::Result<transcription::TranscriptSegment>::failure(
                            valueResult.error);
                    }
                    segment.endMs = valueResult.value;
                } else if (key == "text") {
                    const auto valueResult = parseString();
                    if (!valueResult.ok) {
                        return shared::Result<transcription::TranscriptSegment>::failure(
                            valueResult.error);
                    }
                    segment.text = valueResult.value;
                } else if (key == "words") {
                    const auto wordsResult = parseWords();
                    if (!wordsResult.ok) {
                        return shared::Result<transcription::TranscriptSegment>::failure(
                            wordsResult.error);
                    }
                    segment.words = std::move(wordsResult.value);
                } else {
                    const auto skipResult = skipValue();
                    if (!skipResult.ok) {
                        return shared::Result<transcription::TranscriptSegment>::failure(
                            skipResult.error);
                    }
                }

                skipWhitespace();
                if (matchChar('}')) {
                    break;
                }

                const auto commaResult = expectChar(',');
                if (!commaResult.ok) {
                    return shared::Result<transcription::TranscriptSegment>::failure(
                        commaResult.error);
                }
                skipWhitespace();
            }
        }

        return shared::Result<transcription::TranscriptSegment>::success(std::move(segment));
    }

    shared::Result<std::vector<transcription::TranscriptSegment>> parseSegments() {
        skipWhitespace();
        const auto openResult = expectChar('[');
        if (!openResult.ok) {
            return shared::Result<std::vector<transcription::TranscriptSegment>>::failure(
                openResult.error);
        }

        std::vector<transcription::TranscriptSegment> segments;
        skipWhitespace();
        if (matchChar(']')) {
            return shared::Result<std::vector<transcription::TranscriptSegment>>::success(
                std::move(segments));
        }

        while (true) {
            const auto segmentResult = parseSegmentObject();
            if (!segmentResult.ok) {
                return shared::Result<std::vector<transcription::TranscriptSegment>>::failure(
                    segmentResult.error);
            }
            segments.push_back(segmentResult.value);

            skipWhitespace();
            if (matchChar(']')) {
                break;
            }

            const auto commaResult = expectChar(',');
            if (!commaResult.ok) {
                return shared::Result<std::vector<transcription::TranscriptSegment>>::failure(
                    commaResult.error);
            }
            skipWhitespace();
        }

        return shared::Result<std::vector<transcription::TranscriptSegment>>::success(
            std::move(segments));
    }

    shared::Result<void> skipValue() {
        skipWhitespace();
        if (matchChar('"')) {
            --position_;
            const auto stringResult = parseString();
            if (!stringResult.ok) {
                return shared::Result<void>::failure(stringResult.error);
            }
            return shared::Result<void>::success();
        }

        if (matchChar('{')) {
            int depth = 1;
            while (!isAtEnd() && depth > 0) {
                const char character = consume();
                if (character == '{') {
                    ++depth;
                } else if (character == '}') {
                    --depth;
                } else if (character == '"') {
                    --position_;
                    const auto stringResult = parseString();
                    if (!stringResult.ok) {
                        return shared::Result<void>::failure(stringResult.error);
                    }
                }
            }

            if (depth != 0) {
                return shared::Result<void>::failure(
                    parseError("Unterminated object while skipping transcript JSON value."));
            }
            return shared::Result<void>::success();
        }

        if (matchChar('[')) {
            int depth = 1;
            while (!isAtEnd() && depth > 0) {
                const char character = consume();
                if (character == '[') {
                    ++depth;
                } else if (character == ']') {
                    --depth;
                } else if (character == '"') {
                    --position_;
                    const auto stringResult = parseString();
                    if (!stringResult.ok) {
                        return shared::Result<void>::failure(stringResult.error);
                    }
                }
            }

            if (depth != 0) {
                return shared::Result<void>::failure(
                    parseError("Unterminated array while skipping transcript JSON value."));
            }
            return shared::Result<void>::success();
        }

        if (std::isdigit(static_cast<unsigned char>(peek())) != 0 || peek() == '-') {
            const auto numberResult = parseNumber();
            if (!numberResult.ok) {
                return shared::Result<void>::failure(numberResult.error);
            }
            return shared::Result<void>::success();
        }

        if (input_.substr(position_, 4) == "true") {
            position_ += 4;
            return shared::Result<void>::success();
        }

        if (input_.substr(position_, 5) == "false") {
            position_ += 5;
            return shared::Result<void>::success();
        }

        if (input_.substr(position_, 4) == "null") {
            position_ += 4;
            return shared::Result<void>::success();
        }

        return shared::Result<void>::failure(
            parseError("Unsupported JSON value in transcript JSON."));
    }
};

} // namespace

shared::Result<std::string>
JsonExporter::render(const transcription::TranscriptDocument &transcript) {
    const auto validation = transcription::validateTranscript(transcript);
    if (!validation.ok) {
        return shared::Result<std::string>::failure(validation.error);
    }

    std::ostringstream stream;
    stream << '{';
    appendIndent(stream, 1);
    stream << "\"id\": " << escapeJsonString(transcript.id) << ',';
    appendIndent(stream, 1);
    stream << "\"source_file\": " << escapeJsonString(transcript.sourceFile.generic_string())
           << ',';
    appendIndent(stream, 1);
    stream << "\"requested_language\": " << escapeJsonString(transcript.requestedLanguage) << ',';
    appendIndent(stream, 1);
    stream << "\"detected_language\": " << escapeJsonString(transcript.detectedLanguage) << ',';
    appendIndent(stream, 1);
    stream << "\"model\": " << escapeJsonString(transcript.modelId) << ',';
    appendIndent(stream, 1);
    stream << "\"duration_ms\": " << transcript.durationMs << ',';
    appendIndent(stream, 1);
    stream << "\"created_at\": " << escapeJsonString(transcript.createdAtIso) << ',';
    appendIndent(stream, 1);
    stream << "\"segments\": [";

    for (std::size_t index = 0; index < transcript.segments.size(); ++index) {
        if (index > 0) {
            stream << ',';
        }
        appendSegmentJson(stream, transcript.segments[index], 2);
    }

    appendIndent(stream, 1);
    stream << ']';
    stream << "\n}\n";

    return shared::Result<std::string>::success(stream.str());
}

shared::Result<transcription::TranscriptDocument> JsonExporter::parse(const std::string &json) {
    JsonParser parser(json);
    return parser.parseDocument();
}

shared::Result<void> JsonExporter::exportToFile(const transcription::TranscriptDocument &transcript,
                                                const std::filesystem::path &outputFile) {
    const auto rendered = render(transcript);
    if (!rendered.ok) {
        return shared::Result<void>::failure(rendered.error);
    }

    return writeTextFile(outputFile, rendered.value);
}

} // namespace export_format
