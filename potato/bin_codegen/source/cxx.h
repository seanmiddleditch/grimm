// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "schema.h"

#include <ostream>
#include <sstream>
#include <string_view>

namespace cxx {
    inline bool isIdent(char ch) noexcept { return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_'; }
    inline bool isDigit(char ch) noexcept { return ch >= '0' && ch <= '9'; }

    struct Ident {
        std::string_view ident;

        friend std::ostream& operator<<(std::ostream& os, Ident const&& self) {
            if (self.ident.empty()) {
                return os << '_';
            }

            os << (isIdent(self.ident.front()) ? self.ident.front() : '_');

            for (auto ch : self.ident.substr(1)) {
                os << ((isIdent(ch) || isDigit(ch)) ? ch : '_');
            }

            return os;
        }
    };

    struct TypeNamespace {
        schema::TypeBase const& type;

        friend std::ostream& operator<<(std::ostream& os, TypeNamespace const&& self) {
            using namespace std::literals;

            if (auto const cxxns = getAnnotationArg(self.type.annotations, "cxxnamespace"); cxxns) {
                return os << schema::valueCast<std::string_view>(*cxxns);
            }
            // if (self.type->kind == schema::TypeKind::Attribute) {
            //    return os << "schema::attribute::";
            //}
            if (self.type.qualifiedName == "int"sv || self.type.qualifiedName == "float"sv ||
                self.type.qualifiedName == "bool"sv) {
                return os;
            }
            if (self.type.qualifiedName == "string"sv) {
                return os << "up";
            }
            return os << "up::schema";
        }
    };

    struct TypeNamespacePrefix {
        schema::TypeBase const& type;

        friend std::ostream& operator<<(std::ostream& os, TypeNamespacePrefix const&& self) {
            std::ostringstream buf;
            buf << TypeNamespace{self.type};
            os << buf.str();
            if (!buf.str().empty()) {
                os << "::";
            }
            return os;
        }
    };

    struct QualifiedName {
        schema::TypeBase const& type;

        friend std::ostream& operator<<(std::ostream& os, QualifiedName const&& self) {
            if (auto const cxxname = getAnnotationArg(self.type.annotations, "cxxname"); cxxname) {
                return os << schema::valueCast<std::string_view>(*cxxname);
            }
            if (auto const cxximport = getAnnotationArg(self.type.annotations, "cxximport"); cxximport) {
                return os << schema::valueCast<std::string_view>(*cxximport);
            }

            return os << TypeNamespacePrefix{self.type} << Ident{self.type.name};
        }
    };

    struct Type {
        schema::TypeBase const& type;

        friend std::ostream& operator<<(std::ostream& os, Type const&& self) {
            using namespace schema;

            switch (self.type.kind) {
                case TypeKind::Pointer:
                    return os << "up::box<" << Type{*static_cast<TypeIndirect const&>(self.type).ref} << ">";
                case TypeKind::Alias:
                    return os << Type{*static_cast<TypeIndirect const&>(self.type).ref};
                case TypeKind::Array:
                    return os << "up::vector<" << Type{*static_cast<TypeIndirect const&>(self.type).ref} << ">";
                case TypeKind::Specialized:
                    if (auto const cxximport = getAnnotationArg(
                            static_cast<TypeSpecialized const&>(self.type).ref->annotations,
                            "cxximport");
                        cxximport) {
                        expandPlaceholders(os, valueCast<std::string>(*cxximport), self);
                    }
                    else {
                        buildSpecialization(os, self);
                    }
                    return os;
                default:
                    return os << QualifiedName{self.type};
            }
        }

        static void expandPlaceholders(std::ostream& os, std::string_view cxxname, Type const& self) {
            auto const& type = static_cast<schema::TypeSpecialized const&>(self.type);
            auto const& aggr = static_cast<schema::TypeAggregate const&>(*type.ref);

            auto pos = cxxname.find('$');
            decltype(pos) last = 0;
            while (pos != std::string::npos) {
                for (size_t index = 0; index != aggr.typeParams.size(); ++index) {
                    auto const& typeParam = aggr.typeParams[index]->name;

                    if (std::string_view{cxxname}.substr(pos + 1).starts_with(typeParam)) {
                        os.write(cxxname.data() + last, static_cast<std::streamsize>(pos - last));
                        os << QualifiedName{*type.typeArgs[index]};
                        last = pos = pos + typeParam.length() + 1;
                        break;
                    }
                }
                pos = cxxname.find('$', pos);
            }

            os << std::string_view{cxxname}.substr(last);
        }

        static void buildSpecialization(std::ostream& os, Type const& self) {
            auto const& type = static_cast<schema::TypeSpecialized const&>(self.type);

            os << Type{*type.ref} << "<";
            for (auto const* typeArg : type.typeArgs) {
                if (typeArg != type.typeArgs.front()) {
                    os << ", ";
                }
                os << Type{*typeArg};
            }
            os << ">";
        }
    };

    struct Value {
        schema::Value const& value;

        friend std::ostream& operator<<(std::ostream& os, Value const&& self) {
            using namespace schema;
            std::visit(
                [&os](auto const& value) {
                    using ValueType = std::remove_cvref_t<decltype(value)>;
                    if constexpr (std::is_same_v<ValueType, TypeBase const*>) {
                        os << "::up::reflex::getTypeInfo<" << Type{*value} << ">()";
                    }
                    else if constexpr (std::is_same_v<ValueType, std::string>) {
                        os << '"';
                        for (auto const ch : value) {
                            if (ch == '\\' || ch == '"') {
                                os << '\\';
                            }
                            os << ch;
                        }
                        os << '"';
                    }
                    else {
                        os << value;
                    }
                },
                self.value);
            return os;
        }
    };

    inline std::string typeNamespace(schema::TypeBase const& type) {
        std::ostringstream buf;
        buf << TypeNamespace{type};
        return buf.str();
    }
} // namespace cxx
