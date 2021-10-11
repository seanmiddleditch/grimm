// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "importer_factory.h"
#include "copy_importer.h"
#include "folder_importer.h"
#include "hlsl_importer.h"
#include "ignore_importer.h"
#include "importer_configs_schema.h"
#include "json_importer.h"
#include "material_importer.h"
#include "model_importer.h"

#include "potato/reflex/serialize.h"
#include "potato/runtime/assertion.h"

up::ImporterFactory::ImporterFactory() = default;

up::ImporterFactory::~ImporterFactory() = default;

auto up::ImporterFactory::findImporterByName(string_view name) const noexcept -> Importer* {
    for (auto const& conv : _importers) {
        if (conv->name() == name) {
            return conv.get();
        }
    }
    return nullptr;
}

void up::ImporterFactory::registerImporter(box<Importer> importer) {
    UP_ASSERT(!importer.empty());
    UP_ASSERT(findImporterByName(importer->name()) == nullptr);

    _importers.push_back(std::move(importer));
}

void up::ImporterFactory::registerDefaultImporters() {
    registerImporter(new_box<FolderImporter>());
    registerImporter(new_box<CopyImporter>());
    registerImporter(new_box<HlslImporter>());
    registerImporter(new_box<IgnoreImporter>());
    registerImporter(new_box<JsonImporter>());
    registerImporter(new_box<MaterialImporter>());
    registerImporter(new_box<ModelImporter>());
}

auto up::ImporterFactory::parseConfig(Importer const& importer, nlohmann::json const& config) const
    -> box<ImporterConfig> {
    reflex::TypeInfo const& typeInfo = importer.configType();
    if (typeInfo.schema == nullptr) {
        return new_box<ImporterConfig>();
    }

    void* mem = ::operator new(typeInfo.size);
    typeInfo.ops.defaultConstructor(mem);
    box<ImporterConfig> boxed(static_cast<ImporterConfig*>(mem));

    reflex::decodeFromJsonRaw(config, *typeInfo.schema, mem);

    return boxed;
}
