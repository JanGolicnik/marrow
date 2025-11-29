#ifndef JSON_H
#define JSON_H

#include <marrow/marrow.h>

typedef enum JsonType {
    JSON_INVALID = 0,
    JSON_OBJECT = 1,
    JSON_ARRAY = 2,
    JSON_UHHH = 3,
    JSON_INT = 4,
    JSON_SMALLINT = 8, // never exposed to the user
    JSON_FLOAT = 12,
    JSON_STRING = 16
} JsonType;

// header inserted before every element
struct (_Json)
{
    union {
        u8 type : 8;
        struct {
            u8 has_children : 2;
            u8 n_children : 6;
        };
    };
};

struct(JsonValue)
{
    u32 _size; // cached size to know where next element starts in buffer
    JsonType type;
    union{
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
    u32 _i;
    JsonValue val;
};

static inline s8 _json_parse_object(s8 s, u8** buf);

static inline s8 _json_parse_value(s8 s, u8** buf)
{
    // reserve header
    _Json* j = (*(_Json**)buf)++;

    s8 value = s;
    if (*s.start == '"') { // string
        value.start++;
        value = (s8)slice(value.start, s8_skip_until(value, '"'));

        j->type = JSON_STRING;
        for (i32 i = 0; i < (i32)slice_count(value) - 1; i++)
            *((*buf)++) = value.start[i];
        *((*buf)++) = 0;
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

        j->has_children = JSON_OBJECT;
        j->n_children = 0;
        do {
            j->n_children++;
            value = _json_parse_object(value, buf);
        } while(*value.start == ',');
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

        j->has_children = JSON_ARRAY;
        j->n_children = 0;
        value.start++;

        loop {
            value.start = s8_skip_while(value, ' ');
            value = _json_parse_value(value, buf);
            j->n_children++;
            value.start = s8_skip_while(value, ' ');
            if (*value.start != ',') break;
            value.start++;
        }
    }
    else { // int or float
        value.start = s8_skip_while(s, '-');
        value.end = s8_skip(value, FILTER_DIGIT);
        if (*value.end == '.')
        {
            value.end = s8_skip((s8)slice(value.end + 1, s.end), FILTER_DIGIT);
            j->type = JSON_FLOAT;
            *((*(f32**)buf)++) = s8_parse_float(value);
        }
        else
        {
            i32 val = s8_parse_i32(value);
            if (val >= -128 && val <= 127){
                j->type = JSON_SMALLINT;
                *((*(i8**)buf)++) = (i8)val;
            }
            else
            {
                j->type = JSON_INT;
                *((*(i32**)buf)++) = val;
            }
        }
    }

    return (s8)slice(value.end, s.end);
}

static inline s8 _json_parse_object(s8 s, u8** buf)
{
    s8 label = s;
    label.start = s8_skip_until(s, '"');
    s.start = label.end = s8_skip_until(label, '"') - 1;

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
    *(buf++) = 0;

    s8 l = (s8)slice((char*)original_buffer.start, (char*)original_buffer.start);
    return (JsonObject){ .label = l, .val.type = JSON_OBJECT, };
}

static inline JsonObject _json_object_from_bytes(char* p);
static inline JsonValue _json_value_from_bytes(char* p)
{
    JsonValue val = { 0 };
    _Json j = *(_Json*)p;
    val.type = j.type;
    void* ptr = p + sizeof(_Json);
    if (j.has_children == JSON_OBJECT)
    {
        val.type = JSON_OBJECT;
        for (i32 i = 0; i < j.n_children; i++)
        {
            JsonObject child = _json_object_from_bytes((char*)ptr + val._size);
            val._size += slice_size(child.label) + child.val._size + sizeof(_Json);
        }
    }
    else if (j.has_children == JSON_ARRAY)
    {
        val.type = JSON_ARRAY;
        for (i32 i = 0; i < j.n_children; i++)
            val._size += _json_value_from_bytes((char*)ptr + val._size)._size + sizeof(_Json);
    }
    else if (val.type == JSON_INT){
        val._size = sizeof(i32);
        val.integer = *(i32*)ptr;
    }
    else if (val.type == JSON_SMALLINT){
        val.type = JSON_INT;
        val._size = sizeof(i8);
        val.integer = *(i8*)ptr;
    }
    else if (val.type == JSON_FLOAT)
    {
        val._size = sizeof(f32);
        val.decimal = *(f32*)ptr;
    }
    else if (val.type == JSON_STRING)
    {
        val.string = (s8)slice(ptr, ptr);
        do {val._size++;} while(*(val.string.end++));
    }
    return val;
}

static inline JsonObject _json_object_from_bytes(char* p)
{
    JsonObject obj = { 0 };
    obj.label = (s8)slice(p, s8_skip((s8)slice_to(p, 9999), FILTER_DIGIT | FILTER_CHAR));
    obj.val = _json_value_from_bytes((char*)obj.label.end);
    return obj;
}

static inline JsonObject json_next(JsonObject json)
{
    if (json.val.type == JSON_INVALID || json._i == 1) return (JsonObject) { 0 };
    JsonObject obj = _json_object_from_bytes(json.label.end + sizeof(_Json) + json.val._size);
    obj._i = json._i - 1;
    return obj;
}

static inline JsonObject json_first(JsonObject json)
{
    if (json.val.type != JSON_OBJECT && json.val.type != JSON_ARRAY) return (JsonObject) { 0 };
    JsonObject obj = _json_object_from_bytes(json.label.end + sizeof(_Json));
    obj._i = ((_Json*)json.label.end)->n_children;
    return obj;
}

static inline JsonObject json_find(JsonObject json, s8 label)
{
    for (json = json_first(json); json.val.type; json = json_next(json))
        if (s8_cmp(json.label, label) == 0) break;
    return json;
}

#endif // JSON_H
