#ifndef JSON_H
#define JSON_H

#include <marrow/marrow.h>

#define _JSON_DELIM    (*(u8*)&(_Json){.always_negtwo = -2})
#define _JSON_IS_DELIM(p) (((_Json*)(p))->always_negtwo == -2)

typedef enum JsonType {
    JSON_INVALID,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_INT,
    JSON_FLOAT
} JsonType;

typedef enum _JsonType {
    _JSON_OBJECT = 1,
    _JSON_ARRAY = 2,
    _JSON_STRING = 3,
    _JSON_INVALID = 0,
    _JSON_INT = 1,
    _JSON_SMALLINT = 2,
    _JSON_FLOAT = 3
} _JsonType;

// header inserted before every element
union(_Json)
{
    struct {
        u8 _1 : 2;
        u8 type : 4;
        u8 _2 : 2;
    };
    struct {
        u8 dynamic_size : 2;
        u8 size_exponent : 4;
        u8 _3 : 2;
    };
    struct {
        u8 _4 : 2;
        u8 _5 : 4;
        i8 always_negtwo : 2;
    };
};

struct(JsonValue)
{
    u64 _size; // cached size to know where next element starts in buffer
    JsonType type;
    union {
        i32 integer;
        f32 decimal;
        s8 string;
    };
};

// this sorta mimics the actual in memory layout
// [label]<type>[data], and label.end points to type
struct(JsonObject)
{
    s8 label;
    JsonValue val;
};

static inline s8 _json_parse_object(s8 s, u8** buf);

static inline s8 _json_parse_value(s8 s, u8** buf)
{
    // reserve header
    _Json* header_ptr = (*(_Json**)buf)++;
    _Json header = { .always_negtwo = -2 };
    s8 value = s;
    if (*s.start == '"') { // string
        value.start++;
        value = (s8)slice(value.start, s8_skip_until(value, '"'));

        header.dynamic_size = _JSON_STRING;
        for (i32 i = 0; i < (i32)slice_count(value) - 1; i++)
            *((*buf)++) = value.start[i];
        *((*buf)++) = _JSON_DELIM;
    }
    else if (*s.start == '{') { // object
        value.end = value.start;
        i32 i = 0;
        while(value.end != s.end) // find enclosing bracket
        {
            i += (*value.end == '{') ? 1 : (*value.end == '}' ? -1 : 0);
            value.end++;
            if (i <= 0) break;
        }

        header.dynamic_size = _JSON_OBJECT;
        do {
            value = _json_parse_object(value, buf);
        } while(*value.start == ',');
        *((*buf)++) = _JSON_DELIM;
    }
    else if (*s.start == '[') { // array
        value.end = value.start;
        i32 i = 0;
        while(value.end != s.end) // find enclosing bracket
        {
            i += (*value.end == '[') ? 1 : (*value.end == ']' ? -1 : 0);
            value.end++;
            if (i <= 0) break;
        }
        header.dynamic_size = _JSON_ARRAY;
        value.start++;
        loop {
            value.start = s8_skip_while(value, ' ');
            value = _json_parse_value(value, buf);
            value.start = s8_skip_while(value, ' ');
            if (*value.start != ',') break;
            value.start++;
        }
        *((*buf)++) = _JSON_DELIM;
    }
    else { // int or float
        value.start = s8_skip_while(s, '-');
        value.end = s8_skip_filter(value, FILTER_DIGIT);
        if (*value.end == '.')
        {
            value.end = s8_skip_filter((s8)slice(value.end + 1, s.end), FILTER_DIGIT);
            header.type = _JSON_FLOAT;
            *((*(f32**)buf)++) = s8_parse_float(value);
        }
        else
        {
            i32 val = s8_parse_i32(value);
            if (val >= -128 && val <= 127){
                header.type = _JSON_SMALLINT;
                *((*(i8**)buf)++) = (i8)val;
            }
            else
            {
                header.type = _JSON_INT;
                *((*(i32**)buf)++) = val;
            }
        }
    }
    *header_ptr = header;
    return (s8)slice(value.end, s.end);
}

static inline s8 _json_parse_object(s8 s, u8** buf)
{
    s8 label = s;
    label.start = s8_skip_until(s, '"');
    s.start = s8_skip_until(label, '"');
    label.end = s.start - 1;
    slice_for_each(label, c)
        *((*buf)++) = *c;

    s.start = s8_skip_until(s, ':');
    s.start = s8_skip_while(s, ' ');

    return _json_parse_value(s, buf);
}

static inline JsonObject json_parse(s8 s)
{
    u8Slice original_buffer = slice_t(s, u8);
    u8* buf = original_buffer.start;

    s.start = s8_skip_while(s, ' ');
    _json_parse_value(s, &buf);
    *(buf++) = _JSON_DELIM;

    s8 l = (s8)slice((char*)original_buffer.start, (char*)original_buffer.start);
    return (JsonObject){ .label = l, .val.type = JSON_OBJECT, };
}

static inline JsonObject _json_object_from_bytes(char* p);
static inline JsonValue _json_value_from_bytes(char* p)
{
    JsonValue val = { 0 };
    _Json j = *(_Json*)p;
    p += sizeof(_Json);
    if (j.dynamic_size == _JSON_OBJECT)
    {
        val.type = JSON_OBJECT;
        loop {
            JsonObject child = _json_object_from_bytes(p + val._size);
            val._size += slice_size(child.label) + child.val._size + sizeof(_Json);
            if (child.val.type == JSON_INVALID) break;
        }
    }
    else if (j.dynamic_size == _JSON_ARRAY)
    {
        val.type = JSON_ARRAY;
        loop {
            JsonValue v = _json_value_from_bytes(p + val._size);
            val._size += v._size + sizeof(_Json);
            if (v.type == JSON_INVALID) break;
        }
    }
    else if (j.dynamic_size == _JSON_STRING)
    {
        val.type = JSON_STRING;
        val.string = (s8)slice(p, p);
        while(!_JSON_IS_DELIM(val.string.end)) val.string.end++;
        val._size = slice_size(val.string) + 1;
    }
    else if (j.type == _JSON_INT){
        val.type = JSON_INT;
        val._size = sizeof(i32);
        val.integer = *(i32*)p;
    }
    else if (j.type == _JSON_SMALLINT){
        val.type = JSON_INT;
        val._size = sizeof(i8);
        val.integer = *(i8*)p;
    }
    else if (j.type == _JSON_FLOAT)
    {
        val.type = JSON_FLOAT;
        val._size = sizeof(f32);
        val.decimal = *(f32*)p;
    }
    return val;
}

static inline JsonObject _json_object_from_bytes(char* p)
{
    JsonObject obj = { .label = { .start = p, .end = p } };
    while(!_JSON_IS_DELIM(obj.label.end)) obj.label.end++;
    obj.val = _json_value_from_bytes((char*)obj.label.end);
    return obj;
}

static inline JsonObject json_next(JsonObject json)
{
    if (json.val.type == JSON_INVALID) return (JsonObject) { 0 };
    return _json_object_from_bytes(json.label.end + sizeof(_Json) + json.val._size);
}

static inline JsonObject json_first(JsonObject json)
{
    if (json.val.type != JSON_OBJECT && json.val.type != JSON_ARRAY) return (JsonObject) { 0 };
    return _json_object_from_bytes(json.label.end + sizeof(_Json));
}

static inline JsonObject json_find(JsonObject json, s8 label)
{
    for (json = json_first(json); json.val.type; json = json_next(json))
        if (s8_cmp(json.label, label) == 0) break;
    return json;
}

struct(JsonStringifyConfig) {
    s8 indent;
    s8 newline;
    u32 _i;
};

static inline usize json_stringify(JsonObject json, s8 s, JsonStringifyConfig* config)
{
    static JsonStringifyConfig default_stringify = { .indent = sstr("\t"), .newline = sstr("\n") };
    if (config == nullptr) config = &default_stringify;
    s8 original_s = s;
    char indent_buf[slice_size(config->indent) * config->_i];
    s8 indent = (s8)array_slice(indent_buf);
    slice_fill(indent, config->indent);
    if (slice_size(indent) > 0) s.start += print(s.start, slice_size(s), "{}{}", config->newline, indent);
    if (slice_size(json.label) > 0)
        s.start += print(s.start, slice_size(s), "\"{}\": ", json.label);
    if (json.val.type == JSON_INT)
        s.start += print(s.start, slice_size(s), "{}", json.val.integer);
    else if (json.val.type == JSON_STRING)
        s.start += print(s.start, slice_size(s), "\"{}\"", json.val.string);
    else if (json.val.type == JSON_FLOAT)
        s.start += print(s.start, slice_size(s), "{}", json.val.decimal);
    else {
        *(s.start++) = json.val.type == JSON_OBJECT ? '{' : '[';
        config->_i++;
        for (JsonObject child = json_first(json); child.val.type; child = json_next(child))
        {
            s.start += json_stringify(child, s, config);
            *(s.start++) = ',';
        }
        s.start -= 1;
        s.start += print(s.start, slice_size(s), "{}{}", config->newline, indent);
        *(s.start++) = json.val.type == JSON_OBJECT ? '}' : ']';
        config->_i--;
    }
    return s.start - original_s.start;
}

#endif // JSON_H
