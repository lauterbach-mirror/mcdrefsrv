import importlib
import dataclasses
import inspect
import primitives
import modifiers

FREE_TABLE = {}
def freeable(struct):
    if struct in FREE_TABLE:
        return FREE_TABLE[struct]

    indent = 0
    fields = [f for f in dataclasses.fields(struct)]
    indent = 0
    indent += 4
    for f in fields:
        mod = f.default
        type = f.type.__name__
        if mod.varLen:
            FREE_TABLE[struct] = True
            return True

        if freeable(f.type):
            FREE_TABLE[struct] = True
            return True

    FREE_TABLE[struct] = False
    return False

MARSH_DECLARE = "static uint32_t marshal_{0}(const {0} *obj, uint8_t *buf)"
UNMARSH_DECLARE = "static uint32_t unmarshal_{0}(const uint8_t *buf, {0} *obj)"
FREE_DECLARE = "static void free_{0}({0} *obj)"

MARSHAL_CALL = "tail += marshal_{0}({1}, tail);"
UNMARSHAL_CALL = "head += unmarshal_{0}(head, {1});"

def print_indent(indent, s):
    print(f"{' ' * indent}{s}")

def prolog(structs):
    print ("""#define DEFINE_PRIMITIVE(type) \\
static int marshal_##type (type obj, uint8_t *buf) \\
{ \\
    const int BYTES = sizeof(type); \\
    if (HOST_BIG_ENDIAN && BYTES > 1) \\
        for (int i = BYTES-1; i >= 0; i--) \\
            *buf++ = ((uint8_t *)&obj)[i]; \\
    else *(type *)buf = obj; \\
    return BYTES; \\
} \\
static int unmarshal_##type (const uint8_t *buf, type *obj) \\
{ \\
    const int BYTES = sizeof(type); \\
    if (HOST_BIG_ENDIAN && BYTES > 1) \\
        for (int i = 0; i < BYTES; i++) \\
            ((uint8_t *)obj)[i] = buf[BYTES-i-1]; \\
    else *obj = *(const type *)buf; \\
    return BYTES; \\
}
""")

    for struct in structs:
        print(f"DEFINE_PRIMITIVE({struct.__name__})")

    print()

def marsh_define(struct):
    indent = 0
    fields = [f for f in dataclasses.fields(struct)]
    indent = 0

    print_indent(indent, MARSH_DECLARE.format(struct.__name__))
    print_indent(indent, '{')
    indent += 4
    print_indent(indent, "uint8_t *tail = buf;")
    print()

    for f in fields:
        mod = f.default
        name = mod.rename if mod.rename else f.name
        type = f.type.__name__
        isprimitive = "primitive" in f.type.__module__

        if mod.optional:
            print_indent(indent, MARSHAL_CALL.format("uint8_t", mod.optional))
            print_indent(indent, f"if ({mod.optional}) {{")
            indent += 4

        if mod.fixedLen:
            print_indent(indent, MARSHAL_CALL.format("uint32_t", f"(uint32_t) {mod.fixedLen}"))
            print_indent(indent, f"for (uint32_t i = 0; i < (uint32_t) {mod.fixedLen}; i++) {{")
            indent += 4
        elif mod.varLen:
            print_indent(indent, MARSHAL_CALL.format("uint32_t", f"(uint32_t) {mod.varLen}"))
            print_indent(indent, f"for (uint32_t i = 0; i < (uint32_t) {mod.varLen}; i++) {{")
            indent += 4

        if mod.fixedLen or mod.varLen:
            print_indent(indent, MARSHAL_CALL.format(type, f"obj->{name}{'[i]' if isprimitive else ' + i'}"))
        else:
            print_indent(indent, MARSHAL_CALL.format(type, f"{'' if isprimitive else '&'}obj->{name}"))

        if mod.fixedLen or mod.varLen:
            indent -= 4
            print_indent(indent, "}")

        if mod.optional:
            indent -= 4
            print_indent(indent, "}")

        print()
    print_indent(indent, "return (uint32_t) (tail - buf);")
    indent -= 4
    print_indent(indent, '}')
    print()

def unmarsh_define(struct):
    indent = 0
    fields = [f for f in dataclasses.fields(struct)]
    indent = 0
    print_indent(indent, UNMARSH_DECLARE.format(struct.__name__))
    print_indent(indent, '{')
    indent += 4
    print_indent(indent, "const uint8_t *head = buf;")
    print()

    for f in fields:
        mod = f.default
        name = mod.rename if mod.rename else f.name
        type = f.type.__name__

        if mod.optional:
            print_indent(indent, "{")
            indent += 4
            print_indent(indent, "uint8_t opt;")
            print_indent(indent, UNMARSHAL_CALL.format("uint8_t", "&opt"))
            print_indent(indent, "if (opt) {")
            indent += 4

        if mod.fixedLen or mod.varLen:
            print_indent(indent, '{')
            indent += 4
            print_indent(indent, "uint32_t len;")
            print_indent(indent, UNMARSHAL_CALL.format("uint32_t", "&len"))
            print()
        else:
            print_indent(indent, UNMARSHAL_CALL.format(type, f"&obj->{name}"))

        if mod.varLen:
            print_indent(indent, "if (len) {")
            print_indent(indent + 4, f"obj->{name} = ({type}*) malloc(sizeof(*obj->{name}) * len);")
            print_indent(indent, "} else {")
            print_indent(indent + 4, f"obj->{name} = 0;")
            print_indent(indent, "}")
            print()


        if mod.fixedLen or mod.varLen:
            print_indent(indent, f"for (uint32_t i = 0; i < len; i++) {{")
            print_indent(indent+4, UNMARSHAL_CALL.format(type, f"obj->{name} + i"))
            print_indent(indent, "}")
            indent -= 4
            print_indent(indent, "}")

        if mod.optional:
            indent -= 4
            print_indent(indent, "}")
            if mod.varLen:
                print_indent(indent, "else {")
                print_indent(indent + 4, f"obj->{name} = 0;")
                print_indent(indent, "}")
            indent -= 4
            print_indent(indent, "}")

        print()

    print_indent(indent, "return (uint32_t) (head - buf);")
    indent -= 4
    print_indent(indent, '}')
    print()

def free_define(struct):
    indent = 0
    fields = [f for f in dataclasses.fields(struct)]
    indent = 0
    print_indent(indent, FREE_DECLARE.format(struct.__name__))
    print_indent(indent, '{')
    indent += 4
    for f in fields:
        mod = f.default
        name = mod.rename if mod.rename else f.name
        type = f.type.__name__
        freecontent = not f.type.__module__.startswith("primitives") and freeable(f.type)

        if mod.varLen or mod.fixedLen:
            print_indent(indent, f"if (obj->{name}) {{")
            indent += 4

        if freecontent:
            if mod.optional:
                print_indent(indent, f"if ({mod.optional}) {{")
                indent += 4

            if mod.fixedLen:
                print_indent(indent, f"for (uint32_t i = 0; i < (uint32_t) {mod.fixedLen}; i++) {{")
                print_indent(indent + 4, f"free_{type}(obj->{name} + i);")
                print_indent(indent, '}')
            if mod.varLen:
                print_indent(indent, f"for (uint32_t i = 0; i < (uint32_t) {mod.varLen}; i++) {{")
                print_indent(indent + 4, f"free_{type}(obj->{name} + i);")
                print_indent(indent, '}')
            else:
                print_indent(indent, f"free_{type}(&obj->{name});")

            if mod.optional:
                indent -= 4
                print_indent(indent, '}')

        if mod.varLen:
            print_indent(indent, f"free(obj->{name});")
            indent -= 4
            print_indent(indent, '}')

    indent -= 4
    print_indent(indent, '}')
    print()

def generate(structs):
    for struct in structs:
        print(MARSH_DECLARE.format(struct.__name__) + ";")
        print(UNMARSH_DECLARE.format(struct.__name__) + ";")
        if (freeable(struct)):
            print(FREE_DECLARE.format(struct.__name__) + ";")
        print()


    for struct in structs:
        marsh_define(struct)
        unmarsh_define(struct)
        if freeable(struct):
            free_define(struct)

def import_dataclasses(module_name):
    module = importlib.import_module(module_name)
    members = inspect.getmembers(module)
    dcs = [obj for _, obj in members if inspect.isclass(obj)
                                     and obj.__module__.startswith(module_name)]
    return dcs

if __name__ == "__main__":
    prolog(import_dataclasses("primitives"))
    for dc in import_dataclasses("manualprimitives"):
        print(dc.MARSH_DEFINE)
        print()
        print(dc.UNMARSH_DEFINE)
        print()
    generate(import_dataclasses("structs"))