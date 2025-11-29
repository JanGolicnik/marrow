#ifndef JSON_H
#define JSON_H

#include <marrow/marrow.h>

typedef enum JsonType {
    JSON_INVALID,
    JSON_OBJECT,
    JSON_INT,
    JSON_SMALLINT, // never exposed to the user
    JSON_FLOAT,
    JSON_STRING,
    JSON_ARRAY
} JsonType;

// header inserted before every element
struct (_Json)
{
    u8 type : 3;
    u8 n_children : 5;
};

struct(JsonObject)
{
    s8 label;
    u32 _size; // cached size to know where next element starts in buffer
    JsonType type;
    union{
        i32 integer;
        f32 decimal;
        s8 string;
    };
};

static inline s8 _json_parse_object(s8 s, u8** buf);

static inline s8 _json_parse_value(s8 s, u8** buf)
{
    // reserve header
    _Json* j = (*(_Json**)buf)++;
    u32 n_children = 0;

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

        j->type = JSON_OBJECT;
        do {
            n_children++;
            value = _json_parse_object(value, buf);
        } while(*value.start == ',');
    }
    else if (*s.start == '[') { // array

    }
    else { // int or float
        value.start = s8_skip_while(s, '-');
        value.end = s8_skip_digit(value);
        if (*value.end == '.')
        {
            value.end = s8_skip_digit((s8)slice(value.end + 1, s.end));
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

    j->n_children = n_children;
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
    return (JsonObject){ .label = l, .type = JSON_OBJECT, };
}

static inline JsonObject _json_object_from_bytes(char* p)
{
    JsonObject obj = { 0 };
    obj.label = (s8)slice(p, s8_skip_char((s8)slice_to(p, 9999)));
    _Json j = *(_Json*)obj.label.end;
    obj.type = j.type;
    void* val = obj.label.end + sizeof(_Json);
    if (obj.type == JSON_INT){
      obj._size = sizeof(i32);
      obj.integer = *(i32*)val;
    }
    else if (obj.type == JSON_SMALLINT){
      obj.type = JSON_INT;
      obj._size = sizeof(i8);
      obj.integer = *(i8*)val;
    }
    else if (obj.type == JSON_FLOAT)
    {
        obj._size = sizeof(f32);
        obj.decimal = *(f32*)val;
    }
    else if (obj.type == JSON_STRING)
    {
        obj.string.start = obj.string.end = val;
        do {obj._size++;} while(*(obj.string.end++));
    }
    return obj;
}

static inline JsonObject json_next(JsonObject json)
{
    if (json.type == JSON_INVALID) return (JsonObject) { 0 };
    return _json_object_from_bytes(json.label.end + sizeof(_Json) + json._size);
}

static inline JsonObject json_next_sibling(JsonObject json)
{
    if (json.type == JSON_INVALID) return (JsonObject) { 0 };
    i32 n_children = 0;
    do {
        n_children += -1 + ((_Json*)json.label.end)->n_children;
        json = json_next(json);
    } while(n_children >= 0);
    return json;
}

static inline JsonObject json_find(JsonObject json, s8 label)
{
    if (json.type == JSON_INVALID) return (JsonObject) { 0 };
    if (((_Json*)json.label.end)->n_children == 0) return (JsonObject) { 0 };
    JsonObject child = json_next(json);
    do {
        if (s8_cmp(child.label, label) == 0) return child;
        child = json_next_sibling(child);
    }
    while(child.type != JSON_INVALID);
    return child;
}

#endif // JSON_H
