internal int
clamp(int min, int val, int max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

internal f32
clampf(f32 min, f32 val, f32 max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

// Color
inline u32
make_color_from_grey(u8 grey) {
	return (grey << 0) | (grey << 8) | (grey << 16);
}

inline u32
make_color_from_rgb(u8 r, u8 g, u8 b) {
	return (b << 0) | (g << 8) | (r << 16);
}

// Lerp
inline f32
lerp (f32 a, f32 t, f32 b) {
	return (1-t)*a + t*b;
}

inline u32
lerp_color (u32 a, f32 t, u32 b) {
	f32 a_r = (f32)((a & 0xff0000) >> 16);
	f32 a_g = (f32)((a & 0xff00) >> 8);
	f32 a_b = (f32)(a & 0xff);

	f32 b_r = (f32)((b & 0xff0000) >> 16);
	f32 b_g = (f32)((b & 0xff00) >> 8);
	f32 b_b = (f32)(b & 0xff);

	return make_color_from_rgb((u8)lerp(a_r, t, b_r), (u8)lerp(a_g, t, b_g), (u8)lerp(a_b, t, b_b));
}

// Absolute of f32
inline f32
absf(f32 a) {
	if (a < 0) return -a;
	return a;
}

inline f32
square(f32 a) {
	return a*a;
}

inline f32
map_into_range_normalized(f32 min, f32 val, f32 max) {
	f32 range = max - min;
	return clampf(0, (val-min)/range, 1);
}

// Vector 2

struct {
	union {
		struct {
			f32 x;
			f32 y;
		};
		
		f32 e[2];
	};
} typedef v2;

inline v2
add_v2(v2 a, v2 b) {
	return (v2){a.x + b.x, a.y + b.y};
}

inline v2
sub_v2(v2 a, v2 b) {
	return (v2){a.x - b.x, a.y - b.y};
}

inline v2
mul_v2(v2 a, f32 s) {
	return (v2){s * a.x, s * a.y};
}

inline f32
len_sq(v2 v) {
    return square(v.x) + square(v.y);
}

struct {
	union {
		struct {
			int x;
			int y;
		};
		
		int e[2];
	};
} typedef v2i;

inline v2i
add_v2i(v2i a, v2i b) {
	return (v2i){a.x + b.x, a.y + b.y};
}

inline v2i
sub_v2i(v2i a, v2i b) {
	return (v2i){a.x - b.x, a.y - b.y};
}

inline v2i
mul_v2i(v2i a, f32 s) {
	return (v2i){(int)s * a.x, (int)s * a.y};
}

// Random

u32 random_state = 0; // Change the seed to be unique per game-run

inline void
set_random_seed(u32 random_seed) {
	random_state = random_seed;
}

inline u32
random_u32() {
	u32 result = random_state;
	result ^= result << 13;
	result ^= result >> 17;
	result ^= result << 5;
	random_state = result;
	return result;
}

inline u32
random_int_in_range(int min, int max) {
	int range = max-min+1;
	int result = random_u32() % range;
	result += min;
	return result;
}

inline b32
random_b32() {
	return random_u32()%2;
}

inline b32
random_choice(int chance) {
	return random_u32()%chance == 0;
}

inline f32
random_unilateral() {
	return (f32)random_u32() / (f32)MAX_U32;
}

inline f32
random_bilateral() {
	return random_unilateral() * 2.f - 1.f;
}