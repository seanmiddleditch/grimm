// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/recon/importer.h"

#include "potato/schema/importer_configs_schema.h"

auto up::Importer::configType() const -> reflex::TypeInfo const& {
    return reflex::getTypeInfo<ImporterConfig>();
}
