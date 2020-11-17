# Copyright by Potato Engine contributors.  See accompanying License.txt for
# copyright details.
from enum import Enum

class TypeKind(Enum):
    """Type kinds differentiate declarations such as structs and enums"""

    ATTRIBUTE = "attribute"
    OPAQUE = "opaque"
    STRUCT = "struct"
    ENUM = "enum"

class Annotation:
    """A single annotation"""
    def __init__(self, type_name):
        self.__name = ''
        self.__type = None
        self.__type_name = type_name
        self.__values_ordered = []
        self.__values = {}

    @property
    def type(self):
        return self.__type

    @property
    def values(self):
        return self.__values_ordered

    def value_or(self, key, default):
        return self.__values[key] if key in self.__values else default

    def load_from_json(self, json):
        self.__values = {key:value for key,value in json.items()}

    def resolve(self, db):
        self.__type = db.types[self.__type_name]
        for field in self.__type.fields_ordered:
            self.__values_ordered.append(self.__values[field.name])

class AnnotationsBase:
    """Holder for annotations"""
    def __init__(self):
        self.__annotations = {}

    @property
    def annotations(self):
        return self.__annotations.values()

    def __contains__(self, key):
        return key in self.__annotations

    def has_annotation(self, key):
        return key in self.__annotations

    def get_annotation(self, key, sub):
        return self.__annotations[key] if key in self.__annotations else None

    def get_annotation_field_or(self, key, sub, otherwise):
        return self.__annotations[key].value_or(sub, otherwise) if key in self.__annotations else otherwise

    def load_from_json(self, json):
        def annotate(name, json):
            anno = Annotation(name)
            anno.load_from_json(json)
            return anno
        self.__annotations = {key:annotate(key, value) for key,value in json['annotations'].items()}

    def resolve(self, db):
        for key,anno in self.__annotations.items():
            anno.resolve(db)

class TypeDatabase:
    """Collection of all known types"""
    def __init__(self):
        self.__types = {}
        self.__module = ''
        self.__exports = []
        self.__imports = []

    @property
    def module(self):
        return self.__module

    @property
    def types(self):
        return self.__types

    @property
    def imports(self):
        return self.__imports

    @property
    def exports(self):
        return {name:(self.__types[name]) for name in self.__exports}

    def type(self, name):
        return self.__types[name]

    def load_from_json(self, doc):
        AnnotationsBase.load_from_json(self, doc)

        self.__module = doc['module']
        self.__imports = [name for name in doc['imports']]
        self.__exports = [name for name in doc['exports']]
        for key,type_json in doc['types'].items():
            kind = type_json['kind']
            if kind == TypeKind.STRUCT.value:
                type = TypeStruct(self, name=key)
            elif kind == TypeKind.ATTRIBUTE.value:
                type = TypeAttribute(self, name=key)
            elif kind == TypeKind.OPAQUE.value:
                type = TypeOpaque(self, name=key)
            elif kind == TypeKind.ENUM.value:
                type = TypeEnum(self, name=key)
            else:
                raise Exception(f'Unknown kind {kind}')
            self.__types[type.name] = type
            type.load_from_json(type_json)

        self.__resolve_types()

    def __resolve_types(self):
        for type in self.__types.values():
            type.resolve(self)

class TypeBase(AnnotationsBase):
    """User-friendly wrapper of a type-definition in the type database"""
    __builtin_types = ['char', 'float', 'double']

    def __init__(self, db, name):
        self.__db = db
        self.__module = db.module
        self.__name = name
        self.__base_type_name = ''
        self.__base_type = None

    @property
    def name(self):
        return self.__name

    @property
    def module(self):
        return self.__module

    @property
    def kind(self):
        return None

    @property
    def exported(self):
        return self.__module == self.__db.module

    @property
    def builtin(self):
        return self.__module == '@core'

    @property
    def cxxname(self):
        return self.get_annotation_field_or('cxxname', 'id', self.get_annotation_field_or('cxximport', 'id', self.__name))

    @property
    def qualified_cxxname(self):
        if self.module == '$core': return self.cxxname

        cxximport = self.get_annotation_field_or('cxximport', 'id', None)
        if cxximport is not None: return cxximport

        if self.__name in TypeBase.__builtin_types:
            return self.__name

        cxxname = self.get_annotation_field_or('cxxname', 'id', self.__name)
        ns = self.get_annotation_field_or('cxxnamespace', 'ns', 'up::schema')

        return f'{ns}::{cxxname}'

    @property
    def base(self):
        return self.__base_type

    def load_from_json(self, json):
        AnnotationsBase.load_from_json(self, json)
        self.__base_type_name = json['base'] if 'base' in json and json['base'] is not None else ''
        self.__module = json['module']

    def resolve(self, db):
        if self.__base_type_name != '':
            self.__base_type = db.type(self.__base_type_name)
        AnnotationsBase.resolve(self, db)

class TypeOpaque(TypeBase):
    """User-friendly wrapper of an opaque definition"""
    @property
    def kind(self):
        return TypeKind.OPAQUE

class TypeStruct(TypeBase):
    """User-friendly wrapper of a struct definition"""
    def __init__(self, db, name):
        TypeBase.__init__(self, db, name)
        self.__fields_ordered = []
        self.__fields = {}

    @property
    def kind(self):
        return TypeKind.STRUCT

    @property
    def fields(self):
        return self.__fields

    @property
    def fields_ordered(self):
        return [self.__fields[name] for name in self.__fields_ordered]

    def load_from_json(self, json):
        TypeBase.load_from_json(self, json)
        self.__fields_ordered = [name for name in json['order']]
        for key,field_json in json['fields'].items():
            field = TypeField(owner=self, name=key)
            field.load_from_json(field_json)
            self.__fields[key] = field

    def resolve(self, db):
        TypeBase.resolve(self, db)
        for field in self.__fields.values():
            field.resolve(db)

class TypeEnum(TypeBase):
    """User-friendly wrapper of an enum definition"""
    def __init__(self, db, name):
        TypeBase.__init__(self, db, name)
        self.__names = []
        self.__values = {}

    @property
    def kind(self):
        return TypeKind.ENUM

    def value_or(self, key, otherwise=None):
        return self.__values[key] if key in self.__values else otherwise

    @property
    def values(self):
        return self.__values

    @property
    def names(self):
        return self.__names

    def load_from_json(self, json):
        TypeBase.load_from_json(self, json)
        for key,value in json['values'].items():
            self.__values[key] = value
        self.__names = [key for key in json['names']]

class TypeAttribute(TypeStruct):
    """User-friendly wrapper for an attribute struct"""
    @property
    def kind(self):
        return TypeKind.ATTRIBUTE

class TypeField(AnnotationsBase):
    """User-friendly wrapper for a field definition of a type"""
    def __init__(self, owner, name):
        self.__owner = owner
        self.__name = name
        self.__type_name = ''
        self.__type = None
        self.__default = None
        self.__has_default = False
        self.__is_array = False

    @property
    def name(self):
        return self.__name

    @property
    def cxxname(self):
        return self.get_annotation_field_or('cxxname', 'id', self.__name)

    @property
    def cxxtype(self):
        cxxname = self.__type.qualified_cxxname
        return f"vector<{cxxname}>" if self.__is_array else cxxname

    @property
    def type(self):
        return self.__type

    @property
    def is_array(self):
        return self.__is_array

    def default_or(self, otherwise):
        if self.__has_default:
            return self.__default
        else:
            return otherwise

    @property
    def has_default(self):
        return self.__has_default

    def load_from_json(self, json):
        AnnotationsBase.load_from_json(self, json)
        type = json['type']
        if isinstance(type, str):
            self.__type_name = json['type']
        elif type['kind'] == 'array':
            self.__type_name = type['of']
            self.__is_array = True
        if 'default' in json:
            self.__default = json['default']
            self.__has_default = True

    def resolve(self, db):
        self.__type = db.type(self.__type_name)
        AnnotationsBase.resolve(self, db)
