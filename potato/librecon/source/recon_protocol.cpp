// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/recon/recon_protocol.h"

#include "potato/runtime/io_loop.h"
#include "potato/runtime/json.h"
#include "potato/spud/find.h"

#include <charconv>

static constexpr up::string_view headerMessageType{"Message-Type"};
static constexpr up::string_view headerContentLength{"Content-Length"};

bool up::ReconProtocol::HandlerBase::decode(nlohmann::json const& data, reflex::Schema const& schema, void* object) {
    return reflex::decodeFromJsonRaw(data, schema, object);
}

bool up::ReconProtocol::receive(view<char> data) {
    if (data.empty()) {
        return false;
    }

    _buffer.insert(_buffer.end(), data.begin(), data.end());

    size_t consumed = 0;
    bool work = true;
    bool handled = false;
    while (work && consumed < _buffer.size()) {
        view<char> const bytes{_buffer.cbegin() + consumed, _buffer.cend()};

        switch (_state) {
            case DecodeState::Ready:
                if (bytes.front() == '\n') {
                    consumed += 1;
                }
                else {
                    _state = DecodeState::Header;
                }

                break;

            case DecodeState::Header:
                if (bytes.front() == '\n') {
                    consumed += 1;
                    if (_headerContentLength > 0) {
                        _state = DecodeState::Body;
                    }
                    else {
                        _state = DecodeState::Ready;
                    }
                    break;
                }

                if (auto nl = find(bytes, '\n'); nl != bytes.end()) {
                    consumed += nl - bytes.begin() + 1 /*NL*/;

                    string_view const line{bytes.begin(), nl};

                    auto const sep = line.find(':');
                    if (sep == string_view::npos) {
                        continue;
                    }

                    string_view const headerName{bytes.begin(), sep};
                    string_view headerData = line.substr(sep + 1);
                    if (!headerData.empty() && headerData.front() == ' ') {
                        headerData.pop_front();
                    }

                    if (headerName == headerMessageType) {
                        nanofmt::format_to(_headerMessageType, "{}", headerData);
                    }
                    else if (headerName == headerContentLength) {
                        std::from_chars(headerData.data(), headerData.data() + headerData.size(), _headerContentLength);
                    }
                }
                else {
                    work = false;
                }

                break;

            case DecodeState::Body:
                if (bytes.size() >= _headerContentLength) {
                    if (_headerMessageType[0] != '\0') {
                        handled |= _handle(_headerMessageType, {bytes.data(), _headerContentLength});
                    }
                    consumed += _headerContentLength;

                    _headerMessageType[0] = '\0';
                    _headerContentLength = 0;

                    _state = DecodeState::Ready;
                }
                else {
                    work = false;
                }

                break;
        }
    }

    _buffer.erase(_buffer.begin(), _buffer.begin() + consumed);

    return handled;
}

bool up::ReconProtocol::_send(string_view name, reflex::Schema const& schema, void const* object) {
    nlohmann::json doc;
    if (!reflex::encodeToJsonRaw(doc, schema, object)) {
        return false;
    }

    auto const str = doc.dump();

    char headersBuf[128] = {0};
    char const* const headersEnd =
        nanofmt::format_to(headersBuf, "{}: {}\n{}: {}\n\n", headerMessageType, name, headerContentLength, str.size());

    IOStream& ioSink = sink();

    ioSink.write({headersBuf, static_cast<size_t>(headersEnd - headersBuf)});
    ioSink.write({str.data(), str.size()});
    ioSink.write({"\n", 1});
    return true;
}

bool up::ReconProtocol::_handle(string_view message, view<char> body) {
    nlohmann::json doc = nlohmann::json::parse(body, nullptr, false, true);
    if (!doc.is_object()) {
        return false;
    }

    bool handled = false;
    for (auto const& handler : _handlers) {
        if (handler->match(message)) {
            handled |= handler->handle(doc);
        }
    }

    return handled;
}
