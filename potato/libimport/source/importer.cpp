// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "importer.h"
#include "importer_configs_schema.h"

auto up::Importer::configType() const -> reflex::TypeInfo const& {
    return reflex::getTypeInfo<ImporterConfig>();
}
