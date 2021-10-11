// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "recon_message.h"

#include "potato/reflex/schema.h"
#include "potato/reflex/serialize.h"
#include "potato/spud/delegate.h"

#include <nlohmann/json.hpp>

namespace up {
    class IOStream;

    class ReconProtocol {
    public:
        template <ReconMessage MessageT>
        using Callback = delegate<void(typename MessageT::type const&)>;

        template <ReconMessage MessageT>
        bool send(typename MessageT::type const& msg) {
            return _send(MessageT::name, reflex::getSchema<typename MessageT::type>(), &msg);
        }

        template <ReconMessage MessageT>
        void on(Callback<MessageT> callback) {
            _handlers.push_back(new_box<Handler<MessageT>>(move(callback)));
        }

    protected:
        ReconProtocol() = default;
        ~ReconProtocol() = default;

        virtual IOStream& sink() = 0;

        UP_RECON_API bool receive(view<char> data);

    private:
        struct HandlerBase;
        template <ReconMessage MessageT>
        struct Handler;

        enum class DecodeState {
            Ready,
            Header,
            Body,
        };

        UP_RECON_API bool _send(string_view name, reflex::Schema const& schema, void const* object);
        bool _handle(string_view message, view<char> body);

        vector<char> _buffer;
        DecodeState _state = DecodeState::Ready;
        char _headerMessageType[32] = {0};
        size_t _headerContentLength = 0;
        vector<box<HandlerBase>> _handlers;
    };

    struct ReconProtocol::HandlerBase {
        virtual ~HandlerBase() = default;

        virtual bool match(string_view nm) const noexcept = 0;
        virtual bool handle(nlohmann::json const& data) = 0;
        UP_RECON_API bool decode(nlohmann::json const& data, reflex::Schema const& schema, void* object);
    };

    template <ReconMessage MessageT>
    struct ReconProtocol::Handler : HandlerBase {
        using Callback = ReconProtocol::Callback<MessageT>;

        explicit Handler(Callback cb) : schema(reflex::getSchema<typename MessageT::type>()), callback(move(cb)) {}

        bool match(string_view name) const noexcept override { return name == MessageT::name; }

        bool handle(nlohmann::json const& data) override {
            typename MessageT::type msg;
            if (!decode(data, schema, &msg)) {
                return false;
            }
            callback(msg);
            return true;
        }

        reflex::Schema const& schema;
        Callback callback;
    };
} // namespace up
