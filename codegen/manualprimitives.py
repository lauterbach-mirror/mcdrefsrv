# Primitives which require manual marshal functions

from dataclasses import dataclass

@dataclass
class mcd_bool_t:
    MARSH_DEFINE = """static int marshal_mcd_bool_t (mcd_bool_t obj, uint8_t *buf)
{
    uint8_t b = !!obj;
    return marshal_uint8_t(b, buf);
}"""

    UNMARSH_DEFINE = """static int unmarshal_mcd_bool_t (const uint8_t *buf, mcd_bool_t *obj)
{
    uint8_t b;
    int l = unmarshal_uint8_t(buf, &b);
    *obj = b ? TRUE : FALSE;
    return l;
}"""
