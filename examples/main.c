#include <marrow/marrow.h>
#include <marrow/json.h>

#include <stdio.h>

void print_json(JsonObject obj, bool print_label)
{
    if (print_label)
        mrw_debug("{}", obj.label);
    if (obj.val.type == JSON_INT)    mrw_debug("int: {}", obj.val.integer);
    if (obj.val.type == JSON_OBJECT) {
        mrw_debug("object");
        for (obj = json_first(obj); obj.val.type; obj = json_next(obj))
            print_json(obj, true);
    }
    if (obj.val.type == JSON_ARRAY)
    {
        mrw_debug("[");
        for (obj = json_first(obj); obj.val.type; obj = json_next(obj))
        {
            print_json(obj, false);
            mrw_debug(",");
        }
        mrw_debug("]");
    }
    if (obj.val.type == JSON_STRING) mrw_debug("string: {}", obj.val.string);
    if (obj.val.type == JSON_FLOAT)  mrw_debug("float: {}", obj.val.decimal);
}

int main()
{
    FILE* fp = fopen("stvari.json", "rb");
    fseek(fp, 0, SEEK_END);
    usize size = ftell(fp);
    rewind(fp);
    char buf[size];
    fread(buf, 1, size, fp);
    fclose(fp);

    s8 file_slice = array_slice(buf);
    mrw_debug("{}", file_slice);
    JsonObject json = json_parse(file_slice);
    mrw_debug("{}", file_slice);
    print_json(json, true);

    JsonObject alo = json_find(json, str("alo"));
    mrw_debug("finding val of alo\n {}", alo.val.string);
}

thread_local u32 thread_local_val = 0;

int main2(void)
{
    u8   v_u8   = 1;
    u16  v_u16  = 2;
    u32  v_u32  = 3;
    u64  v_u64  = 4;

    i8   v_i8   = -1;
    i16  v_i16  = -2;
    i32  v_i32  = -3;
    i64  v_i64  = -4;

    usize v_usize = 123;

    f32   v_f32  = 0.5f;
    f64   v_f64  = 3.14159;

    bool  v_bool_true  = true;
    bool  v_bool_false = false;

    u32 bit_val = 0;
    BIT_SET(bit_val, 1);
    BIT_SET(bit_val, 3);
    bool bit1_set = BIT_IS_SET(bit_val, 1);
    BIT_CLEAR(bit_val, 1);
    BIT_TOGGLE(bit_val, 3);

    usize align_hsv = alignof(HSV);
    f32 clamped     = clamp(1.5f, 0.0f, 1.0f);
    f32 min_val     = min(3.0f, 4.0f);
    f32 max_val     = max(3.0f, 4.0f);
    f32 wrapped     = wrap_float(-10.0f, 360.0f);

    bool between      = is_between(5, 1, 10);
    bool between_inc1 = is_between_inclusive(10, 1, 10);
    bool between_inc2 = is_between_inclusive(0, 1, 10);

    char src_buf[8] = "abc";
    char dst_buf[8] = {0};

    buf_copy(dst_buf, src_buf, sizeof(src_buf));
    i32 r = buf_cmp(dst_buf, src_buf, sizeof(src_buf));
    buf_set(dst_buf, 0x7f, sizeof(dst_buf));

    i32 int_array[4] = { 10, 20, 30, 40 };
    SLICE(i32) i32_slice = array_slice(int_array);

    slice_for_each(i32_slice, it) {
        *it += 1;
    }

    s8 s1 = str("hello world");
    s8 s2 = str("hello world!");

    char *last_o = s8_find_r(s1, 'o');
    u32  s_cmp   = s8_cmp(s1, s2);

    u8Slice s1_bytes = slice_bytes(s1);

    u64 hash1 = hash_bytes(s1_bytes);

    u64 hash2 = hash_slice((s8)slice_range(s1.start, 1, 5));
    u64 hash3 = hash_u64(0x12345678u);
    u64 hash4 = hash_combine(hash1, hash2);

    u32 gray_rgb   = value_to_rgb(0.5f);
    u32 byte_fromf = to_byte(0.75f);

    HSV hsv_color = {
        .hue        = 200.0f,
        .saturation = 0.6f,
        .value      = 0.9f
    };

    u32 rgb_color = hsv_to_rgb(hsv_color);
    HSV hsv_back  = rgb_to_hsv(rgb_color);

    struct(CoolStruct) { i32 a; f32 b; };
    CoolStruct cool_struct = { .a = 10, .b = 2.0f };
    CoolStructSlice cool_struct_slice = slice(&cool_struct, &cool_struct + 1);

    thread_local_val++;

    mrw_debug("pozdravljeni {}", (u32)v_u32);
    mrw_error("besedilo {}", (i32)v_i32);
    mrw_abort("lp {}", 0);

    return 0;
}
